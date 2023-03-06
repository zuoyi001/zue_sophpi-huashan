#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "app_ipcam_record.h"

#define MMC_DEV                     "/dev/mmcblk0p1"
#define RECORD_PATH                 "/mnt/sd"
#define RECORD_STORAGE_CAPACITY     (16 * 1024)
#define RECORD_ONE_DAY              (24 * 60 * 60)
#define REPLAY_ONLY_I_FRAME         (0)

#define REPLAY_VIDEO_PATH           "/mnt/sd/video_%s.%s"
#define REPLAY_AUDIO_PATH           "/mnt/sd/audio_%s.g711u"
#define REPLAY_MAX_SEG              80

static HANDLE iPkgHand = NULL;
static CVI_S32 gst_ReplayVideoFd = -1;
static CVI_S32 gst_ReplayAudioFd = -1;
static RUN_THREAD_PARAM mRecordThread;
pthread_mutex_t s_iRecordMutex = PTHREAD_MUTEX_INITIALIZER;

typedef enum
{
    SDCARD_STATUS_INSERT = 0,
    SDCARD_STATUS_OUT,
}SDCARD_STATUS;

static bool gst_bRecordKeyFrame = 0;
static char *pstCheckSeg;
static int gst_bStart = 0;

static char *app_ipcam_Record_GetFileExtensions(CVI_CHAR iFrameType)
{
    switch (iFrameType)
    {
        case CVI_RECORD_FRAME_TYPE_H265:
            return "265";
        case CVI_RECORD_FRAME_TYPE_H264:
            return "264";
        case CVI_RECORD_FRAME_TYPE_G711U:
            return "g711u";
        default:
            return "data";
    }
}

static int app_ipcam_Record_Create()
{
    char cmd[64];
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "fsck.fat -a -V -v %s", (char *)RECORD_PATH);
    if (cvi_system(cmd))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "fsck.fat failed!\n");
    }

    CVI_RECORD_Init((char *)RECORD_PATH);

    CVI_STORAGE_STATUS_E iPKGStatus = CVI_STORAGE_STATUS_ERROR;
    CVI_RECORD_GetStatus(&iPKGStatus, NULL, NULL, NULL);

    if (0 != CVI_RECORD_Pause())
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RECORD_Pause failed!\n");
        return -1;
    }

    if (iPKGStatus == CVI_STORAGE_STATUS_UNINIT)
    {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "SD Format Start!!!\n");
        if (0 != CVI_RECORD_Format(RECORD_STORAGE_CAPACITY))
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RECORD_Format failed!\n");
            return -1;
        }
        APP_PROF_LOG_PRINT(LEVEL_INFO, "SD Format End!!!\n");
    }
    else if (iPKGStatus == CVI_STORAGE_STATUS_ERROR)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "SD Storage Status Error!\n");
        return -1;
    }

    if (0 != CVI_RECORD_Resume())
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RECORD_Resume failed!\n");
        return -1;
    }
    gst_bStart = 1;
    return 0;
}

static int app_ipcam_Record_Destory()
{
    if (iPkgHand)
    {
        CVI_REPLAY_Release(iPkgHand);
    }
    if (gst_ReplayVideoFd)
    {
        close(gst_ReplayVideoFd);
        gst_ReplayVideoFd = -1;
    }
    if (gst_ReplayAudioFd)
    {
        close(gst_ReplayAudioFd);
        gst_ReplayAudioFd = -1;
    }
    CVI_RECORD_Uninit();
    return 0;
}

static CVI_S32 app_ipcam_Record_GetStream(HANDLE hDataPopper, PT_FRAME_INFO pFrameInfo, CVI_REPLAY_CB_EVENT_E EventID)
{
    if (NULL == pFrameInfo || CVI_REPLAY_CB_NONE != EventID)
    {
        app_ipcam_Record_StopReplay();
        return 0;
    }

    char cFileName[128];
    char cReplayTime[32];

    struct tm Begin;
    time_t tBeginTime = pFrameInfo->iShowTime;

    if (pFrameInfo->iFrameType != CVI_RECORD_FRAME_TYPE_G711U)
    {
        if (gst_ReplayVideoFd <= 0)
        {
            localtime_r(&tBeginTime, &Begin);
            memset(cReplayTime, 0, sizeof(cReplayTime));
            if (snprintf(cReplayTime, sizeof(cReplayTime), "%04d%02d%02d%02d%02d%02d",
                (Begin.tm_year + 1900), (Begin.tm_mon + 1), Begin.tm_mday, Begin.tm_hour, Begin.tm_min, Begin.tm_sec) < 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "snprintf err\n");
            }
            memset(cFileName, 0, sizeof(cFileName));
            if (snprintf(cFileName, sizeof(cFileName), REPLAY_VIDEO_PATH, cReplayTime, app_ipcam_Record_GetFileExtensions(pFrameInfo->iFrameType)) < 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "snprintf err\n");
            }
            gst_ReplayVideoFd = open(cFileName, O_WRONLY | O_CREAT | O_TRUNC);
            if (gst_ReplayVideoFd <= 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "open %s failed!\n", cFileName);
                app_ipcam_Record_StopReplay();
                return 0;
            }
        }

        if (gst_ReplayVideoFd)
        {
            if (pFrameInfo->iFrameLen != (CVI_U32)write(gst_ReplayVideoFd, pFrameInfo->pBuf, pFrameInfo->iFrameLen))
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "video write err\n");
            }
        }
    }
    else
    {
        if (gst_ReplayAudioFd <= 0)
        {
            localtime_r(&tBeginTime, &Begin);
            memset(cReplayTime, 0, sizeof(cReplayTime));
            if (snprintf(cReplayTime, sizeof(cReplayTime), "%04d%02d%02d%02d%02d%02d",
                (Begin.tm_year + 1900), (Begin.tm_mon + 1), Begin.tm_mday, Begin.tm_hour, Begin.tm_min, Begin.tm_sec) < 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "snprintf err\n");
            }
            memset(cFileName, 0, sizeof(cFileName));
            if (snprintf(cFileName, sizeof(cFileName), REPLAY_AUDIO_PATH, cReplayTime) < 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "snprintf err\n");
            }
            gst_ReplayAudioFd = open(cFileName, O_WRONLY | O_CREAT | O_TRUNC);
            if (gst_ReplayAudioFd <= 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "open %s failed!\n", cFileName);
                app_ipcam_Record_StopReplay();
                return 0;
            }
        }

        if (gst_ReplayAudioFd)
        {
            if (pFrameInfo->iFrameLen != (CVI_U32)write(gst_ReplayAudioFd, pFrameInfo->pBuf, pFrameInfo->iFrameLen))
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "audio write err\n");
            }
        }
    }
    
    return 0;
}

static CVI_VOID *Thread_Record_Proc(CVI_VOID *pArgs)
{
    FILE *fd = NULL;
    char buf[1024] = {0};
    char *seek = NULL;
    SDCARD_STATUS status = SDCARD_STATUS_OUT;
    while (mRecordThread.bRun_flag)
    {
        if ((status == SDCARD_STATUS_OUT) && (access(MMC_DEV, F_OK) == 0))
        {
            fd = fopen("/proc/mounts", "r");
            if (fd)
            {
                memset(buf, 0, sizeof(buf));
                int len = fread(buf, 1, sizeof(buf), fd);
                fclose(fd);
                if (len > 0) {
                    seek = strstr(buf, MMC_DEV);
                    if (seek != NULL)
                    {
                        if (0 == app_ipcam_Record_Create())
                        {
                            status = SDCARD_STATUS_INSERT;
                        }
                    }
                }
            }
        }
        else if ((status == SDCARD_STATUS_INSERT) && (access(MMC_DEV, F_OK) != 0))
        {
            status = SDCARD_STATUS_OUT;
            app_ipcam_Record_Destory();
        }
        sleep(1);
    }

    if (status == SDCARD_STATUS_INSERT)
    {
        app_ipcam_Record_Destory();
    }
    return NULL;
}

int app_ipcam_Record_Init()
{
    memset(&mRecordThread, 0, sizeof(mRecordThread));
    mRecordThread.bRun_flag = 1;
    if (pthread_create(&mRecordThread.mRun_PID, NULL, Thread_Record_Proc, NULL) != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Thread_Record_Proc create failed:%s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int app_ipcam_Record_UnInit()
{
    //avoid lost video
    CVI_RECORD_Pause();
    mRecordThread.bRun_flag = 0;
    if (mRecordThread.mRun_PID != 0)
    {
        pthread_join(mRecordThread.mRun_PID, NULL);
        mRecordThread.mRun_PID = 0;
    }
    return 0;
}

CVI_S32 app_ipcam_Record_AudioInput(PAYLOAD_TYPE_E enType, AUDIO_STREAM_S *pstStream, CVI_S32 stFrameNum)
{
    CVI_S32 iRet = -1;
    CVI_S32 iDataType = 0;
    CVI_S32 iFrameType = 0;
    static CVI_S32 stLastAudioNum = 0;
    T_FRAME_INFO pstFrameInfo;
    memset(&pstFrameInfo, 0x0, sizeof(pstFrameInfo));

    if ((enType == PT_G711U) && (stLastAudioNum != 0) && ((stLastAudioNum + 1) != stFrameNum) && gst_bRecordKeyFrame)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "====audio miss frame (%d %d)=====\n", stLastAudioNum, stFrameNum);
    }

    pthread_mutex_lock(&s_iRecordMutex);
    if (enType == PT_G711U)
    {
        stLastAudioNum = stFrameNum;
        if (gst_bRecordKeyFrame)
        {
            iFrameType = CVI_RECORD_FRAME_TYPE_G711U;
        }
        else
        {
            goto AUDIO_RECORD_EXIT;
        }
    }
    else
    {
        goto AUDIO_RECORD_EXIT;
    }

    pstFrameInfo.iFrameType     = iFrameType;
    pstFrameInfo.iFrameNo       = stFrameNum;
    pstFrameInfo.iFrameLen      = pstStream->u32Len;
    pstFrameInfo.iFrame_absTime = (90 * pstStream->u64TimeStamp);
    pstFrameInfo.iShowTime      = time(NULL);
    pstFrameInfo.pBuf           = pstStream->pStream;
    pstFrameInfo.iSubType = iDataType;

    iRet = CVI_RECORD_DataInput(&pstFrameInfo);

AUDIO_RECORD_EXIT:
    pthread_mutex_unlock(&s_iRecordMutex);
    return iRet;
}

CVI_S32 app_ipcam_Record_VideoInput(PAYLOAD_TYPE_E enType, VENC_STREAM_S *pstStream, CVI_S32 stFrameNum)
{
    CVI_U32 i = 0;
    CVI_S32 iRet = -1;
    CVI_S32 iDataType = 0;
    CVI_S32 iFrameType = 0;
    CVI_U32 iCurPacks = 0;
    CVI_S32 iFrameLen = 0;
    CVI_S32 iBufOffset = 0;
    static CVI_S32 stLastVideoNum = 0;
    T_FRAME_INFO pstFrameInfo;
    memset(&pstFrameInfo, 0x0, sizeof(pstFrameInfo));

    pthread_mutex_lock(&s_iRecordMutex);
    CVI_STORAGE_STATUS_E iPKGStatus = CVI_STORAGE_STATUS_ERROR;
    CVI_RECORD_GetStatus(&iPKGStatus, NULL, NULL, NULL);
    if (iPKGStatus != CVI_STORAGE_STATUS_NORMAL)
    {
        gst_bRecordKeyFrame = 0;
        goto VIDEO_RECORD_EXIT;
    }

    if ((stLastVideoNum != 0) && (stLastVideoNum + 1 != stFrameNum))
    {
        gst_bRecordKeyFrame = 0;
    }

    iCurPacks = pstStream->u32PackCount;
    for (i = 0; i < iCurPacks; i++)
    {
        iFrameLen += (pstStream->pstPack[i].u32Len - pstStream->pstPack[i].u32Offset);
    }

    if (enType == PT_H264)
    {
        iFrameType = CVI_RECORD_FRAME_TYPE_H264;
        if (H264E_NALU_PSLICE != pstStream->pstPack[0].DataType.enH264EType)
        {
            iDataType = CVI_RECORD_H264_TYPE_I_FRAME;
            gst_bRecordKeyFrame = 1;
        }
        else
        {
            iDataType = CVI_RECORD_H264_TYPE_P_FRAME;
        }
        stLastVideoNum = stFrameNum;
    }
    else if (enType == PT_H265)
    {
        iFrameType = CVI_RECORD_FRAME_TYPE_H265;
        if (H265E_NALU_PSLICE != pstStream->pstPack[0].DataType.enH265EType)
        {
            iDataType = CVI_RECORD_H265_TYPE_I_FRAME;
            gst_bRecordKeyFrame = 1;
        }
        else
        {
            iDataType = CVI_RECORD_H265_TYPE_P_FRAME;
        }
        stLastVideoNum = stFrameNum;
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "====enType :%d unsupport=====\n", enType);
        goto VIDEO_RECORD_EXIT;
    }

    pstFrameInfo.iFrameType     = iFrameType;
    pstFrameInfo.iFrameNo       = stFrameNum;
    pstFrameInfo.iFrameLen      = iFrameLen;
    pstFrameInfo.iFrame_absTime = (90 * pstStream->pstPack[0].u64PTS);
    pstFrameInfo.iShowTime      = time(NULL);

    if (iCurPacks == 1)
    {
        pstFrameInfo.pBuf = (pstStream->pstPack[0].pu8Addr + pstStream->pstPack[0].u32Offset);
    }
    else if (iCurPacks > 1)
    {
        iDataType = CVI_RECORD_H265_TYPE_I_FRAME;
        gst_bRecordKeyFrame = 1;
        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "====receive %d pack in 1 frame=====\n", iCurPacks);
        pstFrameInfo.pBuf = malloc(iFrameLen);

        for (i = 0; i < iCurPacks; i++)
        {
            memcpy(pstFrameInfo.pBuf + iBufOffset,
                    (pstStream->pstPack[i].pu8Addr + pstStream->pstPack[i].u32Offset),
                    (pstStream->pstPack[i].u32Len - pstStream->pstPack[i].u32Offset));
            iBufOffset += (pstStream->pstPack[i].u32Len - pstStream->pstPack[i].u32Offset);
        }
    }
    pstFrameInfo.iSubType = iDataType;
    if (gst_bRecordKeyFrame)
    {
        iRet = CVI_RECORD_DataInput(&pstFrameInfo);
        if (iCurPacks > 1)
        {
            free(pstFrameInfo.pBuf);
        }
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "====WAIT IDR FRAME=====\n");
    }

VIDEO_RECORD_EXIT:
    pthread_mutex_unlock(&s_iRecordMutex);

    return iRet;
}

CVI_S32 app_ipcam_Record_StartReplay(char *pReplayTime)
{
    if (NULL == pReplayTime)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pReplayTime is NULL!\n");
        return CVI_FAILURE;
    }

    CVI_S32 iYear = 0;
    CVI_S32 iMonth = 0;
    CVI_S32 iDays = 0;
    CVI_S32 iHour = 0;
    CVI_S32 iMin = 0;
    CVI_S32 iSec = 0;
    CVI_U32 MonthDays = 0;
    CVI_S32 pRecSegCount = 0;
    CVI_RECORD_RECORD_TS_S * pRecSegHead = NULL;
    CVI_RECORD_RECORD_TS_S * pReadHead = NULL;

    //20210520141414
    sscanf(pReplayTime, "%04d%02d%02d%02d%02d%02d", &iYear, &iMonth, &iDays, &iHour, &iMin, &iSec);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "Replay Time:%04d-%02d-%02d %02d:%02d:%02d\n", iYear, iMonth, iDays, iHour, iMin, iSec);

    CVI_REPLAY_QueryByMonth(iYear, iMonth, &MonthDays);
    if (0 != MonthDays)
    {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "MonthDays:%d have record!\n", MonthDays);
        if (0 == (MonthDays & (1 << (iDays - 1))))
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Days:%d no record!\n", iDays);
            return CVI_FAILURE;
        }
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Months:%d no record!\n", iMonth);
        return CVI_FAILURE;
    }

    if (0 == CVI_REPLAY_QueryByDay(iYear, iMonth, iDays, CVI_RECORD_EVENT_ALL_ALARM, &pRecSegHead, &pRecSegCount))
    {
        pReadHead = pRecSegHead;
        while (pReadHead)
        {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "Time(%lu - %lu) iEvenType:%d  RecSegCount:%d\n", 
                pReadHead->tBeginTime, pReadHead->tEndTime, pReadHead->iEvenType, pRecSegCount);
            pReadHead = pReadHead->ptNext;
        }
        CVI_REPLAY_QueryFreeTsArr(pRecSegHead);
        pRecSegHead = NULL;
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "query record failed!\n");
        return CVI_FAILURE;
    }

    time_t tBeginTime = 0;
    time_t tSeekTime = 0;
    time_t tEndTime = 0;
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year  = iYear - 1900;
    tm.tm_mon   = iMonth - 1;
    tm.tm_mday  = iDays;
    tm.tm_hour  = iHour;
    tm.tm_min   = iMin;
    tm.tm_sec   = iSec;
    tSeekTime   = mktime(&tm);
    
    tm.tm_hour  = 0;
    tm.tm_min   = 0;
    tm.tm_sec   = 0;
    tBeginTime  = mktime(&tm);
    tEndTime    = tBeginTime + RECORD_ONE_DAY;
    
    iPkgHand = CVI_REPLAY_Create(tBeginTime, tEndTime, CVI_RECORD_EVENT_ALL_ALARM, app_ipcam_Record_GetStream);
    if (iPkgHand)
    {
        CVI_REPLAY_Seek(iPkgHand, tSeekTime);
        CVI_REPLAY_DataSetKeyFrame(iPkgHand, REPLAY_ONLY_I_FRAME);
    }

    return CVI_SUCCESS;
}

CVI_VOID app_ipcam_Record_StopReplay()
{
    if (iPkgHand)
    {
        CVI_REPLAY_Release(iPkgHand);
    }
    if (gst_ReplayVideoFd)
    {
        close(gst_ReplayVideoFd);
        gst_ReplayVideoFd = -1;
    }
    if (gst_ReplayAudioFd)
    {
        close(gst_ReplayAudioFd);
        gst_ReplayAudioFd = -1;
    }
}

/*****************************************************************
 *  The following API for command test used             Front
 * **************************************************************/
static int app_ipcam_CmdTask_Record_Parse(CVI_MQ_MSG_t *msg, CVI_VOID *userdate)
{
    if (NULL == msg)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "msg is NULL!\n");
        return -1;
    }
    CVI_CHAR param[512] = {0};
    snprintf(param, sizeof(param), "%s", msg->payload);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s param:%s\n", __FUNCTION__, param);

    CVI_CHAR *temp = strtok(param, ":");
    while (NULL != temp)
    {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "%s switch case -> %c \n", __FUNCTION__, *temp);
        switch (*temp)
        {
            case 's': 
            {
                temp = strtok(NULL, "/");
                if (atoi(temp))
                {
                    CVI_RECORD_Resume();
                }
                else
                {
                    CVI_RECORD_Pause();
                }
                break;
            }
            case 'r': 
            {
                temp = strtok(NULL, "/");
                if (strlen("20220707121100") == strlen(temp))
                {
                    app_ipcam_Record_StartReplay(temp);
                }
                else
                {
                    app_ipcam_Record_StopReplay();
                }
                break;
            }
            case 'm':
            {
                temp = strtok(NULL, "/");
                if (strlen("202207") == strlen(temp))
                {
                    CVI_S32 iYear = 0;
                    CVI_S32 iMonth = 0;
                    CVI_U32 iDays = 0;
                    sscanf(temp, "%04d%02d", &iYear, &iMonth);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "Check Month Time:%04d-%02d\n", iYear, iMonth);
                    CVI_REPLAY_QueryByMonth(iYear, iMonth, &iDays);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "iDays:%d have record!\n", iDays);
                }
                break;
            }
            case 'd':
            {
                temp = strtok(NULL, "/");
                if (strlen("20220707") == strlen(temp))
                {
                    CVI_S32 iYear = 0;
                    CVI_S32 iMonth = 0;
                    CVI_S32 iDays = 0;
                    CVI_S32 pRecSegCount = 0;
                    CVI_RECORD_RECORD_TS_S * pRecSegHead = NULL;
                    CVI_RECORD_RECORD_TS_S * pReadHead = NULL;

                    sscanf(temp, "%04d%02d%02d", &iYear, &iMonth, &iDays);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "Check Month Time:%04d-%02d-%02d\n", iYear, iMonth, iDays);
                    if (0 == CVI_REPLAY_QueryByDay(iYear, iMonth, iDays, CVI_RECORD_EVENT_ALL_ALARM, &pRecSegHead, &pRecSegCount))
                    {
                        pReadHead = pRecSegHead;
                        while (pReadHead)
                        {
                            APP_PROF_LOG_PRINT(LEVEL_INFO, "Time(%lu - %lu) iEvenType:%d  RecSegCount:%d\n", 
                                pReadHead->tBeginTime, pReadHead->tEndTime, pReadHead->iEvenType, pRecSegCount);
                            pReadHead = pReadHead->ptNext;
                        }
                        CVI_REPLAY_QueryFreeTsArr(pRecSegHead);
                        pRecSegHead = NULL;
                    }
                    else
                    {
                        APP_PROF_LOG_PRINT(LEVEL_ERROR, "query record failed!\n");
                    }
                }
                break;
            }
            case 'c':
            {
                CVI_S32 iPrecent = 0;
                CVI_U32 iTotalSize = 0;
                CVI_U32 iRemainSize = 0;
                CVI_STORAGE_STATUS_E iPKGStatus = CVI_STORAGE_STATUS_ERROR;
                CVI_RECORD_GetStatus(&iPKGStatus, &iPrecent, &iTotalSize, &iRemainSize);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "iPKGStatus:%d  iPrecent:%d  iTotalSize:%d  iRemainSize:%d\n", 
                    iPKGStatus, iPrecent, iTotalSize, iRemainSize);
                break;
            }
            default:
                return 0;
                break;
        }

        temp = strtok(NULL, ":");
    }

    return CVI_SUCCESS;
}

int app_ipcam_CmdTask_Record_Task(CVI_MQ_MSG_t *msg, CVI_VOID *userdate)
{
    if (NULL == msg)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "msg is NULL!\n");
        return -1;
    }

    APP_CHK_RET(app_ipcam_CmdTask_Record_Parse(msg, userdate), "record task Parse");

    return CVI_SUCCESS;
}

int app_ipcam_Set_Record_Status(int iStatus)
{
    gst_bStart = iStatus;
    if (iStatus)
    {
        CVI_RECORD_Resume();
    }
    else
    {
        CVI_RECORD_Pause();
    }
    return 0;
}

int app_ipcam_Get_Record_Status()
{
    CVI_STORAGE_STATUS_E iPKGStatus = CVI_STORAGE_STATUS_ERROR;
    CVI_RECORD_GetStatus(&iPKGStatus, NULL, NULL, NULL);
    if (CVI_STORAGE_STATUS_NORMAL != iPKGStatus)
    {
        return 0;
    }
    else
    {
        return gst_bStart;

    }
    return 1;
}

CVI_S32 app_ipcam_Set_Record_Check(char *pReplayTime)
{
    if (NULL == pReplayTime)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pReplayTime is NULL!\n");
        return CVI_FAILURE;
    }

    CVI_S32 iYear = 0;
    CVI_S32 iMonth = 0;
    CVI_S32 iDays = 0;
    CVI_U32 MonthDays = 0;
    CVI_S32 pRecSegCount = 0;
    CVI_RECORD_RECORD_TS_S * pRecSegHead = NULL;
    CVI_RECORD_RECORD_TS_S * pReadHead = NULL;

    //20210520141414
    sscanf(pReplayTime, "%04d%02d%02d", &iYear, &iMonth, &iDays);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "Replay Time:%04d-%02d-%02d\n", iYear, iMonth, iDays);

    CVI_REPLAY_QueryByMonth(iYear, iMonth, &MonthDays);
    if (0 != MonthDays)
    {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "MonthDays:%d have record!\n", MonthDays);
        if (0 == (MonthDays & (1 << (iDays - 1))))
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Days:%d no record!\n", iDays);
            return CVI_FAILURE;
        }
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Months:%d no record!\n", iMonth);
        return CVI_FAILURE;
    }

    if (0 == CVI_REPLAY_QueryByDay(iYear, iMonth, iDays, CVI_RECORD_EVENT_ALL_ALARM, &pRecSegHead, &pRecSegCount))
    {
        int iLen = 0;
        app_ipcam_Clean_Record_Seg();

        int iBufSize = (pRecSegCount + 1) * 24;
        pstCheckSeg = malloc(iBufSize);
        if (pstCheckSeg == NULL)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "pDstSeg malloc failed!\n");
            return CVI_FAILURE;
        }
        memset(pstCheckSeg, 0, iBufSize);

        int iSegCnt = 0;
        pReadHead = pRecSegHead;
        while (pReadHead)
        {
            //avoid seg too much; web show fail
            if (iSegCnt > REPLAY_MAX_SEG) {
                break;
            }
            iSegCnt++;

            struct tm Begin;
            struct tm End;
            time_t tBeginTime = pReadHead->tBeginTime;
            time_t tEndTime = pReadHead->tEndTime;
            localtime_r(&tBeginTime, &Begin);
            localtime_r(&tEndTime, &End);
            iLen += snprintf(pstCheckSeg + iLen, iBufSize - iLen, "%02d:%02d:%02d-%02d:%02d:%02d %d\r\n",
                Begin.tm_hour, Begin.tm_min, Begin.tm_sec, End.tm_hour, End.tm_min, End.tm_sec, pReadHead->iEvenType);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "Time(%lu - %lu) iEvenType:%d  RecSegCount:%d\n", 
                pReadHead->tBeginTime, pReadHead->tEndTime, pReadHead->iEvenType, pRecSegCount);
            pReadHead = pReadHead->ptNext;
        }
        CVI_REPLAY_QueryFreeTsArr(pRecSegHead);
        pRecSegHead = NULL;
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "query record failed!\n");
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Get_Record_Check(char **pCheckSeg)
{
    if (pstCheckSeg == NULL)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "check no seg!\n");
        return CVI_FAILURE;
    }
    *pCheckSeg = pstCheckSeg;
    return CVI_SUCCESS;
}

CVI_VOID app_ipcam_Clean_Record_Seg()
{
    if (pstCheckSeg == NULL)
    {
        free(pstCheckSeg);
        pstCheckSeg = NULL;
    }
}

