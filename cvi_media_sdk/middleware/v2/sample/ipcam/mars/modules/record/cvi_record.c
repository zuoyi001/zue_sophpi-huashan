#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <errno.h>
#include "cvi_record.h"

//debug
/* tag setting */
#define STRINGIFY(x, fmt)           #x fmt
#define ADDTRACECTAG(fmt)           STRINGIFY([app][trace][%s %u], fmt)
#define ADDDBGTAG(fmt)              STRINGIFY([app][dbg][%s %u], fmt)
#define ADDTAG(fmt)                 STRINGIFY([app], fmt)
#define ADDWARNTAG(fmt)             STRINGIFY([app][warn][%s %u], fmt)
#define ADDERRTAG(fmt)              STRINGIFY([app][err][%s %u], fmt)

#define _RECORD_DBG(fmt, args...)                                                                              \
        if(RECORD_LEVEL_DEBUG <= RECORD_LEVEL) {                                                               \
            {printf(ADDDBGTAG(fmt), __FUNCTION__, __LINE__, ##args);}                                          \
        }
#define _RECORD_INFO(fmt, args...)                                                                             \
        if(RECORD_LEVEL_INFO <= RECORD_LEVEL) {                                                                \
            {printf(ADDTAG(fmt), ##args);}                                                                     \
        }
#define _RECORD_WARN(fmt, args...))                                                                            \
        if(RECORD_LEVEL_WARN <= RECORD_LEVEL) {                                                                \
            {printf(ADDWARNTAG(fmt), __FUNCTION__, __LINE__, ##args);}                                         \
        }
#define _RECORD_ERR(fmt, args...)                                                                              \
        if(RECORD_LEVEL_ERROR <= RECORD_LEVEL) {                                                               \
            {printf(ADDERRTAG(fmt), __FUNCTION__, __LINE__, ##args);}                                          \
        }

#define RECORD_OK                                 (0)       /*! 没有错误 */
#define RECORD_ERROR                              (-1)    /*! 其他错误*/

#define CVI_RECORD_FUNC_START  _RECORD_DBG("CVI_RECORD_FUNC_START\n")
#define CVI_RECORD_FUNC_END    _RECORD_DBG("CVI_RECORD_FUNC_END\n")

#define CVI_RECORD_MIN_TIMES  (60)
#define CVI_RECORD_HOUR_TIMES (60*CVI_RECORD_MIN_TIMES)
#define CVI_RECORD_DAY_TIMES  (24*CVI_RECORD_HOUR_TIMES)

#define CVI_RECORD_BAD_FILE_WRITE_TIMES  (10) //写三次失败则标记坏道文件

#define CVI_RECORD_MAX_MISS_AVFILES      (3) //检测最大多少个视频文件不存在

#define HIDWORD(a) ((CVI_U32)(((CVI_U64)(a)) >> 32))
#define LODWORD(a) ((CVI_U32)(CVI_U64)(a))
//#define fallocate(fd, mode, offset, len) syscall(__NR_fallocate, fd, mode, LODWORD(offset), HIDWORD(offset), LODWORD(len), HIDWORD(len))

#define CVI_RECORD_FILES_GET_INDEX_NUM(iFileIndex)      (iFileIndex / CVI_RECORD_INDEX_MAX_AVFILES) //从文件号获取文件索引序号
#define CVI_RECORD_FILES_GET_INDEX_FILES(iFileIndex)    (iFileIndex % CVI_RECORD_INDEX_MAX_AVFILES) //从文件号获取文件索引中的文件号

//对应索引的seek位置
#define CVI_RECORD_FILE_INDEX_HEADER_SEEK                    (0)
#define CVI_RECORD_FILE_INDEX_RECORD_SEEK(iFileIndex)        (CVI_RECORD_FILE_INDEX_HEADER_SEEK + PKG_HEADER_INDEX_MAX_LEN + (sizeof(CVI_RECORD_FILE_INDEX_RECORD)*CVI_RECORD_FILES_GET_INDEX_FILES(iFileIndex)))
#define CVI_RECORD_SEGMENT_INDEX_RECORD_SEEK(iFileIndex, iSegIndex)        (CVI_RECORD_FILE_INDEX_HEADER_SEEK + PKG_HEADER_INDEX_MAX_LEN + PKG_RECORD_INDEX_MAX_LEN  + (sizeof(CVI_RECORD_SEGMENT_INDEX_RECORD)*(iSegIndex + (CVI_RECORD_AV_FILE_MAX_SEGMENT*CVI_RECORD_FILES_GET_INDEX_FILES(iFileIndex)))))

#define CVI_RECORD_REMAIN_INDEX_SIZE     (CVI_RECORD_INDEX_FILE_MAX_SIZE - (CVI_RECORD_SEGMENT_INDEX_RECORD_SEEK(CVI_RECORD_INDEX_MAX_AVFILES-1, CVI_RECORD_AV_FILE_MAX_SEGMENT-1)+sizeof(CVI_RECORD_SEGMENT_INDEX_RECORD)))

// 视频文件索引从尾部算INFO获取
#define CVI_RECORD_FRAME_HEADER_INFO_SEEK(pSegIndexRecord)        (pSegIndexRecord->iInfoEndOffset-CVI_RECORD_FRAME_HEADER_INFO_MAX*sizeof(CVI_RECORD_SEGMENT_INFO))

#define CVI_RECORD_FRAME_AVINDEX_GET_START_OFFSET(pSegIndexRecord)    (pSegIndexRecord->iInfoEndOffset - (pSegIndexRecord->iInfoCount*sizeof(CVI_RECORD_SEGMENT_INFO)))

//清空缓存
#define CVI_RECORD_FREE_CACHE cvi_system("sync;echo 3 > /proc/sys/vm/drop_caches")
#define CVI_RECORD_FREE_CACHE_MAX_TOTAL    (5*1024*1024) //读写字符超过这个数据后清cache

typedef struct
{
    CVI_U32  buffersize;   //申请的chunk 长度默认VIDEO_CHUNK_BUFFER_SIZE，超出重申
    CVI_U32  bufferused;   //使用的chunk长度
    CVI_U8*  buff;         //数据
}pkg_buffer;


typedef struct
{
    CVI_U8*  iReadBuff;                //数据
    CVI_S32  iReadFd;                  //Buf的读数据fd
    CVI_U32  iStartOffSet;             //Buf的起始帧offset
    CVI_U32  iReadLen;                 //Buf的读了offset
    CVI_U32  iMaxReadLen;              //Buf读取最大长度
}pkg_read_buffer;


/**************************************************************************************************
回放搜索的起始结束片段
****************************************************************************************************/
typedef struct _SEEK_RECORD
{
    time_t                     tBeginTime;                         //起始时间
    time_t                     tEndTime;                           //结束时间
    CVI_S32                    iRecFileNo;                         //文件号
    CVI_S32                    iStartSegNum;                       //开始片段
    CVI_S32                    iEndSegNum;                         //结束片段
    struct _SEEK_RECORD        *ptNext;
    struct _SEEK_RECORD        *ptPrevious;
}__attribute__((packed)) CVI_REPLAY_SEEK_RECORD, *PCVI_REPLAY_SEEK_RECORD;


/*数据弹出器结构体*/
typedef struct _T_PSS_DataPoper
{
    CVI_S32                    pThreadRun;                                 //线程运行状态
    pthread_t                  pPopThread;                                 //弹出线程句柄
    pthread_mutex_t            iPopMutex;                                  //数据锁
    CVI_S32                    bOpen;                                      //弹出器状态，正在使用中为True, 否则为False.
    CVI_S32                    bStartPop;                                  //是否开启弹出
    time_t                     tBeginTime;                                 //起始时间
    time_t                     tEndTime;                                   //结束时间
    time_t                     tSeekTime;                                  //跳转时间,0不跳转 大于0跳转
    CVI_U32                    iEvenType;                                  //录像当前类型
    CVI_UL                     PopKeyInterval;                             //只弹关键帧模式下，0全部弹出大于0弹出关键帧
    CVI_REPLAY_DATAPOP_CALLBACK  pGetDataCallback;                         //回放回调函数句柄
} T_PSS_DataPoper, *pTPPS_DataPoper;

/* 录像文件索引头 ,总字节1280*/
typedef struct
{
    CVI_S32                        iPkginited;                                          //是否初始化
    CVI_S32                        bPkgstop;                                            //是否暂停
    CVI_CHAR                       iPkgPathName[CVI_RECORD_MAX_PATH_NAME];              //文件路径
    CVI_S32                        iCurrFileRecNo;                                      //当前写文件号
    CVI_S32                        iCurrFileRecSegment;                                 //当前写文件号片段
    CVI_S32                        iMaxAVFileNum;                                       //文件最大数
    CVI_S32                        iMaxIndexNum;                                        //文件索引最大数
    CVI_S32                        iReMainAVFileNum;                                    //未写剩余
    CVI_S32                        iPkgFormatPrecent;                                   //当前格式化进度
    CVI_STORAGE_STATUS_E           iPkgStatus;                                          //当前存储状态
    CVI_RECORD_RUN_MODE_E          iPkgRunMode;                                         //运行状态
    CVI_RECORD_WRITE_MODE_E        iWriteMode;                                          //写录像模式
    CVI_U32                        iPreTimes;                                           //写录像预录时间
    CVI_U32                        iEvenType;                                           //写录像当前类型
    CVI_S32                        bWaitKeyFrame;                                       //等待I帧
    CVI_S32                        bNewSegment;                                         //新片段标记
    CVI_S32                        iIsRead;                                             //读输入数量
    CVI_S32                        iIsWrite;                                            //是否写文件
    CVI_S32                        iCurIndexFd;                                         //索引FD
    CVI_S32                        iCurIndexBakFd;                                      //索引备份fd
    CVI_S32                        iIndexErrCount;                                      //写文件失败次数
    CVI_S32                        iCurRecordFd;                                        //录像文件fd
    CVI_S32                        iCurRecordAvFd;                                      //录像文件fd
    CVI_S32                        iFileErrCount;                                       //写文件失败次数
    pkg_buffer                     iPkgWriteBuff;                                       //buf大小格式化及写录像使用
    CVI_U32                        iPkgSegWriteOffset;                                  //当前写录像的位置
    CVI_RECORD_SEGMENT_INFO        iPkgWriteIndexHead[CVI_RECORD_FRAME_HEADER_INFO_MAX];//当前录像文件索引缓存
    CVI_RECORD_SEGMENT_INFO*       iPkgWriteIndexBuff;                                  //当前录像文件索引缓存CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_COUNT
    CVI_U32                        iPkgWriteSegmentNum;                                 //当前片段写入帧数
    pkg_buffer                     iPkgCurFileIndexBuff;                                //当前文件索引buf
}CVI_RECORD_PARAM, *PCVI_RECORD_PARAM;

CVI_CHAR szVersion_PKGSTREAM [] = "1.0.0.1 Build "__DATE__"-"__TIME__;
static CVI_RECORD_PARAM s_mPkgParam;

T_PSS_DataPoper            s_mDataPoper[CVI_RECORD_MAX_RECORD_READER];
pthread_mutex_t            s_iDataPopMutex[CVI_RECORD_MAX_RECORD_READER];
pthread_mutex_t            s_iPkgRunMutex = PTHREAD_MUTEX_INITIALIZER;                                        //PKG状态锁
#ifdef CVI_RECORD_FREE_CACHE_MAX_TOTAL
static CVI_U32 s_iCacheRWTotal = 0;
#endif

/*****************************************************************************
 函 数 名  : CVI_RECORD_WriteFrameIndex
 功能描述  :写入录像帧索引
*****************************************************************************/
static CVI_S32 CVI_RECORD_WriteFrameIndex(PT_FRAME_INFO  pstFrameInfo, CVI_S32 bIndexFrame, CVI_S32 bWriteSegment, CVI_S32 bChangeFile);

/*****************************************************************************
 函 数 名  : CVI_RECORD_WriteFrame
 功能描述  :写入录像帧
*****************************************************************************/
static CVI_S32 CVI_RECORD_WriteFrame(PT_FRAME_INFO  pstFrameInfo, CVI_S32 bFlush, CVI_S32 bIndexFrame);

static CVI_S32 CVI_RECORD_LoadCurrentInfo(CVI_U32 iCurrFileRecNo);

static CVI_S32 CVI_RECORD_GetNextNumber(CVI_U32 iCurrFileRecNo);

/**
 * \brief 睡眠
 * \param ntime 睡眠的微妙数
 */
static void rsleep (CVI_U32 ntime)
{
    fd_set  rfds;
    struct  timeval tv;

    /* Watch stdin  (fd 0) to see when it has input. */
    FD_ZERO (&rfds);

    FD_SET (0, &rfds);
    /* Wait up to five seconds. */
    tv.tv_sec = ntime / 1000000;
    tv.tv_usec = ntime % 1000000;
    select (0, &rfds, NULL, NULL, &tv);
}

CVI_U64 monotonic_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ULL + tv.tv_usec;
}

static CVI_S32 CVI_RECORD_GetCrc(const CVI_CHAR* pData,CVI_U32 nDataNum)
{
    CVI_U32 i = 0;
    CVI_S32 iSum = 0;
    if (NULL == pData) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }
    for (i = 0;i < nDataNum; i++) {
        iSum += pData[i];
    }
    return iSum;
}

/*****************************************************************************
 函 数 名  : get_file_size
 功能描述  : 获取文件大小
 输入参数  :  const CVI_CHAR *path 文件名
 输出参数  : 
 返 回 值  :  -1失败，其他成功
*****************************************************************************/
static CVI_UL CVI_RECORD_GetFileSize(const CVI_CHAR *path)
{
    CVI_UL filesize = -1;
    struct stat statbuff;
    if (stat(path, &statbuff) < 0) {
        return filesize;
    } else {
        filesize = statbuff.st_size;
    }
    return filesize;
}

static CVI_S32 CVI_RECORD_CopyFile(const CVI_CHAR *srcFilePath, const CVI_CHAR *destFilePath, CVI_S32 bCompara)
{
    CVI_S32 dstfd = -1;
    CVI_S32 srcfd = -1;
    CVI_S32 iRet = 0;
    
    CVI_U8 *iSrcBuf = NULL;
    CVI_U8 *iDstBuf = NULL;
    CVI_S32 iBufLen = 0;
    CVI_S32 iSrcRWlen = 0;
    CVI_S32 iDstRWlen = 0;
    if (srcFilePath == NULL || destFilePath == NULL)
        return -1;

    _RECORD_DBG("Copy file: %s->%s\n", srcFilePath, destFilePath);
    iBufLen = 128 * 1024;
    iSrcBuf = malloc(iBufLen);
    if (NULL == iSrcBuf) {
        _RECORD_ERR("Malloc failed:%d\n", iBufLen);
        iRet = RECORD_ERROR;
        goto endFunc;
    }

    srcfd = open(srcFilePath, O_RDONLY);
    if (srcfd <= 0) {
        _RECORD_ERR("%s open failed\n", srcFilePath);
        iRet = RECORD_ERROR;
        goto endFunc;
    }

    if (bCompara) {
        CVI_UL iSrcSize = CVI_RECORD_GetFileSize(srcFilePath);
        CVI_UL iDstSize = CVI_RECORD_GetFileSize(destFilePath);
        if ((iSrcSize > 0) && (iSrcSize == iDstSize)) {
            //长度一样才对比拷贝
            dstfd = open(destFilePath, O_RDWR);
            if (dstfd > 0) {
                iDstBuf = malloc(iBufLen);
                if (NULL == iDstBuf) {
                    _RECORD_ERR("Malloc failed:%d\n", iBufLen);
                    iRet = RECORD_ERROR;
                    goto endFunc;
                }
                
                while (1) {
                    iSrcRWlen = read(srcfd, iSrcBuf, iBufLen);
                    iDstRWlen = read(dstfd, iDstBuf, iBufLen);
                    if (iSrcRWlen != iDstRWlen) {
                        break;
                    } else if (iSrcRWlen <= 0) {
                        break;
                    } else {
                        iSrcSize -= iSrcRWlen;
                        if (0 != memcmp(iSrcBuf, iDstBuf, iSrcRWlen)) {
                            _RECORD_DBG("write cmp(%d) diff, size:%lu, remain:%lu\n", iSrcRWlen, iDstSize, iSrcSize);
                            lseek(dstfd, -1 * iSrcRWlen, SEEK_CUR);
                            write(dstfd, iSrcBuf, iSrcRWlen);
                            usleep(10 * 1000);
                        }
                    }
                }
                close(dstfd);
                dstfd = -1;
                if (0 == iSrcSize) {
                    //对比完成
                    _RECORD_DBG("Copy cmp file ok,size:%lu\n", iDstSize);
                    iRet = 0;
                    goto endFunc;
                } else {
                    _RECORD_DBG("Copy cmp file err,size:%lu, remain:%lu\n", iDstSize, iSrcSize);
                }
            }
        }
    }

    dstfd = open(destFilePath, O_RDWR|O_CREAT|O_TRUNC);
    if (dstfd <= 0) {
        _RECORD_ERR("%s open failed,del check\n", destFilePath);
        remove(destFilePath);
        
        dstfd = open(destFilePath, O_RDWR|O_CREAT|O_TRUNC);
        if (dstfd <= 0) {
            _RECORD_ERR("%s open failed out\n", destFilePath);
            iRet = RECORD_ERROR;
            goto endFunc;
        }
    }

    do
    {
        iSrcRWlen = read(srcfd, iSrcBuf, iBufLen);
        if (iSrcRWlen > 0) {
            write(dstfd, iSrcBuf, iSrcRWlen);
            usleep(10*1000);
        }
    } while(iSrcRWlen > 0);

    iRet = 0;

endFunc:
    if (srcfd > 0) {
        close(srcfd);
        srcfd = -1;
    }
    
    if (dstfd > 0) {
        close(dstfd);
        dstfd = -1;
    }
    if (iSrcBuf) {
        free(iSrcBuf);
        iSrcBuf = NULL;
    }
    if (iDstBuf) {
        free(iDstBuf);
        iDstBuf = NULL;
    }
    return iRet;
}


/*****************************************************************************
 函 数 名  : CVI_RECORD_DirIsExist
 功能描述  : 获取目录存在
 输入参数  :  const CVI_CHAR *pathDir 目录名
 输出参数  : 
 返 回 值  :  1存在
*****************************************************************************/
static CVI_BOOL CVI_RECORD_DirIsExist(const CVI_CHAR *pathDir)
{
    struct stat iStbuffer;
    if (NULL == pathDir) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return 0;
    }

    if (stat(pathDir, &iStbuffer) < 0) {
        _RECORD_ERR("pkg dir no exits0:%s\n", pathDir);
        return 0;
    }

    if (!S_ISDIR(iStbuffer.st_mode)) {
        _RECORD_ERR("pkg dir no exits1:%s\n", pathDir);
        return 0;
    }
    return 1;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_GetIndexFileFd
 功能描述  : 获取索引文件fd
*****************************************************************************/
static CVI_S32 CVI_RECORD_PreFormatFiles(const CVI_CHAR * stPathName, CVI_UL ifileSize)
{
    CVI_S32 iRet = 0;
    CVI_S32 fd = 0;
    
    if (NULL == stPathName) {
        _RECORD_ERR("Invalid Input:%s\n", stPathName);
        return RECORD_ERROR;
    }

    _RECORD_DBG("pre_format_files :%s,%lu\n", stPathName, ifileSize);
    if (0 == access(stPathName, F_OK)) {
        _RECORD_ERR("Invalid stPathName(%s) is exist, remove file\n", stPathName);
        remove(stPathName);
    }
    fd = open(stPathName, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (0 >= fd) {
        _RECORD_ERR("Invalid Input:%d,%s\n", fd, strerror(errno));
        return RECORD_ERROR;
    }
    iRet = fallocate(fd, FALLOC_FL_KEEP_SIZE, 0, ifileSize);
    if  (0 != iRet) {
        _RECORD_ERR("fallocate file err, fd:%d, ret:%d,%s\n", fd, iRet, strerror(errno));
        close(fd);
        return RECORD_ERROR;
    }

    iRet = ftruncate(fd, ifileSize);
    if (0 != iRet) {
        _RECORD_ERR("ftruncate file err, fd:%d, ret:%d,%s\n", fd, iRet, strerror(errno));
        close(fd);
        return RECORD_ERROR;
    }
    close(fd);
    return iRet;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_GetIndexFileFd
 功能描述  : 获取索引文件fd
*****************************************************************************/
static CVI_S32 CVI_RECORD_GetIndexBakFileFd(const CVI_CHAR *path, CVI_S32 iIndex, mode_t mode, CVI_UL *ifileSize)
{
    CVI_S32 fd = -1;
    CVI_CHAR stPathName[CVI_RECORD_MAX_PATH_NAME];

    if (NULL == path) {
        _RECORD_ERR("Invalid Input:%s\n", stPathName);
        return RECORD_ERROR;
    }
    snprintf(stPathName, sizeof(stPathName), "%s/"CVI_RECORD_INDEX_FILE_BAK, path, iIndex);
    fd = open(stPathName, mode);
    if (fd <= 0) {
        _RECORD_ERR("pkg open file err:%s\n", stPathName);
    } else {
        if (ifileSize)
            *ifileSize = CVI_RECORD_GetFileSize(stPathName);
    }
    return fd;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_GetIndexFileFd
 功能描述  : 获取索引文件fd
*****************************************************************************/
static CVI_S32 CVI_RECORD_GetIndexFileName(const CVI_CHAR *path, CVI_S32 iIndex, CVI_CHAR* pFileName, CVI_S32 iLen)
{
    if  ((NULL == path) || (NULL == pFileName) || (iLen <= 0)) {
        _RECORD_ERR("Invalid Input:%s\n", path);
        return RECORD_ERROR;
    }

    snprintf(pFileName, iLen, "%s/"CVI_RECORD_INDEX_FILE, path, iIndex);

    return 0;
}



/*****************************************************************************
 函 数 名  : CVI_RECORD_GetIndexFileFd
 功能描述  : 获取索引文件fd
*****************************************************************************/
static CVI_S32 CVI_RECORD_GetIndexFileFd(const CVI_CHAR *path, CVI_S32 iIndex, mode_t mode, CVI_UL *ifileSize)
{
    CVI_S32 fd = -1;
    CVI_CHAR stPathName[CVI_RECORD_MAX_PATH_NAME];

    if (NULL == path) {
        _RECORD_ERR("Invalid Input\n");
        return RECORD_ERROR;
    }

    if (0 != CVI_RECORD_GetIndexFileName(path, iIndex, stPathName, sizeof(stPathName))) {
        _RECORD_ERR("Invalid Input\n");
        return RECORD_ERROR;
    }

    fd = open(stPathName, mode);
    if (fd <= 0) {
        _RECORD_ERR("pkg open file err:%s\n", stPathName);
    } else {
        if (ifileSize) {
            *ifileSize = CVI_RECORD_GetFileSize(stPathName);
        }
    }
    return fd;
}


/*****************************************************************************
 函 数 名  : CVI_RECORD_GetAvFilesFd
 功能描述  :获取视频文件fd
*****************************************************************************/
static CVI_S32 CVI_RECORD_GetAvFilesFd(const CVI_CHAR *path, CVI_U32 iRecIndexNo, mode_t mode, CVI_UL *ifileSize)
{
    CVI_S32 fd = -1;
    CVI_CHAR stPathName[CVI_RECORD_MAX_PATH_NAME];
    if (NULL == path) {
        _RECORD_ERR("Invalid Input\n");
        return RECORD_ERROR;
    }

    snprintf(stPathName, sizeof(stPathName), "%s/"CVI_RECORD_AV_FILE, path, iRecIndexNo);
    fd = open(stPathName, mode);
    if (fd <= 0)
        _RECORD_ERR("pkg open file err:%s\n", stPathName);

    if (ifileSize && (fd > 0))
        *ifileSize = CVI_RECORD_GetFileSize(stPathName);

    return fd;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_GetAvFilesFromCard
 功能描述  :获取文件视频总数
*****************************************************************************/
static CVI_S32 CVI_RECORD_GetAvFilesFromCard(const CVI_CHAR *path)
{
    CVI_S32 index = 0;
    CVI_S32 iAVFiles = 0;
    CVI_S32 iMissFiles = 0;
    CVI_CHAR stPathName[CVI_RECORD_MAX_PATH_NAME];
    _RECORD_INFO("get avfiles form file\n");

    if (NULL == path) {
        _RECORD_ERR("Invalid Input:%s\n", stPathName);
        return 0;
    }

    while (iMissFiles <= CVI_RECORD_MAX_MISS_AVFILES) {
        snprintf(stPathName, sizeof(stPathName), "%s/"CVI_RECORD_AV_FILE, path, index);
        index++;
        if ((0 != access(stPathName, F_OK))) {
            iMissFiles++;
            _RECORD_INFO("no avfiles:%s\n", stPathName);
        } else {
            iMissFiles = 0;
            iAVFiles = index;
        }
    }
    _RECORD_INFO("get avfiles form file:%d\n", iAVFiles);
    return iAVFiles;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_ReadFileOffset
 功能描述  读出录像帧索引
 输入参数  : 
 PT_FRAME_INFO  pstFrameInfo   帧数据参数到文件
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
static CVI_S32 CVI_RECORD_ReadFileOffset(CVI_S32 fd, CVI_U32 iOffset, CVI_U8* pBuf, CVI_S32 bufLen)
{
    CVI_S32 nReadLen = 0;
    if ((NULL == pBuf) || (0 >= bufLen) || (fd <= 0)) {
        _RECORD_ERR("Invalid Input:%p, %d\n", pBuf, fd);
        return RECORD_ERROR;
    }

    if (-1 == lseek(fd, iOffset, SEEK_SET)) {
        _RECORD_ERR("pkg read cannot iOffset to %u fd:%d\n", iOffset, fd);
        return ERR_OPEN_FILE_ERROR;
    }

    nReadLen = read(fd, pBuf, bufLen);
    if(nReadLen != bufLen) {
        _RECORD_ERR("pkg read fd:%d, ERR:%d != %d\n", fd, nReadLen, bufLen);
        //return ERR_OPEN_FILE_ERROR;
    }

    s_iCacheRWTotal += nReadLen;

    return nReadLen;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_MallocBuffer
 功能描述  : 申请内存
*****************************************************************************/
static CVI_S32 CVI_RECORD_MallocBuffer(pkg_buffer* pPkgBuff, CVI_U32 len)
{
    if (pPkgBuff) {
        pPkgBuff->buff = (CVI_U8*)malloc(len);
        if (NULL == pPkgBuff->buff) {
            _RECORD_ERR("Malloc Err:size %d\n", len);
            return RECORD_ERROR;
        }
        pPkgBuff->buffersize = len;
        pPkgBuff->bufferused = 0;
        memset(pPkgBuff->buff, 0, len);
        return 0;
    }
    _RECORD_ERR("Invalid Input\n");
    return RECORD_ERROR;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_FreeBuffer
 功能描述  : 释放内存
*****************************************************************************/
static void CVI_RECORD_FreeBuffer(pkg_buffer* pPkgBuff)
{
    if (pPkgBuff && pPkgBuff->buff) {
        free(pPkgBuff->buff);
        pPkgBuff->buff = NULL;
        pPkgBuff->buffersize = 0;
        pPkgBuff->bufferused = 0;
    }
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_FlushBuffer
 功能描述  : 写入内存
 返回值 :   0写入缓存,大于0写入文件长度,小于0失败
*****************************************************************************/
static CVI_S32 CVI_RECORD_FlushBuffer(pkg_buffer* pPkgBuff, CVI_S32 fd, CVI_U32 iSeek)
{
    CVI_S32 nWriteLen = 0;
    if ((NULL == pPkgBuff) || (NULL == pPkgBuff->buff) || (fd <= 0)) {
        _RECORD_ERR("Invalid Input:%p, %d\n", pPkgBuff, fd);
        return RECORD_ERROR;
    }

    if (pPkgBuff->bufferused > 0) {
        if (iSeek > 0) {
            if (-1 == lseek(fd, iSeek, SEEK_SET))
            {
                _RECORD_ERR("pkg write1 cannot seek to %u fd:%d, len: %d\n", iSeek, fd, pPkgBuff->bufferused);
                pPkgBuff->bufferused = 0;
                return ERR_OPEN_FILE_ERROR;
            }
        }

        nWriteLen = write(fd, pPkgBuff->buff, pPkgBuff->bufferused);
        if (nWriteLen != (CVI_S32)pPkgBuff->bufferused) {
            _RECORD_ERR("pkg write1 fd:%d, ERR:%d != %d\n", fd, nWriteLen, pPkgBuff->bufferused);
            pPkgBuff->bufferused = 0;
            return ERR_OPEN_FILE_ERROR;
        }
        s_iCacheRWTotal += nWriteLen;

#ifdef CVI_RECORD_WRITE_INT
        usleep(CVI_RECORD_WRITE_INT);
#endif
        pPkgBuff->bufferused = 0;
    }
    return nWriteLen;
}

/*****************************************************************************
 函 数 名  :  CVI_RECORD_WriteBuffer
 功能描述  :  写入内存
 返回值 :   0写入缓存,大于0写入文件长度,小于0失败
*****************************************************************************/
static CVI_S32 CVI_RECORD_WriteBuffer(pkg_buffer* pPkgBuff, const CVI_U8* buff, CVI_S32 len, CVI_S32 fd, CVI_U32 iSeek)
{
    CVI_S32 leftsize = 0;
    CVI_S32 nWriteLen = 0;
    CVI_S32 iWriteTotal = 0;
    const CVI_U8* pWritebuff = buff;
    CVI_S32 nWriteSize = len;

    if ((NULL == pPkgBuff) || (NULL == pPkgBuff->buff) || (NULL == buff) || (PKG_FRAME_MAX_LEN < len) || (fd <= 0)) {
        _RECORD_ERR("Invalid Input:%p, %p, %d, %d\n", pPkgBuff, buff, len, fd);
        return RECORD_ERROR;
    }
    leftsize = pPkgBuff->buffersize - pPkgBuff->bufferused;
    if (nWriteSize > leftsize) {
        if (leftsize > 0) {
            memcpy(pPkgBuff->buff + pPkgBuff->bufferused, pWritebuff, leftsize);
            pWritebuff += leftsize;
            nWriteSize -= leftsize;
            pPkgBuff->bufferused += leftsize;
        }
        
        if (pPkgBuff->bufferused > 0) {
            if (iSeek > 0) {
                if (-1 == lseek(fd, iSeek, SEEK_SET)) {
                    _RECORD_ERR("pkg write1 cannot seek to %u fd:%d, len: %d\n", iSeek, fd, pPkgBuff->bufferused);
                    pPkgBuff->bufferused = 0;
                    return ERR_OPEN_FILE_ERROR;
                }
                iSeek = 0;//seek过后，后面不需要seek
            }
            //_RECORD_INFO("write buffer, size: %d\n", pPkgBuff->bufferused);
            nWriteLen = write(fd, pPkgBuff->buff, pPkgBuff->bufferused);
            if (nWriteLen != (CVI_S32)pPkgBuff->bufferused) {
                _RECORD_ERR("pkg write1 fd:%d, ERR:%d != %d\n", fd, nWriteLen, pPkgBuff->bufferused);
                pPkgBuff->bufferused = 0;
                return ERR_OPEN_FILE_ERROR;
            }
            s_iCacheRWTotal += nWriteLen;

#ifdef CVI_RECORD_WRITE_INT
            usleep(CVI_RECORD_WRITE_INT);
#endif

            iWriteTotal += nWriteLen;
            pPkgBuff->bufferused = 0;
        }
        if (nWriteSize > (CVI_S32)pPkgBuff->buffersize) {
            _RECORD_INFO("write big buffer, size: %d\n", nWriteLen);
            if (iSeek > 0) {
                if (-1 == lseek(fd, iSeek, SEEK_SET)) {
                    _RECORD_ERR("pkg write1 cannot seek to %u fd:%d, len: %d\n", iSeek, fd, pPkgBuff->bufferused);
                    pPkgBuff->bufferused = 0;
                    return ERR_OPEN_FILE_ERROR;
                }
            }
            nWriteLen = write(fd, pWritebuff, nWriteSize);
            if (nWriteLen != nWriteSize) {
                _RECORD_ERR("pkg write2 fd:%d, ERR:%d != %d\n", fd, nWriteLen, pPkgBuff->bufferused);
                pPkgBuff->bufferused = 0;
                return ERR_OPEN_FILE_ERROR;
            }
            s_iCacheRWTotal += nWriteLen;

#ifdef CVI_RECORD_WRITE_INT
            usleep(CVI_RECORD_WRITE_INT);
#endif

            iWriteTotal += nWriteLen;
        } else {
            memcpy(pPkgBuff->buff + pPkgBuff->bufferused, pWritebuff, nWriteSize);
            pPkgBuff->bufferused += nWriteSize;
        }
    } else {
        memcpy(pPkgBuff->buff + pPkgBuff->bufferused, pWritebuff, nWriteSize);
        pPkgBuff->bufferused += nWriteSize;
    }
    return iWriteTotal;
}


/*****************************************************************************
 函 数 名  : CVI_RECORD_save_buffer
 功能描述  : 写入内存
*****************************************************************************/
static CVI_S32 CVI_RECORD_ClearBuffer(pkg_buffer* pPkgBuff)
{
    if ((NULL == pPkgBuff) || (NULL == pPkgBuff->buff)) {
        _RECORD_ERR("Invalid Input:%p\n", pPkgBuff);
        return RECORD_ERROR;
    }
    _RECORD_INFO("pkgstorage clear buffer: %p, size: %u, %u\n", pPkgBuff, pPkgBuff->buffersize, pPkgBuff->bufferused);
    pPkgBuff->bufferused = 0;
    return 0;
}


/*****************************************************************************
 函 数 名  : CVI_RECORD_MallocBuffer
 功能描述  : 申请内存
*****************************************************************************/
static CVI_S32 CVI_RECORD_MallocReadBuffer(pkg_read_buffer* pPkgReadBuff, CVI_U32 iMaxReadLen)
{
    CVI_RECORD_FUNC_START;
    _RECORD_ERR("malloc Read buf,maxRead:%u\n", iMaxReadLen);
    if (pPkgReadBuff) {
        pPkgReadBuff->iReadBuff = (CVI_U8*)malloc(iMaxReadLen);
        if (NULL == pPkgReadBuff->iReadBuff) {
            _RECORD_ERR("Malloc Err:Read Len:%d\n", iMaxReadLen);
            return RECORD_ERROR;
        }
        memset(pPkgReadBuff->iReadBuff, 0, iMaxReadLen);
        pPkgReadBuff->iStartOffSet = 0;
        pPkgReadBuff->iReadLen = 0;
        pPkgReadBuff->iReadFd = -1;
        pPkgReadBuff->iMaxReadLen = iMaxReadLen;
        return 0;
    }
    _RECORD_ERR("Invalid Input\n");
    CVI_RECORD_FUNC_END;
    return RECORD_ERROR;
}



/*****************************************************************************
 函 数 名  : CVI_RECORD_FreeFrameBuffer
 功能描述  : 释放内存
*****************************************************************************/
static CVI_S32 CVI_RECORD_GetFrameBuffer(pkg_read_buffer* pPkgReadBuff, CVI_S32 iReadFd, CVI_U32 iStartOffset, CVI_U8* iReadBuf, CVI_U32 iReadLen)
{
    CVI_S32 iLen = 0;
    CVI_U32 iEndOffset = 0;
    if ((NULL == pPkgReadBuff) || (NULL == pPkgReadBuff->iReadBuff)) {
        _RECORD_ERR("Invalid Input:%p\n", pPkgReadBuff);
        CVI_RECORD_FUNC_END;
        return RECORD_ERROR;
    }

    if ((NULL == iReadBuf) || (iReadLen <= 0) || (iReadLen > pPkgReadBuff->iMaxReadLen) || (iReadFd <= 0)) {
        _RECORD_ERR("Invalid Input,fd:%d,iReadLen:%u,max:%u\n", iReadFd, iReadLen, pPkgReadBuff->iMaxReadLen);
        CVI_RECORD_FUNC_END;
        return RECORD_ERROR;
    }

    iEndOffset = iStartOffset + iReadLen;

    if ((iReadFd != pPkgReadBuff->iReadFd)
        || (iReadLen > pPkgReadBuff->iReadLen)
        || (iStartOffset < pPkgReadBuff->iStartOffSet)
        || (iEndOffset > pPkgReadBuff->iStartOffSet + pPkgReadBuff->iReadLen)) {
        //BUFF中数据不包含读的数据
        pPkgReadBuff->iReadFd = iReadFd;

        if (iEndOffset > pPkgReadBuff->iMaxReadLen)
            pPkgReadBuff->iStartOffSet = iEndOffset - pPkgReadBuff->iMaxReadLen;
        else
            pPkgReadBuff->iStartOffSet = 0;

        iLen = CVI_RECORD_ReadFileOffset(iReadFd, pPkgReadBuff->iStartOffSet, pPkgReadBuff->iReadBuff, pPkgReadBuff->iMaxReadLen);
        
        //_RECORD_DBG("Read buf fd:%d, read:%u,%u start:%u,max:%u, read %d\n", iReadFd, iStartOffset, iEndOffset,
        //    pPkgReadBuff->iStartOffSet, pPkgReadBuff->iMaxReadLen, iLen);
        if (iLen > 0)
            pPkgReadBuff->iReadLen = iLen;
        else
            pPkgReadBuff->iReadLen = 0;
    }

    if ((iReadFd != pPkgReadBuff->iReadFd)
        || (iReadLen > pPkgReadBuff->iReadLen)
        || (iStartOffset < pPkgReadBuff->iStartOffSet)
        || (iEndOffset > pPkgReadBuff->iStartOffSet + pPkgReadBuff->iReadLen)) {
        _RECORD_ERR("Read err,fd:%d, start:%u,len:%u\n", iReadFd, iStartOffset, pPkgReadBuff->iMaxReadLen);
        CVI_RECORD_FUNC_END;
        return RECORD_ERROR;
    } else {
        memcpy(iReadBuf, pPkgReadBuff->iReadBuff + (iStartOffset-pPkgReadBuff->iStartOffSet), iReadLen);
    }
    return 0;
}



/*****************************************************************************
 函 数 名  : CVI_RECORD_FreeFrameBuffer
 功能描述  : 释放内存
*****************************************************************************/
static void CVI_RECORD_FreeFrameBuffer(pkg_read_buffer* pPkgReadBuff)
{
    if (pPkgReadBuff && pPkgReadBuff->iReadBuff) {
        free(pPkgReadBuff->iReadBuff);
        pPkgReadBuff->iReadBuff = NULL;
        pPkgReadBuff->iStartOffSet = 0;
        pPkgReadBuff->iReadLen = 0;
        pPkgReadBuff->iMaxReadLen = 0;
        pPkgReadBuff->iReadFd = -1;
    }
}

/*****************************************************************************
 函 数 名  : time_to_string
 功能描述  : 帧转时间
*****************************************************************************/
static CVI_CHAR* time_to_string(time_t ctime, CVI_CHAR *astring)
{
    CVI_CHAR acDate[20];
    struct tm local;

    localtime_r(&ctime, &local);
    memset(acDate, 0, sizeof(acDate));
    sprintf(acDate, "%04d%02d%02d-%02d%02d%02d",(1900+local.tm_year), (1+local.tm_mon), local.tm_mday, local.tm_hour,local.tm_min, local.tm_sec);
    strcpy(astring, acDate);
    return astring;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_CheckInTime
 功能描述  : 检测时间是否在范围内
*****************************************************************************/
static CVI_S32 CVI_RECORD_CheckInTime(time_t tBeginTime, time_t tEndTime, time_t maxBeginTime, time_t maxEndTime)
{
    //开始时间在范围内 或者结束时间在范围内都是有效时间
    if (((tBeginTime >= maxBeginTime) && (tBeginTime < maxEndTime))
        || ((tEndTime >= maxBeginTime) && (tBeginTime < maxEndTime)))
        return 1;
    return 0;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_CheckInContinue
 功能描述  : 检测是否连续
*****************************************************************************/
static CVI_S32 CVI_RECORD_CheckInContinue(time_t tNowBeginTime, time_t tNowEndTime, time_t tNextBeginTime, time_t tNextEndTime)
{
    if ((tNowBeginTime <= tNextBeginTime)
            && ((tNowEndTime == tNextBeginTime) || ((tNowEndTime+1) == tNextBeginTime)))
        return 1;
    return 0;
}


/*****************************************************************************
 函 数 名  : CVI_RECORD_CheckInEvent
 功能描述  : 是否符合录像类型
*****************************************************************************/
static CVI_S32 CVI_RECORD_CheckInEvent(CVI_S32 iCheckEven,CVI_S32 iRecordEven)
{
    if (0 == iRecordEven)
    {
        //0代表定时录像类型
        if(iCheckEven & CVI_RECORD_EVEN_NONE)
        {
            return 1;
        }
    }
    else if (0 == iCheckEven)
    {
        //检测类型为0代表所有录像类型
        return 1;
    }
    else if (iRecordEven & iCheckEven)
    {
        //事件有交集代表录像类型包含
        return 1;
    }
    return 0;
}



/*****************************************************************************
 函 数 名  : CVI_RECORD_CheckFrameValid
 功能描述  : 检测帧是否有效
 输入: iFrameType 帧类型， iSubType 帧子类型
 输出: CVI_S32* bKeyFrame 是否关键帧(I帧)
输出: CVI_S32* bIndexFrame 是否索引帧
返回值:0无效,1有效
*****************************************************************************/
static CVI_BOOL CVI_RECORD_CheckFrameValid(CVI_CHAR iFrameType, CVI_CHAR iSubType, CVI_S32* bKeyFrame, CVI_S32* bIndexFrame)
{
    if (bKeyFrame)
        *bKeyFrame = 0;
    if (bIndexFrame)
        *bIndexFrame = 0;
    switch (iFrameType)
    {
        case CVI_RECORD_FRAME_TYPE_H264:
            {
                if (CVI_RECORD_H264_TYPE_I_FRAME == iSubType) {
                    if(bKeyFrame)
                        *bKeyFrame = 1;

                    if(bIndexFrame)
                        *bIndexFrame = 1;
                    return 1;
                } else if(CVI_RECORD_H264_TYPE_P_FRAME == iSubType) {
                    return 1;
                } else {
                    _RECORD_ERR("Invalid H264 SubType:%d\n", iSubType);
                    return 0;
                }
            }
            break;
        case CVI_RECORD_FRAME_TYPE_H265:
            {
                if(CVI_RECORD_H265_TYPE_I_FRAME == iSubType) {
                    if(bKeyFrame)
                        *bKeyFrame = 1;

                    if(bIndexFrame)
                        *bIndexFrame = 1;
                    return 1;
                }  else if(CVI_RECORD_H265_TYPE_P_FRAME == iSubType) {
                    return 1;
                } else if(CVI_RECORD_H265_TYPE_SP_FRAME == iSubType) {
                    if(bIndexFrame)
                        *bIndexFrame = 1;
                    return 1;
                } else {
                    _RECORD_ERR("Invalid H264 SubType:%d\n", iSubType);
                    return 0;
                }
            }
            break;
        case CVI_RECORD_FRAME_TYPE_G711U:
            {
                //音频没有子类型
                return 1;
            }
            break;
        default:
            _RECORD_ERR("Invalid FrameType:%d,SubType:%d\n", iFrameType, iSubType);
            return 0;
    }

    return 0;
}


static CVI_S32 CVI_RECORD_ShowFileIndexHeader(CVI_RECORD_FILE_INDEX_HEADER* pFileIndexHeader)
{
    if (NULL == pFileIndexHeader) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }
    _RECORD_INFO("Show info:iFileStartCode:%x,iModifyTimes:%llu,iVersion:%d,iAVFiles:%d,iCurrFileRecNo:%d,iCrcSum:%d\n",
        pFileIndexHeader->iFileStartCode, pFileIndexHeader->iModifyTimes, pFileIndexHeader->iVersion,
        pFileIndexHeader->iAVFiles, pFileIndexHeader->iCurrFileRecNo, pFileIndexHeader->iCrcSum);
    return 0;
}

static CVI_S32 CVI_RECORD_ShowFileIndex(CVI_RECORD_FILE_INDEX_RECORD* pFileIndexRecord)
{
    if (NULL == pFileIndexRecord) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }
    
    CVI_CHAR sStartTime[20];
    CVI_CHAR sEndTime[20];

    _RECORD_INFO("Show info:iStatus:%u,iSegRecNums:%u,Time:%s-%s,iCrcSum:%d\n",
        pFileIndexRecord->iStatus, pFileIndexRecord->iSegRecNums,
        time_to_string(pFileIndexRecord->tBeginTime, sStartTime),
        time_to_string(pFileIndexRecord->tEndTime, sEndTime),
        pFileIndexRecord->iCrcSum);
    return 0;
}

static CVI_S32 CVI_RECORD_ShowSegmentIndex(CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecord)
{
    if (NULL == pSegIndexRecord) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }
    
    CVI_CHAR sStartTime[20];
    CVI_CHAR sEndTime[20];

    _RECORD_INFO("Show info:iEvenType:%d,iStatus:%u,Time:%s-%s,iFrameStartOffset:%u-%u,iSegmentNo:%u,iInfoCount:%u,iInfoEndOffset:%u,iCrcSum:%d,remain:%u\n",
        pSegIndexRecord->iEvenType, pSegIndexRecord->iStatus,
        time_to_string(pSegIndexRecord->tBeginTime, sStartTime),
        time_to_string(pSegIndexRecord->tEndTime, sEndTime),
        pSegIndexRecord->iFrameStartOffset, pSegIndexRecord->iFrameEndOffset, pSegIndexRecord->iSegmentNo,
        pSegIndexRecord->iInfoCount, pSegIndexRecord->iInfoEndOffset, pSegIndexRecord->iCrcSum,
        ((CVI_RECORD_FRAME_AVINDEX_GET_START_OFFSET(pSegIndexRecord) - pSegIndexRecord->iFrameEndOffset)/CVI_RECORD_CHAR_TO_MB));
    return 0;
}

static CVI_S32 CVI_RECORD_ShowFramInfo(CVI_RECORD_SEGMENT_INFO* pFrameInfo)
{
    if (NULL == pFrameInfo) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }
    if (CVI_RECORD_INFO_TYPE_HEADER == pFrameInfo->iInfoType) {
        _RECORD_INFO("header frame info:code:%#x,type:%u(even:%#x,sta:%u,ExNums:%u,IndexNums:%u,SegmentNo:%u)iCrcSum:%d\n",
            pFrameInfo->iSegmentStartCode, pFrameInfo->iInfoType, pFrameInfo->iHeaderInfo.iEvenType,
            pFrameInfo->iHeaderInfo.iStatus, pFrameInfo->iHeaderInfo.iInfoExNums, pFrameInfo->iHeaderInfo.iIndexFrameNums,
            pFrameInfo->iHeaderInfo.iSegmentNo, pFrameInfo->iCrcSum);
    } else if(CVI_RECORD_INFO_TYPE_KEY_INDEX == pFrameInfo->iInfoType) {
        _RECORD_INFO("index frame info:code:%#x,type:%u,index[%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u],iCrcSum:%d\n",
            pFrameInfo->iSegmentStartCode, pFrameInfo->iInfoType, pFrameInfo->iIndexInfo.iIndexFrame[0], pFrameInfo->iIndexInfo.iIndexFrame[1],
            pFrameInfo->iIndexInfo.iIndexFrame[2], pFrameInfo->iIndexInfo.iIndexFrame[3], pFrameInfo->iIndexInfo.iIndexFrame[4],
            pFrameInfo->iIndexInfo.iIndexFrame[5], pFrameInfo->iIndexInfo.iIndexFrame[6], pFrameInfo->iIndexInfo.iIndexFrame[7],
            pFrameInfo->iIndexInfo.iIndexFrame[8], pFrameInfo->iIndexInfo.iIndexFrame[9], pFrameInfo->iIndexInfo.iIndexFrame[10],
            pFrameInfo->iIndexInfo.iIndexFrame[11], pFrameInfo->iIndexInfo.iIndexFrame[12], pFrameInfo->iIndexInfo.iIndexFrame[13],
            pFrameInfo->iIndexInfo.iIndexFrame[14], pFrameInfo->iIndexInfo.iIndexFrame[15], pFrameInfo->iCrcSum);
    } else if(CVI_RECORD_INFO_TYPE_FRAME == pFrameInfo->iInfoType) {
            CVI_CHAR sStartTime[20];
            _RECORD_INFO("data frame info:code:%#x,type:%u,frame(type:%u.%u,No:%u,offset:%u,len:%u,time:%llu,%s),iCrcSum:%d, %d\n",
                pFrameInfo->iSegmentStartCode, pFrameInfo->iInfoType, pFrameInfo->iFrameInfo.iFrameType, 
                pFrameInfo->iFrameInfo.iSubType, pFrameInfo->iFrameInfo.iFrameNo, pFrameInfo->iFrameInfo.iStartOffset,
                pFrameInfo->iFrameInfo.iFrameLen, pFrameInfo->iFrameInfo.iFrame_absTime, time_to_string(pFrameInfo->iFrameInfo.iShowTime, sStartTime),
                pFrameInfo->iCrcSum, CVI_RECORD_GetCrc((const CVI_CHAR*)pFrameInfo, sizeof(CVI_RECORD_SEGMENT_INFO) - sizeof(pFrameInfo->iCrcSum)));
    } else {
        _RECORD_ERR("Invalid Frame Info type:%d\n", pFrameInfo->iInfoType);
    }
    return 0;
}

static CVI_S32 CVI_RECORD_GetInitFileIndexHeader(CVI_RECORD_FILE_INDEX_HEADER* pFileIndexHeader, CVI_S32 avFiles)
{
    if (NULL == pFileIndexHeader) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }

    memset(pFileIndexHeader, 0, sizeof(CVI_RECORD_FILE_INDEX_HEADER));
    pFileIndexHeader->iFileStartCode = CVI_RECORD_FILE_STARTCODE;
    pFileIndexHeader->iVersion = CVI_RECORD_VERSION;
    pFileIndexHeader->iAVFiles = avFiles;
    pFileIndexHeader->iCrcSum = CVI_RECORD_GetCrc((const CVI_CHAR*)pFileIndexHeader, sizeof(CVI_RECORD_FILE_INDEX_HEADER) - sizeof(pFileIndexHeader->iCrcSum));

    return 0;
}

static CVI_S32 CVI_RECORD_GetInitFileIndex(CVI_RECORD_FILE_INDEX_RECORD* pFileIndexRecord, CVI_CHAR iStatus)
{
    if (NULL == pFileIndexRecord) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }

    memset(pFileIndexRecord, 0, sizeof(CVI_RECORD_FILE_INDEX_RECORD));
    pFileIndexRecord->iStatus = iStatus;
    pFileIndexRecord->iCrcSum = CVI_RECORD_GetCrc((const CVI_CHAR*)pFileIndexRecord, sizeof(CVI_RECORD_FILE_INDEX_RECORD) - sizeof(pFileIndexRecord->iCrcSum));
    return 0;
}

static CVI_S32 CVI_RECORD_GetInitSegmentIndex(CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecord, CVI_CHAR iStatus)
{
    if (NULL == pSegIndexRecord) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }
    
    memset(pSegIndexRecord, 0, sizeof(CVI_RECORD_SEGMENT_INDEX_RECORD));
    pSegIndexRecord->iStatus = iStatus;
    pSegIndexRecord->iCrcSum = CVI_RECORD_GetCrc((const CVI_CHAR*)pSegIndexRecord, sizeof(CVI_RECORD_SEGMENT_INDEX_RECORD) - sizeof(pSegIndexRecord->iCrcSum));
    return 0;
}

static CVI_S32 CVI_RECORD_GetInitSegmentFrameInfo(CVI_RECORD_SEGMENT_INFO* pSegFrameInfo)
{
    if (NULL == pSegFrameInfo) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }

    memset(pSegFrameInfo, 0, sizeof(CVI_RECORD_SEGMENT_INFO));
    pSegFrameInfo->iSegmentStartCode = CVI_RECORD_SEGMENT_STARTCODE;
    pSegFrameInfo->iInfoType = CVI_RECORD_INFO_TYPE_INIT;
    pSegFrameInfo->iCrcSum = CVI_RECORD_GetCrc((const CVI_CHAR*)pSegFrameInfo, sizeof(CVI_RECORD_SEGMENT_INFO) - sizeof(pSegFrameInfo->iCrcSum));
    return 0;
}

static CVI_RECORD_FILE_INDEX_HEADER* CVI_RECORD_GetFileIndexHeader(pkg_buffer* pPkgBuff)
{
    if ((NULL == pPkgBuff) || (NULL == pPkgBuff->buff) || (pPkgBuff->bufferused < PKG_FILE_INDEX_MAX_LEN)) {
        _RECORD_ERR("Invalid Input\n");
        return NULL;
    }
    return (CVI_RECORD_FILE_INDEX_HEADER*)(pPkgBuff->buff + CVI_RECORD_FILE_INDEX_HEADER_SEEK);
}

static CVI_RECORD_FILE_INDEX_RECORD* CVI_RECORD_GetFileIndexRecord(pkg_buffer* pPkgBuff, CVI_U32 iFileIndex)
{
    if ((NULL == pPkgBuff) || (NULL == pPkgBuff->buff) || (pPkgBuff->bufferused < PKG_FILE_INDEX_MAX_LEN)) {
        _RECORD_ERR("Invalid Input\n");
        return NULL;
    }
    return (CVI_RECORD_FILE_INDEX_RECORD*)(pPkgBuff->buff + CVI_RECORD_FILE_INDEX_RECORD_SEEK(iFileIndex));
}

static CVI_RECORD_SEGMENT_INDEX_RECORD* CVI_RECORD_GetSegmentIndexRecord(pkg_buffer* pPkgBuff, CVI_U32 iSegIndex)
{
    if ((NULL == pPkgBuff) || (NULL == pPkgBuff->buff) || (pPkgBuff->bufferused < PKG_FILE_INDEX_MAX_LEN)) {
        _RECORD_ERR("Invalid Input\n");
        return NULL;
    }
    return (CVI_RECORD_SEGMENT_INDEX_RECORD*)(pPkgBuff->buff + CVI_RECORD_SEGMENT_INDEX_RECORD_SEEK(0, iSegIndex));
}

static CVI_S32 CVI_RECORD_GetSegmentFormFileIndex(pkg_buffer* pPkgBuff, CVI_S32 fd, CVI_U32 iFileIndex)
{
    CVI_S32 iRead = 0;
    CVI_U32 iSeek = 0;
    CVI_S32 i = 0;
    CVI_S32 iReadLen = 0;
    CVI_U8* pReadbuff = NULL;
    CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecord = NULL;
    if ((NULL == pPkgBuff) || (NULL == pPkgBuff->buff) || (pPkgBuff->bufferused < PKG_FILE_INDEX_MAX_LEN) || (fd <= 0)) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }

    iSeek = CVI_RECORD_SEGMENT_INDEX_RECORD_SEEK(iFileIndex, 0);
    if (-1 == lseek(fd, iSeek, SEEK_SET)) {
        _RECORD_ERR("seek error cannot seek to %u\n", iSeek);
        return ERR_INVALID_ARGUMENT;
    }
    iReadLen = PKG_SEGMENT_INDEX_MAX_LEN;
    pReadbuff = pPkgBuff->buff + PKG_HEADER_INDEX_MAX_LEN + PKG_RECORD_INDEX_MAX_LEN;
    iRead = read(fd, pReadbuff, iReadLen);
    if(iRead != iReadLen) {
        _RECORD_ERR("read error %u != %u\n", iRead, iReadLen);
        return ERR_INVALID_ARGUMENT;
    }

    s_iCacheRWTotal += iRead;

    for (i = 0; i < CVI_RECORD_INDEX_MAX_AVFILES; i++) {
        pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(pPkgBuff, i);
        if((NULL == pSegIndexRecord)
            || (pSegIndexRecord->iCrcSum != CVI_RECORD_GetCrc((const CVI_CHAR*)pSegIndexRecord, sizeof(CVI_RECORD_SEGMENT_INDEX_RECORD) - sizeof(pSegIndexRecord->iCrcSum))))
        {
            return ERR_BUF_CHECK_ERROR;
        }
    }
    
    return 0;
}


static CVI_S32 CVI_RECORD_GetFileIndex(pkg_buffer* pPkgBuff, CVI_S32 fd, CVI_U32 iFileIndex)
{
    CVI_S32 iRead = 0;
    CVI_U32 iSeek = 0;
    CVI_S32 i = 0;
    CVI_S32 iReadLen = 0;
    CVI_U8* pReadbuff = NULL;
    CVI_RECORD_FILE_INDEX_HEADER* pFileIndexHeader = NULL;
    CVI_RECORD_FILE_INDEX_RECORD* pFileIndexRecord = NULL;
    CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecord = NULL;
    if ((NULL == pPkgBuff) || (NULL == pPkgBuff->buff) || (pPkgBuff->buffersize < PKG_FILE_INDEX_MAX_LEN) || (fd <= 0)) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }

    iSeek = CVI_RECORD_FILE_INDEX_HEADER_SEEK;
    if (-1 == lseek(fd, iSeek, SEEK_SET)) {
        _RECORD_ERR("seek error cannot seek to %u\n", iSeek);
        return ERR_INVALID_ARGUMENT;
    }
    iReadLen = PKG_HEADER_INDEX_MAX_LEN + PKG_RECORD_INDEX_MAX_LEN;
    pReadbuff = pPkgBuff->buff;
    iRead = read(fd, pReadbuff, iReadLen);

    if(iRead != iReadLen) {
        _RECORD_ERR("read error %u != %u\n", iRead, iReadLen);
        return ERR_INVALID_ARGUMENT;
    }
    s_iCacheRWTotal += iRead;

    iSeek = CVI_RECORD_SEGMENT_INDEX_RECORD_SEEK(iFileIndex, 0);
    if (-1 == lseek(fd, iSeek, SEEK_SET)) {
        _RECORD_ERR("seek error cannot seek to %u\n", iSeek);
        return ERR_INVALID_ARGUMENT;
    }
    iReadLen = PKG_SEGMENT_INDEX_MAX_LEN;
    pReadbuff = pPkgBuff->buff + PKG_HEADER_INDEX_MAX_LEN + PKG_RECORD_INDEX_MAX_LEN;
    iRead = read(fd, pReadbuff, iReadLen);

    if(iRead != iReadLen) {
        _RECORD_ERR("read error %u != %u\n", iRead, iReadLen);
        return ERR_INVALID_ARGUMENT;
    }
    s_iCacheRWTotal += iRead;

    pPkgBuff->bufferused = PKG_FILE_INDEX_MAX_LEN;
    pFileIndexHeader = CVI_RECORD_GetFileIndexHeader(pPkgBuff);
    if((NULL == pFileIndexHeader)
        || (CVI_RECORD_FILE_STARTCODE != pFileIndexHeader->iFileStartCode)
        || (pFileIndexHeader->iCrcSum != CVI_RECORD_GetCrc((const CVI_CHAR*)pFileIndexHeader, sizeof(CVI_RECORD_FILE_INDEX_HEADER) - sizeof(pFileIndexHeader->iCrcSum)))) {
        _RECORD_ERR("check header error\n");
        pPkgBuff->bufferused = 0;
        return ERR_BUF_CHECK_ERROR;
    }

    for (i = 0; i < CVI_RECORD_INDEX_MAX_AVFILES; i++) {
        pFileIndexRecord = CVI_RECORD_GetFileIndexRecord(pPkgBuff, i);
        if((NULL == pFileIndexRecord)
            || (pFileIndexRecord->iCrcSum != CVI_RECORD_GetCrc((const CVI_CHAR*)pFileIndexRecord, sizeof(CVI_RECORD_FILE_INDEX_RECORD) - sizeof(pFileIndexRecord->iCrcSum)))) {
            _RECORD_ERR("check record error, %d\n", i);
            pPkgBuff->bufferused = 0;
            return ERR_BUF_CHECK_ERROR;
        }
    }

    for (i = 0; i < CVI_RECORD_AV_FILE_MAX_SEGMENT; i++) {
        pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(pPkgBuff, i);
        if((NULL == pSegIndexRecord)
            || (pSegIndexRecord->iCrcSum != CVI_RECORD_GetCrc((const CVI_CHAR*)pSegIndexRecord, sizeof(CVI_RECORD_SEGMENT_INDEX_RECORD) - sizeof(pSegIndexRecord->iCrcSum)))) {
            _RECORD_ERR("check segment error, %d\n", i);
            pPkgBuff->bufferused = 0;
            return ERR_BUF_CHECK_ERROR;
        }
    }
    
    return 0;
}

static CVI_S32 CVI_RECORD_ReadFileIndexHeader(CVI_RECORD_FILE_INDEX_HEADER* pFileIndexHeader, CVI_S32 fd)
{
    CVI_S32 iRead = 0;
    CVI_U32 iSeek = 0;
    if((NULL == pFileIndexHeader) || (fd <= 0)) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }

    iSeek = CVI_RECORD_FILE_INDEX_HEADER_SEEK;
    if (-1 == lseek(fd, iSeek, SEEK_SET)) {
        _RECORD_ERR("seek error cannot seek to %u\n", iSeek);
        return ERR_INVALID_ARGUMENT;
    }
    
    iRead = read(fd, pFileIndexHeader, sizeof(CVI_RECORD_FILE_INDEX_HEADER));
    if(iRead != sizeof(CVI_RECORD_FILE_INDEX_HEADER)) {
        _RECORD_ERR("read error %u != %u\n", iRead, sizeof(CVI_RECORD_FILE_INDEX_HEADER));
        return ERR_INVALID_ARGUMENT;
    }
    
    s_iCacheRWTotal += iRead;

    if((CVI_RECORD_FILE_STARTCODE == pFileIndexHeader->iFileStartCode)
        && (pFileIndexHeader->iCrcSum == CVI_RECORD_GetCrc((const CVI_CHAR*)pFileIndexHeader, sizeof(CVI_RECORD_FILE_INDEX_HEADER) - sizeof(pFileIndexHeader->iCrcSum))))
        return 0;

    return RECORD_ERROR;
}

static CVI_S32 CVI_RECORD_ReadFileIndex(CVI_RECORD_FILE_INDEX_RECORD* pFileIndexRecord, CVI_S32 fd, CVI_U32 iFileIndex)
{
    CVI_S32 iRead = 0;
    CVI_U32 iSeek = 0;
    if ((NULL == pFileIndexRecord) || (fd <= 0)) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }

    iSeek = CVI_RECORD_FILE_INDEX_RECORD_SEEK(iFileIndex);
    if (-1 == lseek(fd, iSeek, SEEK_SET)) {
        _RECORD_ERR("seek error cannot seek to %u\n", iSeek);
        return ERR_INVALID_ARGUMENT;
    }

    iRead = read(fd, pFileIndexRecord, sizeof(CVI_RECORD_FILE_INDEX_RECORD));
    if (iRead != sizeof(CVI_RECORD_FILE_INDEX_RECORD)) {
        _RECORD_ERR("read error %u != %u\n", iRead, sizeof(CVI_RECORD_FILE_INDEX_RECORD));
        return ERR_INVALID_ARGUMENT;
    }
    s_iCacheRWTotal += iRead;

    if (pFileIndexRecord->iCrcSum == CVI_RECORD_GetCrc((const CVI_CHAR*)pFileIndexRecord, sizeof(CVI_RECORD_FILE_INDEX_RECORD) - sizeof(pFileIndexRecord->iCrcSum)))
        return 0;

    return RECORD_ERROR;
}

static CVI_S32 CVI_RECORD_ReadSegmentFrameInfo(CVI_S32 fd, CVI_U32 iOffset, CVI_RECORD_SEGMENT_INFO* pSegFrameInfo, CVI_S32 iSegNum, pkg_read_buffer* pPkgReadBuff)
{
    CVI_S32 iRet = 0;
    CVI_S32 i = 0;
    CVI_S32 iRead = 0;
    CVI_U32 iSeek = 0;
    CVI_RECORD_SEGMENT_INFO* pSegFrameInfoTmp = NULL;
    if ((NULL == pSegFrameInfo) || (fd <= 0) || (iSegNum <= 0)) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }
    iSeek = iOffset;
    if (pPkgReadBuff) {
        if  (0 != CVI_RECORD_GetFrameBuffer(pPkgReadBuff, fd, iSeek, (CVI_U8 *)pSegFrameInfo, iSegNum*sizeof(CVI_RECORD_SEGMENT_INFO))) {
            _RECORD_ERR("get frame buffer err\n");
            return ERR_INVALID_ARGUMENT;
        }
    } else {
        if (-1 == lseek(fd, iSeek, SEEK_SET)) {
            _RECORD_ERR("seek error cannot seek to %u\n", iSeek);
            return ERR_INVALID_ARGUMENT;
        }
        
        iRead = read(fd, pSegFrameInfo, iSegNum*sizeof(CVI_RECORD_SEGMENT_INFO));
        if (iRead != (CVI_S32)(iSegNum*sizeof(CVI_RECORD_SEGMENT_INFO)))
        {
            _RECORD_ERR("read error %u != %u\n", iRead, sizeof(CVI_RECORD_SEGMENT_INFO));
            return ERR_INVALID_ARGUMENT;
        }
        s_iCacheRWTotal += iRead;
    }

    pSegFrameInfoTmp = pSegFrameInfo;
    for (i = 0; i < iSegNum; i++) {
        if (((CVI_RECORD_SEGMENT_STARTCODE != pSegFrameInfoTmp->iSegmentStartCode))
            || (pSegFrameInfoTmp->iCrcSum != CVI_RECORD_GetCrc((const CVI_CHAR*)pSegFrameInfoTmp, sizeof(CVI_RECORD_SEGMENT_INFO) - sizeof(pSegFrameInfoTmp->iCrcSum)))) {
            _RECORD_ERR("check Fame err crc:%d,%d\n", pSegFrameInfoTmp->iCrcSum,
                CVI_RECORD_GetCrc((const CVI_CHAR*)pSegFrameInfoTmp, sizeof(CVI_RECORD_SEGMENT_INFO) - sizeof(pSegFrameInfoTmp->iCrcSum)));
            CVI_RECORD_ShowFramInfo(pSegFrameInfoTmp);
            iRet = ERR_BUF_CHECK_ERROR;
        }
        pSegFrameInfoTmp++;
    }

    return iRet;
}


static CVI_S32 CVI_RECORD_WriteSegmentFrameInfo(CVI_S32 fd, CVI_U32 iOffset, CVI_RECORD_SEGMENT_INFO* pSegFrameInfo, CVI_S32 iSegHeaderInfo)
{
    CVI_RECORD_SEGMENT_INFO* pSegFrameInfoTmp = NULL;
    CVI_S32 nWriteLen = 0;
    CVI_U32 iSeek = 0;
    CVI_S32 i = 0;
    if ((NULL == pSegFrameInfo) || (fd <= 0) || (iSegHeaderInfo <= 0)) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }

    iSeek = iOffset;
    if (-1 == lseek(fd, iSeek, SEEK_SET)) {
        _RECORD_ERR("seek error cannot seek to %u\n", iSeek);
        return ERR_INVALID_ARGUMENT;
    }
    pSegFrameInfoTmp = pSegFrameInfo;
    _RECORD_DBG("write frame info offset %u,num:%d, len:%d\n", iOffset, iSegHeaderInfo, iSegHeaderInfo * sizeof(CVI_RECORD_SEGMENT_INFO));
    for (i = 0; i < iSegHeaderInfo; i++) {
        pSegFrameInfoTmp->iCrcSum = CVI_RECORD_GetCrc((const CVI_CHAR*)pSegFrameInfoTmp, sizeof(CVI_RECORD_SEGMENT_INFO) - sizeof(pSegFrameInfoTmp->iCrcSum));
        if (CVI_RECORD_FRAME_HEADER_INFO_MAX == iSegHeaderInfo)
        {
            CVI_RECORD_ShowFramInfo(pSegFrameInfoTmp);
        }
        pSegFrameInfoTmp++;
    }

    nWriteLen = write(fd, pSegFrameInfo, iSegHeaderInfo * sizeof(CVI_RECORD_SEGMENT_INFO));
    if (nWriteLen != (CVI_S32)(iSegHeaderInfo * sizeof(CVI_RECORD_SEGMENT_INFO))) {
        _RECORD_ERR("read error %u != %u\n", nWriteLen, iSegHeaderInfo * sizeof(CVI_RECORD_SEGMENT_INFO));
        return ERR_INVALID_ARGUMENT;
    }
    s_iCacheRWTotal += nWriteLen;

#ifdef CVI_RECORD_WRITE_INT
    usleep(CVI_RECORD_WRITE_INT);
#endif

    return 0;
}


static CVI_S32 CVI_RECORD_WriteFileIndexHeader(CVI_RECORD_FILE_INDEX_HEADER* pFileIndexHeader, CVI_S32 fd)
{
    CVI_S32 nWriteLen = 0;
    CVI_U32 iSeek = 0;
    if ((NULL == pFileIndexHeader) || (fd <= 0)) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }

    iSeek = CVI_RECORD_FILE_INDEX_HEADER_SEEK;
    if (-1 == lseek(fd, iSeek, SEEK_SET)) {
        _RECORD_ERR("seek error cannot seek to %u\n", iSeek);
        return ERR_INVALID_ARGUMENT;
    }
    pFileIndexHeader->iCrcSum = CVI_RECORD_GetCrc((const CVI_CHAR*)pFileIndexHeader, sizeof(CVI_RECORD_FILE_INDEX_HEADER) - sizeof(pFileIndexHeader->iCrcSum));

    nWriteLen = write(fd, pFileIndexHeader, sizeof(CVI_RECORD_FILE_INDEX_HEADER));
    if (nWriteLen != sizeof(CVI_RECORD_FILE_INDEX_HEADER)) {
        _RECORD_ERR("read error %u != %u\n", nWriteLen, sizeof(CVI_RECORD_FILE_INDEX_HEADER));
        return ERR_INVALID_ARGUMENT;
    }
    s_iCacheRWTotal += nWriteLen;

#ifdef CVI_RECORD_WRITE_INT
    usleep(CVI_RECORD_WRITE_INT);
#endif

    return RECORD_OK;
}

static CVI_S32 CVI_RECORD_WriteFileIndex(CVI_RECORD_FILE_INDEX_RECORD* pFileIndexRecord, CVI_S32 fd, CVI_U32 iFileIndex, CVI_S32 iWriteFiles)
{
    CVI_S32 nWriteLen = 0;
    CVI_U32 iSeek = 0;
    CVI_S32 i = 0;
    CVI_RECORD_FILE_INDEX_RECORD* pFileIndexRecordTmp = NULL;
    if ((NULL == pFileIndexRecord) || (iWriteFiles <= 0) || (fd <= 0)) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }

    iSeek = CVI_RECORD_FILE_INDEX_RECORD_SEEK(iFileIndex);
    if (-1 == lseek(fd, iSeek, SEEK_SET)) {
        _RECORD_ERR("seek error cannot seek to %u\n", iSeek);
        return ERR_INVALID_ARGUMENT;
    }

    pFileIndexRecordTmp = pFileIndexRecord;
    for (i = 0; i < iWriteFiles; i++) {
        pFileIndexRecordTmp->iCrcSum = CVI_RECORD_GetCrc((const CVI_CHAR*)pFileIndexRecordTmp, sizeof(CVI_RECORD_FILE_INDEX_RECORD) - sizeof(pFileIndexRecordTmp->iCrcSum));
        pFileIndexRecordTmp++;
    }

    nWriteLen = write(fd, pFileIndexRecord, iWriteFiles*sizeof(CVI_RECORD_FILE_INDEX_RECORD));

    if (nWriteLen != (CVI_S32)(iWriteFiles*sizeof(CVI_RECORD_FILE_INDEX_RECORD)))
    {
        _RECORD_ERR("read error %u != %u\n", nWriteLen, iWriteFiles*sizeof(CVI_RECORD_FILE_INDEX_RECORD));
        return ERR_INVALID_ARGUMENT;
    }
    s_iCacheRWTotal += nWriteLen;

#ifdef CVI_RECORD_WRITE_INT
    usleep(CVI_RECORD_WRITE_INT);
#endif
    return RECORD_OK;
}

static CVI_S32 CVI_RECORD_WriteSegmentIndex(CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecord, CVI_S32 fd, CVI_U32 iFileIndex, CVI_U32 iSegIndex, CVI_S32 iWriteSegNum)
{
    CVI_S32 i = 0;
    CVI_S32 nWriteLen = 0;
    CVI_U32 iSeek = 0;
    CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecordTmp = NULL;
    
    if((NULL == pSegIndexRecord) || (iWriteSegNum <= 0) || (fd <= 0) || (iSegIndex >= CVI_RECORD_AV_FILE_MAX_SEGMENT)) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }
    
    iSeek = CVI_RECORD_SEGMENT_INDEX_RECORD_SEEK(iFileIndex, iSegIndex);
    if (-1 == lseek(fd, iSeek, SEEK_SET)) {
        _RECORD_ERR("seek error cannot seek to %u\n", iSeek);
        return ERR_INVALID_ARGUMENT;
    }
    pSegIndexRecordTmp = pSegIndexRecord;
    for (i = 0; i < iWriteSegNum; i++) {
        pSegIndexRecordTmp->iCrcSum = CVI_RECORD_GetCrc((const CVI_CHAR*)pSegIndexRecordTmp, sizeof(CVI_RECORD_SEGMENT_INDEX_RECORD) - sizeof(pSegIndexRecordTmp->iCrcSum));
        pSegIndexRecordTmp++;
    }

    nWriteLen = write(fd, pSegIndexRecord, iWriteSegNum*sizeof(CVI_RECORD_SEGMENT_INDEX_RECORD));

    if(nWriteLen != (CVI_S32)(iWriteSegNum*sizeof(CVI_RECORD_SEGMENT_INDEX_RECORD))) {
        _RECORD_ERR("read error %u != %u\n", nWriteLen, iWriteSegNum*sizeof(CVI_RECORD_SEGMENT_INDEX_RECORD));
        return ERR_INVALID_ARGUMENT;
    }

    s_iCacheRWTotal += nWriteLen;
    return RECORD_OK;
}


static CVI_S32 CVI_RECORD_GetSegmentIndex(pkg_buffer *pPkgBuff, CVI_U32 iFileIndex)
{
    CVI_S32 iRet = 0;
    CVI_S32 iIndexfd = -1;
    CVI_S32 index = 0;
    if (iFileIndex >= (CVI_U32)s_mPkgParam.iMaxAVFileNum) {
        _RECORD_ERR("Invalid Input:NULL,iFileIndex:%d\n", iFileIndex);
        return ERR_INVALID_ARGUMENT;
    }

    index = CVI_RECORD_FILES_GET_INDEX_NUM(iFileIndex);
    iIndexfd = CVI_RECORD_GetIndexFileFd(s_mPkgParam.iPkgPathName, index, O_RDWR, NULL);
    iRet = CVI_RECORD_GetFileIndex(pPkgBuff, iIndexfd, iFileIndex); 
    if(0 != iRet)
        _RECORD_ERR("CVI_RECORD_GetFileIndex err\n");
    if(iIndexfd > 0) {
        close(iIndexfd);
        iIndexfd = -1;
    }

    return iRet;

}


static CVI_S32 CVI_RECORD_RepairIndex(CVI_S32* pCurrFileRecNo)
{
    CVI_RECORD_FUNC_START;
    CVI_S32 iRet = 0;
    CVI_S32 iMaxAVFileNum = 0;
    CVI_S32 iMaxIndexNum = 0;
    CVI_S32 iCurrFileRecNo = 0;
    CVI_S32 iRecNo = 0;
    CVI_S32 index = 0;
    CVI_S32 iIndexFd = -1;
    CVI_S32 iFilefd = -1;
    CVI_S32 i = 0;
    CVI_S32 iRecNoSeg = 0;
    CVI_U32 iReadOffset = 0;
    CVI_U32 iMaxSegNo = 0;
    CVI_RECORD_FILE_INDEX_HEADER* pFileIndexHeader = NULL;
    CVI_RECORD_FILE_INDEX_RECORD* pFileIndexRecord = NULL;
    CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecord = NULL;
    CVI_RECORD_SEGMENT_INFO stSegFrameInfo;
    CVI_RECORD_SEGMENT_INFO stSegFrameInfoHeader;
    pkg_read_buffer pPkgReadBuff;
    CVI_U8 *iRemainIndexBuf = NULL;
    
    CVI_CHAR stPathName[CVI_RECORD_MAX_PATH_NAME];
    
    iMaxAVFileNum = CVI_RECORD_GetAvFilesFromCard(s_mPkgParam.iPkgPathName);
    
    //索引文件数重新计算可能刚好没有视频文件了
    iMaxIndexNum = CVI_RECORD_FILES_GET_INDEX_NUM(iMaxAVFileNum);
    if (CVI_RECORD_FILES_GET_INDEX_FILES(iMaxAVFileNum) > 0)
        iMaxIndexNum++;

    iRemainIndexBuf = malloc(CVI_RECORD_REMAIN_INDEX_SIZE);
    if (NULL == iRemainIndexBuf) {
        _RECORD_ERR("pkg init repair error, malloc err %d\n", CVI_RECORD_REMAIN_INDEX_SIZE);
        CVI_RECORD_FUNC_END;
        return RECORD_ERROR;
    }
    memset(iRemainIndexBuf, 0, CVI_RECORD_REMAIN_INDEX_SIZE);

    if(0 != CVI_RECORD_MallocReadBuffer(&pPkgReadBuff, CVI_RECORD_FRAME_DATA_INFO_READ_MAX_REPAIR_SIZE)) {
        if(iRemainIndexBuf) {
            free(iRemainIndexBuf);
            iRemainIndexBuf = NULL;
        }
        _RECORD_ERR("pkg init repair error, malloc err\n");
        CVI_RECORD_FUNC_END;
        return RECORD_ERROR;
    }

    _RECORD_INFO("pkg repair Now AVFiles:%d iMaxIndexFiles:%d\n", iMaxAVFileNum, iMaxIndexNum);
    iRecNo = 0;
    memset(s_mPkgParam.iPkgCurFileIndexBuff.buff, 0, PKG_FILE_INDEX_MAX_LEN);
    s_mPkgParam.iPkgCurFileIndexBuff.bufferused = PKG_FILE_INDEX_MAX_LEN;
    pFileIndexHeader = CVI_RECORD_GetFileIndexHeader(&s_mPkgParam.iPkgCurFileIndexBuff);
    CVI_RECORD_GetInitFileIndexHeader(pFileIndexHeader, iMaxAVFileNum);

    for (index = 0; index < iMaxIndexNum; index++) {
        if (iIndexFd > 0) {
            close(iIndexFd);
            iIndexFd = -1;
        }
        
        iIndexFd = CVI_RECORD_GetIndexFileFd(s_mPkgParam.iPkgPathName, index, O_RDWR|O_CREAT|O_SYNC|O_TRUNC, NULL);
        if (iIndexFd <= 0) {
            _RECORD_ERR("pkg init repair error, open err file:%s\n", stPathName);
            iRet = RECORD_ERROR;
            goto endFunc;
        }
        
        for (i = 0; i < CVI_RECORD_INDEX_MAX_AVFILES; i++) {
            pFileIndexRecord = CVI_RECORD_GetFileIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, i);
            CVI_RECORD_GetInitFileIndex(pFileIndexRecord, CVI_RECORD_STATUS_INIT);
        }

        for (i = 0; (i < CVI_RECORD_INDEX_MAX_AVFILES) && (iRecNo < iMaxAVFileNum); i++,iRecNo++) {
            CVI_S32 bBadFile = 1;
            
            iReadOffset = CVI_RECORD_AV_FILE_MAX_SIZE;
            
            if(iFilefd > 0) {
                close(iFilefd);
                iFilefd = -1;
            }
            
            iFilefd = CVI_RECORD_GetAvFilesFd(s_mPkgParam.iPkgPathName, iRecNo, O_RDWR|O_SYNC, NULL);

            for(iRecNoSeg = 0; iRecNoSeg < CVI_RECORD_AV_FILE_MAX_SEGMENT; iRecNoSeg++) {
                pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, iRecNoSeg);
                CVI_RECORD_GetInitSegmentIndex(pSegIndexRecord, CVI_RECORD_STATUS_INIT);
            }
            pFileIndexRecord = CVI_RECORD_GetFileIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, iRecNo);
            CVI_RECORD_GetInitFileIndex(pFileIndexRecord, CVI_RECORD_STATUS_INIT);
            iRecNoSeg = 0;
            _RECORD_DBG("####[Index Repair]check file:%d\n", iRecNo);
            while (1) {
                iReadOffset -= sizeof(CVI_RECORD_SEGMENT_INFO);
                if (0 != CVI_RECORD_ReadSegmentFrameInfo(iFilefd, iReadOffset, &stSegFrameInfo, 1, &pPkgReadBuff)) {
                    //读文件数据异常
                    if (bBadFile) {
                        //第一个数据就读失败，认为文件损坏坏道文件
                        CVI_RECORD_GetInitFileIndex(pFileIndexRecord, CVI_RECORD_STATUS_BAD_FILE);
                    }
                    _RECORD_DBG("####[Index Repair]Invalid frame\n");
                    break;
                } else {
                    //有数据获取成功这不是坏道文件
                    bBadFile = 0;
                    pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, iRecNoSeg);
                    if (CVI_RECORD_INFO_TYPE_HEADER == stSegFrameInfo.iInfoType) {
                        //读扩展帧失败,文件读结束
                        memcpy(&stSegFrameInfoHeader, &stSegFrameInfo, sizeof(stSegFrameInfoHeader));
                        if (pSegIndexRecord->iInfoCount > stSegFrameInfoHeader.iHeaderInfo.iInfoExNums) {
                            iRecNoSeg++;
                            if(iRecNoSeg >= CVI_RECORD_AV_FILE_MAX_SEGMENT) {
                                _RECORD_DBG("####[Index Repair]Invalid frame type3 %d, num:%d, index:%d,no:%d,seg:%d\n", stSegFrameInfo.iInfoType, pSegIndexRecord->iInfoCount, index, iRecNo, iRecNoSeg);
                                break;
                            }
                        }
                        pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, iRecNoSeg);
                        //head是第一帧
                        pSegIndexRecord->iEvenType = stSegFrameInfoHeader.iHeaderInfo.iEvenType;
                        pSegIndexRecord->iStatus = stSegFrameInfoHeader.iHeaderInfo.iStatus;
                        pSegIndexRecord->iSegmentNo = stSegFrameInfoHeader.iHeaderInfo.iSegmentNo;
                        pSegIndexRecord->iInfoCount = 1;
                        pSegIndexRecord->iInfoEndOffset = iReadOffset + sizeof(CVI_RECORD_SEGMENT_INFO);

                        //读剩余扩展帧
                        while (stSegFrameInfoHeader.iHeaderInfo.iInfoExNums > pSegIndexRecord->iInfoCount) {
                            iReadOffset -= sizeof(CVI_RECORD_SEGMENT_INFO);
                            if (0 != CVI_RECORD_ReadSegmentFrameInfo(iFilefd, iReadOffset, &stSegFrameInfo, 1, &pPkgReadBuff)) {
                                _RECORD_ERR("####[Index Repair]Invalid Read\n");
                                break;
                            } else {
                                pSegIndexRecord->iInfoCount++;
                            }
                        }
                        //读扩展帧失败,文件读结束
                        if(stSegFrameInfoHeader.iHeaderInfo.iInfoExNums != pSegIndexRecord->iInfoCount) {
                            pSegIndexRecord->iStatus = CVI_RECORD_STATUS_INIT;
                            _RECORD_DBG("####[Index Repair]Invalid frame type2 %d, num:%d, index:%d,no:%d,seg:%d\n", stSegFrameInfo.iInfoType, pSegIndexRecord->iInfoCount, index, iRecNo, iRecNoSeg);
                            break;
                        }

                        if(iMaxSegNo < stSegFrameInfoHeader.iHeaderInfo.iSegmentNo) {
                            iMaxSegNo = stSegFrameInfoHeader.iHeaderInfo.iSegmentNo;
                            iCurrFileRecNo = iRecNo;
                        }
                        pFileIndexRecord->iSegRecNums++;
                    } else if(CVI_RECORD_INFO_TYPE_FRAME == stSegFrameInfo.iInfoType) {
                        //读取帧数据
                        if (pSegIndexRecord->iInfoCount > 0) {
                            if ((0 == pFileIndexRecord->tBeginTime)
                                || (stSegFrameInfo.iFrameInfo.iShowTime < pFileIndexRecord->tBeginTime)) {
                                _RECORD_DBG("####[Index Repair]File(%d) start time %lu, index:%d,no:%d,seg:%d\n", iRecNo, stSegFrameInfo.iFrameInfo.iShowTime,
                                        index, iRecNo, iRecNoSeg);
                                pFileIndexRecord->tBeginTime = stSegFrameInfo.iFrameInfo.iShowTime;
                            }

                            if (stSegFrameInfo.iFrameInfo.iShowTime > pFileIndexRecord->tEndTime) {
                                //_RECORD_DBG("####[Index Repair]File(%d) End time %u, index:%d,no:%d,seg:%d\n", iRecNo, stSegFrameInfo.iFrameInfo.iShowTime,
                                //        index, iRecNo, iRecNoSeg);
                                pFileIndexRecord->tEndTime = stSegFrameInfo.iFrameInfo.iShowTime;
                            }

                            if ((0 == pSegIndexRecord->tBeginTime)
                                || (stSegFrameInfo.iFrameInfo.iShowTime < pSegIndexRecord->tBeginTime)) {
                                pSegIndexRecord->tBeginTime = stSegFrameInfo.iFrameInfo.iShowTime;
                                pSegIndexRecord->tEndTime = stSegFrameInfo.iFrameInfo.iShowTime;
                                pSegIndexRecord->iFrameStartOffset = stSegFrameInfo.iFrameInfo.iStartOffset;
                            }

                            if(stSegFrameInfo.iFrameInfo.iShowTime > pSegIndexRecord->tEndTime) {
                                pSegIndexRecord->tEndTime = stSegFrameInfo.iFrameInfo.iShowTime;
                            }
                            pSegIndexRecord->iFrameEndOffset = stSegFrameInfo.iFrameInfo.iStartOffset + stSegFrameInfo.iFrameInfo.iFrameLen;
                            pSegIndexRecord->iInfoCount++;
                        } else {
                            _RECORD_DBG("####[Index Repair]Invalid frame type1 %d, num:%d, index:%d,no:%d,seg:%d\n", stSegFrameInfo.iInfoType, pSegIndexRecord->iInfoCount, index, iRecNo, iRecNoSeg);
                            pSegIndexRecord->iStatus = CVI_RECORD_STATUS_INIT;
                            break;
                        }
                    } else {
                        _RECORD_DBG("####[Index Repair]Invalid frame type0 %d, num:%d, index:%d,no:%d,seg:%d\n", stSegFrameInfo.iInfoType, pSegIndexRecord->iInfoCount, index, iRecNo, iRecNoSeg);
                        if(pSegIndexRecord->iInfoCount <= 0)
                            pSegIndexRecord->iStatus = CVI_RECORD_STATUS_INIT;
                        break;
                    }
                }
            }
            if(pFileIndexRecord->iSegRecNums > 0) {
                pFileIndexRecord->iStatus = CVI_STORAGE_STATUS_NORMAL;
            }

            //写片段
            pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, 0);
            iRet = CVI_RECORD_WriteSegmentIndex(pSegIndexRecord, iIndexFd, iRecNo, 0, CVI_RECORD_AV_FILE_MAX_SEGMENT);
            if (0 != iRet) {
                _RECORD_ERR("repair write index seg err,RecNo:%d\n", iRecNo);
                iRet = RECORD_ERROR;
                goto endFunc;
            }
            
            if (iFilefd > 0) {
                close(iFilefd);
                iFilefd = -1;
            }
        }

        //剩余片段数据
        for (; i < CVI_RECORD_INDEX_MAX_AVFILES; i++)
        {
            for (iRecNoSeg = 0; iRecNoSeg < CVI_RECORD_AV_FILE_MAX_SEGMENT; iRecNoSeg++)
            {
                pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, iRecNoSeg);
                CVI_RECORD_GetInitSegmentIndex(pSegIndexRecord, CVI_RECORD_STATUS_INIT);
            }
            //写片段
            pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, 0);
            iRet = CVI_RECORD_WriteSegmentIndex(pSegIndexRecord, iIndexFd, i, 0, CVI_RECORD_AV_FILE_MAX_SEGMENT);
            if (0 != iRet) {
                _RECORD_ERR("repair write index seg err,RecNo:%d\n", iRecNo);
                iRet = RECORD_ERROR;
                goto endFunc;
            }
        }

        //写入剩余容量
        if (CVI_RECORD_REMAIN_INDEX_SIZE != write(iIndexFd, iRemainIndexBuf, CVI_RECORD_REMAIN_INDEX_SIZE)) {
            _RECORD_ERR("repair write index remain err,%d\n", CVI_RECORD_REMAIN_INDEX_SIZE);
            iRet = RECORD_ERROR;
            goto endFunc;
        }

#ifdef CVI_RECORD_WRITE_INT
        usleep(CVI_RECORD_WRITE_INT);
#endif

        pFileIndexRecord = CVI_RECORD_GetFileIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, 0);
        iRet = CVI_RECORD_WriteFileIndex(pFileIndexRecord, iIndexFd, 0, CVI_RECORD_INDEX_MAX_AVFILES);
        if (0 != iRet) {
            _RECORD_ERR("repair write index seg err,RecNo:%d\n", iRecNo);
            iRet = RECORD_ERROR;
            goto endFunc;
        }

        if(iIndexFd > 0) {
            close(iIndexFd);
            iIndexFd = -1;
        }
    }

    if (iIndexFd > 0) {
        close(iIndexFd);
        iIndexFd = -1;
    }
    pFileIndexHeader->iCurrFileRecNo = iCurrFileRecNo;
    for (index = 0; index < iMaxIndexNum; index++) {
        iIndexFd = CVI_RECORD_GetIndexFileFd(s_mPkgParam.iPkgPathName, index, O_WRONLY, NULL);
        if (iIndexFd > 0) {
             iRet = CVI_RECORD_WriteFileIndexHeader(pFileIndexHeader, iIndexFd);
             if (0 != iRet) {
                 _RECORD_ERR("repair write index header err, index:%d\n", index);
                 iRet = RECORD_ERROR;
                 goto endFunc;
             }

             close(iIndexFd);
             iIndexFd = -1;
        }
    }
    _RECORD_DBG("####[Index Repair]End iRec:%d\n", iCurrFileRecNo);
    if (pCurrFileRecNo)
        *pCurrFileRecNo = iCurrFileRecNo;
    iRet = 0;

endFunc:
    if(iIndexFd > 0) {
        close(iIndexFd);
        iIndexFd = -1;
    }
    if(iFilefd > 0) {
        close(iFilefd);
        iFilefd = -1;
    }

    if(iRemainIndexBuf) {
        free(iRemainIndexBuf);
        iRemainIndexBuf = NULL;
    }
    CVI_RECORD_FreeFrameBuffer(&pPkgReadBuff);
    CVI_RECORD_FUNC_END;
    return iRet;
}

static CVI_S32 CVI_RECORD_SetCurrentFileInit(CVI_U32 iCurrFileRecNo)
{
    CVI_S32 iRet = 0;
    CVI_S32 iEndAVIndex = -1;
    CVI_S32 index = 0;
    CVI_S32 iSeekIndex = 0;
    CVI_S32 j = 0;
    CVI_S32 iWriteTotal = 0;
    CVI_S32 fd = -1;
    pkg_buffer stWriteBuff;
    CVI_RECORD_FILE_INDEX_RECORD stFileIndexRecord;
    CVI_RECORD_SEGMENT_INDEX_RECORD stSegIndexRecord;

    if((s_mPkgParam.iMaxAVFileNum > 0) && (s_mPkgParam.iMaxIndexNum > 0)) {
        if(iCurrFileRecNo >= (CVI_U32)s_mPkgParam.iMaxAVFileNum) {
            _RECORD_ERR("Invalid iCurrFileRecNo:%d, to RecNo:0\n", iCurrFileRecNo);
            iCurrFileRecNo = 0;
        }
    }

    memset(&stWriteBuff, 0, sizeof(stWriteBuff));
    if(0 != CVI_RECORD_MallocBuffer(&stWriteBuff, PKG_WRITE_BUFFER_TMP_MAX_LEN)) {
        _RECORD_ERR("pkgstorage malloc err: stWriteBuff %d\n", PKG_WRITE_BUFFER_TMP_MAX_LEN);
        return ERR_HANDLE_ALLOC_ERROR;
    }
    
    index = CVI_RECORD_FILES_GET_INDEX_NUM(iCurrFileRecNo);
    
    _RECORD_INFO("Init RecNo:%d Index:%d\n", iCurrFileRecNo, index);

    //清空视频索引数据
    fd = CVI_RECORD_GetAvFilesFd(s_mPkgParam.iPkgPathName, iCurrFileRecNo, O_WRONLY, NULL);
    if (fd <= 0) {
        _RECORD_ERR("open fd err,RecNo:%d\n", iCurrFileRecNo);
        iRet = ERR_OPEN_FILE_ERROR;
    } else {
        CVI_RECORD_SEGMENT_INFO stSegFrameInfo;
        iEndAVIndex = CVI_RECORD_AV_FILE_MAX_SIZE - CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_SIZE;
        iSeekIndex = lseek(fd, iEndAVIndex, SEEK_SET);
        if (iSeekIndex != iEndAVIndex) {
            _RECORD_ERR("lseek file ret:%d != %d\n", iSeekIndex, iEndAVIndex);
            iRet = ERR_INVALID_ARGUMENT;
            goto endFunc0;
        }
        iWriteTotal = 0;
        //写CVI_RECORD_FILE_INDEX_RECORD（对应文件的录像索引数据 每个录像文件Rec00XXX.cvitek都有1个对应索引数据 ）
        CVI_RECORD_GetInitSegmentFrameInfo(&stSegFrameInfo);
        for (j = 0; j < CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_COUNT; j++) {
            if (0 > CVI_RECORD_WriteBuffer(&stWriteBuff, (CVI_U8*)&stSegFrameInfo, sizeof(stSegFrameInfo), fd, 0)) {
                _RECORD_ERR("pkg format avFiles ERR, size %d\n", sizeof(stSegFrameInfo));
                iRet = ERR_INVALID_ARGUMENT;
                goto endFunc0;
            }
            iWriteTotal += sizeof(stSegFrameInfo);
        }

        if (0 > CVI_RECORD_FlushBuffer(&stWriteBuff, fd, 0)) {
            _RECORD_ERR("pkg format avFiles ERR, size %d\n", sizeof(stSegFrameInfo));
            iRet = ERR_INVALID_ARGUMENT;
            goto endFunc0;
        }

        _RECORD_INFO("pkg write init avFiles iWriteTotal:%d\n", iWriteTotal);
    }
endFunc0:
    if(fd > 0) {
        close(fd);
        fd = -1;
    }
    
    //清空文件索引数据
    fd = CVI_RECORD_GetIndexFileFd(s_mPkgParam.iPkgPathName, index, O_RDWR, NULL);
    if(fd < 0) {
        _RECORD_ERR("open file err, ret:%d\n", fd);
        iRet = ERR_INVALID_ARGUMENT;
        goto endFunc1;
    }
    iWriteTotal = 0;

    CVI_RECORD_GetInitFileIndex(&stFileIndexRecord, CVI_RECORD_STATUS_INIT);
    iRet = CVI_RECORD_WriteFileIndex(&stFileIndexRecord, fd, iCurrFileRecNo, 1);
    if (0 != iRet) {
        _RECORD_ERR("open file err,ret:%d\n", iRet);
        iRet = ERR_INVALID_ARGUMENT;
        goto endFunc1;
    }

    iEndAVIndex = CVI_RECORD_SEGMENT_INDEX_RECORD_SEEK(iCurrFileRecNo, 0);
    iSeekIndex = lseek(fd, iEndAVIndex, SEEK_SET);
    if (iSeekIndex != iEndAVIndex) {
        _RECORD_ERR("lseek file ret:%d != %d\n", iSeekIndex, iEndAVIndex);
        iRet = ERR_INVALID_ARGUMENT;
        goto endFunc1;
    }

    CVI_RECORD_GetInitSegmentIndex(&stSegIndexRecord, CVI_RECORD_STATUS_INIT);
    for (j = 0; j < CVI_RECORD_AV_FILE_MAX_SEGMENT; j++) {
        if (0 > CVI_RECORD_WriteBuffer(&stWriteBuff, (CVI_U8*)&stSegIndexRecord, sizeof(stSegIndexRecord), fd, 0)) {
            _RECORD_ERR("pkg format stFileIndexRecord ERR, size %d\n", sizeof(stSegIndexRecord));
            iRet = ERR_INVALID_ARGUMENT;
            goto endFunc1;
        }
        iWriteTotal += sizeof(stSegIndexRecord);
    }

    if (0 > CVI_RECORD_FlushBuffer(&stWriteBuff, fd, 0)) {
        _RECORD_ERR("pkg format avFiles ERR\n");
        iRet = ERR_INVALID_ARGUMENT;
        goto endFunc1;
    }
    _RECORD_INFO("pkg write index Files,iWriteTotal:%d\n", iWriteTotal);
endFunc1:
    if(fd > 0) {
        close(fd);
        fd = -1;
    }
    CVI_RECORD_FreeBuffer(&stWriteBuff);
    return iRet;
}

static CVI_U32 CVI_RECORD_GetRemainNum()
{
    CVI_RECORD_FILE_INDEX_RECORD stFileIndexRecord;
    CVI_S32 iIndexfd = -1;
    CVI_S32 index = 0;
    
    if ((s_mPkgParam.iMaxAVFileNum <= 0) || (s_mPkgParam.iMaxIndexNum <= 0)) {
        return 0;
    } else {
        CVI_U32 iNextFileRecNo = s_mPkgParam.iCurrFileRecNo + 1;
        if ((iNextFileRecNo >= (CVI_U32)s_mPkgParam.iMaxAVFileNum)) {
            //写到最后一个片段了
            return 0;
        }

        index = CVI_RECORD_FILES_GET_INDEX_NUM(iNextFileRecNo);
        iIndexfd = CVI_RECORD_GetIndexFileFd(s_mPkgParam.iPkgPathName, index, O_RDWR, NULL);

        memset(&stFileIndexRecord, 0, sizeof(stFileIndexRecord));
        if (0 == CVI_RECORD_ReadFileIndex(&stFileIndexRecord, iIndexfd, iNextFileRecNo)) {
            if (CVI_RECORD_STATUS_INIT == stFileIndexRecord.iStatus) {
                if (iIndexfd > 0) {
                    close(iIndexfd);
                    iIndexfd = -1;
                }
                return (s_mPkgParam.iMaxAVFileNum - iNextFileRecNo + 1);
            }
        } else {
            _RECORD_ERR("load err record no:%d\n", iNextFileRecNo);
        }
    }

    if(iIndexfd > 0) {
        close(iIndexfd);
        iIndexfd = -1;
    }
    return 0;
}

static CVI_S32 CVI_RECORD_AddCurrentIndexBad(CVI_S32 bAdd)
{
    if(bAdd)
        s_mPkgParam.iFileErrCount++;
    else
        s_mPkgParam.iFileErrCount = 0;
    return 0;
}

static CVI_S32 CVI_RECORD_AddCurrentStreamFileBad(CVI_S32 bAdd)
{
    CVI_RECORD_FUNC_START;
    if (bAdd) {
        s_mPkgParam.iIndexErrCount++;
        _RECORD_ERR("########[BAD AVFILE] fileNo:%d fd:%d, times:%d\n",
            s_mPkgParam.iCurrFileRecNo, s_mPkgParam.iCurRecordFd, s_mPkgParam.iIndexErrCount);
        if (s_mPkgParam.iCurRecordFd > 0) {
            close(s_mPkgParam.iCurRecordFd);
            s_mPkgParam.iCurRecordFd = -1;
        }
        if (s_mPkgParam.iCurRecordAvFd > 0) {
            close(s_mPkgParam.iCurRecordAvFd);
            s_mPkgParam.iCurRecordAvFd = -1;
        }
        if (CVI_RECORD_BAD_FILE_WRITE_TIMES <= s_mPkgParam.iIndexErrCount) {
            _RECORD_ERR("########[BAD AVFILE] setting  fileNo:%d times:%d\n", s_mPkgParam.iCurrFileRecNo, s_mPkgParam.iIndexErrCount);
            CVI_RECORD_FILE_INDEX_RECORD stFileIndexRecord;
            CVI_S32 index = 0;
            CVI_S32 iIndexFd = -1;
            for (index = 0; index < s_mPkgParam.iMaxIndexNum; index++) {
                iIndexFd = CVI_RECORD_GetIndexFileFd(s_mPkgParam.iPkgPathName, index, O_WRONLY, NULL);
                if (iIndexFd <= 0) {
                    CVI_RECORD_AddCurrentIndexBad(1);
                    _RECORD_ERR("########[BAD AVFILE]open fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
                    break;
                }

                memset(&stFileIndexRecord, 0, sizeof(CVI_RECORD_FILE_INDEX_RECORD));
                stFileIndexRecord.iStatus = CVI_RECORD_STATUS_BAD_FILE;

                if (0 != CVI_RECORD_WriteFileIndex(&stFileIndexRecord, iIndexFd, s_mPkgParam.iCurrFileRecNo, 1)) {
                    close(iIndexFd);
                    CVI_RECORD_AddCurrentIndexBad(1);
                    _RECORD_ERR("########[BAD AVFILE]write index fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
                    break;
                }
                close(iIndexFd);
                _RECORD_ERR("########[BAD AVFILE] seccess index:%d fileNo:%d times:%d\n", index, s_mPkgParam.iCurrFileRecNo, s_mPkgParam.iIndexErrCount);
            }

            s_mPkgParam.iCurrFileRecNo++;
            CVI_RECORD_LoadCurrentInfo(s_mPkgParam.iCurrFileRecNo);
            if (s_mPkgParam.iReMainAVFileNum > 0) {
                s_mPkgParam.iReMainAVFileNum--;
            }
            s_mPkgParam.iIndexErrCount = 0;
            return 1;
        }
    } else {
        s_mPkgParam.iIndexErrCount = 0;
    }
    CVI_RECORD_FUNC_END;
    return 0;
    
}

static CVI_S32 CVI_RECORD_GetNextNumber(CVI_U32 iCurrFileRecNo)
{
    CVI_S32 iIndexfd = -1;
    CVI_S32 index = 0;
    CVI_S32 iCheckTimes = 0;
    CVI_RECORD_FILE_INDEX_RECORD stFileIndexRecord;
    CVI_U32 iNextFileRecNo = iCurrFileRecNo;
checkNextFile:

    iNextFileRecNo++;
    iCheckTimes++;

    if (iIndexfd > 0) {
        close(iIndexfd);
        iIndexfd = -1;
    }

    if (iCheckTimes > s_mPkgParam.iMaxAVFileNum) {
        _RECORD_ERR("Invalid check times:%d, iMaxAVFileNum:%d, no:%u\n", iCheckTimes, s_mPkgParam.iMaxAVFileNum, iCurrFileRecNo);
        return iCurrFileRecNo;
    }

    if((s_mPkgParam.iMaxAVFileNum > 0) && (s_mPkgParam.iMaxIndexNum > 0)) {
        if(iNextFileRecNo >= (CVI_U32)s_mPkgParam.iMaxAVFileNum) {
            _RECORD_ERR("Invalid iCurrFileRecNo:%d, to RecNo:0\n", iNextFileRecNo);
            iNextFileRecNo = 0;
        }
    }

    index = CVI_RECORD_FILES_GET_INDEX_NUM(iNextFileRecNo);
    iIndexfd = CVI_RECORD_GetIndexFileFd(s_mPkgParam.iPkgPathName, index, O_RDWR, NULL);
    memset(&stFileIndexRecord, 0, sizeof(stFileIndexRecord));
    if(0 != CVI_RECORD_ReadFileIndex(&stFileIndexRecord, iIndexfd, iNextFileRecNo)) {
        _RECORD_ERR("CVI_RECORD_GetFileIndex err\n");
        goto checkNextFile;
    } else {
        if(CVI_RECORD_STATUS_BAD_FILE == stFileIndexRecord.iStatus)
        {
            _RECORD_ERR("Check bad File %d, check next times %d\n", iNextFileRecNo, iCheckTimes);
            goto checkNextFile;
        }
    }

    if(iIndexfd > 0) {
        close(iIndexfd);
        iIndexfd = -1;
    }

    _RECORD_DBG("Get next record file:%u\n", iNextFileRecNo);
    return iNextFileRecNo;
}


static CVI_S32 CVI_RECORD_LoadCurrentInfo(CVI_U32 iCurrFileRecNo)
{
    CVI_S32 iRet = 0;
    CVI_S32 iIndexfd = -1;
    CVI_S32 index = 0;
    CVI_S32 iMaxAVFileNum = 0;
    CVI_S32 iMaxIndexNum = 0;
    CVI_S32 iCurrFileRecSegment = 0;
    CVI_S32 iCheckTimes = 0;
    CVI_RECORD_FILE_INDEX_HEADER* pFileIndexHeader = NULL;
    CVI_RECORD_FILE_INDEX_RECORD* pFileIndexRecord = NULL;
    CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecord = NULL;
    CVI_RECORD_SEGMENT_INDEX_RECORD* pLastSegIndexRecord = NULL;
    CVI_RECORD_FILE_INDEX_RECORD* pLastFileIndexRecord = NULL;
checkNextFile:

    if (iIndexfd > 0) {
        close(iIndexfd);
        iIndexfd = -1;
    }

    if ((s_mPkgParam.iMaxAVFileNum > 0) && (s_mPkgParam.iMaxIndexNum > 0)) {
        if (iCurrFileRecNo >= (CVI_U32)s_mPkgParam.iMaxAVFileNum) {
            _RECORD_ERR("Invalid iCurrFileRecNo:%d, to RecNo:0\n", iCurrFileRecNo);
            iCurrFileRecNo = 0;
        }
    }

    index = CVI_RECORD_FILES_GET_INDEX_NUM(iCurrFileRecNo);

    iIndexfd = CVI_RECORD_GetIndexFileFd(s_mPkgParam.iPkgPathName, index, O_RDWR, NULL);
    if (0 != CVI_RECORD_GetFileIndex(&s_mPkgParam.iPkgCurFileIndexBuff, iIndexfd, iCurrFileRecNo)) {
        _RECORD_ERR("CVI_RECORD_GetFileIndex err\n");
        iRet = ERR_INVALID_ARGUMENT;
        goto endFunc;
    } else {
        pFileIndexHeader = CVI_RECORD_GetFileIndexHeader(&s_mPkgParam.iPkgCurFileIndexBuff);
        pFileIndexRecord = CVI_RECORD_GetFileIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, iCurrFileRecNo);
        if ((NULL == pFileIndexHeader) || (NULL == pFileIndexRecord)) {
            _RECORD_ERR("t_pkgstorage Invalid iCurrFileRecNo %d,index:%d err\n", pFileIndexHeader->iCurrFileRecNo, iCurrFileRecNo);
            iRet = ERR_INVALID_ARGUMENT;
            goto endFunc;
        } else {
            CVI_RECORD_ShowFileIndexHeader(pFileIndexHeader);
            CVI_RECORD_ShowFileIndex(pFileIndexRecord);

            if ((pFileIndexHeader->iCurrFileRecNo != (CVI_S32)iCurrFileRecNo) || (pFileIndexHeader->iAVFiles <= (CVI_S32)iCurrFileRecNo)) {
                _RECORD_ERR("t_pkgstorage Invalid AVFiles:%d RecNo:%d,%d,index:%d err\n",
                        pFileIndexHeader->iAVFiles, pFileIndexHeader->iCurrFileRecNo, iCurrFileRecNo, index);
            }
            if ((s_mPkgParam.iMaxAVFileNum <= 0) || (s_mPkgParam.iMaxIndexNum <= 0)) {
                //获取最大文件数
                if(pFileIndexHeader->iAVFiles <= (CVI_S32)iCurrFileRecNo) {
                    _RECORD_ERR("t_pkgstorage Invalid iCurrFileRecNo %d,index:%d err\n", iCurrFileRecNo, index);
                    iRet = ERR_INVALID_ARGUMENT;
                    goto endFunc;
                } else {
                    iMaxAVFileNum = pFileIndexHeader->iAVFiles;
                    //索引文件数重新计算可能刚好没有视频文件了
                    iMaxIndexNum = CVI_RECORD_FILES_GET_INDEX_NUM(iMaxAVFileNum);
                    if (CVI_RECORD_FILES_GET_INDEX_FILES(iMaxAVFileNum) > 0)
                        iMaxIndexNum++;
                }
            } else {
                iMaxAVFileNum = s_mPkgParam.iMaxAVFileNum;
                iMaxIndexNum = s_mPkgParam.iMaxIndexNum;
            }
            if (CVI_RECORD_STATUS_BAD_FILE == pFileIndexRecord->iStatus) {
                if(iCheckTimes < s_mPkgParam.iMaxAVFileNum) {
                    _RECORD_ERR("Check bad File %d, check next times %d\n", iCurrFileRecNo, iCheckTimes);
                    iCurrFileRecNo++;
                    iCheckTimes++;
                    goto checkNextFile;
                } else {
                    iRet = ERR_INVALID_ARGUMENT;
                    goto endFunc;
                }
            } else if(CVI_RECORD_STATUS_LOCK == pFileIndexRecord->iStatus) {
                //当前写的文件片段为当前片段号
                if(pFileIndexRecord->iSegRecNums >= CVI_RECORD_AV_FILE_MAX_SEGMENT) {
                    pFileIndexRecord->iSegRecNums = 0;
                    iCurrFileRecSegment = 0;
                } else {
                    iCurrFileRecSegment = pFileIndexRecord->iSegRecNums;
                }
            } else {
                //当前写的文件片段为当前片段号
                pFileIndexRecord->iStatus = CVI_RECORD_STATUS_LOCK;
                pFileIndexRecord->iSegRecNums = 0;
                pFileIndexRecord->tBeginTime = 0;
                pFileIndexRecord->tEndTime = 0;
                iCurrFileRecSegment = 0;
                _RECORD_INFO("Invalid Status, unLock Status to reinit\n");
            }
        }
    }
endFunc:
    if(iIndexfd > 0) {
        close(iIndexfd);
        iIndexfd = -1;
    }

    if (0 == iRet) {
        s_mPkgParam.iMaxAVFileNum = iMaxAVFileNum;
        s_mPkgParam.iMaxIndexNum = iMaxIndexNum;
        s_mPkgParam.iCurrFileRecNo = iCurrFileRecNo;
        s_mPkgParam.iCurrFileRecSegment = iCurrFileRecSegment;
        pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, iCurrFileRecSegment);

        CVI_RECORD_ShowSegmentIndex(pSegIndexRecord);

        //当前帧数清空
        pSegIndexRecord->iEvenType = CVI_RECORD_EVEN_NONE;
        pSegIndexRecord->iStatus = CVI_RECORD_STATUS_INIT;
        pSegIndexRecord->tBeginTime = 0;
        pSegIndexRecord->tEndTime = 0;
        
        pSegIndexRecord->iInfoCount = 0;

        if (0 == iCurrFileRecSegment) {
            pSegIndexRecord->iFrameStartOffset = 0;
            pSegIndexRecord->iFrameEndOffset = 0;
            pSegIndexRecord->iSegmentNo = 0;
            //开一帧肯定是最后一个位
            pSegIndexRecord->iInfoEndOffset = CVI_RECORD_AV_FILE_MAX_SIZE;

            pkg_buffer stPkgBuff;
            memset(&stPkgBuff, 0, sizeof(stPkgBuff));
            if (0 != CVI_RECORD_MallocBuffer(&stPkgBuff, PKG_FILE_INDEX_MAX_LEN)) {
                _RECORD_ERR("pkgstorage malloc err: pPkgBuff %d\n", PKG_FILE_INDEX_MAX_LEN);
            } else {
                CVI_U32 iLastFileRecNo = 0;
                if(0 == iCurrFileRecNo)
                    iLastFileRecNo = s_mPkgParam.iMaxAVFileNum - 1;
                else
                    iLastFileRecNo = iCurrFileRecNo - 1;
                if (0 == CVI_RECORD_GetSegmentIndex(&stPkgBuff, iLastFileRecNo)) {
                    pLastFileIndexRecord = CVI_RECORD_GetFileIndexRecord(&stPkgBuff, iLastFileRecNo);
                    if((CVI_RECORD_STATUS_NORMAL == pLastFileIndexRecord->iStatus) && (0 < pLastFileIndexRecord->iSegRecNums)) {
                        pLastSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&stPkgBuff, pLastFileIndexRecord->iSegRecNums-1);
                        if(pLastSegIndexRecord)
                            pSegIndexRecord->iSegmentNo = pLastSegIndexRecord->iSegmentNo + 1;
                    }
                }
                CVI_RECORD_FreeBuffer(&stPkgBuff);
            }
        } else {
            pLastSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, iCurrFileRecSegment-1);
            pSegIndexRecord->iFrameStartOffset = pLastSegIndexRecord->iFrameEndOffset;
            pSegIndexRecord->iFrameEndOffset = pLastSegIndexRecord->iFrameEndOffset;
            
            //上一帧的起始offset就是这个帧的结束offset
            pSegIndexRecord->iInfoEndOffset = CVI_RECORD_FRAME_AVINDEX_GET_START_OFFSET(pLastSegIndexRecord);
            pSegIndexRecord->iSegmentNo = pLastSegIndexRecord->iSegmentNo + 1;
        }

        _RECORD_INFO("t_pkgstorage load cur info ok index:%d, iMaxAVFileNum:%d,iMaxIndexNum:%d,iCurrFileRecNo:%d,iCurrFileRecSegment:%d\n",
            index, s_mPkgParam.iMaxAVFileNum, s_mPkgParam.iMaxIndexNum, s_mPkgParam.iCurrFileRecNo, s_mPkgParam.iCurrFileRecSegment);
        CVI_RECORD_ShowSegmentIndex(pSegIndexRecord);

        //重新更新FD
        if(s_mPkgParam.iCurIndexFd > 0) {
            close(s_mPkgParam.iCurIndexFd);
            s_mPkgParam.iCurIndexFd = -1;
        }
        if(s_mPkgParam.iCurIndexBakFd > 0) {
            close(s_mPkgParam.iCurIndexBakFd);
            s_mPkgParam.iCurIndexBakFd = -1;
        }
        CVI_RECORD_AddCurrentIndexBad(0);
        
        if(s_mPkgParam.iCurRecordFd > 0) {
            close(s_mPkgParam.iCurRecordFd);
            s_mPkgParam.iCurRecordFd = -1;
        }
        if(s_mPkgParam.iCurRecordAvFd > 0) {
            close(s_mPkgParam.iCurRecordAvFd);
            s_mPkgParam.iCurRecordAvFd = -1;
        }
        CVI_RECORD_AddCurrentStreamFileBad(0);
    } else {
        _RECORD_ERR("t_pkgstorage load cur info err index:%d, recno:%d\n", index, iCurrFileRecNo);
        s_mPkgParam.iPkgStatus = CVI_STORAGE_STATUS_ERROR;
    }
    return iRet;
}

static CVI_S32 CVI_RECORD_CheckInit()
{
    CVI_S32 iIndexfd = -1;
    CVI_S32 iRet = 0;
    CVI_S32 iCurrFileRecNo = -1;
    CVI_S32 iMaxAVFileNum = 0;
    CVI_S32 iMaxIndexNum = 0;
    CVI_S32 index = 0;
    CVI_S32 bRepairIndex = 0;
    CVI_UL iFileSize = 0;
    CVI_S32 bCheckIndex = -1;
    
    CVI_CHAR stPathName[CVI_RECORD_MAX_PATH_NAME];
    CVI_CHAR stPathNameBak[CVI_RECORD_MAX_PATH_NAME];
    
    CVI_RECORD_FILE_INDEX_HEADER stFileIndexHeader;
    CVI_RECORD_FUNC_START;
checkAgain:

    iRet = 0;
    iCurrFileRecNo = -1;
    iMaxAVFileNum = 0;
    iMaxIndexNum = 0;
    index = 0;
    bRepairIndex = 0;
    iFileSize = 0;
    if (iIndexfd > 0) {
        close(iIndexfd);
        iIndexfd = -1;
    }

    //检查文件视频数
    iIndexfd = CVI_RECORD_GetIndexFileFd(s_mPkgParam.iPkgPathName, index, O_RDWR, &iFileSize);
    if (iIndexfd <= 0) {
        _RECORD_ERR("open pkg index(%d) pkgstorage_get_file_index err\n", index);
        return RECORD_ERROR;
    }
    if (0 != CVI_RECORD_ReadFileIndexHeader(&stFileIndexHeader, iIndexfd)) {
        iMaxAVFileNum = CVI_RECORD_GetAvFilesFromCard(s_mPkgParam.iPkgPathName);
        bRepairIndex = 1;
    } else {
        iMaxAVFileNum = stFileIndexHeader.iAVFiles;
        iCurrFileRecNo = stFileIndexHeader.iCurrFileRecNo;
        bRepairIndex = (CVI_RECORD_INDEX_FILE_MAX_SIZE != iFileSize) ? 1 : 0;
    }

    if (iIndexfd > 0) {
        close(iIndexfd);
        iIndexfd = -1;
    }

    if (iMaxAVFileNum <= 0 || (iCurrFileRecNo < 0) || (iCurrFileRecNo >= iMaxAVFileNum)) {
        bRepairIndex = 1;
    }
    //索引文件数重新计算可能刚好没有视频文件了
    iMaxIndexNum = CVI_RECORD_FILES_GET_INDEX_NUM(iMaxAVFileNum);
    if (CVI_RECORD_FILES_GET_INDEX_FILES(iMaxAVFileNum) > 0) {
        iMaxIndexNum++;
    }
    _RECORD_INFO("pkg check init AVFiles:%d iMaxIndexFiles:%d, iCurNo:%d, bFix:%d\n", iMaxAVFileNum, iMaxIndexNum, iCurrFileRecNo, bRepairIndex);

    //检查文件索引有效性
    if (0 == bRepairIndex) {
        CVI_S32 iFileIndex = 0;
        CVI_RECORD_FILE_INDEX_HEADER* pFileIndexHeader = NULL;

        for (index = 0; index < iMaxIndexNum; index++) {
            iIndexfd = CVI_RECORD_GetIndexFileFd(s_mPkgParam.iPkgPathName, index, O_RDWR, NULL);
            iFileIndex = 0;
            //检测文件索引有效性
            if (0 != CVI_RECORD_GetFileIndex(&s_mPkgParam.iPkgCurFileIndexBuff, iIndexfd, iFileIndex)) {
                _RECORD_ERR("pkg index(%d) pkgstorage_get_file_index err\n", index);
                bRepairIndex = 1;
                break;
            }
            iFileIndex++;

            pFileIndexHeader = CVI_RECORD_GetFileIndexHeader(&s_mPkgParam.iPkgCurFileIndexBuff);
            if ((NULL == pFileIndexHeader)
                || (stFileIndexHeader.iAVFiles != iMaxAVFileNum)
                || (stFileIndexHeader.iCurrFileRecNo != iCurrFileRecNo)) {
                _RECORD_ERR("pkg index(%d) file head err\n", index);
                bRepairIndex = 1;
                break;
            }

            //检测剩余片段是否有效
            for (;iFileIndex<CVI_RECORD_INDEX_MAX_AVFILES;iFileIndex++) {
                if (0 != CVI_RECORD_GetSegmentFormFileIndex(&s_mPkgParam.iPkgCurFileIndexBuff, iIndexfd, iFileIndex)) {
                    _RECORD_ERR("pkg index(%d) CVI_RECORD_GetSegmentFormFileIndex(%d) err\n", index, iFileIndex);
                    bRepairIndex = 1;
                    break;
                }
            }

            if (iIndexfd > 0) {
                close(iIndexfd);
                iIndexfd = -1;
            }

            if (bRepairIndex) {
                break;
            }
        }
    }

    if (iIndexfd > 0) {
        close(iIndexfd);
        iIndexfd = -1;
    }

    //检查文件索引存在无效，修复文件索引
    if (bRepairIndex) {
        _RECORD_ERR("Check index err,repair index:%d,%d, files:%d indexs:%d\n", bCheckIndex, index, iMaxAVFileNum, iMaxIndexNum);
        if  ((bCheckIndex < index) && (index < iMaxIndexNum)) {
            bCheckIndex = index;
            if (0 != CVI_RECORD_GetIndexFileName(s_mPkgParam.iPkgPathName, bCheckIndex, stPathName, sizeof(stPathName))) {
                _RECORD_ERR("pkg invalid path\n");
            } else {
                snprintf(stPathNameBak, sizeof(stPathNameBak), "%s/"CVI_RECORD_INDEX_FILE_BAK, s_mPkgParam.iPkgPathName, index);
                if (0 != CVI_RECORD_CopyFile(stPathNameBak, stPathName, 1)) {
                    _RECORD_ERR("pkg copy bak index Err\n");
                }
            }
            goto checkAgain;
        } else {
            iRet = CVI_RECORD_RepairIndex(&iCurrFileRecNo);
        }
    }

    //索引已修复或者索引正常
    if (0 == iRet) {
        //加载当前写入录像索引等信息
        iRet = CVI_RECORD_LoadCurrentInfo(iCurrFileRecNo);

        //加载剩余容量文件数
        s_mPkgParam.iReMainAVFileNum = CVI_RECORD_GetRemainNum(s_mPkgParam.iCurrFileRecNo);

        for (index = 0; index < iMaxIndexNum; index++) {
            if (0 != CVI_RECORD_GetIndexFileName(s_mPkgParam.iPkgPathName, index, stPathName, sizeof(stPathName))) {
                _RECORD_ERR("pkg invalid path\n");
                continue;
            }
            snprintf(stPathNameBak, sizeof(stPathNameBak), "%s/"CVI_RECORD_INDEX_FILE_BAK, s_mPkgParam.iPkgPathName, index);
            if (0 != CVI_RECORD_CopyFile(stPathName, stPathNameBak, 1))
                _RECORD_ERR("pkg copy index to bak\n");
        }

        _RECORD_DBG("PKG check init success, remainFiles:%u\n", s_mPkgParam.iReMainAVFileNum);
    }

    CVI_RECORD_FUNC_END;

    return iRet;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_WriteCurrentFileIndex
 功能描述  :写入录像索引文件及切换下个写索引文件
 输入参数  : 
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
static CVI_S32 CVI_RECORD_WriteCurrentFileIndex(CVI_S32 bChangeFile)
{
    CVI_RECORD_FILE_INDEX_HEADER* pFileIndexHeader = NULL;
    CVI_RECORD_FILE_INDEX_RECORD* pFileIndexRecord = NULL;
    CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecord = NULL;
    CVI_RECORD_SEGMENT_INDEX_RECORD* pNewSegIndexRecord = NULL;
    CVI_S32 bRecNo = 0;
    CVI_S32 i = 0;
    CVI_S32 iRet = 0;
    CVI_S32 index = CVI_RECORD_FILES_GET_INDEX_NUM(s_mPkgParam.iCurrFileRecNo);
    if (s_mPkgParam.iCurIndexFd <= 0) {
        s_mPkgParam.iCurIndexFd = CVI_RECORD_GetIndexFileFd(s_mPkgParam.iPkgPathName, index, O_WRONLY, NULL);
        if(s_mPkgParam.iCurIndexFd <= 0) {
            CVI_RECORD_AddCurrentIndexBad(1);
            _RECORD_ERR("open fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
            return ERR_OPEN_FILE_ERROR;
        }
    }
    if(s_mPkgParam.iCurIndexBakFd <= 0) {
        s_mPkgParam.iCurIndexBakFd = CVI_RECORD_GetIndexBakFileFd(s_mPkgParam.iPkgPathName, index, O_WRONLY, NULL);
        if(s_mPkgParam.iCurIndexBakFd <= 0)
            _RECORD_ERR("open bak fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
    }
    pFileIndexHeader = CVI_RECORD_GetFileIndexHeader(&s_mPkgParam.iPkgCurFileIndexBuff);
    pFileIndexRecord = CVI_RECORD_GetFileIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, s_mPkgParam.iCurrFileRecNo);
    pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, s_mPkgParam.iCurrFileRecSegment);
    if((NULL == pFileIndexHeader) || (NULL == pFileIndexRecord) || (NULL == pSegIndexRecord)) {
        _RECORD_ERR("Invalid param\n");
        return RECORD_ERROR;
    }
    if(pSegIndexRecord->iInfoCount <= 0) {
        _RECORD_ERR("no frame input, no:%u, seg:%u\n", s_mPkgParam.iCurrFileRecNo, s_mPkgParam.iCurrFileRecSegment);
        return RECORD_ERROR;
    }
    _RECORD_INFO("write segment no:%u, seg:%u\n", s_mPkgParam.iCurrFileRecNo, s_mPkgParam.iCurrFileRecSegment);
    pSegIndexRecord->iStatus = CVI_RECORD_STATUS_NORMAL;
    iRet = CVI_RECORD_WriteSegmentIndex(pSegIndexRecord, s_mPkgParam.iCurIndexFd, s_mPkgParam.iCurrFileRecNo, s_mPkgParam.iCurrFileRecSegment, 1);
    if (0 != iRet) {
        _RECORD_ERR("write index fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
        CVI_RECORD_AddCurrentIndexBad(1);
        //return RECORD_ERROR;
    }

    iRet = CVI_RECORD_WriteSegmentIndex(pSegIndexRecord, s_mPkgParam.iCurIndexBakFd, s_mPkgParam.iCurrFileRecNo, s_mPkgParam.iCurrFileRecSegment, 1);
    if(0 != iRet) {
        _RECORD_ERR("write bak index fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
    }

    CVI_RECORD_ShowSegmentIndex(pSegIndexRecord);

    pFileIndexRecord->iSegRecNums = s_mPkgParam.iCurrFileRecSegment+1;
    if ((0 == pFileIndexRecord->tBeginTime) || (pSegIndexRecord->tBeginTime < pFileIndexRecord->tBeginTime))
        pFileIndexRecord->tBeginTime = pSegIndexRecord->tBeginTime;

    if ((0 == pFileIndexRecord->tEndTime) || (pSegIndexRecord->tEndTime > pFileIndexRecord->tEndTime))
        pFileIndexRecord->tEndTime = pSegIndexRecord->tEndTime;

    if (bChangeFile || s_mPkgParam.iCurrFileRecSegment >= CVI_RECORD_AV_FILE_MAX_SEGMENT - 1) {
        //写入文件时为写完成时NORMAL
        pFileIndexRecord->iStatus = CVI_RECORD_STATUS_NORMAL;
        iRet = CVI_RECORD_WriteFileIndex(pFileIndexRecord, s_mPkgParam.iCurIndexFd, s_mPkgParam.iCurrFileRecNo, 1);
        if (0 != iRet) {
            _RECORD_ERR("write index fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
            CVI_RECORD_AddCurrentIndexBad(1);
            //return RECORD_ERROR;
        }
        iRet = CVI_RECORD_WriteFileIndex(pFileIndexRecord, s_mPkgParam.iCurIndexBakFd, s_mPkgParam.iCurrFileRecNo, 1);
        if (0 != iRet)
            _RECORD_ERR("write bak index fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);

        s_mPkgParam.iCurrFileRecSegment = 0;
        s_mPkgParam.iCurrFileRecNo = CVI_RECORD_GetNextNumber(s_mPkgParam.iCurrFileRecNo);
        bRecNo = 1;
    } else {
        //写入文件时为LOCK
        pFileIndexRecord->iStatus = CVI_RECORD_STATUS_LOCK;
        iRet = CVI_RECORD_WriteFileIndex(pFileIndexRecord, s_mPkgParam.iCurIndexFd, s_mPkgParam.iCurrFileRecNo, 1);
        if (0 != iRet) {
            _RECORD_ERR("write index fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
            CVI_RECORD_AddCurrentIndexBad(1);
            //return RECORD_ERROR;
        }

        iRet = CVI_RECORD_WriteFileIndex(pFileIndexRecord, s_mPkgParam.iCurIndexBakFd, s_mPkgParam.iCurrFileRecNo, 1);
        if (0 != iRet)
            _RECORD_ERR("write index fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);

        bRecNo = 0;
        s_mPkgParam.iCurrFileRecSegment++;
    }
    CVI_RECORD_ShowFileIndex(pFileIndexRecord);

    //写索引文件头
    pFileIndexHeader->iFileStartCode = CVI_RECORD_FILE_STARTCODE;
    pFileIndexHeader->iModifyTimes++;
    pFileIndexHeader->iVersion = CVI_RECORD_VERSION;
    pFileIndexHeader->iAVFiles = s_mPkgParam.iMaxAVFileNum;
    pFileIndexHeader->iCurrFileRecNo = s_mPkgParam.iCurrFileRecNo;

    iRet = CVI_RECORD_WriteFileIndexHeader(pFileIndexHeader, s_mPkgParam.iCurIndexFd);
    if (0 != iRet) {
        _RECORD_ERR("write index fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
        CVI_RECORD_AddCurrentIndexBad(1);
        //return RECORD_ERROR;
    }
    iRet = CVI_RECORD_WriteFileIndexHeader(pFileIndexHeader, s_mPkgParam.iCurIndexBakFd);
    if(0 != iRet)
        _RECORD_ERR("write index bak fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);

    CVI_RECORD_ShowFileIndexHeader(pFileIndexHeader);
    _RECORD_INFO("write next segment no:%u, seg:%u\n", s_mPkgParam.iCurrFileRecNo, s_mPkgParam.iCurrFileRecSegment);
    if(bRecNo) {
        //先覆盖重置之前的录像
        iRet = CVI_RECORD_SetCurrentFileInit(s_mPkgParam.iCurrFileRecNo);
        for(i=0;i<s_mPkgParam.iMaxIndexNum;i++)
        {
            if(i != index)
            {
                CVI_S32 fd = CVI_RECORD_GetIndexFileFd(s_mPkgParam.iPkgPathName, i, O_WRONLY, NULL);
                if(fd > 0)
                {
                     iRet = CVI_RECORD_WriteFileIndexHeader(pFileIndexHeader, fd);
                     close(fd);
                     fd = -1;
                }
            }
        }
        CVI_RECORD_LoadCurrentInfo(s_mPkgParam.iCurrFileRecNo);
        if(s_mPkgParam.iReMainAVFileNum > 0)
        {
            s_mPkgParam.iReMainAVFileNum--;
        }
    }
    else
    {
        pNewSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, s_mPkgParam.iCurrFileRecSegment);
        
        //CVI_RECORD_ShowSegmentIndex(pNewSegIndexRecord);
        //当前帧数清空
        pNewSegIndexRecord->iEvenType = CVI_RECORD_EVEN_NONE;
        pNewSegIndexRecord->iStatus = CVI_RECORD_STATUS_INIT;
        pNewSegIndexRecord->tBeginTime = 0;
        pNewSegIndexRecord->tEndTime = 0;
        pNewSegIndexRecord->iInfoCount = 0;
        pNewSegIndexRecord->iFrameStartOffset = pSegIndexRecord->iFrameEndOffset;
        pNewSegIndexRecord->iFrameEndOffset = pSegIndexRecord->iFrameEndOffset;
        
        //上一帧的起始offset就是这个帧的结束offset
        pNewSegIndexRecord->iInfoEndOffset = CVI_RECORD_FRAME_AVINDEX_GET_START_OFFSET(pSegIndexRecord);
        pNewSegIndexRecord->iSegmentNo = pSegIndexRecord->iSegmentNo + 1;
        
        CVI_RECORD_ShowSegmentIndex(pNewSegIndexRecord);
    }

    //重新更新FD
    if(s_mPkgParam.iCurIndexFd > 0)
    {
        close(s_mPkgParam.iCurIndexFd);
        s_mPkgParam.iCurIndexFd = -1;
    }
    
    if(s_mPkgParam.iCurIndexBakFd > 0)
    {
        close(s_mPkgParam.iCurIndexBakFd);
        s_mPkgParam.iCurIndexBakFd = -1;
    }
    
    if(s_mPkgParam.iCurRecordFd > 0)
    {
        close(s_mPkgParam.iCurRecordFd);
        s_mPkgParam.iCurRecordFd = -1;
    }
    if(s_mPkgParam.iCurRecordAvFd > 0)
    {
        close(s_mPkgParam.iCurRecordAvFd);
        s_mPkgParam.iCurRecordAvFd = -1;
    }
    return iRet;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_WriteFrameIndex
 功能描述  :写入录像帧索引
 输入参数  : 
 PT_FRAME_INFO  pstFrameInfo   帧数据参数到文件
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
static CVI_S32 CVI_RECORD_WriteFrameIndex(PT_FRAME_INFO  pstFrameInfo, CVI_S32 bIndexFrame, CVI_S32 bWriteSegment, CVI_S32 bChangeFile)
{
    CVI_S32 bWrite = 0;
    CVI_S32 iRet = 0;
    
    CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecord = NULL;
    pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, s_mPkgParam.iCurrFileRecSegment);
    if (NULL == pSegIndexRecord) {
        _RECORD_ERR("Invalid param\n");
        return RECORD_ERROR;
    }

    CVI_RECORD_SEGMENT_INFO* pSegFrameInfo = NULL;
    CVI_RECORD_SEGMENT_INFO* pSegFrameInfo_Header = &s_mPkgParam.iPkgWriteIndexHead[CVI_RECORD_FRAME_HEADER_INFO_HEAD];
    if (s_mPkgParam.iCurRecordFd <= 0) {
        s_mPkgParam.iCurRecordFd = CVI_RECORD_GetAvFilesFd(s_mPkgParam.iPkgPathName, s_mPkgParam.iCurrFileRecNo, O_WRONLY, NULL);
        if(s_mPkgParam.iCurRecordFd <= 0) {
            _RECORD_ERR("open av fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
            CVI_RECORD_AddCurrentStreamFileBad(1);
            return ERR_OPEN_FILE_ERROR;
        }
    }

    if (NULL != pstFrameInfo) {
        if (0 == pSegIndexRecord->iInfoCount) {
            //第一帧则需要加head数据
            pSegFrameInfo = &s_mPkgParam.iPkgWriteIndexHead[CVI_RECORD_FRAME_HEADER_INFO_HEAD];

            pSegFrameInfo->iSegmentStartCode = CVI_RECORD_SEGMENT_STARTCODE;
            pSegFrameInfo->iInfoType = CVI_RECORD_INFO_TYPE_HEADER;
            pSegFrameInfo->iHeaderInfo.iEvenType = s_mPkgParam.iEvenType;
            pSegFrameInfo->iHeaderInfo.iStatus = CVI_RECORD_STATUS_LOCK;
            pSegFrameInfo->iHeaderInfo.iInfoExNums = CVI_RECORD_FRAME_HEADER_INFO_MAX;
            pSegFrameInfo->iHeaderInfo.iSegmentNo = pSegIndexRecord->iSegmentNo;
            pSegFrameInfo->iHeaderInfo.iIndexFrameNums = 0;
            pSegIndexRecord->iInfoCount++;

            pSegFrameInfo = &s_mPkgParam.iPkgWriteIndexHead[CVI_RECORD_FRAME_HEADER_INFO_KEY0];
            pSegFrameInfo->iSegmentStartCode = CVI_RECORD_SEGMENT_STARTCODE;
            pSegFrameInfo->iInfoType = CVI_RECORD_INFO_TYPE_KEY_INDEX;
            memset(pSegFrameInfo->iIndexInfo.iIndexFrame, 0, sizeof(pSegFrameInfo->iIndexInfo.iIndexFrame));
            pSegIndexRecord->iInfoCount++;

            pSegFrameInfo = &s_mPkgParam.iPkgWriteIndexHead[CVI_RECORD_FRAME_HEADER_INFO_KEY1];
            pSegFrameInfo->iSegmentStartCode = CVI_RECORD_SEGMENT_STARTCODE;
            pSegFrameInfo->iInfoType = CVI_RECORD_INFO_TYPE_KEY_INDEX;
            memset(pSegFrameInfo->iIndexInfo.iIndexFrame, 0, sizeof(pSegFrameInfo->iIndexInfo.iIndexFrame));
            pSegIndexRecord->iInfoCount++;
            s_mPkgParam.iPkgWriteSegmentNum = 0;

            pSegIndexRecord->iEvenType = s_mPkgParam.iEvenType;
            pSegIndexRecord->tBeginTime = pstFrameInfo->iShowTime;
        }

        if ((pSegFrameInfo_Header->iHeaderInfo.iIndexFrameNums >= CVI_RECORD_KEYINDEX_MAX_NUM)
            || (s_mPkgParam.iPkgWriteSegmentNum >= CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_COUNT)) {
            _RECORD_ERR("Invalid Key Index:%d, %d\n", pSegFrameInfo_Header->iHeaderInfo.iIndexFrameNums, s_mPkgParam.iPkgWriteSegmentNum);
            s_mPkgParam.iPkgWriteSegmentNum =0;
            pSegFrameInfo_Header->iHeaderInfo.iIndexFrameNums = 0;
        }

        //写帧索引
        pSegFrameInfo = &s_mPkgParam.iPkgWriteIndexBuff[CVI_RECORD_FRAME_DATA_INFO_WRITE_INDEX(s_mPkgParam.iPkgWriteSegmentNum)];
        pSegFrameInfo->iSegmentStartCode = CVI_RECORD_SEGMENT_STARTCODE;
        pSegFrameInfo->iInfoType = CVI_RECORD_INFO_TYPE_FRAME;
        pSegFrameInfo->iFrameInfo.iFrameType = pstFrameInfo->iFrameType;
        pSegFrameInfo->iFrameInfo.iSubType = pstFrameInfo->iSubType;
        pSegFrameInfo->iFrameInfo.iFrameNo = pstFrameInfo->iFrameNo;
        pSegFrameInfo->iFrameInfo.iStartOffset = pSegIndexRecord->iFrameEndOffset;
        pSegFrameInfo->iFrameInfo.iFrameLen = pstFrameInfo->iFrameLen;
        pSegFrameInfo->iFrameInfo.iFrame_absTime = pstFrameInfo->iFrame_absTime;
        pSegFrameInfo->iFrameInfo.iShowTime = pstFrameInfo->iShowTime;
        pSegIndexRecord->tEndTime = pstFrameInfo->iShowTime;
        pSegIndexRecord->iFrameEndOffset += pstFrameInfo->iFrameLen;

        //修改Head数据
        if(bIndexFrame) {
            CVI_S32 iKeyIndex = CVI_RECORD_FRAME_HEADER_INFO_KEY0 - (pSegFrameInfo_Header->iHeaderInfo.iIndexFrameNums / CVI_RECORD_KEYINDEX_PER_NUMS);
            if(iKeyIndex < CVI_RECORD_FRAME_HEADER_INFO_MAX) {
                pSegFrameInfo = &s_mPkgParam.iPkgWriteIndexHead[iKeyIndex];
                pSegFrameInfo->iIndexInfo.iIndexFrame[pSegFrameInfo_Header->iHeaderInfo.iIndexFrameNums % CVI_RECORD_KEYINDEX_PER_NUMS] = pSegIndexRecord->iInfoCount;
            } else {
                _RECORD_ERR("Invalid Key Index:%d\n", pSegFrameInfo_Header->iHeaderInfo.iIndexFrameNums);
                iRet = RECORD_ERROR;
            }
            pSegFrameInfo_Header->iHeaderInfo.iIndexFrameNums++;
        }
        pSegIndexRecord->iInfoCount++;

        s_mPkgParam.iPkgWriteSegmentNum++;
        if (s_mPkgParam.iPkgWriteSegmentNum >= CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_COUNT)
            bWrite = 1;

        if (pSegFrameInfo_Header->iHeaderInfo.iIndexFrameNums >= CVI_RECORD_KEYINDEX_MAX_NUM) {
            _RECORD_INFO("skip segment,force:index:%d\n", pSegFrameInfo_Header->iHeaderInfo.iIndexFrameNums);
            bWriteSegment = 1;
        }
    }

    if (bWriteSegment || bWrite) {
        if (0 == pSegIndexRecord->iInfoCount) {
            _RECORD_ERR("No frame input, curr:%d,%d\n", s_mPkgParam.iCurrFileRecNo, s_mPkgParam.iCurrFileRecSegment);
        } else {
            CVI_U32 iInfoStartOffset = CVI_RECORD_FRAME_AVINDEX_GET_START_OFFSET(pSegIndexRecord);
            //写完剩余帧数据
            iRet = CVI_RECORD_WriteFrame(NULL, 1, bIndexFrame);
            if (0 != iRet) {
                _RECORD_ERR("write index fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
                pSegIndexRecord->tBeginTime = 0;
                pSegIndexRecord->tEndTime = 0;
                pSegIndexRecord->iInfoCount = 0;
                s_mPkgParam.iPkgWriteSegmentNum = 0;
                return RECORD_ERROR;
            }
            
            //写片段
            if (bWriteSegment) {
                _RECORD_INFO("Write segment,frame(%d), curr:%d,%d\n", pSegIndexRecord->iInfoCount, s_mPkgParam.iCurrFileRecNo, s_mPkgParam.iCurrFileRecSegment);
                //写帧索引头到视频文件
                pSegFrameInfo_Header->iHeaderInfo.iStatus = CVI_RECORD_STATUS_NORMAL;
                iRet = CVI_RECORD_WriteSegmentFrameInfo(s_mPkgParam.iCurRecordFd, CVI_RECORD_FRAME_HEADER_INFO_SEEK(pSegIndexRecord), s_mPkgParam.iPkgWriteIndexHead, CVI_RECORD_FRAME_HEADER_INFO_MAX);
                if (0 != iRet) {
                    _RECORD_ERR("write av fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
                    CVI_RECORD_AddCurrentStreamFileBad(1);
                    pSegIndexRecord->tBeginTime = 0;
                    pSegIndexRecord->tEndTime = 0;
                    pSegIndexRecord->iInfoCount = 0;
                    s_mPkgParam.iPkgWriteSegmentNum = 0;
                    return RECORD_ERROR;
                }

                //写剩余帧索引
                if (s_mPkgParam.iPkgWriteSegmentNum > 0 && (s_mPkgParam.iPkgWriteSegmentNum <= CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_COUNT)) {
                    pSegFrameInfo = &s_mPkgParam.iPkgWriteIndexBuff[CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_COUNT - s_mPkgParam.iPkgWriteSegmentNum];
                    iRet = CVI_RECORD_WriteSegmentFrameInfo(s_mPkgParam.iCurRecordFd, iInfoStartOffset, pSegFrameInfo, s_mPkgParam.iPkgWriteSegmentNum);
                    if (0 != iRet) {
                        _RECORD_ERR("write index fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
                        CVI_RECORD_AddCurrentStreamFileBad(1);
                        pSegIndexRecord->tBeginTime = 0;
                        pSegIndexRecord->tEndTime = 0;
                        pSegIndexRecord->iInfoCount = 0;
                        s_mPkgParam.iPkgWriteSegmentNum = 0;
                        return RECORD_ERROR;
                    }
                } else {
                    _RECORD_INFO("invalid segment frame index write: %d\n", s_mPkgParam.iPkgWriteSegmentNum);
                }

                //写索引
                iRet = CVI_RECORD_WriteCurrentFileIndex(bChangeFile);
                if (0 != iRet) {
                    _RECORD_ERR("write index fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
                    CVI_RECORD_AddCurrentStreamFileBad(1);
                    pSegIndexRecord->tBeginTime = 0;
                    pSegIndexRecord->tEndTime = 0;
                    pSegIndexRecord->iInfoCount = 0;
                    s_mPkgParam.iPkgWriteSegmentNum = 0;
                    return RECORD_ERROR;
                }
                _RECORD_INFO("Write segment End,frame(%d), curr:%d,%d\n", pSegIndexRecord->iInfoCount, s_mPkgParam.iCurrFileRecNo, s_mPkgParam.iCurrFileRecSegment);
            } else {
                if(CVI_RECORD_STATUS_LOCK == pSegFrameInfo_Header->iHeaderInfo.iStatus)
                {
                    //第一次写入时写lock 然后改normal，下次写header是写片段结束
                    pSegFrameInfo_Header->iHeaderInfo.iStatus = CVI_RECORD_STATUS_NORMAL;
                    //写帧索引头到视频文件
                    iRet = CVI_RECORD_WriteSegmentFrameInfo(s_mPkgParam.iCurRecordFd, CVI_RECORD_FRAME_HEADER_INFO_SEEK(pSegIndexRecord), s_mPkgParam.iPkgWriteIndexHead, CVI_RECORD_FRAME_HEADER_INFO_MAX);
                    if(0 != iRet)
                    {
                        _RECORD_ERR("write index fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
                        CVI_RECORD_AddCurrentStreamFileBad(1);
                        pSegIndexRecord->tBeginTime = 0;
                        pSegIndexRecord->tEndTime = 0;
                        pSegIndexRecord->iInfoCount = 0;
                        s_mPkgParam.iPkgWriteSegmentNum = 0;
                        return RECORD_ERROR;
                    }
                }
                
                //写剩余帧索引
                if(s_mPkgParam.iPkgWriteSegmentNum > 0 && (s_mPkgParam.iPkgWriteSegmentNum <= CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_COUNT))
                {
                    _RECORD_INFO("segment frame index write: %d\n", s_mPkgParam.iPkgWriteSegmentNum);
                    pSegFrameInfo = &s_mPkgParam.iPkgWriteIndexBuff[CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_COUNT - s_mPkgParam.iPkgWriteSegmentNum];
                    iRet = CVI_RECORD_WriteSegmentFrameInfo(s_mPkgParam.iCurRecordFd, iInfoStartOffset, pSegFrameInfo, s_mPkgParam.iPkgWriteSegmentNum);
                    if(0 != iRet)
                    {
                        _RECORD_ERR("write index fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
                        CVI_RECORD_AddCurrentStreamFileBad(1);
                        pSegIndexRecord->tBeginTime = 0;
                        pSegIndexRecord->tEndTime = 0;
                        pSegIndexRecord->iInfoCount = 0;
                        s_mPkgParam.iPkgWriteSegmentNum = 0;
                        return RECORD_ERROR;
                    }
                    _RECORD_INFO("segment frame index write End: %d\n", s_mPkgParam.iPkgWriteSegmentNum);
                }
                else
                {
                    _RECORD_INFO("invalid segment frame index write: %d\n", s_mPkgParam.iPkgWriteSegmentNum);
                }
            }
        }
        
        s_mPkgParam.iPkgWriteSegmentNum = 0;
    }
    if(0 > iRet)
    {
        _RECORD_ERR("invalid frame err: %d\n", iRet);
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_WriteFrame
 功能描述  :写入录像帧
 输入参数  : 
 PT_FRAME_INFO  pstFrameInfo   帧数据参数到文件
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
static CVI_S32 CVI_RECORD_WriteFrame(PT_FRAME_INFO  pstFrameInfo, CVI_S32 bFlush, CVI_S32 bIndexFrame)
{
    CVI_S32 iRet = 0;
    CVI_S32 iWriteLen = 0;
    CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecord = NULL;
    pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, s_mPkgParam.iCurrFileRecSegment);
    if (NULL == pSegIndexRecord) {
        _RECORD_ERR("Invalid param\n");
        return RECORD_ERROR;
    }

    if (s_mPkgParam.iCurRecordAvFd <= 0) {
        s_mPkgParam.iCurRecordAvFd = CVI_RECORD_GetAvFilesFd(s_mPkgParam.iPkgPathName, s_mPkgParam.iCurrFileRecNo, O_WRONLY, NULL);
        if(s_mPkgParam.iCurRecordAvFd <= 0) {
            _RECORD_ERR("open fd err,RecNo:%d\n", s_mPkgParam.iCurrFileRecNo);
            CVI_RECORD_AddCurrentStreamFileBad(1);
            return ERR_OPEN_FILE_ERROR;
        }
    }

    if (0 == pSegIndexRecord->iInfoCount) {
        _RECORD_INFO("first info, curr:%d,%d,FrameStartOffset:%d,iInfoEndOffset:%d seek\n", s_mPkgParam.iCurrFileRecNo, s_mPkgParam.iCurrFileRecSegment,
                pSegIndexRecord->iFrameStartOffset, pSegIndexRecord->iInfoEndOffset);
        s_mPkgParam.iPkgSegWriteOffset = pSegIndexRecord->iFrameStartOffset;
        if (-1 == lseek(s_mPkgParam.iCurRecordAvFd, s_mPkgParam.iPkgSegWriteOffset, SEEK_SET)) {
            _RECORD_ERR("pkg seek cannot seek to %u fd:%d\n", s_mPkgParam.iPkgSegWriteOffset, s_mPkgParam.iCurRecordAvFd);
            return ERR_OPEN_FILE_ERROR;
        }
    }

    if (NULL != pstFrameInfo) {
        iWriteLen = CVI_RECORD_WriteBuffer(&s_mPkgParam.iPkgWriteBuff, pstFrameInfo->pBuf, pstFrameInfo->iFrameLen, s_mPkgParam.iCurRecordAvFd, 0);
        if (0 > iWriteLen) {
            CVI_RECORD_AddCurrentStreamFileBad(1);
            _RECORD_ERR("pkg write avFiles ERR, fd %d,RecNo:%d,path:%s,seek:%u\n",
                            s_mPkgParam.iCurRecordAvFd, s_mPkgParam.iCurrFileRecNo, s_mPkgParam.iPkgPathName, s_mPkgParam.iPkgSegWriteOffset);
            return ERR_OPEN_FILE_ERROR;
        } else {
            s_mPkgParam.iPkgSegWriteOffset += iWriteLen;
        }
    }

    if (bFlush) {
        iWriteLen = CVI_RECORD_FlushBuffer(&s_mPkgParam.iPkgWriteBuff, s_mPkgParam.iCurRecordAvFd, 0);
        if(0 > iWriteLen) {
            CVI_RECORD_AddCurrentStreamFileBad(1);
            _RECORD_ERR("pkg flush avFiles ERR, fd %d,RecNo:%d,path:%s,seek:%u\n",
                            s_mPkgParam.iCurRecordAvFd, s_mPkgParam.iCurrFileRecNo, s_mPkgParam.iPkgPathName, s_mPkgParam.iPkgSegWriteOffset);
            return ERR_OPEN_FILE_ERROR;
        } else {
            s_mPkgParam.iPkgSegWriteOffset += iWriteLen;
        }
    }

    if(NULL != pstFrameInfo) {
        iRet = CVI_RECORD_WriteFrameIndex(pstFrameInfo, bIndexFrame, 0, 0);
    }

    return iRet;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_CheckWriteSegment
 功能描述  :检测是否需要写片段
 输入参数  : 
 PT_FRAME_INFO  pstFrameInfo   帧数据参数到文件
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
static CVI_S32 CVI_RECORD_CheckWriteSegment(PT_FRAME_INFO  pstFrameInfo)
{
    CVI_S32 iRet = 0;
    CVI_S32 bWriteSegment = 0;
    CVI_S32 bChangeFile = 0;
    CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecord = NULL;
    pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, s_mPkgParam.iCurrFileRecSegment);
    if (NULL == pSegIndexRecord) {
        _RECORD_ERR("Invalid param\n");
        return RECORD_ERROR;
    }

    if (0 == pSegIndexRecord->iInfoCount) {
        //片段为新片段无需更新片段
        s_mPkgParam.bNewSegment = 0;
        CVI_RECORD_ClearBuffer(&s_mPkgParam.iPkgWriteBuff);
        return 0;
    }
    if(s_mPkgParam.bNewSegment) {
        //强制切换片段
        _RECORD_INFO("skip segment,force\n");
        bWriteSegment = 1;
        s_mPkgParam.bNewSegment = 0;
    } else if (NULL != pstFrameInfo) {
        CVI_S32 iAddLen = 0;
        CVI_S32 iFileRemainStartOffse = 0;
        CVI_S32 iFileRemainEndOffse = 0;
        iAddLen = pstFrameInfo->iFrameLen + sizeof(CVI_RECORD_SEGMENT_INFO); //每帧需要增加的长度
        //剩余控件的起始未为已写入的大小+ 未写入大小
        iFileRemainStartOffse = pSegIndexRecord->iFrameEndOffset;
        iFileRemainEndOffse = CVI_RECORD_FRAME_AVINDEX_GET_START_OFFSET(pSegIndexRecord);
        if ((iFileRemainEndOffse - iFileRemainStartOffse) <= iAddLen) {
            //文件尺寸不足切换片段
            bWriteSegment = 1;
            bChangeFile = 1;
            _RECORD_INFO("skip segment,over size: %d - %d <= %d\n", iFileRemainEndOffse, iFileRemainStartOffse, iAddLen);
        } else if (((pSegIndexRecord->tEndTime - pSegIndexRecord->tBeginTime) < 0)
                    || ((pSegIndexRecord->tEndTime - pSegIndexRecord->tBeginTime) > CVI_RECORD_MAX_PER_SEGMENT_TIME)) {
            //超过最大片段超过时长切换片段
            bWriteSegment = 1;
            _RECORD_INFO("skip segment,over time:%ld-%ld\n", pSegIndexRecord->tBeginTime, pSegIndexRecord->tEndTime);
        }
        else if (((pstFrameInfo->iShowTime - pSegIndexRecord->tEndTime) < 0)
                    || ((pstFrameInfo->iShowTime - pSegIndexRecord->tEndTime) > CVI_RECORD_MAX_PER_SKIP_TIME)) {
            //传入录像时间出现跳秒等情况切换片段
            bWriteSegment = 1;
            _RECORD_INFO("skip segment,invalid time:%ld-%ld\n", pstFrameInfo->iShowTime, pSegIndexRecord->tEndTime);
        }
    }

    if (bWriteSegment) {
        //写片段
        iRet = CVI_RECORD_WriteFrameIndex(NULL, 0, 1, bChangeFile);
    }

    return iRet;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_Init
 功能描述  : 初始化存储库
 输入参数  : CVI_CHAR *pPkgPathName 存储的路径
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_Init(IN CVI_CHAR *pPkgPathName)
{
    CVI_S32 i = 0;
    CVI_CHAR stPathName[CVI_RECORD_MAX_PATH_NAME];
    if (NULL == pPkgPathName) {
        _RECORD_ERR("Invalid Input:NULL\n");
        return ERR_INVALID_ARGUMENT;
    }
    if(s_mPkgParam.iPkginited) {
        _RECORD_ERR("pkgstorage had been init:%s, %s\n", pPkgPathName, s_mPkgParam.iPkgPathName);
        return ERR_SYS_NOT_INIT;
    }
    
    _RECORD_INFO("pkgstorage Init start,path: %s,ver:%s\n", pPkgPathName, szVersion_PKGSTREAM);

    //检测目录是否存在
    struct stat iStbuffer; 
    if (stat(pPkgPathName, &iStbuffer) < 0) {
        _RECORD_ERR("pkg dir no exits0:%s\n", pPkgPathName);
        return ERR_SYS_NOT_INIT;
    }
    
    if (!S_ISDIR(iStbuffer.st_mode)) {
        _RECORD_ERR("pkg dir no exits1:%s\n", pPkgPathName);
        return ERR_SYS_NOT_INIT;
    }

    //初始化数据弹出器
    memset(&s_mDataPoper, 0, sizeof(s_mDataPoper));
    for (i = 0; i < CVI_RECORD_MAX_RECORD_READER; i ++){
        pthread_mutex_init(&s_mDataPoper[i].iPopMutex, NULL);
    }

    memset(&s_mPkgParam, 0, sizeof(CVI_RECORD_PARAM));
    strncpy(s_mPkgParam.iPkgPathName, pPkgPathName, sizeof(s_mPkgParam.iPkgPathName) - 1);

    if(0 != CVI_RECORD_MallocBuffer(&s_mPkgParam.iPkgWriteBuff, PKG_WRITE_BUFFER_MAX_LEN)){
        _RECORD_ERR("pkgstorage malloc err: iPkgWriteBuff %d\n", PKG_WRITE_BUFFER_MAX_LEN);
        return ERR_HANDLE_ALLOC_ERROR;
    }
    
    if(0 != CVI_RECORD_MallocBuffer(&s_mPkgParam.iPkgCurFileIndexBuff, PKG_FILE_INDEX_MAX_LEN)){
        _RECORD_ERR("pkgstorage malloc err: iPkgCurFileIndexBuff %d\n", PKG_FILE_INDEX_MAX_LEN);
        CVI_RECORD_FreeBuffer(&s_mPkgParam.iPkgWriteBuff);
        return ERR_HANDLE_ALLOC_ERROR;
    }
    
    s_mPkgParam.iPkgWriteIndexBuff = (CVI_RECORD_SEGMENT_INFO*)malloc(CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_SIZE);
    if(NULL == s_mPkgParam.iPkgWriteIndexBuff) {
        _RECORD_ERR("pkgstorage malloc err: iPkgWriteIndexBuff %d\n", CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_SIZE);
        CVI_RECORD_FreeBuffer(&s_mPkgParam.iPkgCurFileIndexBuff);
        CVI_RECORD_FreeBuffer(&s_mPkgParam.iPkgWriteBuff);
        return ERR_HANDLE_ALLOC_ERROR;
    }
    s_mPkgParam.iPkgWriteSegmentNum = 0;
    memset((CVI_U8*)s_mPkgParam.iPkgWriteIndexBuff, 0, CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_SIZE);
    CVI_RECORD_FREE_CACHE;
    
    snprintf(stPathName, sizeof(stPathName), "%s/"CVI_RECORD_FORMAT_FLAG, s_mPkgParam.iPkgPathName);
    if (0 == access(stPathName, F_OK)) {
        _RECORD_ERR("pkgstorage format err flag %s\n", stPathName);
        s_mPkgParam.iPkgStatus = CVI_STORAGE_STATUS_ERROR;
    } else {
        s_mPkgParam.iPkgStatus = CVI_STORAGE_STATUS_NORMAL;
    }

    //检测是否有索引等关键文件
    if (CVI_STORAGE_STATUS_NORMAL == s_mPkgParam.iPkgStatus) {
        //检测3个视频文件如果都不存在则没格式化状态
        s_mPkgParam.iPkgStatus = CVI_STORAGE_STATUS_UNINIT;
        for (i = 0; i < CVI_RECORD_MAX_MISS_AVFILES; i++) {
            snprintf(stPathName, sizeof(stPathName), "%s/"CVI_RECORD_AV_FILE, s_mPkgParam.iPkgPathName, i);
            if((0 == access(stPathName, F_OK))) {
                s_mPkgParam.iPkgStatus = CVI_STORAGE_STATUS_NORMAL;
                break;
            } else {
                _RECORD_ERR("pkgstorage init,no find %s\n", stPathName);
            }
        }
    }

    if(CVI_STORAGE_STATUS_NORMAL == s_mPkgParam.iPkgStatus) {
        if (0 != CVI_RECORD_CheckInit()) {
            _RECORD_ERR("CVI_RECORD_CheckInit err\n");
            s_mPkgParam.iPkgStatus = CVI_STORAGE_STATUS_ERROR;
        } else {
            _RECORD_INFO("CVI_RECORD_CheckInit normal\n");
        }
    }

    s_mPkgParam.iPkgRunMode = CVI_RECORD_RUN_MODE_NORMAL;
    s_mPkgParam.iWriteMode = CVI_RECORD_WRITE_MODE_ALL;
    s_mPkgParam.iPreTimes = CVI_RECORD_PRE_RECORD_TIMES;
    s_mPkgParam.iEvenType = CVI_RECORD_EVEN_NONE;
    s_mPkgParam.bWaitKeyFrame = 1;

    s_mPkgParam.iPkginited = 1;
    s_mPkgParam.bPkgstop = 0;
    _RECORD_INFO("pkgstorage Init End\n");
    return 0;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_Uninit
 功能描述  : 去初始化存储库
 输入参数  : 
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_Uninit()
{
    CVI_RECORD_FUNC_START;
    CVI_S32 i = 0;
    if (!s_mPkgParam.iPkginited) {
        _RECORD_ERR("pkgstorage had been uninit\n");
        CVI_RECORD_FUNC_END;
        return -2;
    }

    _RECORD_INFO("pkgstorage unInit start,path: %s\n", s_mPkgParam.iPkgPathName);

    pthread_mutex_lock(&s_iPkgRunMutex);
    s_mPkgParam.bPkgstop = 1;
    pthread_mutex_unlock(&s_iPkgRunMutex);
    
    for (i = 0; i < CVI_RECORD_MAX_RECORD_READER; i++) {
        CVI_REPLAY_Release(&s_mDataPoper[i]);
        pthread_mutex_destroy(&s_mDataPoper[i].iPopMutex);
    }

    //调用一次锁避免外部有使用锁的位置出现异常
    pthread_mutex_lock(&s_iPkgRunMutex);

    if (s_mPkgParam.iPkgWriteIndexBuff) {
        free(s_mPkgParam.iPkgWriteIndexBuff);
        s_mPkgParam.iPkgWriteIndexBuff = NULL;
    }
    CVI_RECORD_FreeBuffer(&s_mPkgParam.iPkgCurFileIndexBuff);
    CVI_RECORD_FreeBuffer(&s_mPkgParam.iPkgWriteBuff);

    //重新更新FD
    if (s_mPkgParam.iCurIndexFd > 0) {
        close(s_mPkgParam.iCurIndexFd);
        s_mPkgParam.iCurIndexFd = -1;
    }

    //重新更新FD
    if (s_mPkgParam.iCurIndexBakFd > 0) {
        close(s_mPkgParam.iCurIndexBakFd);
        s_mPkgParam.iCurIndexBakFd = -1;
    }

    if (s_mPkgParam.iCurRecordFd > 0) {
        close(s_mPkgParam.iCurRecordFd);
        s_mPkgParam.iCurRecordFd = -1;
    }
    if (s_mPkgParam.iCurRecordAvFd > 0) {
        close(s_mPkgParam.iCurRecordAvFd);
        s_mPkgParam.iCurRecordAvFd = -1;
    }

    memset(&s_mPkgParam, 0, sizeof(CVI_RECORD_PARAM));
    pthread_mutex_unlock(&s_iPkgRunMutex);
    CVI_RECORD_FUNC_END;
    return 0;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_Format
 功能描述  : 格式化存储路径
 输入参数  : CVI_U32 iFileSize录像分区最大支持尺寸单位MB
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_Format(CVI_U32 iFileSize)
{
    CVI_S32 iMinFilesSize = 0;
    CVI_S32 iRemainIndexFileSize = 0;
    CVI_S32 iMaxAVFileNum = 0;
    CVI_S32 iMaxIndexNum = 0;
    CVI_S32 i = 0;
    CVI_S32 j = 0;
    CVI_S32 fd = -1;
    CVI_S32 iRet = -1;
    CVI_S32 iWriteTotal = 0;
    CVI_RECORD_FILE_INDEX_HEADER stFileIndexHeader;
    CVI_RECORD_FILE_INDEX_RECORD stFileIndexRecord;
    CVI_RECORD_SEGMENT_INDEX_RECORD stSegIndexRecord;
    CVI_RECORD_SEGMENT_INFO stSegFrameInfo;
    CVI_CHAR stPathName[CVI_RECORD_MAX_PATH_NAME];
    CVI_CHAR stPathNameBak[CVI_RECORD_MAX_PATH_NAME];
    CVI_U8 stWriteBuf[1024];
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    if (s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    //剩余多少空间创建avfile + 索引index
    iMinFilesSize = (CVI_RECORD_REMAIN_FILE_MIN_SIZE + CVI_RECORD_INDEX_FILE_COUNT*CVI_RECORD_INDEX_FILE_MAX_SIZE)/CVI_RECORD_CHAR_TO_MB; //最少文件大小
    iRemainIndexFileSize = iFileSize - iMinFilesSize;

    if (iRemainIndexFileSize < (2*CVI_RECORD_AV_FILE_MAX_SIZE/CVI_RECORD_CHAR_TO_MB)) {
        _RECORD_ERR("Invalid FileSize, min:%u, set:%u\n", iMinFilesSize + (2*CVI_RECORD_AV_FILE_MAX_SIZE/CVI_RECORD_CHAR_TO_MB), iFileSize);
        return ERR_INVALID_ARGUMENT;
    }

    //视频文件数
    iMaxAVFileNum = 0;
    iMaxIndexNum = 1;
    while (iRemainIndexFileSize >= (CVI_RECORD_AV_FILE_MAX_SIZE/CVI_RECORD_CHAR_TO_MB)) {
        iMaxAVFileNum++;
        iRemainIndexFileSize -= (CVI_RECORD_AV_FILE_MAX_SIZE/CVI_RECORD_CHAR_TO_MB);
        if (0 == (iMaxAVFileNum % CVI_RECORD_INDEX_MAX_AVFILES)) {
            iMaxIndexNum++;
            iRemainIndexFileSize -= (CVI_RECORD_INDEX_FILE_COUNT*CVI_RECORD_INDEX_FILE_MAX_SIZE/CVI_RECORD_CHAR_TO_MB);
        }
    }

    //索引文件数重新计算可能刚好没有视频文件了
    iMaxIndexNum = CVI_RECORD_FILES_GET_INDEX_NUM(iMaxAVFileNum);
    if(CVI_RECORD_FILES_GET_INDEX_FILES(iMaxAVFileNum) > 0)
        iMaxIndexNum++;

    _RECORD_INFO("pkg format FileSize %u, Index:%d,AVFiles: %d\n", iFileSize, iMaxIndexNum, iMaxAVFileNum);    
    _RECORD_INFO("pkg format size file_index_header: %u,file_index_record: %u,Segment_index_record: %u,Segment_frame_info: %u\n",
            sizeof(CVI_RECORD_FILE_INDEX_HEADER), sizeof(CVI_RECORD_FILE_INDEX_RECORD), sizeof(CVI_RECORD_SEGMENT_INDEX_RECORD), sizeof(CVI_RECORD_SEGMENT_INFO));
    pthread_mutex_lock(&s_iPkgRunMutex);
    if ((0 == s_mPkgParam.iPkginited)
        || (s_mPkgParam.bPkgstop)
        || (CVI_STORAGE_STATUS_FORMAT == s_mPkgParam.iPkgStatus)
        || (CVI_RECORD_RUN_MODE_STOP != s_mPkgParam.iPkgRunMode)
        || (0 == CVI_RECORD_DirIsExist(s_mPkgParam.iPkgPathName))) {
        _RECORD_ERR("Pkg mode InvalidEx, init:%d,sta:%d run:%d, path:%s\n", s_mPkgParam.iPkginited, s_mPkgParam.iPkgStatus, s_mPkgParam.iPkgRunMode, s_mPkgParam.iPkgPathName);
        iRet = ERR_SYS_NOT_INIT;
    } else {
        s_mPkgParam.iPkgFormatPrecent = 0;
        s_mPkgParam.iPkgStatus = CVI_STORAGE_STATUS_FORMAT;
        CVI_RECORD_ClearBuffer(&s_mPkgParam.iPkgWriteBuff);
        memset(stWriteBuf, 0, sizeof(stWriteBuf));
        CVI_RECORD_FREE_CACHE;

        snprintf(stPathName, sizeof(stPathName), "%s/"CVI_RECORD_FORMAT_FLAG, s_mPkgParam.iPkgPathName);
        fd = open(stPathName, O_RDWR | O_CREAT);
        if (fd > 0) {
            write(fd, "1", 1);
            close(fd);
            fd = -1;
        } else {
            _RECORD_ERR("create format flag file:%s,%s\n", stPathName, strerror(errno));
        }

        usleep(CVI_RECORD_FORMAT_PRE_WRITE_INT);

        //创建视频文件
        for (i = 0; i < iMaxAVFileNum; i++) {
            if(s_mPkgParam.bPkgstop) {
                _RECORD_ERR("sd card stop format file\n");
                iRet = ERR_INVALID_ARGUMENT;
                goto funcEnd;
            }

            iWriteTotal = 0;
            snprintf(stPathName, sizeof(stPathName), "%s/"CVI_RECORD_AV_FILE, s_mPkgParam.iPkgPathName, i);
            iRet = CVI_RECORD_PreFormatFiles(stPathName, CVI_RECORD_AV_FILE_MAX_SIZE);
            if (0 != iRet) {
                _RECORD_ERR("pre format file err:%s, %u, ret:%d\n", stPathName, CVI_RECORD_AV_FILE_MAX_SIZE, iRet);
                iRet = ERR_INVALID_ARGUMENT;
                goto funcEnd;
            }
            usleep(CVI_RECORD_FORMAT_PRE_CREATE_INT);
            s_mPkgParam.iPkgFormatPrecent = (80 * (i + 1) / iMaxAVFileNum);
        }
        s_mPkgParam.iPkgFormatPrecent = 80;

        //创建索引文件
        for (i = 0; i < iMaxIndexNum; i++) {
            if(s_mPkgParam.bPkgstop) {
                _RECORD_ERR("sd card stop format file\n");
                iRet = ERR_INVALID_ARGUMENT;
                goto funcEnd;
            }
            usleep(CVI_RECORD_FORMAT_PRE_WRITE_INT);

            iWriteTotal = 0;
            snprintf(stPathName, sizeof(stPathName), "%s/"CVI_RECORD_INDEX_FILE, s_mPkgParam.iPkgPathName, i);
            snprintf(stPathNameBak, sizeof(stPathNameBak), "%s/"CVI_RECORD_INDEX_FILE_BAK, s_mPkgParam.iPkgPathName, i);
            iRet = CVI_RECORD_PreFormatFiles(stPathName, CVI_RECORD_INDEX_FILE_MAX_SIZE);
            if (0 != iRet) {
                _RECORD_ERR("pre format file err:%s, %u, ret:%d\n", stPathName, CVI_RECORD_INDEX_FILE_MAX_SIZE, iRet);
                iRet = ERR_INVALID_ARGUMENT;
                goto funcEnd;
            }
            usleep(CVI_RECORD_FORMAT_PRE_CREATE_INT);

            fd = open(stPathName, O_RDWR);
            if (fd < 0) {
                _RECORD_ERR("open file err:%s, ret:%d\n", stPathName, fd);
                iRet = ERR_INVALID_ARGUMENT;
                goto funcEnd;
            }
            _RECORD_INFO("pkg write index Files: %s, files:%d\n", stPathName, iMaxAVFileNum);

            //写 CVI_RECORD_FILE_INDEX_HEADER（录像文件索引头）
            CVI_RECORD_GetInitFileIndexHeader(&stFileIndexHeader, iMaxAVFileNum);
            if (0 > CVI_RECORD_WriteBuffer(&s_mPkgParam.iPkgWriteBuff, (CVI_U8*)&stFileIndexHeader, sizeof(stFileIndexHeader), fd, 0)) {
                _RECORD_ERR("pkg format stFileIndexHeader ERR, size %d\n", sizeof(stFileIndexHeader));
                iRet = ERR_INVALID_ARGUMENT;
                goto funcEnd;
            }

            iWriteTotal += sizeof(stFileIndexHeader);

            //写CVI_RECORD_FILE_INDEX_RECORD（对应文件的录像索引数据 每个录像文件Rec00XXX.tps都有1个对应索引数据 ）
            CVI_RECORD_GetInitFileIndex(&stFileIndexRecord, CVI_RECORD_STATUS_INIT);
            for (j = 0; j < CVI_RECORD_INDEX_MAX_AVFILES; j++) {
                iRet = CVI_RECORD_WriteBuffer(&s_mPkgParam.iPkgWriteBuff, (CVI_U8*)&stFileIndexRecord, sizeof(stFileIndexRecord), fd, 0);
                if (0 > iRet) {
                    _RECORD_ERR("pkg format stFileIndexRecord ERR, size %d\n", sizeof(stFileIndexRecord));
                    iRet = ERR_INVALID_ARGUMENT;
                    goto funcEnd;
                } else if (iRet > 0) {
                    usleep(CVI_RECORD_FORMAT_PRE_WRITE_INT);
                }
                iWriteTotal += sizeof(stFileIndexRecord);
            }

            if (s_mPkgParam.bPkgstop) {
                _RECORD_ERR("sd card stop format file\n");
                iRet = ERR_INVALID_ARGUMENT;
                goto funcEnd;
            }
            //写CVI_RECORD_FILE_INDEX_RECORD（对应文件的录像索引数据 每个录像文件Rec00XXX.tps都有1个对应索引数据 ）
            CVI_RECORD_GetInitSegmentIndex(&stSegIndexRecord, CVI_RECORD_STATUS_INIT);
            for (j = 0; j < CVI_RECORD_INDEX_MAX_AVFILES * CVI_RECORD_AV_FILE_MAX_SEGMENT; j++) {
                iRet = CVI_RECORD_WriteBuffer(&s_mPkgParam.iPkgWriteBuff, (CVI_U8*)&stSegIndexRecord, sizeof(stSegIndexRecord), fd, 0);
                if (0 > iRet) {
                    _RECORD_ERR("pkg format stFileIndexRecord ERR, size %d\n", sizeof(stSegIndexRecord));
                    iRet = ERR_INVALID_ARGUMENT;
                    goto funcEnd;
                } else if (iRet > 0) {
                    usleep(CVI_RECORD_FORMAT_PRE_WRITE_INT);
                }
                iWriteTotal += sizeof(stSegIndexRecord);

                if (s_mPkgParam.bPkgstop) {
                    _RECORD_ERR("sd card stop format file\n");
                    iRet = ERR_INVALID_ARGUMENT;
                    goto funcEnd;
                }
            }

            if (CVI_RECORD_INDEX_FILE_MAX_SIZE > iWriteTotal) {
                CVI_S32 iReMainBuf = CVI_RECORD_INDEX_FILE_MAX_SIZE - iWriteTotal;
                CVI_S32 iWrite = 0;
                _RECORD_INFO("pkg write Remain Files End: iWriteTotal:%d\n", iReMainBuf);
                do
                {
                    iWrite = (iReMainBuf > (CVI_S32)sizeof(stWriteBuf)) ? (CVI_S32)sizeof(stWriteBuf) : iReMainBuf;
                    if (0 > CVI_RECORD_WriteBuffer(&s_mPkgParam.iPkgWriteBuff, stWriteBuf, iWrite, fd, 0)) {
                        _RECORD_ERR("pkg format stFileIndexRecord ERR, size %d\n", iWrite);
                        iRet = ERR_INVALID_ARGUMENT;
                        goto funcEnd;
                    }
                    iReMainBuf -= iWrite;
                    iWriteTotal += iWrite;
                }while (iReMainBuf > 0);
                usleep(CVI_RECORD_FORMAT_PRE_WRITE_INT);
            }

            if (0 > CVI_RECORD_FlushBuffer(&s_mPkgParam.iPkgWriteBuff, fd, 0)) {
                _RECORD_ERR("pkg format avFiles ERR, size %d\n", sizeof(stSegFrameInfo));
                iRet = ERR_INVALID_ARGUMENT;
                goto funcEnd;
            }
            _RECORD_DBG("pkg write index Files End: %s->%s,iWriteTotal:%d\n", stPathName, stPathNameBak, iWriteTotal);
            close(fd);
            fd = -1;
            s_mPkgParam.iPkgFormatPrecent = 80 + (20 * (i + 1) / iMaxIndexNum) / 2;

            if (s_mPkgParam.bPkgstop) {
                _RECORD_ERR("sd card stop format file\n");
                iRet = ERR_INVALID_ARGUMENT;
                goto funcEnd;
            }

            if (0 != CVI_RECORD_CopyFile(stPathName, stPathNameBak, 0)) {
                _RECORD_ERR("pkg format avFiles ERR, size %d\n", sizeof(stSegFrameInfo));
                iRet = ERR_INVALID_ARGUMENT;
                goto funcEnd;
            }
            _RECORD_DBG("pkg write index Files success: %s->%s\n", stPathName, stPathNameBak);

            s_mPkgParam.iPkgFormatPrecent = 80 + (20 * (i+1)/iMaxIndexNum);
        }

        iRet = 0;
funcEnd:
        if (fd > 0) {
            close(fd);
            fd = -1;
        }

        CVI_RECORD_ClearBuffer(&s_mPkgParam.iPkgWriteBuff);
        if (0 == iRet) {
            iRet = CVI_RECORD_LoadCurrentInfo(0);
            if (0 == iRet) {
                snprintf(stPathName, sizeof(stPathName), "%s/"CVI_RECORD_FORMAT_FLAG, s_mPkgParam.iPkgPathName);
                remove(stPathName);
                usleep(200 * 1000);
                if (0 == access(stPathName, F_OK)) {
                    _RECORD_ERR("sd card del flag err:%s, try\n", stPathName);
                    remove(stPathName);
                    sleep(1);
                    if (0 == access(stPathName, F_OK)) {
                        remove(stPathName);
                        _RECORD_ERR("sd card del flag err:%s, err\n", stPathName);
                    }
                }
                s_mPkgParam.iPkgStatus = CVI_RECORD_STATUS_NORMAL;
                s_mPkgParam.iPkgFormatPrecent = 100;
            } else {
                s_mPkgParam.iPkgStatus = CVI_STORAGE_STATUS_ERROR;
            }
            s_mPkgParam.iReMainAVFileNum = s_mPkgParam.iMaxAVFileNum;
            //重新加载
        } else {
            _RECORD_ERR("pkg format err, del file\n");
            s_mPkgParam.iPkgStatus = CVI_STORAGE_STATUS_ERROR;
        }
        s_mPkgParam.bWaitKeyFrame = 1;
        CVI_RECORD_FREE_CACHE;
    }

    pthread_mutex_unlock(&s_iPkgRunMutex);
    return iRet;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_Resume
 功能描述  :设置存储库状态开始
 输入参数  : 
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_Resume()
{
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        return ERR_SYS_NOT_INIT;
    }
    
    if (s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    if(CVI_RECORD_RUN_MODE_NORMAL == s_mPkgParam.iPkgRunMode) {
        return 0;
    }
    pthread_mutex_lock(&s_iPkgRunMutex);
    s_mPkgParam.iPkgRunMode = CVI_RECORD_RUN_MODE_NORMAL;
    s_mPkgParam.bNewSegment = 1;
    pthread_mutex_unlock(&s_iPkgRunMutex);
    _RECORD_ERR("pkg storage resume\n");
    return 0;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_Pause
 功能描述  :设置存储库状态停止
 输入参数  : 
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_Pause()
{
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        return ERR_SYS_NOT_INIT;
    }
    
    if (s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    if (CVI_RECORD_RUN_MODE_STOP == s_mPkgParam.iPkgRunMode)
        return 0;

    pthread_mutex_lock(&s_iPkgRunMutex);
    
    s_mPkgParam.bNewSegment = 1;
    if ((CVI_RECORD_RUN_MODE_NORMAL == s_mPkgParam.iPkgRunMode)
         && (CVI_STORAGE_STATUS_NORMAL == s_mPkgParam.iPkgStatus))
        if(0 != CVI_RECORD_CheckWriteSegment(NULL))
            _RECORD_ERR("check write Frame Err\n");

    s_mPkgParam.iPkgRunMode = CVI_RECORD_RUN_MODE_STOP;
    //暂停的时候强制写片段避免重启等情况丢录像
    pthread_mutex_unlock(&s_iPkgRunMutex);
    _RECORD_ERR("pkg storage pause\n");
    return 0;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_dataInput
 功能描述  :输入录像数据
 输入参数  : 
 PT_FRAME_INFO  pstFrameInfo   帧数据参数
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_DataInput(PT_FRAME_INFO  pstFrameInfo)
{
    CVI_S32 iRet = 0;
    CVI_S32 bKeyFrame = 0;
    CVI_S32 bIndexFrame = 0;
    static CVI_U32 iLastVideoNo = 0;
    static CVI_U32 iLastAudioNo = 0;
    static time_t iLastVideoTime = 0;
    static time_t iLastAudioTime = 0;
    
    static CVI_U64 iLastVideoAbsTime = 0;
    static CVI_U64 iLastAudioAbsTime = 0;
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        return ERR_SYS_NOT_INIT;
    }

    if (s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    if (NULL == pstFrameInfo) {
        _RECORD_ERR("Invalid Input NULL\n");
        return ERR_SYS_NOT_INIT;
    }

    if ((NULL == pstFrameInfo->pBuf)
        || (0 >= pstFrameInfo->iFrameLen)
        || (PKG_FRAME_MAX_LEN < pstFrameInfo->iFrameLen)
        || (0 == CVI_RECORD_CheckFrameValid(pstFrameInfo->iFrameType, pstFrameInfo->iSubType, &bKeyFrame, &bIndexFrame))) {
        _RECORD_ERR("Invalid Input Frame, pBuf:%p, iLen:%u\n", pstFrameInfo->pBuf, pstFrameInfo->iFrameLen);
        return ERR_SYS_NOT_INIT;
    }

    pthread_mutex_lock(&s_iPkgRunMutex);

    if ((CVI_RECORD_RUN_MODE_NORMAL != s_mPkgParam.iPkgRunMode) || (CVI_STORAGE_STATUS_NORMAL != s_mPkgParam.iPkgStatus)) {
        //_RECORD_ERR("Pkg not normal, run:%d, sta:%d\n", s_mPkgParam.iPkgRunMode, s_mPkgParam.iPkgStatus);
        iRet = -1;
        goto endFunc;
    }

    if (CVI_RECORD_FRAME_TYPE_G711U == pstFrameInfo->iFrameType) {
        if (pstFrameInfo->iFrameNo != iLastAudioNo + 1) {
            _RECORD_ERR("[skip Audio]write Frame no:%u->%u, time:%lu->%lu,%lu, rTime:%llu->%llu\n", iLastAudioNo, pstFrameInfo->iFrameNo,
                iLastAudioTime, pstFrameInfo->iShowTime, time(NULL),
                iLastAudioAbsTime, pstFrameInfo->iFrame_absTime);
        }
        iLastAudioNo = pstFrameInfo->iFrameNo;
        iLastAudioTime = pstFrameInfo->iShowTime;
        iLastAudioAbsTime = pstFrameInfo->iFrame_absTime;
    } else {
        if(pstFrameInfo->iFrameNo != iLastVideoNo+1) {
            _RECORD_ERR("[skip video]write Frame no:%u->%u, time:%lu->%lu,%lu, rTime:%llu->%llu\n", iLastVideoNo, pstFrameInfo->iFrameNo,
                iLastVideoTime, pstFrameInfo->iShowTime, time(NULL),
                iLastVideoAbsTime, pstFrameInfo->iFrame_absTime);
        }
        iLastVideoNo = pstFrameInfo->iFrameNo;
        iLastVideoTime = pstFrameInfo->iShowTime;
        iLastVideoAbsTime = pstFrameInfo->iFrame_absTime;
    }

    if (0 != CVI_RECORD_CheckWriteSegment(pstFrameInfo)) {
        _RECORD_ERR("check write Frame Err\n");
        iRet = -4;
        goto endFunc;
    }

    if(s_mPkgParam.bWaitKeyFrame) {
        if(bKeyFrame) {
            s_mPkgParam.bWaitKeyFrame = 0;
        } else {
            _RECORD_ERR("Pkg wait key frame,type(%d,%d)\n", pstFrameInfo->iFrameType, pstFrameInfo->iSubType);
            iRet = -2;
            goto endFunc;
        }
    }

    iRet = CVI_RECORD_WriteFrame(pstFrameInfo, 0, bIndexFrame);
    if (0 != iRet) {
        _RECORD_ERR("Write Frame Err\n");
        iRet = -4;
        goto endFunc;
    }

endFunc:

    pthread_mutex_unlock(&s_iPkgRunMutex);
    return iRet;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_GetStatus
 功能描述  :获取存储库状态
 输入参数  : 
 输出参数  : 
 OUT CVI_STORAGE_STATUS_E* iPKGStatus 获取状态 OUT CVI_S32 iPrecent格式化百分比
 OUT CVI_U32 *iTotalSize, OUT CVI_U32 *iRemainSize 总容量及剩余容量单位MB
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_GetStatus(OUT CVI_STORAGE_STATUS_E* iPKGStatus, OUT CVI_S32* iPrecent, OUT CVI_U32 *iTotalSize, OUT CVI_U32 *iRemainSize)
{
    if (0 == s_mPkgParam.iPkginited) {
        //_RECORD_ERR("Pkg not init\n");
        return ERR_SYS_NOT_INIT;
    }

    if (s_mPkgParam.bPkgstop) {
        //_RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return RECORD_ERROR;
    }

    if (NULL == iPKGStatus) {
        _RECORD_ERR("Invalid Input\n");
        return RECORD_ERROR;
    }

    *iPKGStatus = s_mPkgParam.iPkgStatus;

    if (NULL != iPrecent) {
        if (CVI_STORAGE_STATUS_FORMAT == *iPKGStatus)
            *iPrecent = s_mPkgParam.iPkgFormatPrecent;
        else
            *iPrecent = 0;
    }
    if (NULL != iTotalSize)
        *iTotalSize = s_mPkgParam.iMaxAVFileNum * (CVI_RECORD_AV_FILE_MAX_SIZE/CVI_RECORD_CHAR_TO_MB);
    if (NULL != iRemainSize)
        *iRemainSize = s_mPkgParam.iReMainAVFileNum * (CVI_RECORD_AV_FILE_MAX_SIZE/CVI_RECORD_CHAR_TO_MB);
    return 0;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_SetWriteMode
 功能描述  : 存储库写模式
 输入参数  : IN CVI_RECORD_WRITE_MODE_E iWriteMode 不录像，事件录像，全天录像
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_SetWriteMode(IN CVI_RECORD_WRITE_MODE_E iWriteMode)
{
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        return ERR_SYS_NOT_INIT;
    }
    
    if (s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }
    
    if (iWriteMode == s_mPkgParam.iWriteMode)
        return 0;

    pthread_mutex_lock(&s_iPkgRunMutex);
    s_mPkgParam.iWriteMode = iWriteMode;
    pthread_mutex_unlock(&s_iPkgRunMutex);

    return 0;
}


/*****************************************************************************
 函 数 名  : CVI_RECORD_SetPreRecordTime
 功能描述  :设置预录时间
 输入参数  : IN CVI_U32 iPreTimes 预录时间
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_SetPreRecordTime(IN CVI_U32 iPreTimes)
{
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        return ERR_SYS_NOT_INIT;
    }

    if (s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    if (iPreTimes == s_mPkgParam.iPreTimes)
        return 0;

    pthread_mutex_lock(&s_iPkgRunMutex);
    s_mPkgParam.iPreTimes = iPreTimes;
    pthread_mutex_unlock(&s_iPkgRunMutex);
    return 0;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_StartEvent
 功能描述  :触发事件
 输入参数  : IN CVI_S32 iEvenType 触发的事件类型
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_StartEvent(IN CVI_RECORD_EVENT_MASK_E iEvenMask)
{
    CVI_U32 iEvenType = 0;
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        return ERR_SYS_NOT_INIT;
    }

    if (s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    iEvenType = CVI_RECORD_EVEN_SET_MASK(s_mPkgParam.iEvenType, iEvenMask);
    if (iEvenType == s_mPkgParam.iEvenType)
        return 0;

    pthread_mutex_lock(&s_iPkgRunMutex);
    _RECORD_INFO("Record iEvemType start %d->%d\n", s_mPkgParam.iEvenType, iEvenType);
    s_mPkgParam.bNewSegment = 1;
    if ((CVI_RECORD_RUN_MODE_NORMAL == s_mPkgParam.iPkgRunMode) && (CVI_STORAGE_STATUS_NORMAL == s_mPkgParam.iPkgStatus)) {
        if (0 != CVI_RECORD_CheckWriteSegment(NULL))
            _RECORD_ERR("check write Frame Err\n");
    }
    s_mPkgParam.iEvenType = CVI_RECORD_EVEN_SET_MASK(s_mPkgParam.iEvenType, iEvenMask);
    pthread_mutex_unlock(&s_iPkgRunMutex);

    return 0;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_StopEvent
 功能描述  :结束事件
 输入参数  : IN CVI_S32 iEvenType 触发的事件类型
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_StopEvent(IN CVI_RECORD_EVENT_MASK_E iEvenMask)
{
    CVI_U32 iEvenType = 0;
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        return ERR_SYS_NOT_INIT;
    }

    if(s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    if (CVI_RECORD_EVENT_ALL_ALARM_MASK == iEvenMask)
        iEvenType = CVI_RECORD_EVEN_NONE;
    else
        iEvenType = CVI_RECORD_EVEN_CLEAR_MASK(s_mPkgParam.iEvenType, iEvenMask);

    if (iEvenType == s_mPkgParam.iEvenType)
        return 0;

    pthread_mutex_lock(&s_iPkgRunMutex);
    _RECORD_INFO("Record iEvemType stop %d->%d\n", s_mPkgParam.iEvenType, iEvenType);
    s_mPkgParam.bNewSegment = 1;
    if ((CVI_RECORD_RUN_MODE_NORMAL == s_mPkgParam.iPkgRunMode) && (CVI_STORAGE_STATUS_NORMAL == s_mPkgParam.iPkgStatus)) {
        if (0 != CVI_RECORD_CheckWriteSegment(NULL))
            _RECORD_ERR("check write Frame Err\n");
    }
    if (CVI_RECORD_EVENT_ALL_ALARM_MASK == iEvenMask)
        s_mPkgParam.iEvenType = CVI_RECORD_EVEN_NONE;
    else
        s_mPkgParam.iEvenType = CVI_RECORD_EVEN_CLEAR_MASK(s_mPkgParam.iEvenType, iEvenMask);

    pthread_mutex_unlock(&s_iPkgRunMutex);

    return 0;
}

/*****************************************************************************
 函 数 名  : CVI_RECORD_show_file_info
 功能描述  :设置存储库状态停止
 输入参数  : 
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_ShowRecordInfo(CVI_U32 iCurrFileRecNo)
{
    if (0 == s_mPkgParam.iPkginited || iCurrFileRecNo >= (CVI_U32)s_mPkgParam.iMaxAVFileNum) {
        _RECORD_ERR("Pkg not init\n");
        return ERR_SYS_NOT_INIT;
    }

    if (iCurrFileRecNo >= (CVI_U32)s_mPkgParam.iMaxAVFileNum) {
        _RECORD_ERR("Invalid RecNo:%d, max:%d\n", iCurrFileRecNo, s_mPkgParam.iMaxAVFileNum);
        return ERR_SYS_NOT_INIT;
    }
    CVI_S32 iRet = 0;
    CVI_S32 iIndexfd = -1;
    CVI_S32 index = 0;
    CVI_S32 i = 0;
    CVI_S32 iFilefd = -1;
    CVI_S32 iSeek = 256*1024*1024;
    CVI_RECORD_FILE_INDEX_HEADER* pFileIndexHeader = NULL;
    CVI_RECORD_FILE_INDEX_RECORD* pFileIndexRecord = NULL;
    CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecord = NULL;
    index = CVI_RECORD_FILES_GET_INDEX_NUM(iCurrFileRecNo);
    
    _RECORD_ERR("t_pkgstorage show RecNo:%d,index:%d Info\n", iCurrFileRecNo, index);

    iIndexfd = CVI_RECORD_GetIndexFileFd(s_mPkgParam.iPkgPathName, index, O_RDWR, NULL);
    if (0 != CVI_RECORD_GetFileIndex(&s_mPkgParam.iPkgCurFileIndexBuff, iIndexfd, iCurrFileRecNo)) {
        _RECORD_ERR("CVI_RECORD_GetFileIndex err\n");
        iRet = ERR_INVALID_ARGUMENT;
        goto endFunc;
    } else {
        pFileIndexHeader = CVI_RECORD_GetFileIndexHeader(&s_mPkgParam.iPkgCurFileIndexBuff);
        pFileIndexRecord = CVI_RECORD_GetFileIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, iCurrFileRecNo);
        CVI_RECORD_ShowFileIndexHeader(pFileIndexHeader);
        CVI_RECORD_ShowFileIndex(pFileIndexRecord);

        //检测剩余片段是否有效
        for (i = 0; i < CVI_RECORD_AV_FILE_MAX_SEGMENT; i++) {
            pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&s_mPkgParam.iPkgCurFileIndexBuff, i);
            if (NULL == pSegIndexRecord) {
                iRet = ERR_INVALID_ARGUMENT;
                break;
            } else {
                CVI_RECORD_ShowSegmentIndex(pSegIndexRecord);
            }
        }
    }

    CVI_RECORD_SEGMENT_INFO  iPkgWriteIndexBuff;
    iFilefd = CVI_RECORD_GetAvFilesFd(s_mPkgParam.iPkgPathName, iCurrFileRecNo, O_RDONLY, NULL);
    if (iFilefd <= 0) {
        _RECORD_ERR("[PB EXIT] Read File(%d) Err err, out of read\n", iCurrFileRecNo);
        goto endFunc;
    }
    
    while (1) {
        iSeek -= sizeof(CVI_RECORD_SEGMENT_INFO);
        i++;
        if (0 != CVI_RECORD_ReadSegmentFrameInfo(iFilefd, iSeek, &iPkgWriteIndexBuff, 1, NULL))
            break;

        _RECORD_INFO("get seekFrame offset %d, count %d\n", iSeek, i);
        CVI_RECORD_ShowFramInfo(&iPkgWriteIndexBuff);
    }

endFunc:
    if(iIndexfd > 0) {
        close(iIndexfd);
        iIndexfd = -1;
    }
    if(iFilefd > 0) {
        close(iFilefd);
        iFilefd = -1;
    }

    return iRet;
}

/*****************************************************************************
 函 数 名  : CVI_REPLAY_QueryByMonth
 功能描述  :增加一条满足条件的时间列表
 输入参数  : 
  * \param pRecSegCount 保存返回获取片断的个数
 * \param pSegHead 链表头
 * \param pSegTail 链表尾
 * \param tBeginTime 起始时间
 * \param tEndTime 结束时间
 * \param iEvenType 录像类型
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
static void CVI_REPLAY_AddRecseglist(CVI_S32 *pRecSegCount, CVI_RECORD_RECORD_TS_S **pSegHead, CVI_RECORD_RECORD_TS_S **pSegTail, time_t tBeginTime, time_t tEndTime, CVI_S32 iEvenType)
{
    CVI_RECORD_RECORD_TS_S *pSegNew;
    CVI_RECORD_RECORD_TS_S *pHead;
    CVI_RECORD_RECORD_TS_S *pTail;

    pSegNew = (CVI_RECORD_RECORD_TS_S *)malloc(sizeof(CVI_RECORD_RECORD_TS_S));
    if (pSegNew == NULL) {
        _RECORD_ERR("Exit Error! malloc failed!\n");
        return;
    }
    memset((CVI_CHAR*)pSegNew, 0, sizeof(CVI_RECORD_RECORD_TS_S));
    pSegNew->ptNext = NULL;
    pSegNew->iEvenType = iEvenType;
    pSegNew->tBeginTime = tBeginTime;
    pSegNew->tEndTime = tEndTime;
    pHead = *pSegHead;
    pTail = *pSegTail;
    if (NULL == pHead) {
        pHead = pSegNew;
        pTail = pSegNew;
    } else {
        pTail->ptNext = pSegNew;
        pTail = pSegNew;
    }
    *pSegHead = pHead;
    *pSegTail = pTail;
    if (pRecSegCount)
        *pRecSegCount = (*pRecSegCount) + 1;
    return;
}

/*****************************************************************************
 函 数 名  : CVI_REPLAY_QueryFreeTsArr
 功能描述  :释放获取的录像片段
 输入参数  : IN CVI_RECORD_RECORD_TS_S *pRecSegHead 录像片段
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
static void CVI_REPLAY_FreeSeekArr(CVI_REPLAY_SEEK_RECORD *pRecSeekHead)
{
    CVI_RECORD_FUNC_START;
    CVI_REPLAY_SEEK_RECORD *pHead = NULL;
    CVI_REPLAY_SEEK_RECORD *p = NULL;
    //参数检查
    if (pRecSeekHead == NULL) {
        _RECORD_ERR("Pkg Invalid input\n");
        CVI_RECORD_FUNC_END;
        return ;
    }

    pHead = pRecSeekHead;
    while (pHead != NULL) {
        p = pHead->ptNext;
        free(pHead);
        pHead = NULL;
        pHead = p;
    }
    pRecSeekHead = NULL;

    CVI_RECORD_FUNC_END;
    return ;
}

/*****************************************************************************
 函 数 名  : CVI_REPLAY_SortSlice
 功能描述  :对片段进行排序
 输入参数  : 时间,录像类型
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
static CVI_REPLAY_SEEK_RECORD * CVI_REPLAY_SortSlice(CVI_REPLAY_SEEK_RECORD *pRecSeekHead, CVI_S32 bSequential)
{
    CVI_REPLAY_SEEK_RECORD *head = pRecSeekHead;
    CVI_REPLAY_SEEK_RECORD *first = NULL; /*排列后有序链的表头指针*/
    CVI_REPLAY_SEEK_RECORD *tail = NULL; /*排列后有序链的表尾指针*/
    CVI_REPLAY_SEEK_RECORD *p_min = NULL; /*保留键值更小的节点的前驱节点的指针*/
    CVI_REPLAY_SEEK_RECORD *min = NULL; /*存储最小节点*/
    CVI_REPLAY_SEEK_RECORD *p = NULL; /*当前比较的节点*/

    first = NULL;
    /*在链表中找键值最小的节点。*/
    while (head != NULL) {
        /*注意：这里for语句就是体现选择排序思想的地方*/
        /*循环遍历链表中的节点，找出此时最小的节点。*/
        for (p = head, min = head; p->ptNext != NULL; p = p->ptNext) {
            if (bSequential) {
                /*找到一个比当前min小的节点。*/
                if (p->ptNext->tBeginTime < min->tBeginTime) {
                    p_min = p; /*保存找到节点的前驱节点：显然p->next的前驱节点是p。*/
                    min = p->ptNext; /*保存键值更小的节点。*/
                }
            } else {
                /*找到一个比当前min小的节点。*/
                if (p->ptNext->tEndTime < min->tEndTime) {
                    p_min = p; /*保存找到节点的前驱节点：显然p->next的前驱节点是p。*/
                    min = p->ptNext; /*保存键值更小的节点。*/
                }
            }
        }
        /*上面for语句结束后，就要做两件事；一是把它放入有序链表中；二是根据相应的条件判断，安排它离开原来的链表。*/
        /*第一件事*/
        /*如果有序链表目前还是一个空链表*/
        if (first == NULL) {
            first = min; /*第一次找到键值最小的节点。*/ 
            tail = min; /*注意：尾指针让它指向最后的一个节点。*/
        } else {
            /*有序链表中已经有节点*/
            min->ptPrevious = tail;
            tail->ptNext = min; /*把刚找到的最小节点放到最后，即让尾指针的next指向它。*/ 
            tail = min; /*尾指针也要指向它。*/ 
        }
        /*第二件事*/
        if (min == head) /*如果找到的最小节点就是第一个节点*/
            head = head->ptNext; /*显然让head指向原head->next,即第二个节点，就OK*/
        else /*如果不是第一个节点*/
            p_min->ptNext = min->ptNext; /*前次最小节点的next指向当前min的next,这样就让min离开了原链表。*/
    } 
    /*循环结束得到有序链表first*/
    if (first != NULL) {
        first->ptPrevious = NULL;
        tail->ptNext = NULL; /*单向链表的最后一个节点的next应该指向NULL*/  
    } 
    pRecSeekHead = first;
    return pRecSeekHead; 
}

/*****************************************************************************
 函 数 名  : CVI_REPLAY_GetByTime
 功能描述  :获取这段时间的回放录像数据
 输入参数  : 时间,录像类型
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
static CVI_S32 CVI_REPLAY_GetByTime(time_t tBeginTime, time_t tEndTime, CVI_U32 iEvenType, CVI_REPLAY_SEEK_RECORD **pRecSeekHead)
{
    CVI_RECORD_FUNC_START;
    CVI_S32 iIndexfd = -1;
    CVI_S32 iLastIndex = -1;
    CVI_S32 index = -1;
    CVI_S32 iFileIndex = 0;
    CVI_S32 iFileSegNo = 0;
    CVI_S32 iMaxAVFileNum = 0;
    CVI_S32 iMaxIndexNum = 0;
    pkg_buffer stPkgBuf;
    CVI_S32 nRecSegCount = 0; //满足条件的录像片段个数
    CVI_S32 iRet = 0;
    CVI_REPLAY_SEEK_RECORD *pSegHead = NULL;
    CVI_REPLAY_SEEK_RECORD *pSegTail = NULL;
    CVI_REPLAY_SEEK_RECORD *pSegTailEven = NULL;
    CVI_REPLAY_SEEK_RECORD *pSegTailNew = NULL;
    
    CVI_RECORD_FILE_INDEX_RECORD* pFileIndexRecord = NULL;
    CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecord = NULL;
    CVI_CHAR iPkgPathName[CVI_RECORD_MAX_PATH_NAME];                   //文件路径
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        CVI_RECORD_FUNC_END;
        return RECORD_ERROR;
    }

    if (s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return RECORD_ERROR;
    }
    
    if ((NULL == pRecSeekHead) || (NULL != *pRecSeekHead)) {
        _RECORD_ERR("Pkg Invalid Input\n");
        CVI_RECORD_FUNC_END;
        return RECORD_ERROR;
    }

    if (0 != CVI_RECORD_MallocBuffer(&stPkgBuf, PKG_FILE_INDEX_MAX_LEN)) {
        _RECORD_ERR("pkgstorage malloc err: stPkgBuf %d\n", PKG_FILE_INDEX_MAX_LEN);
        CVI_RECORD_FUNC_END;
        return ERR_HANDLE_ALLOC_ERROR;
    }

    pthread_mutex_lock(&s_iPkgRunMutex);
    memset(iPkgPathName, 0, sizeof(iPkgPathName));
    strncpy(iPkgPathName, s_mPkgParam.iPkgPathName, sizeof(iPkgPathName)-1);
    iMaxAVFileNum = s_mPkgParam.iMaxAVFileNum;
    iMaxIndexNum = s_mPkgParam.iMaxIndexNum;
    pthread_mutex_unlock(&s_iPkgRunMutex);
    CVI_CHAR sStartTime[20];
    CVI_CHAR sEndTime[20];
    _RECORD_INFO("Get time(even:%#x) query %s-%s,path:%s, maxFiles:%d,maxIndex:%d\n", iEvenType,
                time_to_string(tBeginTime, sStartTime),
                time_to_string(tEndTime, sEndTime),
                iPkgPathName, iMaxAVFileNum, iMaxIndexNum);

    //检测剩余片段是否有效
    for (iFileIndex = (iMaxAVFileNum - 1); iFileIndex >= 0; iFileIndex--) {
        index = CVI_RECORD_FILES_GET_INDEX_NUM(iFileIndex);
        if (index != iLastIndex) {
            _RECORD_DBG("Load Index %d->%d\n", iLastIndex, index);
            if(iIndexfd > 0) {
                close(iIndexfd);
                iIndexfd = -1;
            }
            iIndexfd = CVI_RECORD_GetIndexFileFd(iPkgPathName, index, O_RDWR, NULL);
            //检测文件索引有效性
            if (0 != CVI_RECORD_GetFileIndex(&stPkgBuf, iIndexfd, iFileIndex)) {
                _RECORD_ERR("pkg index(%d) pkgstorage_get_file_index err\n", index);
                continue;
            }
        }
        iLastIndex = index;

        pFileIndexRecord = CVI_RECORD_GetFileIndexRecord(&stPkgBuf, iFileIndex);

        if ((CVI_RECORD_STATUS_NORMAL == pFileIndexRecord->iStatus) || (CVI_RECORD_STATUS_LOCK == pFileIndexRecord->iStatus)) {
            if((pFileIndexRecord->iSegRecNums > 0)
                && (0 != CVI_RECORD_CheckInTime(pFileIndexRecord->tBeginTime, pFileIndexRecord->tEndTime, tBeginTime, tEndTime))) {
                if (0 != CVI_RECORD_GetSegmentFormFileIndex(&stPkgBuf, iIndexfd, iFileIndex)) {
                    _RECORD_ERR("pkg index(%d) CVI_RECORD_GetSegmentFormFileIndex(%d) err\n", index, iFileIndex);
                    continue;
                }

                for (iFileSegNo = pFileIndexRecord->iSegRecNums - 1; iFileSegNo >= 0; iFileSegNo--) {
                    pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&stPkgBuf, iFileSegNo);
                    if ((NULL != pSegIndexRecord)
                        && (CVI_RECORD_STATUS_NORMAL == pSegIndexRecord->iStatus)) {
                        if ((pSegIndexRecord->iInfoCount > 0)
                            && (0 != CVI_RECORD_CheckInTime(pSegIndexRecord->tBeginTime, pSegIndexRecord->tEndTime, tBeginTime, tEndTime))) {
                            if (CVI_RECORD_CheckInEvent(iEvenType, pSegIndexRecord->iEvenType)) {
                                if (pSegTail) {
                                    //与上一个片段的时间连续且录像类型也一致，所以判断相同片段叠加
                                    if ((pSegTail->iRecFileNo == iFileIndex)
                                       && ((pSegTail->iStartSegNum-1) == iFileSegNo)
                                       && CVI_RECORD_CheckInContinue(pSegIndexRecord->tBeginTime, pSegIndexRecord->tEndTime, pSegTail->tBeginTime, pSegTail->tEndTime)) {
                                        pSegTail->tBeginTime = pSegIndexRecord->tBeginTime;
                                        pSegTail->iStartSegNum = iFileSegNo;
                                        continue;
                                    }
                                }

                                pSegTailNew = (CVI_REPLAY_SEEK_RECORD *)malloc(sizeof(CVI_REPLAY_SEEK_RECORD));
                                if (NULL == pSegTailNew) {
                                    //只要分配空间失败，就把之前成功分配的释放掉
                                    _RECORD_ERR("malloc buf err, %d\n", sizeof(CVI_REPLAY_SEEK_RECORD));
                                    CVI_REPLAY_FreeSeekArr(pSegHead);
                                    pSegHead = NULL;
                                    nRecSegCount = 0;
                                    iRet = RECORD_ERROR;
                                    goto endFunc;
                                }
                                //新增SEEK 片段
                                pSegTailNew->iRecFileNo = iFileIndex;
                                pSegTailNew->iStartSegNum = iFileSegNo;
                                pSegTailNew->iEndSegNum = iFileSegNo;
                                pSegTailNew->tBeginTime = pSegIndexRecord->tBeginTime;
                                pSegTailNew->tEndTime = pSegIndexRecord->tEndTime;
                                pSegTailNew->ptNext = NULL;
                                pSegTailNew->ptPrevious = NULL;
                                if (NULL == pSegHead) {
                                    pSegHead = pSegTailNew;
                                    pSegTail = pSegTailNew;
                                } else {
                                    pSegTailNew->ptPrevious = pSegTail;
                                    pSegTail->ptNext = pSegTailNew;
                                    pSegTail = pSegTailNew;
                                }
                                pSegTailEven = pSegTail;
                                pSegTailNew = NULL;
                                nRecSegCount++;
                            } else {
                                //上个事件前几秒数据为预览播放帧数据
                                if ((NULL != pSegTail)
                                   && ((pSegTail->iStartSegNum-1) == iFileSegNo)
                                   && (pSegIndexRecord->tEndTime <= pSegTailEven->tBeginTime)
                                   && (pSegIndexRecord->tEndTime >= pSegTailEven->tBeginTime - CVI_RECORD_PRE_RECORD_TIMES)) {
                                    _RECORD_ERR("Add pre even rec, time:%lu-%lu,even:%lu-%lu\n", 
                                        pSegIndexRecord->tBeginTime, pSegIndexRecord->tEndTime,
                                        pSegTailEven->tBeginTime, pSegTailEven->tEndTime);

                                    pSegTailNew = (CVI_REPLAY_SEEK_RECORD *)malloc(sizeof(CVI_REPLAY_SEEK_RECORD));
                                    if (NULL == pSegTailNew) {
                                        //只要分配空间失败，就把之前成功分配的释放掉
                                        _RECORD_ERR("malloc buf err, %d\n", sizeof(CVI_REPLAY_SEEK_RECORD));
                                        CVI_REPLAY_FreeSeekArr(pSegHead);
                                        pSegHead = NULL;
                                        nRecSegCount = 0;
                                        iRet = RECORD_ERROR;
                                        goto endFunc;
                                    }
                                    //新增SEEK 片段
                                    pSegTailNew->iRecFileNo = iFileIndex;
                                    pSegTailNew->iStartSegNum = iFileSegNo;
                                    pSegTailNew->iEndSegNum = iFileSegNo;
                                    //限制播放时间预录
                                    if (pSegIndexRecord->tBeginTime < pSegTailEven->tBeginTime - CVI_RECORD_PRE_RECORD_TIMES)
                                        pSegTailNew->tBeginTime = pSegTailEven->tBeginTime - CVI_RECORD_PRE_RECORD_TIMES;
                                    else
                                        pSegTailNew->tBeginTime = pSegIndexRecord->tBeginTime;

                                    pSegTailNew->tEndTime = pSegIndexRecord->tEndTime;
                                    pSegTailNew->ptNext = NULL;
                                    pSegTailNew->ptPrevious = NULL;
                                    if (NULL == pSegHead) {
                                        pSegHead = pSegTailNew;
                                        pSegTail = pSegTailNew;
                                    } else {
                                        pSegTailNew->ptPrevious = pSegTail;
                                        pSegTail->ptNext = pSegTailNew;
                                        pSegTail = pSegTailNew;
                                    }
                                    pSegTailNew = NULL;
                                    nRecSegCount++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (pSegHead) {
        CVI_REPLAY_SEEK_RECORD *pHead = pSegHead;
        while (pHead != NULL) {
            CVI_CHAR sStartTime[20];
            CVI_CHAR sEndTime[20];
            _RECORD_DBG("pkg seek No(%d) SegNum(%d-%d),time %s-%s\n", 
                        pHead->iRecFileNo, pHead->iStartSegNum, pHead->iEndSegNum,
                        time_to_string(pHead->tBeginTime, sStartTime),
                        time_to_string(pHead->tEndTime, sEndTime));
            pHead = pHead->ptNext;
        }
    }

    *pRecSeekHead = CVI_REPLAY_SortSlice(pSegHead, 1);
    iRet = nRecSegCount;
    _RECORD_DBG("pb get count:%d\n", iRet);
endFunc:

    if(iIndexfd > 0) {
        close(iIndexfd);
        iIndexfd = -1;
    }
    CVI_RECORD_FreeBuffer(&stPkgBuf);
    CVI_RECORD_FUNC_END;
    return iRet;
}



/*****************************************************************************
 函 数 名  : CVI_REPLAY_QueryByTime
 功能描述  :查询这个时间段是否有录像
 输入参数  : 时间,录像类型
 输出参数  : 录像片段信息，片段个数
 返 回 值  : 0
*****************************************************************************/
static CVI_S32 CVI_REPLAY_QueryByTime(time_t tBeginTime, time_t tEndTime, CVI_U32 iEvenType, CVI_RECORD_RECORD_TS_S **pRecSegHead, CVI_S32 *pRecSegCount)
{
    CVI_RECORD_FUNC_START;
    CVI_S32 iIndexfd = -1;
    CVI_S32 index = -1;
    CVI_S32 iLastIndex = -1;
    CVI_S32 iFileIndex = 0;
    CVI_S32 iFileSegNo = 0;
    CVI_S32 iMaxAVFileNum = 0;
    CVI_S32 iMaxIndexNum = 0;
    pkg_buffer stPkgBuf;
    CVI_S32 nRecSegCount = 0; //满足条件的录像片段个数
    CVI_RECORD_RECORD_TS_S *pSegHead = NULL;
    CVI_RECORD_RECORD_TS_S *pSegTail = NULL;
    time_t tAddBeginTime = 0;
    time_t tAddEndTime = 0;
    
    CVI_RECORD_FILE_INDEX_RECORD* pFileIndexRecord = NULL;
    CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecord = NULL;
    CVI_CHAR iPkgPathName[CVI_RECORD_MAX_PATH_NAME];                   //文件路径
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    if (s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    if ((NULL == pRecSegHead) || (NULL != *pRecSegHead) || (NULL == pRecSegCount)) {
        _RECORD_ERR("Pkg Invalid Input\n");
        CVI_RECORD_FUNC_END;
        return ERR_INVALID_ARGUMENT;
    }

    if (0 != CVI_RECORD_MallocBuffer(&stPkgBuf, PKG_FILE_INDEX_MAX_LEN)) {
        _RECORD_ERR("pkgstorage malloc err: stPkgBuf %d\n", PKG_FILE_INDEX_MAX_LEN);
        CVI_RECORD_FUNC_END;
        return ERR_HANDLE_ALLOC_ERROR;
    }

    pthread_mutex_lock(&s_iPkgRunMutex);
    memset(iPkgPathName, 0, sizeof(iPkgPathName));
    strncpy(iPkgPathName, s_mPkgParam.iPkgPathName, sizeof(iPkgPathName)-1);
    iMaxAVFileNum = s_mPkgParam.iMaxAVFileNum;
    iMaxIndexNum = s_mPkgParam.iMaxIndexNum;
    pthread_mutex_unlock(&s_iPkgRunMutex);
    CVI_CHAR sStartTime[20];
    CVI_CHAR sEndTime[20];
    _RECORD_INFO("Query time(even:%#x) query %s-%s,path:%s, maxFiles:%d,maxIndex:%d\n", iEvenType,
                time_to_string(tBeginTime, sStartTime),
                time_to_string(tEndTime, sEndTime),
                iPkgPathName, iMaxAVFileNum, iMaxIndexNum);

    //检测剩余片段是否有效
    for (iFileIndex = (iMaxAVFileNum-1);iFileIndex>=0;iFileIndex--) {
        index = CVI_RECORD_FILES_GET_INDEX_NUM(iFileIndex);
        if (index != iLastIndex) {
            _RECORD_DBG("Load Index %d->%d\n", iLastIndex, index);
            if (iIndexfd > 0) {
                close(iIndexfd);
                iIndexfd = -1;
            }

            iIndexfd = CVI_RECORD_GetIndexFileFd(iPkgPathName, index, O_RDWR, NULL);

            //检测文件索引有效性
            if (0 != CVI_RECORD_GetFileIndex(&stPkgBuf, iIndexfd, iFileIndex)) {
                _RECORD_ERR("pkg index(%d) pkgstorage_get_file_index err\n", index);
                continue;
            }
        }
        iLastIndex = index;

        pFileIndexRecord = CVI_RECORD_GetFileIndexRecord(&stPkgBuf, iFileIndex);
        if ((CVI_RECORD_STATUS_NORMAL == pFileIndexRecord->iStatus) || (CVI_RECORD_STATUS_LOCK == pFileIndexRecord->iStatus)) {
            if ((pFileIndexRecord->iSegRecNums > 0)
                &&(0 != CVI_RECORD_CheckInTime(pFileIndexRecord->tBeginTime, pFileIndexRecord->tEndTime, tBeginTime, tEndTime))) {
                if (0 != CVI_RECORD_GetSegmentFormFileIndex(&stPkgBuf, iIndexfd, iFileIndex)) {
                    _RECORD_ERR("pkg index(%d) CVI_RECORD_GetSegmentFormFileIndex(%d) err\n", index, iFileIndex);
                    continue;
                }
                for (iFileSegNo = pFileIndexRecord->iSegRecNums - 1; iFileSegNo >= 0; iFileSegNo--)
                {
                    pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&stPkgBuf, iFileSegNo);
                    if((NULL != pSegIndexRecord)
                        && ((CVI_RECORD_STATUS_NORMAL == pSegIndexRecord->iStatus) || (CVI_RECORD_STATUS_LOCK == pSegIndexRecord->iStatus))) {
                        if ((pSegIndexRecord->iInfoCount > 0)
                            && (0 != CVI_RECORD_CheckInTime(pSegIndexRecord->tBeginTime, pSegIndexRecord->tEndTime, tBeginTime, tEndTime))) {
                            if (CVI_RECORD_CheckInEvent(iEvenType, pSegIndexRecord->iEvenType)) {
                                if (pSegTail) {
                                    //与上一个片段的时间连续且录像类型也一致，所以判断相同片段叠加
                                    if ((pSegTail->iEvenType == pSegIndexRecord->iEvenType)
                                       && CVI_RECORD_CheckInContinue(pSegIndexRecord->tBeginTime, pSegIndexRecord->tEndTime, pSegTail->tBeginTime, pSegTail->tEndTime)) {
                                        //片段最大时间为查询的EndTime
                                        if (pSegIndexRecord->tBeginTime < tBeginTime)
                                            pSegTail->tBeginTime = tBeginTime;
                                        else
                                            pSegTail->tBeginTime = pSegIndexRecord->tBeginTime;
                                        continue;
                                    }
                                }
                                tAddBeginTime = (pSegIndexRecord->tBeginTime < tBeginTime) ? tBeginTime : pSegIndexRecord->tBeginTime;
                                tAddEndTime = (pSegIndexRecord->tEndTime > tEndTime) ? tEndTime : pSegIndexRecord->tEndTime;                                    
                                CVI_REPLAY_AddRecseglist(&nRecSegCount, &pSegHead, &pSegTail, tAddBeginTime, tAddEndTime, pSegIndexRecord->iEvenType);
                            }
                        }
                    }
                }
            }
        }
    }
    
    if (pSegHead) {
        CVI_RECORD_RECORD_TS_S *pHead = pSegHead;
        while (pHead != NULL) {
            CVI_CHAR sStartTime[20];
            CVI_CHAR sEndTime[20];
            _RECORD_DBG("pkg seg even(%#x),time %s-%s\n", 
                        pHead->iEvenType,
                        time_to_string(pHead->tBeginTime, sStartTime),
                        time_to_string(pHead->tEndTime, sEndTime));
            pHead = pHead->ptNext;
        }
    }

    *pRecSegHead = pSegHead;
    *pRecSegCount = nRecSegCount;
    
    if (iIndexfd > 0) {
        close(iIndexfd);
        iIndexfd = -1;
    }
    CVI_RECORD_FreeBuffer(&stPkgBuf);
    CVI_RECORD_FUNC_END;
    return 0;
}


/*****************************************************************************
 函 数 名  : CVI_REPLAY_QueryByMonth
 功能描述  :查询这个月是否有录像
 输入参数  : IN CVI_U32 query_year, IN CVI_U32 query_month 年月
 输出参数  : OUT CVI_U32 *pMonthDays 32位，每位代码对应天是否有录像
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_QueryByMonth(IN CVI_U32 query_year, IN CVI_U32 query_month, OUT CVI_U32 *pMonthDays)
{
    CVI_S32 iRet = 0;
    CVI_S32 iIndexfd = -1;
    CVI_S32 index = 0;
    CVI_S32 iLastIndex = -1;
    CVI_S32 iFileIndex = 0;
    CVI_S32 iFileSegNo = 0;
    CVI_S32 iMaxAVFileNum = 0;
    CVI_S32 iMaxIndexNum = 0;
    CVI_S32 i = 0;
    time_t tBeginTime = 0;
    time_t tEndTime = 0;
    CVI_U32 stMonthDays = 0;
    struct tm tm;
    pkg_buffer stPkgBuf;
    CVI_RECORD_FILE_INDEX_RECORD* pFileIndexRecord = NULL;
    CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecord = NULL;
    CVI_CHAR iPkgPathName[CVI_RECORD_MAX_PATH_NAME];                   //文件路径
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        return ERR_SYS_NOT_INIT;
    }

    if(s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    if(NULL == pMonthDays) {
        _RECORD_ERR("Pkg Invalid Input\n");
        return ERR_INVALID_ARGUMENT;
    }

    if ((query_year > 2037) || (query_year < 1970) || (query_month > 12) || (query_month < 1)) {
        _RECORD_ERR("Pkg Invalid Input:%d-%d\n", query_year, query_month);
        return ERR_INVALID_ARGUMENT;
    }

    if(0 != CVI_RECORD_MallocBuffer(&stPkgBuf, PKG_FILE_INDEX_MAX_LEN)) {
        _RECORD_ERR("pkgstorage malloc err: stPkgBuf %d\n", PKG_FILE_INDEX_MAX_LEN);
        return ERR_HANDLE_ALLOC_ERROR;
    }

    pthread_mutex_lock(&s_iPkgRunMutex);
    memset(iPkgPathName, 0, sizeof(iPkgPathName));
    strncpy(iPkgPathName, s_mPkgParam.iPkgPathName, sizeof(iPkgPathName)-1);
    iMaxAVFileNum = s_mPkgParam.iMaxAVFileNum;
    iMaxIndexNum = s_mPkgParam.iMaxIndexNum;
    pthread_mutex_unlock(&s_iPkgRunMutex);

    _RECORD_INFO("Get month query %d-%d,path:%s, maxFiles:%d,maxIndex:%d\n", query_year, query_month, iPkgPathName, iMaxAVFileNum, iMaxIndexNum);
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = query_year - 1900;
    tm.tm_mon = query_month - 1;
    tm.tm_mday = 1;
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tBeginTime = mktime(&tm);
    _RECORD_INFO("Start time %d-%d-%d %d:%d:%d  ---  %ld seconds\n",
        tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tBeginTime);

    if(12 == query_month) {
        tm.tm_year = tm.tm_year + 1;
        tm.tm_mon = 0;
    } else {
        tm.tm_mon = tm.tm_mon + 1;
    }
    tEndTime = mktime(&tm);
    _RECORD_INFO("End time %d-%d-%d %d:%d:%d  ---  %ld seconds\n",
        tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tEndTime);

    *pMonthDays = 0;

    //检测剩余片段是否有效
    for (iFileIndex = 0; iFileIndex < iMaxAVFileNum; iFileIndex++) {
        index = CVI_RECORD_FILES_GET_INDEX_NUM(iFileIndex);
        if (index != iLastIndex) {
            _RECORD_DBG("Load Index %d->%d\n", iLastIndex, index);
            if (iIndexfd > 0) {
                close(iIndexfd);
                iIndexfd = -1;
            }

            iIndexfd = CVI_RECORD_GetIndexFileFd(iPkgPathName, index, O_RDWR, NULL);
            //检测文件索引有效性
            if (0 != CVI_RECORD_GetFileIndex(&stPkgBuf, iIndexfd, iFileIndex)) {
                _RECORD_ERR("pkg index(%d) pkgstorage_get_file_index err\n", index);
                continue;
            }
        }
        iLastIndex = index;

        pFileIndexRecord = CVI_RECORD_GetFileIndexRecord(&stPkgBuf, iFileIndex);
        if ((CVI_RECORD_STATUS_NORMAL == pFileIndexRecord->iStatus) || (CVI_RECORD_STATUS_LOCK == pFileIndexRecord->iStatus)) {
            if ((pFileIndexRecord->iSegRecNums > 0)
                && (pFileIndexRecord->tBeginTime >= tBeginTime)
                && (pFileIndexRecord->tBeginTime < tEndTime)) {
                if (0 != CVI_RECORD_GetSegmentFormFileIndex(&stPkgBuf, iIndexfd, iFileIndex)) {
                    _RECORD_ERR("pkg index(%d) CVI_RECORD_GetSegmentFormFileIndex(%d) err\n", index, iFileIndex);
                    continue;
                }
                for (iFileSegNo = 0; iFileSegNo < pFileIndexRecord->iSegRecNums; iFileSegNo++) {
                    pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&stPkgBuf, iFileSegNo);
                    if ((NULL != pSegIndexRecord)
                        && ((CVI_RECORD_STATUS_NORMAL == pSegIndexRecord->iStatus) || (CVI_RECORD_STATUS_LOCK == pSegIndexRecord->iStatus))) {
                        if ((pSegIndexRecord->iInfoCount > 0)
                            && (0 != CVI_RECORD_CheckInTime(pSegIndexRecord->tBeginTime, pSegIndexRecord->tEndTime, tBeginTime, tEndTime))) {
                            for (i = 0; i < 31; i++) {
                                CVI_S32 tStartTimeInt = tBeginTime + i * CVI_RECORD_DAY_TIMES;
                                CVI_S32 tEndTimeInt = tStartTimeInt + CVI_RECORD_DAY_TIMES;
                                if (tStartTimeInt >= tEndTime) {
                                    //检测天数结束
                                    break;
                                }

                                if ((stMonthDays >> i) & 0x01) {
                                    //已经检测到当天有录像
                                    continue;
                                }

                                if (0 != CVI_RECORD_CheckInTime(pSegIndexRecord->tBeginTime, pSegIndexRecord->tEndTime, tStartTimeInt, tEndTimeInt)) {
                                    stMonthDays = stMonthDays | (1<<i);
                                    CVI_CHAR sStartTime[20];
                                    CVI_CHAR sEndTime[20];
                                    _RECORD_INFO("pkg get days(%#x.%d),Index:%d,fileNo:%d,time %s-%s\n", stMonthDays, i,
                                                index, iFileIndex,
                                                time_to_string(pSegIndexRecord->tBeginTime, sStartTime),
                                                time_to_string(pSegIndexRecord->tEndTime, sEndTime));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (iIndexfd > 0) {
        close(iIndexfd);
        iIndexfd = -1;
    }
    CVI_RECORD_FreeBuffer(&stPkgBuf);
    *pMonthDays = stMonthDays;
    return iRet;
}


/*****************************************************************************
 函 数 名  : CVI_REPLAY_QueryByDay
 功能描述  :查询当天录像
 输入参数  : IN CVI_S32 year, IN CVI_S32 month, IN CVI_S32 day 年月日
 输出参数  : OUT CVI_RECORD_RECORD_TS_S **pTsList返回录像片段
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_QueryByDay(IN CVI_U32 query_year, IN CVI_U32 query_month, IN CVI_U32 query_day, IN CVI_U32 iEvenType, OUT CVI_RECORD_RECORD_TS_S **pRecSegHead, OUT CVI_S32 *pRecSegCount)
{
    CVI_RECORD_FUNC_START;
    CVI_S32 iRet = 0;
    time_t tBeginTime = 0;
    struct tm tm;
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    if (s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    if ((query_year > 2037) || (query_year < 1970) || (query_month > 12) || (query_month < 1) || (query_day > 31) || (query_day < 1)) {
        _RECORD_ERR("Pkg Invalid Input:%d-%d-%d\n", query_year, query_month, query_day);
        CVI_RECORD_FUNC_END;
        return ERR_INVALID_ARGUMENT;
    }

    if ((NULL == pRecSegHead) || (NULL != *pRecSegHead) || (NULL == pRecSegCount)) {
        _RECORD_ERR("Pkg Invalid Input\n");
        CVI_RECORD_FUNC_END;
        return ERR_INVALID_ARGUMENT;
    }

    memset(&tm, 0, sizeof(tm));
    tm.tm_year = query_year - 1900;
    tm.tm_mon = query_month - 1;
    tm.tm_mday = query_day;
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tBeginTime = mktime(&tm);

    _RECORD_INFO("Get day query %d-%d-%d, iEvenType:%#x\n", query_year, query_month, query_day, iEvenType);
    iRet = CVI_REPLAY_QueryByTime(tBeginTime, tBeginTime+CVI_RECORD_DAY_TIMES, iEvenType, pRecSegHead, pRecSegCount);
    CVI_RECORD_FUNC_END;
    return iRet;
}


/*****************************************************************************
 函 数 名  : CVI_REPLAY_QueryFreeTsArr
 功能描述  :释放获取的录像片段
 输入参数  : IN CVI_RECORD_RECORD_TS_S *pRecSegHead 录像片段
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_QueryFreeTsArr(IN CVI_RECORD_RECORD_TS_S *pRecSegHead)
{
    CVI_RECORD_FUNC_START;
    CVI_RECORD_RECORD_TS_S *pHead = NULL;
    CVI_RECORD_RECORD_TS_S *p = NULL;

    //判断系统是否已经初始化
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        return ERR_SYS_NOT_INIT;
    }
    //参数检查
    if (pRecSegHead == NULL) {
        _RECORD_ERR("Pkg Invalid input\n");
        return ERR_INVALID_ARGUMENT;
    }

    pHead = pRecSegHead;
    while (pHead != NULL) {
        p = pHead->ptNext;
        free(pHead);
        pHead = NULL;
        pHead = p;
    }
    pRecSegHead = NULL;

    CVI_RECORD_FUNC_END;
    return 0;
}


/*!
 * \brief 判断给定的数据弹出器句柄是否合法
 * \param hPoperHandle 数据弹出器句柄
 * \return 成功返回1,失败返回0。
 */
static CVI_S32 CVI_REPLAY_IsValid(HANDLE hPoperHandle)
{
    CVI_S32 i;
    for (i = 0; i < CVI_RECORD_MAX_RECORD_READER; i++) {
        if (&s_mDataPoper[i] == hPoperHandle) {
            if (s_mDataPoper[i].bOpen)
                return 1;
            else
                return 0;
        }
    }
    return 0;
}

/*!
 * \brief 数据弹出线程，在创建数据弹出器时创建该线程
 * \param lParam 数据弹出器句柄
 */
static void *CVI_REPLAY_DataPopProc(void *lParam)
{
    CVI_RECORD_FUNC_START;
    T_PSS_DataPoper *pPoper = (T_PSS_DataPoper *)lParam;
    CVI_RECORD_SEGMENT_INFO  iPkgWriteIndexHead[CVI_RECORD_FRAME_HEADER_INFO_MAX];     //当前录像文件索引缓存
    CVI_RECORD_SEGMENT_INFO  iPkgWriteIndexBuff;                                  //当前录像文件索引缓存CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_COUNT
    CVI_RECORD_SEGMENT_INDEX_RECORD* pSegIndexRecord = NULL;
    CVI_REPLAY_SEEK_RECORD *pRecSeekHead = NULL;
    CVI_REPLAY_SEEK_RECORD *pRecSeekReader = NULL;
    CVI_S32 iRecSeekSegId = -1;
    CVI_S32 iRecSeekSegFrameId = 0;
    CVI_S32 iRecIndexNo = -1;
    CVI_S32 iIndex = -1;
    CVI_S32 iRecFileNo = -1;
    CVI_S32 iIndexfd = -1;
    CVI_S32 iFilefd = -1;
    CVI_S32 bSendKeyframe = 1;
    CVI_S32 bSendIndexFrame = 1;
    CVI_S32 iLastVideoNo = 0;
    CVI_S32 iLastAudioNo = 0;
    time_t stSeekTime = 0;
    pkg_buffer stPkgBuf;
    CVI_U8 * pFrameBuf = NULL;
    prctl(PR_SET_NAME,"REPLAY:proc");
    //参数检查
    if (pPoper == NULL) {
        _RECORD_ERR("[PB EXIT]Pkg Invalid input\n");
        CVI_RECORD_FUNC_END;
        return NULL;
    }

    if (!CVI_REPLAY_IsValid(pPoper)) {
        _RECORD_ERR("[PB EXIT]Pkg Invalid input Exit ERR! hDataPopper = 0x%x\n", (CVI_U32)pPoper);
        CVI_RECORD_FUNC_END;
        return NULL;
    }

    //创建回放数据
    if (s_mPkgParam.iPkginited && !s_mPkgParam.bPkgstop && pPoper->pThreadRun && pPoper->bOpen) {
        if (0 >= CVI_REPLAY_GetByTime(pPoper->tBeginTime, pPoper->tEndTime, pPoper->iEvenType, &pRecSeekHead)) {
            _RECORD_ERR("[PB EXIT]Pkg get pb time invalid! hDataPopper = 0x%x, time:%lu-%lu\n", (CVI_U32)pPoper, pPoper->tBeginTime, pPoper->tEndTime);
            if (pPoper->pGetDataCallback)
                pPoper->pGetDataCallback(pPoper, NULL, CVI_REPLAY_CB_ERROR);
            CVI_RECORD_FUNC_END;
            return NULL;
        }
        if(0 != CVI_RECORD_MallocBuffer(&stPkgBuf, PKG_FILE_INDEX_MAX_LEN))
        {
            _RECORD_ERR("[PB EXIT]pkgstorage malloc err: stPkgBuf %d\n", PKG_FILE_INDEX_MAX_LEN);
            CVI_REPLAY_FreeSeekArr(pRecSeekHead);
            if(pPoper->pGetDataCallback)
            {
                pPoper->pGetDataCallback(pPoper, NULL, CVI_REPLAY_CB_ERROR);
            }
            CVI_RECORD_FUNC_END;
            return NULL;
        }
        pRecSeekReader = pRecSeekHead;
        iRecSeekSegId = pRecSeekReader->iStartSegNum;
        
    }
    else
    {
        _RECORD_ERR("[PB EXIT]Pkg Invalid input Exit ERR! hDataPopper = 0x%x, init:%d,stop:%d,run:%d,open:%d\n", (CVI_U32)pPoper,
            s_mPkgParam.iPkginited, s_mPkgParam.bPkgstop, pPoper->pThreadRun, pPoper->bOpen);
        
        if(pPoper->pGetDataCallback)
        {
            pPoper->pGetDataCallback(pPoper, NULL, CVI_REPLAY_CB_ERROR);
        }
        CVI_RECORD_FUNC_END;
        return NULL;
    }

    pFrameBuf = malloc(PKG_READ_DEFAULT_FRAME_LEN);
    if(NULL == pFrameBuf)
    {
        _RECORD_ERR("[PB EXIT]pkgstorage malloc err: stPkgBuf %d\n", PKG_READ_DEFAULT_FRAME_LEN);
        
        if(pPoper->pGetDataCallback)
        {
            pPoper->pGetDataCallback(pPoper, NULL, CVI_REPLAY_CB_ERROR);
        }
        
        CVI_REPLAY_FreeSeekArr(pRecSeekHead);
        CVI_RECORD_FreeBuffer(&stPkgBuf);
        CVI_RECORD_FUNC_END;
        return NULL;
    }
    while (s_mPkgParam.iPkginited && !s_mPkgParam.bPkgstop && pPoper->pThreadRun && pPoper->bOpen)
    {
        if(pPoper->bOpen && (NULL != pPoper->pGetDataCallback))
        {
            if(pPoper->bStartPop)
            {
                //根据seek 找 起始reader
                pthread_mutex_lock(&pPoper->iPopMutex);
                if(0 < pPoper->tSeekTime)
                {
                    pRecSeekReader = pRecSeekHead;
                    while(NULL != pRecSeekReader)
                    {
                        iRecSeekSegId = pRecSeekReader->iStartSegNum;
                        if(pRecSeekReader->tEndTime >= pPoper->tSeekTime)
                        {
                            if(iRecFileNo != pRecSeekReader->iRecFileNo)
                            {
                                iIndex = CVI_RECORD_FILES_GET_INDEX_NUM(pRecSeekReader->iRecFileNo);
                                if(iIndex != iRecIndexNo)
                                {
                                    if(iIndexfd > 0)
                                    {
                                        close(iIndexfd);
                                        iIndexfd = -1;
                                    }
                                }
                                iRecIndexNo = iIndex;
                                
                                if(iFilefd > 0)
                                {
                                    close(iFilefd);
                                    iFilefd = -1;
                                }
                                iRecFileNo = pRecSeekReader->iRecFileNo;
                            }
                            
                            if(iIndexfd <= 0)
                            {
                                iIndexfd = CVI_RECORD_GetIndexFileFd(s_mPkgParam.iPkgPathName, iRecIndexNo, O_RDONLY, NULL);
                                if(iIndexfd <= 0)
                                {
                                    pPoper->pGetDataCallback(pPoper, NULL, CVI_REPLAY_CB_ERROR);
                                    _RECORD_ERR("[PB EXIT] Read Index(%d) Err err, out of read\n", iRecIndexNo);
                                    pthread_mutex_unlock(&pPoper->iPopMutex);
                                    goto ENDFUNC;
                                }
                                else
                                {
                                    if(0 != CVI_RECORD_GetFileIndex(&stPkgBuf, iIndexfd, iRecFileNo))
                                    {
                                        pPoper->pGetDataCallback(pPoper, NULL, CVI_REPLAY_CB_ERROR);
                                        _RECORD_ERR("[PB EXIT] Read Index(%d) Err err, out of read\n", iRecIndexNo);
                                        pthread_mutex_unlock(&pPoper->iPopMutex);
                                        goto ENDFUNC;
                                    }
                                }
                            }
                            else
                            {
                                if(0 != CVI_RECORD_GetSegmentFormFileIndex(&stPkgBuf, iIndexfd, iRecFileNo))
                                {
                                    pPoper->pGetDataCallback(pPoper, NULL, CVI_REPLAY_CB_ERROR);
                                    _RECORD_ERR("[PB EXIT] update Index(%d) file(%d) Err err, out of read\n", iRecIndexNo, iRecFileNo);
                                    pthread_mutex_unlock(&pPoper->iPopMutex);
                                    goto ENDFUNC;
                                }
                            }
                            _RECORD_DBG("Seek check, file:%d,seg:%d,seek:%lu,time:%lu-%lu\n", iRecFileNo, iRecSeekSegId,
                                            pPoper->tSeekTime, pRecSeekReader->tBeginTime, pRecSeekReader->tEndTime);
                            //检测剩余片段是否有效
                            for(iRecSeekSegId = pRecSeekReader->iStartSegNum;iRecSeekSegId <= pRecSeekReader->iEndSegNum;iRecSeekSegId++)
                            {
                                pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&stPkgBuf, iRecSeekSegId);
                                if((NULL != pSegIndexRecord)
                                    && ((CVI_RECORD_STATUS_NORMAL == pSegIndexRecord->iStatus) || (CVI_RECORD_STATUS_LOCK == pSegIndexRecord->iStatus)))
                                {                                        
                                    if((pSegIndexRecord->iInfoCount > 0)
                                        && (pSegIndexRecord->tEndTime >= pPoper->tSeekTime))
                                    {
                                        _RECORD_DBG("Seek ok, file:%d,seg:%d,seek:%lu,time:%lu-%lu\n", iRecFileNo, iRecSeekSegId,
                                                        pPoper->tSeekTime, pSegIndexRecord->tBeginTime, pPoper->tEndTime);
                                        break;
                                    }
                                }
                            }
                            
                            if(iRecSeekSegId > pRecSeekReader->iEndSegNum)
                            {
                                pRecSeekReader = pRecSeekReader->ptNext;
                                continue;
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            pRecSeekReader = pRecSeekReader->ptNext;
                        }
                    }
                    //没找到合适时间点，从0开始播放
                    if(NULL == pRecSeekReader)
                    {
                        pRecSeekReader = pRecSeekHead;
                        iRecSeekSegId = pRecSeekReader->iStartSegNum;
                        stSeekTime = 0;
                    }
                    else
                    {
                        stSeekTime = pPoper->tSeekTime;
                    }
                    pPoper->tSeekTime = 0;
                    bSendKeyframe = 1;
                    bSendIndexFrame = 1;
                }
                pthread_mutex_unlock(&pPoper->iPopMutex);

                //从reader开始播放
                if(pRecSeekReader)
                {
                    //判断打开文件设备节点
                    if(iRecFileNo != pRecSeekReader->iRecFileNo)
                    {
                        iIndex = CVI_RECORD_FILES_GET_INDEX_NUM(pRecSeekReader->iRecFileNo);
                        if(iIndex != iRecIndexNo)
                        {
                            if(iIndexfd > 0)
                            {
                                close(iIndexfd);
                                iIndexfd = -1;
                            }
                        }
                        iRecIndexNo = iIndex;
                        
                        if(iFilefd > 0)
                        {
                            close(iFilefd);
                            iFilefd = -1;
                        }
                        _RECORD_DBG("[PB Next] change next File(%d->%d)\n", iRecIndexNo, pRecSeekReader->iRecFileNo);
                        iRecFileNo = pRecSeekReader->iRecFileNo;
                        iRecSeekSegId = -1;
                    }
                    
                    if(iFilefd <= 0)
                    {
                        _RECORD_DBG("[PB Next] open next File(%d)\n", iRecFileNo);
                        iFilefd = CVI_RECORD_GetAvFilesFd(s_mPkgParam.iPkgPathName, iRecFileNo, O_RDONLY, NULL);
                        if(iFilefd <= 0)
                        {
                            pPoper->pGetDataCallback(pPoper, NULL, CVI_REPLAY_CB_ERROR);
                            _RECORD_ERR("[PB EXIT] Read File(%d) Err err, out of read\n", iRecFileNo);
                            goto ENDFUNC;
                        }
                    }
                    
                    if(iIndexfd <= 0)
                    {
                        iIndexfd = CVI_RECORD_GetIndexFileFd(s_mPkgParam.iPkgPathName, iRecIndexNo, O_RDONLY, NULL);
                        if(iIndexfd <= 0)
                        {
                            pPoper->pGetDataCallback(pPoper, NULL, CVI_REPLAY_CB_ERROR);
                            _RECORD_ERR("[PB EXIT] Read Index(%d) Err err, out of read\n", iRecIndexNo);
                            goto ENDFUNC;
                        }
                        else
                        {
                            if(0 != CVI_RECORD_GetFileIndex(&stPkgBuf, iIndexfd, iRecFileNo))
                            {
                                pPoper->pGetDataCallback(pPoper, NULL, CVI_REPLAY_CB_ERROR);
                                _RECORD_ERR("[PB EXIT] Read Index(%d) file(%d) Err err, out of read\n", iRecIndexNo, iRecFileNo);
                                goto ENDFUNC;
                            }
                        }
                    }
                    else
                    {
                        if(0 != CVI_RECORD_GetSegmentFormFileIndex(&stPkgBuf, iIndexfd, iRecFileNo))
                        {
                            pPoper->pGetDataCallback(pPoper, NULL, CVI_REPLAY_CB_ERROR);
                            _RECORD_ERR("[PB EXIT] update Index(%d) file(%d) Err err, out of read\n", iRecIndexNo, iRecFileNo);
                            goto ENDFUNC;
                        }
                    }
                    if((iRecSeekSegId < pRecSeekReader->iStartSegNum) || (iRecSeekSegId > pRecSeekReader->iEndSegNum))
                    {
                        _RECORD_ERR("[skip seg]Reader Invalid Id %d (%d-%d) to startId\n", iRecSeekSegId, pRecSeekReader->iStartSegNum, pRecSeekReader->iEndSegNum);
                        iRecSeekSegId = pRecSeekReader->iStartSegNum;
                    }
                    pSegIndexRecord = CVI_RECORD_GetSegmentIndexRecord(&stPkgBuf, iRecSeekSegId);
                    if(pSegIndexRecord)
                    {
                        CVI_RECORD_ShowSegmentIndex(pSegIndexRecord);
                        if(0 != CVI_RECORD_ReadSegmentFrameInfo(iFilefd, CVI_RECORD_FRAME_HEADER_INFO_SEEK(pSegIndexRecord), iPkgWriteIndexHead, CVI_RECORD_FRAME_HEADER_INFO_MAX, NULL))
                        {
                            _RECORD_ERR("[skip seg]Reader header Invalid files:%d seg:%d, offset:%u, %u\n", iRecFileNo, iRecSeekSegId, pSegIndexRecord->iInfoEndOffset, CVI_RECORD_FRAME_HEADER_INFO_SEEK(pSegIndexRecord));
                        }
                        else
                        {
                             _RECORD_DBG("[seg]Reader header files:%d seg:%d, offset:%u, %u\n", iRecFileNo, iRecSeekSegId, pSegIndexRecord->iInfoEndOffset, CVI_RECORD_FRAME_HEADER_INFO_SEEK(pSegIndexRecord));
                             if((CVI_RECORD_INFO_TYPE_HEADER != iPkgWriteIndexHead[CVI_RECORD_FRAME_HEADER_INFO_HEAD].iInfoType)
                                || (CVI_RECORD_INFO_TYPE_KEY_INDEX != iPkgWriteIndexHead[CVI_RECORD_FRAME_HEADER_INFO_KEY0].iInfoType)
                                || (CVI_RECORD_INFO_TYPE_KEY_INDEX != iPkgWriteIndexHead[CVI_RECORD_FRAME_HEADER_INFO_KEY1].iInfoType))
                             {
                                 _RECORD_ERR("[skip seg]Reader header type(%d,%d,%d) Invalid files:%d seg:%d, offset:%u, %u\n", 
                                        iPkgWriteIndexHead[CVI_RECORD_FRAME_HEADER_INFO_HEAD].iInfoType, iPkgWriteIndexHead[CVI_RECORD_FRAME_HEADER_INFO_KEY0].iInfoType,
                                        iPkgWriteIndexHead[CVI_RECORD_FRAME_HEADER_INFO_KEY1].iInfoType, iRecFileNo, iRecSeekSegId, pSegIndexRecord->iInfoEndOffset, CVI_RECORD_FRAME_HEADER_INFO_SEEK(pSegIndexRecord));
                             }
                             else
                             {
                                 for(iRecSeekSegFrameId=iPkgWriteIndexHead[CVI_RECORD_FRAME_HEADER_INFO_HEAD].iHeaderInfo.iInfoExNums;iRecSeekSegFrameId<(CVI_S32)pSegIndexRecord->iInfoCount;iRecSeekSegFrameId++)
                                 {
                                     if(!s_mPkgParam.iPkginited || s_mPkgParam.bPkgstop || !pPoper->pThreadRun || !pPoper->bOpen)
                                     {
                                         _RECORD_DBG("[PB EXIT]End of flag.iPkginited:%d,bPkgstop:%d,pThreadRun:%d,open:%d\n",
                                            s_mPkgParam.iPkginited, s_mPkgParam.bPkgstop, pPoper->pThreadRun, pPoper->bOpen);
                                         goto BEGIN;
                                     }

                                     if(pPoper->tSeekTime > 0)
                                     {
                                         _RECORD_DBG("[PB SEEK]break seek time now %lu\n", pPoper->tSeekTime);
                                         goto BEGIN;
                                     }
                                     
                                     if(!pPoper->bStartPop)
                                     {
                                         //暂停中
                                         rsleep(50*1000);
                                         iRecSeekSegFrameId--;  //当前帧暂停所以不能退回
                                         continue;
                                     }
                                     
                                     if(bSendKeyframe || bSendIndexFrame || pPoper->PopKeyInterval)
                                     {
                                         CVI_S32 bGetOk = 0;
                                         CVI_S32 j = 0;
                                         for(j=0;j<CVI_RECORD_KEYINDEX_PER_NUMS;j++)
                                         {
                                             if(iRecSeekSegFrameId == iPkgWriteIndexHead[CVI_RECORD_FRAME_HEADER_INFO_KEY0].iIndexInfo.iIndexFrame[j])
                                             {
                                                 bGetOk = 1;
                                                 break;
                                             }
                                         }
                                         if(0 == bGetOk)
                                         {
                                             for(j=0;j<CVI_RECORD_KEYINDEX_PER_NUMS;j++)
                                             {
                                                 if(iRecSeekSegFrameId == iPkgWriteIndexHead[CVI_RECORD_FRAME_HEADER_INFO_KEY1].iIndexInfo.iIndexFrame[j])
                                                 {
                                                     bGetOk = 1;
                                                     break;
                                                 }
                                             }
                                         }
                                         if(0 == bGetOk)
                                         {
                                            continue;
                                         }
                                     }
                                     
                                     if(0 != CVI_RECORD_ReadSegmentFrameInfo(iFilefd, pSegIndexRecord->iInfoEndOffset-((iRecSeekSegFrameId+1)*sizeof(CVI_RECORD_SEGMENT_INFO)), &iPkgWriteIndexBuff, 1, NULL))
                                     {
                                          _RECORD_ERR("[skip frame]Reader frame0 Index:%d seg:%d id:%d, offset:%u, %u\n", iRecFileNo, iRecSeekSegId, iRecSeekSegFrameId, pSegIndexRecord->iInfoEndOffset, ((iRecSeekSegFrameId+1)*sizeof(CVI_RECORD_SEGMENT_INFO)));
                                          continue;
                                     }
                                     else
                                     {
                                         if(CVI_RECORD_INFO_TYPE_FRAME != iPkgWriteIndexBuff.iInfoType)
                                         {
                                             _RECORD_ERR("[skip frame]Reader frame1 Index:%d seg:%d,type:%d,id:%d, offset:%u, %u\n", iRecFileNo, iRecSeekSegId, iPkgWriteIndexBuff.iInfoType, iRecSeekSegFrameId, pSegIndexRecord->iInfoEndOffset, ((iRecSeekSegFrameId+1)*sizeof(CVI_RECORD_SEGMENT_INFO)));
                                             continue;
                                         }
                                         else
                                         {
                                             if((iPkgWriteIndexBuff.iFrameInfo.iShowTime >= stSeekTime)
                                                && (iPkgWriteIndexBuff.iFrameInfo.iShowTime >= pRecSeekReader->tBeginTime)
                                                && (iPkgWriteIndexBuff.iFrameInfo.iShowTime <= pRecSeekReader->tEndTime)
                                                && (iPkgWriteIndexBuff.iFrameInfo.iShowTime >= pPoper->tBeginTime)
                                                && (iPkgWriteIndexBuff.iFrameInfo.iShowTime <= pPoper->tEndTime))
                                             {
                                                 CVI_S32 bKeyFrame = 0;
                                                 CVI_S32 bIndexFrame = 0;
                                                 if(CVI_RECORD_CheckFrameValid(iPkgWriteIndexBuff.iFrameInfo.iFrameType, iPkgWriteIndexBuff.iFrameInfo.iSubType, &bKeyFrame, &bIndexFrame)
                                                      && (iPkgWriteIndexBuff.iFrameInfo.iFrameLen <= PKG_FRAME_MAX_LEN))
                                                 {
                                                     //找到有效帧 
                                                     if(bSendKeyframe)
                                                     {
                                                         if(!bKeyFrame)
                                                         {
                                                             _RECORD_ERR("[skip frame]Reader frame need key frame\n");
                                                             continue;
                                                         }
                                                     }
                                                     else if(bSendIndexFrame)
                                                     {
                                                         if(!bIndexFrame)
                                                         {
                                                             _RECORD_ERR("[skip frame]Reader frame need index frame\n");
                                                             continue;
                                                         }
                                                     }
                                                     if(CVI_RECORD_FRAME_TYPE_G711U != iPkgWriteIndexBuff.iFrameInfo.iFrameType)
                                                     {
                                                          if(!bIndexFrame && !bKeyFrame)
                                                          {
                                                              if(iPkgWriteIndexBuff.iFrameInfo.iFrameNo-iLastVideoNo >= CVI_RECORD_MAX_SKIP_NO)
                                                              {
                                                                  _RECORD_ERR("[skip frame]Reader frame no skip %d->%d\n", iLastVideoNo, iPkgWriteIndexBuff.iFrameInfo.iFrameNo);
                                                                  bIndexFrame = 1;
                                                                  continue;
                                                              }
                                                         }
                                                          
                                                          if(iPkgWriteIndexBuff.iFrameInfo.iFrameNo-iLastVideoNo != 1)
                                                          {
                                                              _RECORD_ERR("[skip frame] Video frame no skip %d->%d\n", iLastVideoNo, iPkgWriteIndexBuff.iFrameInfo.iFrameNo);
                                                          }
                                                     }
                                                     else
                                                     {
                                                        
                                                        if(iPkgWriteIndexBuff.iFrameInfo.iFrameNo-iLastAudioNo != 1)
                                                        {
                                                            _RECORD_ERR("[skip frame] Audio frame no skip %d->%d\n", iLastAudioNo, iPkgWriteIndexBuff.iFrameInfo.iFrameNo);
                                                        }
                                                        
                                                     }
                                                     
                                                     if(PKG_READ_DEFAULT_FRAME_LEN < iPkgWriteIndexBuff.iFrameInfo.iFrameLen)
                                                     {
                                                         _RECORD_DBG("Remalloc frame Len:%d\n", iPkgWriteIndexBuff.iFrameInfo.iFrameLen);
                                                         free(pFrameBuf);
                                                         pFrameBuf = malloc(iPkgWriteIndexBuff.iFrameInfo.iFrameLen);
                                                         if(NULL == pFrameBuf)
                                                         {
                                                             _RECORD_ERR("[PB EXIT]pkgstorage malloc err1: stPkgBuf %d\n", PKG_READ_DEFAULT_FRAME_LEN);
                                                             pPoper->pGetDataCallback(pPoper, NULL, CVI_REPLAY_CB_ERROR);
                                                             goto ENDFUNC;
                                                          }
                                                     }
                                                     
                                                     if(iPkgWriteIndexBuff.iFrameInfo.iFrameLen == (CVI_U32)CVI_RECORD_ReadFileOffset(iFilefd, iPkgWriteIndexBuff.iFrameInfo.iStartOffset, pFrameBuf, iPkgWriteIndexBuff.iFrameInfo.iFrameLen))
                                                     {
                                                         T_FRAME_INFO stFrame;
                                                         memset(&stFrame, 0, sizeof(stFrame));
                                                         stFrame.iFrameType = iPkgWriteIndexBuff.iFrameInfo.iFrameType;
                                                         stFrame.iSubType = iPkgWriteIndexBuff.iFrameInfo.iSubType;
                                                         stFrame.iFrameNo = iPkgWriteIndexBuff.iFrameInfo.iFrameNo;
                                                         stFrame.iFrameLen = iPkgWriteIndexBuff.iFrameInfo.iFrameLen;
                                                         stFrame.iFrame_absTime = iPkgWriteIndexBuff.iFrameInfo.iFrame_absTime;
                                                         stFrame.iShowTime = iPkgWriteIndexBuff.iFrameInfo.iShowTime;
                                                         stFrame.pBuf = pFrameBuf;
                                                         pPoper->pGetDataCallback(pPoper, &stFrame, CVI_REPLAY_CB_NONE);
                                                         if(CVI_RECORD_FRAME_TYPE_G711U != iPkgWriteIndexBuff.iFrameInfo.iFrameType)
                                                         {
                                                             iLastVideoNo = iPkgWriteIndexBuff.iFrameInfo.iFrameNo;
                                                         }
                                                         else
                                                         {
                                                             iLastAudioNo = iPkgWriteIndexBuff.iFrameInfo.iFrameNo;
                                                         }
                                                         
                                                         
                                                         bSendKeyframe = 0;
                                                         bSendIndexFrame = 0;
                                                     }
                                                     else
                                                     {
                                                        bIndexFrame = 1;
                                                         _RECORD_ERR("[skip frame]Reader len err, Index:%d seg:%d,type:%d.%d,id:%d,offset:%d,len:%d\n",
                                                             iRecFileNo, iRecSeekSegId, iPkgWriteIndexBuff.iFrameInfo.iFrameType, iPkgWriteIndexBuff.iFrameInfo.iSubType, iRecSeekSegFrameId,
                                                             iPkgWriteIndexBuff.iFrameInfo.iStartOffset, iPkgWriteIndexBuff.iFrameInfo.iFrameLen);
                                                     }
                                                     
                                                     if(PKG_READ_DEFAULT_FRAME_LEN < iPkgWriteIndexBuff.iFrameInfo.iFrameLen)
                                                     {
                                                         free(pFrameBuf);
                                                         pFrameBuf = malloc(PKG_READ_DEFAULT_FRAME_LEN);
                                                         if(NULL == pFrameBuf)
                                                         {
                                                             _RECORD_ERR("[PB EXIT]pkgstorage malloc err2: stPkgBuf %d\n", PKG_READ_DEFAULT_FRAME_LEN);
                                                             pPoper->pGetDataCallback(pPoper, NULL, CVI_REPLAY_CB_ERROR);
                                                             goto ENDFUNC;
                                                         }
                                                     }
                                                     stSeekTime = 0;
                                                 }
                                                 else
                                                 {
                                                     _RECORD_ERR("[skip frame]Reader frame2 Index:%d seg:%d,type:%d.%d,id:%d.len:%d\n",
                                                         iRecFileNo, iRecSeekSegId, iPkgWriteIndexBuff.iFrameInfo.iFrameType, iPkgWriteIndexBuff.iFrameInfo.iSubType, iRecSeekSegFrameId, iPkgWriteIndexBuff.iFrameInfo.iFrameLen);
                                                     continue;
                                                 }
                                             }
                                             else
                                             {
                                                 //下一帧
                                                 _RECORD_DBG("[skip frame] frame(%lu) over, seek:%lu,reader[%lu-%lu]\n",
                                                     iPkgWriteIndexBuff.iFrameInfo.iShowTime, stSeekTime,
                                                     pRecSeekReader->tBeginTime, pRecSeekReader->tEndTime);
                                                 continue;
                                             }
                                         }
                                     }
                                 }
                            }
                        }
                        
                        iRecSeekSegId++;
                        
                        if(iRecSeekSegId > pRecSeekReader->iEndSegNum)
                        {
                            _RECORD_DBG("Read End reader %d(seg:%d-%d)\n", 
                                pRecSeekReader->iRecFileNo, pRecSeekReader->iStartSegNum, pRecSeekReader->iEndSegNum);
                            
                            pRecSeekReader = pRecSeekReader->ptNext;
                            if(NULL == pRecSeekReader)
                            {
                                pPoper->pGetDataCallback(pPoper, NULL, CVI_REPLAY_CB_FINISH);
                                rsleep(50*1000);
                                goto BEGIN;
                            }
                            else
                            {
                                iRecSeekSegId = pRecSeekReader->iStartSegNum;
                            }
                            _RECORD_DBG("Read Next reader %d(seg:%d-%d)\n", 
                                pRecSeekReader->iRecFileNo, pRecSeekReader->iStartSegNum, pRecSeekReader->iEndSegNum);
                        }
                    }
                    else
                    {
                        pPoper->pGetDataCallback(pPoper, NULL, CVI_REPLAY_CB_ERROR);
                        _RECORD_ERR("[PB EXIT] Read Err, out of read, %d\n", iRecSeekSegId);
                        goto ENDFUNC;
                    }
                }
                else
                {
                    //播放完成
                    _RECORD_DBG("[PB END] Reader is end\n");
                    rsleep(50*1000);
                    goto BEGIN;
                }
            }
            else
            {
                //暂停中
                rsleep(50*1000);
                goto BEGIN;
            }
        }
        else
        {
            _RECORD_ERR("Pkg Err,thread no open(%d),  hDataPopper = 0x%x\n", pPoper->bOpen, (CVI_U32)pPoper);
            break;
        }
BEGIN:
        continue;
    }
    
ENDFUNC:
    if(iIndexfd > 0)
    {
        close(iIndexfd);
        iIndexfd = -1;
    }
    if(iFilefd > 0)
    {
        close(iFilefd);
        iFilefd = -1;
    }
    CVI_REPLAY_FreeSeekArr(pRecSeekHead);
    CVI_RECORD_FreeBuffer(&stPkgBuf);
    if(pFrameBuf)
    {
        free(pFrameBuf);
        pFrameBuf = NULL;
    }
    CVI_RECORD_FUNC_END;
    return NULL;
}

/*****************************************************************************
 函 数 名  : CVI_REPLAY_Create
 功能描述  :创建回放句柄
 输入参数  : 
 IN time_t tStartSeekTime  开始时间
 IN time_t tEndSeekTime   结束时间
 IN CVI_S32 iEvenType             事件 0全天
 IN CVI_REPLAY_DATAPOP_CALLBACK cbReplayCallback   数据回调
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API HANDLE API_CALL CVI_REPLAY_Create(IN time_t tStartSeekTime, IN time_t tEndSeekTime, IN CVI_U32 iEvenType, IN CVI_REPLAY_DATAPOP_CALLBACK cbReplayCallback)
{
    CVI_RECORD_FUNC_START;
    CVI_S32 i = 0;
    CVI_S32 iPoperIndex = 0xFFFFFFFF;
    //判断系统是否已经初始化
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        return NULL;
    }

    if(s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return NULL;
    }

    //参数检查
    if (NULL == cbReplayCallback) {
        _RECORD_ERR("Pkg Invalid input\n");
        return NULL;
    }

    if(0 == tStartSeekTime) {
        _RECORD_ERR("Pkg Invalid input\n");
        return NULL;
    }

    if ((tEndSeekTime <= tStartSeekTime) && (0 != tEndSeekTime)) {
        _RECORD_ERR("Exit ERR! tStartSeekTime = %ld tEndSeekTime = %ld\n", tStartSeekTime, tEndSeekTime);
        return NULL;
    }
    //弹出结束时间为0时,将弹出结束时间设置到最大,表示动态弹出
    if (0 == tEndSeekTime)
        tEndSeekTime = 0x7FFFFFFF;

    for (i = 0; i < CVI_RECORD_MAX_RECORD_READER; i ++) {
        if (pthread_mutex_trylock(&s_mDataPoper[i].iPopMutex))
            continue;
        if (!s_mDataPoper[i].bOpen) {
            iPoperIndex = i;
            _RECORD_INFO("[PkgStream]idleBackupIndex: %d\n", iPoperIndex);
            break;
        }
        pthread_mutex_unlock(&s_mDataPoper[i].iPopMutex);
    }

    if (iPoperIndex != (CVI_S32)0xFFFFFFFF) {
        s_mDataPoper[iPoperIndex].pThreadRun = 1;
        s_mDataPoper[iPoperIndex].bOpen = 1;
        //创建弹出线程
        if (0 != pthread_create(&s_mDataPoper[iPoperIndex].pPopThread, NULL, CVI_REPLAY_DataPopProc, &s_mDataPoper[iPoperIndex])) {
            s_mDataPoper[iPoperIndex].pThreadRun = 0;
            s_mDataPoper[iPoperIndex].bOpen = 0;
            pthread_mutex_unlock(&s_mDataPoper[iPoperIndex].iPopMutex);
            _RECORD_ERR("Exit ERR! pthread_create return\n");
            return NULL;
        }

        s_mDataPoper[iPoperIndex].bStartPop = 0;
        s_mDataPoper[iPoperIndex].tBeginTime = tStartSeekTime;
        s_mDataPoper[iPoperIndex].tEndTime = tEndSeekTime;
        s_mDataPoper[iPoperIndex].tSeekTime = tStartSeekTime;
        s_mDataPoper[iPoperIndex].iEvenType = iEvenType;
        s_mDataPoper[iPoperIndex].PopKeyInterval = 0;
        s_mDataPoper[iPoperIndex].pGetDataCallback = cbReplayCallback;

        pthread_mutex_unlock(&s_mDataPoper[iPoperIndex].iPopMutex);
        _RECORD_INFO("creat OK! Handle = 0x%x\n", (CVI_U32)&s_mDataPoper[iPoperIndex]);
        CVI_RECORD_FUNC_END;
        return (HANDLE)&s_mDataPoper[iPoperIndex];
    }

    CVI_RECORD_FUNC_END;
    return NULL;
}

/*****************************************************************************
 函 数 名  : CVI_REPLAY_DataSetKeyFrame
 功能描述  :回放是否只取I帧
 输入参数  : 
 IN HANDLE hDataPopper                    句柄
 IN CVI_U32 PopKeyInterval           I间隔0全部弹出
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_DataSetKeyFrame(IN HANDLE hDataPopper, IN CVI_U32 PopKeyInterval)
{
    CVI_RECORD_FUNC_START;
    T_PSS_DataPoper *pPoper = NULL;
    CVI_S32 iRet = 0;
    //判断系统是否已经初始化
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        return ERR_SYS_NOT_INIT;
    }

    if (s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    //参数检查
    if (hDataPopper == NULL) {
        _RECORD_ERR("Pkg Invalid input\n");
        return ERR_INVALID_ARGUMENT;
    }

    if (!CVI_REPLAY_IsValid(hDataPopper)) {
        _RECORD_ERR("Pkg Invalid input Exit ERR! hDataPopper = 0x%x\n", (CVI_U32)hDataPopper);
        return ERR_INVALID_HANDLE;
    }
    pPoper = (T_PSS_DataPoper*)hDataPopper;

    pthread_mutex_lock(&pPoper->iPopMutex);
    if(pPoper->bOpen) {
        //阻塞弹出线程
        if(pPoper->PopKeyInterval != (CVI_UL)PopKeyInterval) {
            if (pPoper->PopKeyInterval > 4)
                pPoper->PopKeyInterval = 4;
        }
    } else {
        _RECORD_ERR("Pkg Invalid no open\n");
    }
    pthread_mutex_unlock(&pPoper->iPopMutex);

    CVI_RECORD_FUNC_END;
    return iRet;
}


/*****************************************************************************
 函 数 名  : CVI_REPLAY_Pause
 功能描述  :暂停回放
 输入参数  : 
 IN HANDLE hDataPopper                    句柄
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_Pause(IN HANDLE hDataPopper)
{
    CVI_RECORD_FUNC_START;
    T_PSS_DataPoper *pPoper = NULL;
    CVI_S32 iRet = 0;
    //判断系统是否已经初始化
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    if(s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    //参数检查
    if (hDataPopper == NULL) {
        _RECORD_ERR("Pkg Invalid input\n");
        CVI_RECORD_FUNC_END;
        return ERR_INVALID_ARGUMENT;
    }

    if (!CVI_REPLAY_IsValid(hDataPopper)) {
        _RECORD_ERR("Pkg Invalid input Exit ERR! hDataPopper = 0x%x\n", (CVI_U32)hDataPopper);
        CVI_RECORD_FUNC_END;
        return ERR_INVALID_HANDLE;
    }
    pPoper = (T_PSS_DataPoper*)hDataPopper;

    pthread_mutex_lock(&pPoper->iPopMutex);
    if(pPoper->bOpen)
        pPoper->bStartPop = 0;
    else
        _RECORD_ERR("Pkg Invalid no open\n");
    pthread_mutex_unlock(&pPoper->iPopMutex);
    _RECORD_DBG("Pkg pause hDataPopper = 0x%x\n", (CVI_U32)hDataPopper);
    CVI_RECORD_FUNC_END;
    return iRet;
}

/*****************************************************************************
 函 数 名  : CVI_REPLAY_Resume
 功能描述  :恢复回放
 输入参数  : 
 IN HANDLE hDataPopper                    句柄
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_Resume(IN HANDLE hDataPopper)
{
    CVI_RECORD_FUNC_START;
    T_PSS_DataPoper *pPoper = NULL;
    CVI_S32 iRet = 0;
    //判断系统是否已经初始化
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    if(s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    //参数检查
    if (hDataPopper == NULL) {
        _RECORD_ERR("Pkg Invalid input\n");
        CVI_RECORD_FUNC_END;
        return ERR_INVALID_ARGUMENT;
    }

    if (!CVI_REPLAY_IsValid(hDataPopper)) {
        _RECORD_ERR("Pkg Invalid input Exit ERR! hDataPopper = 0x%x\n", (CVI_U32)hDataPopper);
        CVI_RECORD_FUNC_END;
        return ERR_INVALID_HANDLE;
    }
    pPoper = (T_PSS_DataPoper*)hDataPopper;

    pthread_mutex_lock(&pPoper->iPopMutex);
    if(pPoper->bOpen)
        pPoper->bStartPop = 1;
    else
        _RECORD_ERR("Pkg Invalid no open\n");
    pthread_mutex_unlock(&pPoper->iPopMutex);
    _RECORD_DBG("Pkg resume hDataPopper = 0x%x\n", (CVI_U32)hDataPopper);
    CVI_RECORD_FUNC_END;
    return iRet;
}

/*****************************************************************************
 函 数 名  : CVI_REPLAY_Release
 功能描述  :销毁回放
 输入参数  : 
 IN HANDLE hDataPopper                    句柄
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_Release(IN HANDLE hDataPopper)
{
    CVI_RECORD_FUNC_START;
    T_PSS_DataPoper *pPoper = NULL;
    CVI_S32 iRet = 0;
    //判断系统是否已经初始化
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    //参数检查
    if (hDataPopper == NULL) {
        _RECORD_ERR("Pkg Invalid input\n");
        CVI_RECORD_FUNC_END;
        return ERR_INVALID_ARGUMENT;
    }
    
    if (!CVI_REPLAY_IsValid(hDataPopper)) {
        _RECORD_ERR("Pkg Invalid input Exit ERR! hDataPopper = 0x%x\n", (CVI_U32)hDataPopper);
        CVI_RECORD_FUNC_END;
        return ERR_INVALID_HANDLE;
    }
    pPoper = (T_PSS_DataPoper*)hDataPopper;
    if (pPoper->pThreadRun) {
        pPoper->pThreadRun = 0;
        _RECORD_DBG("Wait end thread, hDataPopper = 0x%x\n", (CVI_U32)hDataPopper);
        pthread_join(pPoper->pPopThread, NULL);
    }
    
    pthread_mutex_lock(&pPoper->iPopMutex);
    if(pPoper->bOpen) {
        pPoper->bOpen = 0;
        pPoper->bStartPop = 0;
        pPoper->tBeginTime = 0;
        pPoper->tEndTime = 0;
        pPoper->iEvenType = 0;
        pPoper->tSeekTime = 0;
        pPoper->PopKeyInterval = 0;
        pPoper->pGetDataCallback = NULL;
    } else {
        _RECORD_ERR("Pkg Invalid no open\n");
    }
    pthread_mutex_unlock(&pPoper->iPopMutex);
    CVI_RECORD_FUNC_END;
    return iRet;
}




/*****************************************************************************
 函 数 名  : CVI_REPLAY_Seek
 功能描述  :回放的位置
 输入参数  : 
 IN HANDLE hDataPopper                    句柄
 IN  time_t SeekTime                         弹出数据时间
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_Seek(IN HANDLE hDataPopper,IN  time_t SeekTime)
{
    CVI_RECORD_FUNC_START;
    T_PSS_DataPoper *pPoper = NULL;
    CVI_S32 iRet = 0;
    //判断系统是否已经初始化
    if (0 == s_mPkgParam.iPkginited) {
        _RECORD_ERR("Pkg not init\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    if(s_mPkgParam.bPkgstop) {
        _RECORD_ERR("Pkg stop\n");
        CVI_RECORD_FUNC_END;
        return ERR_SYS_NOT_INIT;
    }

    //参数检查
    if (hDataPopper == NULL) {
        _RECORD_ERR("Pkg Invalid input\n");
        CVI_RECORD_FUNC_END;
        return ERR_INVALID_ARGUMENT;
    }

    if (!CVI_REPLAY_IsValid(hDataPopper)) {
        _RECORD_ERR("Pkg Invalid input Exit ERR! hDataPopper = 0x%x\n", (CVI_U32)hDataPopper);
        CVI_RECORD_FUNC_END;
        return ERR_INVALID_HANDLE;
    }
    pPoper = (T_PSS_DataPoper*)hDataPopper;
    
    pthread_mutex_lock(&pPoper->iPopMutex);

    CVI_CHAR sStartTime[20];
    CVI_CHAR sEndTime[20];
    CVI_CHAR sTime[20];
    _RECORD_INFO("Pkg seek time %s(%s-%s)\n",
                time_to_string(SeekTime, sTime),
                time_to_string(pPoper->tBeginTime, sStartTime),
                time_to_string(pPoper->tEndTime, sEndTime));

    if (pPoper->bOpen) {
        if ((pPoper->tBeginTime <= SeekTime) && (pPoper->tEndTime > SeekTime)) {
            pPoper->tSeekTime = SeekTime;
            pPoper->bStartPop = 1;
        } else if(0 == SeekTime) {
            pPoper->tSeekTime = pPoper->tBeginTime;
            pPoper->bStartPop = 1;
        } else {
            _RECORD_ERR("Pkg Invalid seek time %lu(%lu-%lu)\n", SeekTime, pPoper->tSeekTime, pPoper->tEndTime);
        }
    } else {
        _RECORD_ERR("Pkg Invalid no open\n");
    }

    pthread_mutex_unlock(&pPoper->iPopMutex);
    CVI_RECORD_FUNC_END;
    return iRet;
}

PSS_API CVI_S32 API_CALL cvi_system(IN const CVI_CHAR *pszCmd)
{
	pid_t pid;
	int status;
	if (pszCmd == NULL) {
		return 1; /**< if cmdstring is NULL return no zero */
	}
	if ((pid = vfork()) < 0) { /**< vfork,child pid share resource with parrent,not copy */
		status = -1;			/**<vfork fail */
	} else if (pid == 0) {
		execl("/bin/sh", "sh", "-c", pszCmd, (CVI_CHAR *)0);
		_exit(127); /**< return 127 only exec fail;the chid procee is not exist normore if exec success fail */
	} else {		/** parrent pid */
		while (waitpid(pid, &status, 0) < 0) {
			if (errno != EINTR) {
				status = -1; /**< return -1 when interrupted by signal except EINTR */
				break;
			}
		}
	}
	return status; /**< return the state of child progress if waitpid success */
}

