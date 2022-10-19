#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <ucontext.h>
#include <semaphore.h>
#include <assert.h>
#include <dlfcn.h>
#include <sys/prctl.h>
#include <pthread.h>

#include "autoconf.h"

#include "cvi_buffer.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_comm_isp.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vi.h"
#include "cvi_isp.h"
#include "cvi_ae.h"
#include <linux/cvi_comm_vpss.h>
#include "sample_comm.h"

#include "cvi_venc.h"
#include "cvi_rtsp.h"
#include <sys/time.h>
#include "app_rtsp.h"


#if defined(CONFIG_RGB_VIDEO_RTSP_SUPPORT) || defined(CONFIG_IR_VIDEO_RTSP_SUPPORT)
static CVI_RTSP_CTX *ctx = NULL;
static CVI_RTSP_SESSION *session[2] = {NULL, NULL};
static pthread_t pthRtsp[2] = {0, 0};
static unsigned int bRtspRun[2] = {0,0};

static void rtsp_connect(const char *ip, void *arg) {
    printf("rtsp connect: %s\n", ip );
	UNUSED(arg);
}

static void rtsp_disconnect(const char *ip, void *arg) {
   printf( "rtsp disconnect: %s\n",ip);
   UNUSED(arg);
}

void SendToRtsp(CVI_RTSP_CTX *ctx, CVI_RTSP_SESSION *session, VENC_STREAM_S *pstStream)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VENC_PACK_S *ppack;
	CVI_RTSP_DATA data;

	memset(&data, 0, sizeof(CVI_RTSP_DATA));

	data.blockCnt = pstStream->u32PackCount;
	for (CVI_U32 i = 0; i < pstStream->u32PackCount; i++) {
		ppack = &pstStream->pstPack[i];
		data.dataPtr[i] = ppack->pu8Addr + ppack->u32Offset;
		data.dataLen[i] = ppack->u32Len - ppack->u32Offset;
	}

	s32Ret = CVI_RTSP_WriteFrame(ctx, session->video, &data);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RTSP_WriteFrame failed\n");
	}
}

unsigned int GetCurTimeInMsec(void)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) < 0)
    {
        return 0;
    }
    return tv.tv_sec * 1000 + tv.tv_usec/1000;
}


static CVI_VOID *GetRgbVencStreamProc(CVI_VOID *pArgs)
{
	VENC_CHN VencChn = *((VENC_CHN *)pArgs);
	CVI_CHAR TaskName[64];
	CVI_S32 s32Ret;
    VENC_CHN_STATUS_S stStat;
    VENC_STREAM_S stStream;
	sprintf(TaskName, "chn%dVencGetStream", VencChn);
	prctl(PR_SET_NAME, TaskName, 0, 0, 0);
	printf("venc task%d start\n", VencChn);

    VENC_RECV_PIC_PARAM_S stRecvParam = {0};
    stRecvParam.s32RecvPicNum = -1;
    CVI_VENC_StartRecvFrame(VencChn, &stRecvParam);

    CVI_S32 iTime = GetCurTimeInMsec();
    CVI_S32 vencFd = 0;
    struct timeval timeoutVal;
    fd_set readFds;

	while (bRtspRun[VencChn]) {
        #if defined(CONFIG_RGB_VIDEO_RTSP_VENC_BIND_DISABLE)
        VIDEO_FRAME_INFO_S stVencFrame;
        s32Ret = CVI_VPSS_GetChnFrame(1, 1, &stVencFrame, 3000);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_VPSS_GetChnFrame failed with %#x\n", s32Ret);
			break;
		}

		CVI_S32 s32SetFrameMilliSec = 20000;
		// VENC_STREAM_S stStream;
		// VENC_CHN_STATUS_S stStat;
		VENC_CHN_ATTR_S stVencChnAttr;

		s32Ret = CVI_VENC_SendFrame(VencChn, &stVencFrame, s32SetFrameMilliSec);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_VENC_SendFrame failed! %d\n", s32Ret);
			break;
		}
		s32Ret = CVI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_VENC_GetChnAttr, VencChn = %d, s32Ret = %d\n",
					VencChn, s32Ret);
			break;
		}
        #endif
        vencFd = CVI_VENC_GetFd(VencChn);
        if (vencFd <= 0) {
            printf("CVI_VENC_GetFd failed with%#x!\n", vencFd);
            break;
        }
        FD_ZERO(&readFds);
        FD_SET(vencFd, &readFds);
        timeoutVal.tv_sec = 0;
        timeoutVal.tv_usec = 120*1000;
        iTime = GetCurTimeInMsec();
        s32Ret = select(vencFd + 1, &readFds, NULL, NULL, &timeoutVal);
        if (s32Ret < 0) {
            if (errno == EINTR)
                continue;
            printf("VencChn(%d) select failed!\n", VencChn);
            break;
        } else if (s32Ret == 0) {
            printf("VencChn(%d) select timeout %u ms \n", 
                                            VencChn, (GetCurTimeInMsec() - iTime));
            continue;
        }
        s32Ret = CVI_VENC_QueryStatus(VencChn, &stStat);
        if (s32Ret != CVI_SUCCESS) {
            printf("CVI_VENC_QueryStatus, Vench = %d, s32Ret = %d\n",
                    VencChn, s32Ret);
            break;
        }
        if (!stStat.u32CurPacks) {
            usleep(10000);
            continue;
        }
        stStream.pstPack =
            (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
        if (stStream.pstPack == NULL) {
            printf("malloc memory failed!\n");
            break;
        }

        s32Ret = CVI_VENC_GetStream(VencChn, &stStream, 120);
        if (s32Ret != CVI_SUCCESS) {
            printf("CVI_VENC_GetStream, VencChn = %d, s32Ret = 0x%X\n",
                    VencChn, s32Ret);
            free(stStream.pstPack);
            stStream.pstPack = NULL;
            continue;;
        }

        SendToRtsp(ctx, session[VencChn], &stStream);
        s32Ret = CVI_VENC_ReleaseStream(VencChn, &stStream);
        if (s32Ret != CVI_SUCCESS) {
            printf("CVI_VENC_ReleaseStream, s32Ret = %d\n", s32Ret);
            free(stStream.pstPack);
            stStream.pstPack = NULL;
            continue;
        }

        #if defined(CONFIG_RGB_VIDEO_RTSP_VENC_BIND_DISABLE)
        if (CVI_VPSS_ReleaseChnFrame(1, 1, &stVencFrame) != CVI_SUCCESS) {
			printf("CVI_VPSS_ReleaseChnFrame NG\n");
        }
        #endif

        free(stStream.pstPack);
        stStream.pstPack = NULL;
    }
	printf("venc task%d end\n", VencChn);

	return (CVI_VOID *) CVI_SUCCESS;
}

static CVI_VOID *GetIrVencStreamProc(CVI_VOID *pArgs)
{
	VENC_CHN VencChn = *((VENC_CHN *)pArgs);
	CVI_CHAR TaskName[64];
	CVI_S32 s32Ret;
    VENC_CHN_STATUS_S stStat;
    VENC_STREAM_S stStream;
#if defined(CONFIG_IR_VIDEO_RTSP_VENC_BIND_DISABLE)
    VIDEO_FRAME_INFO_S stVencFrame;
#endif
	sprintf(TaskName, "chn%dVencGetStream", VencChn);
	prctl(PR_SET_NAME, TaskName, 0, 0, 0);
	printf("venc task%d start\n", VencChn);

    VENC_RECV_PIC_PARAM_S stRecvParam = {0};
    stRecvParam.s32RecvPicNum = -1;
    CVI_VENC_StartRecvFrame(VencChn, &stRecvParam);

    CVI_S32 iTime = GetCurTimeInMsec();
    CVI_S32 vencFd = 0;
    struct timeval timeoutVal;
    fd_set readFds;

	while (bRtspRun[VencChn]) {
        #if defined(CONFIG_IR_VIDEO_RTSP_VENC_BIND_DISABLE)
        s32Ret = CVI_VI_GetChnFrame(0, 1, &stVencFrame, 3000);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_VPSS_GetChnFrame failed with %#x\n", s32Ret);
			break;
		}

		CVI_S32 s32SetFrameMilliSec = 20000;
		// VENC_STREAM_S stStream;
		// VENC_CHN_STATUS_S stStat;
		VENC_CHN_ATTR_S stVencChnAttr;

		s32Ret = CVI_VENC_SendFrame(VencChn, &stVencFrame, s32SetFrameMilliSec);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_VENC_SendFrame failed! %d\n", s32Ret);
			break;
		}
		s32Ret = CVI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_VENC_GetChnAttr, VencChn = %d, s32Ret = %d\n",
					VencChn, s32Ret);
			break;
		}
        #endif

        vencFd = CVI_VENC_GetFd(VencChn);
        if (vencFd <= 0) {
            printf("CVI_VENC_GetFd failed with%#x!\n", vencFd);
            break;
        }
        FD_ZERO(&readFds);
        FD_SET(vencFd, &readFds);
        timeoutVal.tv_sec = 0;
        timeoutVal.tv_usec = 120*1000;
        iTime = GetCurTimeInMsec();
        s32Ret = select(vencFd + 1, &readFds, NULL, NULL, &timeoutVal);
        if (s32Ret < 0) {
            if (errno == EINTR)
                continue;
            printf("VencChn(%d) select failed!\n", VencChn);
            break;
        } else if (s32Ret == 0) {
            printf("VencChn(%d) select timeout %u ms \n", 
                                            VencChn, (GetCurTimeInMsec() - iTime));
            continue;
        }

        s32Ret = CVI_VENC_QueryStatus(VencChn, &stStat);
        if (s32Ret != CVI_SUCCESS) {
            printf("CVI_VENC_QueryStatus, Vench = %d, s32Ret = %d\n",
                    VencChn, s32Ret);
            break;
        }
        if (!stStat.u32CurPacks) {
            usleep(10000);
            continue;
        }
        stStream.pstPack =
            (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
        if (stStream.pstPack == NULL) {
            printf("malloc memory failed!\n");
            break;
        }

        s32Ret = CVI_VENC_GetStream(VencChn, &stStream, 5000);
        if (s32Ret != CVI_SUCCESS) {
            printf("CVI_VENC_GetStream, VencChn = %d, s32Ret = 0x%X\n",
                    VencChn, s32Ret);
            free(stStream.pstPack);
            stStream.pstPack = NULL;
            continue;
        }

        SendToRtsp(ctx, session[VencChn], &stStream);
        s32Ret = CVI_VENC_ReleaseStream(VencChn, &stStream);
        if (s32Ret != CVI_SUCCESS) {
            printf("CVI_VENC_ReleaseStream, s32Ret = %d\n", s32Ret);
            free(stStream.pstPack);
            stStream.pstPack = NULL;
            continue;
        }

        #if defined(CONFIG_IR_VIDEO_RTSP_VENC_BIND_DISABLE)
        if (CVI_VI_ReleaseChnFrame(0, 1, &stVencFrame) != CVI_SUCCESS) {
			printf("CVI_VI_ReleaseChnFrame NG\n");
        }
        #endif

        free(stStream.pstPack);
        stStream.pstPack = NULL;
    }
	printf("venc task%d end\n", VencChn);

	return (CVI_VOID *) CVI_SUCCESS;
}

static int __rtsp_lib_open(void)
{
    static int __init = 0;

    if (__init == 0) {
        CVI_RTSP_CONFIG config;
        memset(&config, 0, sizeof(CVI_RTSP_CONFIG));
        config.port = 8554;
        if (0 > CVI_RTSP_Create(&ctx, &config)) {
            printf("fail to create rtsp contex\n");
            return -1;
        }
        // set listener
        CVI_RTSP_STATE_LISTENER listener = {0,0,0,0};
        listener.onConnect = rtsp_connect;
        listener.argConn = ctx;
        listener.onDisconnect = rtsp_disconnect;
        CVI_RTSP_SetListener(ctx, &listener);

        if (0 > CVI_RTSP_Start(ctx)) {
            printf("fail to start\n");
            return -1;
        }

        __init = 1;
    } 

    return 0;
}

int RTSP_CreateServer(int *VencChn, int num)
{
    int i = num;
    CVI_S32 s32Ret = CVI_SUCCESS;

    if(__rtsp_lib_open() != 0)
        return -1;

    CVI_RTSP_SESSION_ATTR attr;
    memset(&attr, 0x0, sizeof(CVI_RTSP_SESSION_ATTR));
    attr.video.codec = RTSP_VIDEO_H265;
    snprintf(attr.name, sizeof(attr.name), "live%d", i);
    CVI_RTSP_CreateSession(ctx, &attr, &session[i]);

    if(i == 0) {
        s32Ret = pthread_create(&pthRtsp[i], NULL, GetRgbVencStreamProc, (void *)&VencChn[i]);
    } else if (i == 1){
        s32Ret = pthread_create(&pthRtsp[i], NULL, GetIrVencStreamProc, (void *)&VencChn[i]);
    }

    if (s32Ret) {
        printf("[Chn %d]pthread_create failed\n", i);
        return CVI_FAILURE;
    }
    bRtspRun[i] = true;

    return 0;
}

int RTSP_StopServer(int *VencChn, int num)
{
    int i = num;
    bRtspRun[i] = false;
    pthread_join(pthRtsp[i], NULL);
    CVI_RTSP_DestroySession(ctx, session[i]);

    if((bRtspRun[0] == false) && (bRtspRun[1] == false)) {
        CVI_RTSP_Stop(ctx);
        CVI_RTSP_Destroy(&ctx);
        printf("stop rtsp server\n");
    }
	UNUSED(VencChn);

    return 0;
}



#if 0
int start_rtsp_server(int *VencChn, int num)
{
    CVI_RTSP_CONFIG config;
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (num < 1) {
        printf("VencChn num=0\n");
        return -1;
    }
    if (!VencChn) {
        printf("NULL Pointer\n");
        return -1;
    }

	memset(&config, 0, sizeof(CVI_RTSP_CONFIG));
	config.port = 8554;
    if (0 > CVI_RTSP_Create(&ctx, &config)) {
        printf("fail to create rtsp contex\n");
        return -1;
    }

    // set listener
    CVI_RTSP_STATE_LISTENER listener = {0,0,0,0};
    listener.onConnect = rtsp_connect;
    listener.argConn = ctx;
    listener.onDisconnect = rtsp_disconnect;
    CVI_RTSP_SetListener(ctx, &listener);

    if (0 > CVI_RTSP_Start(ctx)) {
        printf("fail to start\n");
        return -1;
    }

    for (int i = 0; i < num; i++) {
        CVI_RTSP_SESSION_ATTR attr;
		memset(&attr, 0x0, sizeof(CVI_RTSP_SESSION_ATTR));
        attr.video.codec = RTSP_VIDEO_H264;
        snprintf(attr.name, sizeof(attr.name), "live%d", i);
        CVI_RTSP_CreateSession(ctx, &attr, &session[i]);
    }

    bRun = true;
    for (int i = 0; i < num; i++) {
        s32Ret = pthread_create(&pthRtsp[i], NULL, GetVencStreamProc, (void *)&VencChn[i]);
        if (s32Ret) {
            printf("[Chn %d]pthread_create failed\n", i);
            return CVI_FAILURE;
        }
    }
    printf("start rtsp server\n");

    return 0;
}

void stop_rtsp_server(int *VencChn, int num)
{
    bRun = false;
    for (int i = 0; i < num; i++)
        pthread_join(pthRtsp[i], NULL);

    CVI_RTSP_Stop(ctx);
    for (int i = 0; i < num; i++)
        CVI_RTSP_DestroySession(ctx, session[i]);

    CVI_RTSP_Destroy(&ctx);
    printf("stop rtsp server\n");
	UNUSED(VencChn);
}
#endif
static void _VencinitInputCfg(chnInputCfg *pIc, commonInputCfg *pCic)
{
    memset(pIc, 0, sizeof(chnInputCfg));
    memset(pCic, 0, sizeof(commonInputCfg));
    pCic->ifInitVb = 0;
	pCic->vbMode = VB_SOURCE_COMMON;
	pCic->bSingleEsBuf_jpege = 0;
	pCic->bSingleEsBuf_h264e = 0;
	pCic->bSingleEsBuf_h265e = 0;
	pCic->singleEsBufSize_jpege = 0;
	pCic->singleEsBufSize_h264e = 0;
	pCic->singleEsBufSize_h265e = 0;
	pCic->h265RefreshType = 0;
	pCic->jpegMarkerOrder = 0;
	pCic->bThreadDisable = 0;

	pIc->u32Profile = 0;
	pIc->rcMode = 0;
	pIc->iqp = 38;
	pIc->pqp = 38;
	pIc->gop = 50;
	pIc->gopMode = 0;
	pIc->bitrate = 1800;
	pIc->firstFrmstartQp = 35;
	pIc->num_frames = -1;
	pIc->framerate = 25;
	pIc->bVariFpsEn = 0;
	pIc->maxIprop = 10;
	pIc->minIprop = 1;
	pIc->maxQp = 51;
	pIc->minQp = 20;
	pIc->maxIqp = 51;
	pIc->minIqp = 20;
	pIc->quality = -1;
	pIc->maxbitrate = 1800;
	pIc->statTime = 2;
	pIc->bind_mode = VENC_BIND_DISABLE;
	pIc->pixel_format = 0;
	pIc->bitstreamBufSize = 1024*1024;
	pIc->single_LumaBuf = 0;
	pIc->single_core = 0;
	pIc->forceIdr = -1;
	pIc->chgNum = -1;
	pIc->tempLayer = 0;
	pIc->bgInterval = 250;
	pIc->frameLost = -1;
	pIc->frameLostBspThr = -1;
	pIc->frameLostGap = -1;
	pIc->MCUPerECS = 0;
	pIc->s32IPQpDelta = 2;
	pIc->s32BgQpDelta = -2;
	pIc->s32ViQpDelta = 0;
	pIc->initialDelay = 1000;
	pIc->u32RowQpDelta = 0;
	pIc->s32MaxReEncodeTimes = 0;
    pIc->u32ThrdLv = 2;
    pIc->bBgEnhanceEn = 0;
    pIc->s32BgDeltaQp = 0;
    pIc->s32ChangePos = 75;
    pIc->s32AvbrPureStillThr = 4;
    pIc->s32AvbrFrmGap = 1;
    pIc->s32AvbrFrmLostOpen = 0;
    pIc->u32MotionSensitivity = 24;
    pIc->u32MaxStillQP = 38;
    pIc->s32MinStillPercent = 10;
}
chnInputCfg pIc[2]; //for venc
commonInputCfg pcic[2];


#if defined(CONFIG_RGB_VIDEO_RTSP_SUPPORT)
int APP_RgbVideoVencInit(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VENC_CHN VencChn = 0;
    PAYLOAD_TYPE_E enPayLoad = PT_H264;
	PIC_SIZE_E enSize = PIC_1080P;
    VENC_GOP_MODE_E enGopMode = VENC_GOPMODE_NORMALP;
    VENC_GOP_ATTR_S stGopAttr;
    SAMPLE_RC_E enRcMode;
    CVI_U32 u32Profile = 0;

    memset(&pIc[0], 0, sizeof(chnInputCfg));
    memset(&pcic[0], 0, sizeof(commonInputCfg));
    _VencinitInputCfg(&pIc[0], &pcic[0]);
    strcpy(pIc[0].codec, "264");

    enRcMode = (SAMPLE_RC_E) pIc[0].rcMode;
    s32Ret = SAMPLE_COMM_VENC_SetModParam(&pcic[0]);
    if (s32Ret != CVI_SUCCESS) {
        printf("SAMPLE_COMM_VENC_SetModParam failure\n");
        return CVI_FAILURE;
    }
    s32Ret = SAMPLE_COMM_VENC_GetGopAttr(enGopMode, &stGopAttr);
    if (s32Ret != CVI_SUCCESS) {
        printf("[Err]Venc Get GopAttr for %#x!\n", s32Ret);
        return CVI_FAILURE;
    }

    #if defined(CONFIG_RGB_VIDEO_RTSP_VENC_BIND_VI)
        printf("APP_RgbVideoVencInit--CONFIG_RGB_VIDEO_RTSP_VENC_BIND_VI\r\n");
	pIc[0].bind_mode = VENC_BIND_VI;
	pIc[0].width = 1080;
	pIc[0].height = 1920;
    #elif defined(CONFIG_RGB_VIDEO_RTSP_VENC_BIND_VPSS)
        printf("APP_RgbVideoVencInit--CONFIG_RGB_VIDEO_RTSP_VENC_BIND_VPSS\r\n");
    pIc[0].bind_mode = VENC_BIND_VPSS;
    pIc[0].width = 1280;
    pIc[0].height = 720;
    pIc[0].vpssGrp = 1,
    pIc[0].vpssChn = 1,
    #elif defined(CONFIG_IR_VIDEO_RTSP_VENC_BIND_DISABLE)
        printf("APP_RgbVideoVencInit--CONFIG_IR_VIDEO_RTSP_VENC_BIND_DISABLE\r\n");
    pIc[0].bind_mode = VENC_BIND_DISABLE;
	pIc[0].width = 720;
	pIc[0].height = 1280;
    #endif
    
	enSize = PIC_CUSTOMIZE;
	s32Ret = SAMPLE_COMM_VENC_Start(
			&pIc[0],
			VencChn,
			enPayLoad,
			enSize,
			enRcMode,
			u32Profile,
			CVI_FALSE,
			&stGopAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("[Err]Venc%d Start failed for %#x!\n", 0, s32Ret);
		return CVI_FAILURE;
	}
    return s32Ret; 
}
#endif


#if defined(CONFIG_IR_VIDEO_RTSP_SUPPORT)
int APP_IrVideoVencInit(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VENC_CHN VencChn = 1;
    PAYLOAD_TYPE_E enPayLoad = PT_H264;
	PIC_SIZE_E enSize = PIC_1080P;
    VENC_GOP_MODE_E enGopMode = VENC_GOPMODE_NORMALP;
    VENC_GOP_ATTR_S stGopAttr;
    SAMPLE_RC_E enRcMode;
    CVI_U32 u32Profile = 0;

    memset(&pIc[1], 0, sizeof(chnInputCfg));
    memset(&pcic[1], 0, sizeof(commonInputCfg));
    _VencinitInputCfg(&pIc[1], &pcic[1]);
    strcpy(pIc[1].codec, "264");

    enRcMode = (SAMPLE_RC_E) pIc[1].rcMode;
    s32Ret = SAMPLE_COMM_VENC_SetModParam(&pcic[1]);
    if (s32Ret != CVI_SUCCESS) {
        printf("SAMPLE_COMM_VENC_SetModParam failure\n");
        return CVI_FAILURE;
    }
    s32Ret = SAMPLE_COMM_VENC_GetGopAttr(enGopMode, &stGopAttr);
    if (s32Ret != CVI_SUCCESS) {
        printf("[Err]Venc Get GopAttr for %#x!\n", s32Ret);
        return CVI_FAILURE;
    }

    #if defined(CONFIG_IR_VIDEO_RTSP_VENC_BIND_VI)
	pIc[1].bind_mode = VENC_BIND_VI; 
	pIc[1].width = 1080;
	pIc[1].height = 1920;
    #elif defined(CONFIG_IR_VIDEO_RTSP_VENC_BIND_VPSS)
    pIc[1].bind_mode = VENC_BIND_VPSS;
    pIc[1].width = 1280;
    pIc[1].height = 720;
    pIc[1].vpssGrp = 2,
    pIc[1].vpssChn = 1,
    #elif defined(CONFIG_IR_VIDEO_RTSP_VENC_BIND_DISABLE)
    pIc[1].bind_mode = VENC_BIND_DISABLE;
	pIc[1].width = 1080;
	pIc[1].height = 1920;
    #endif
	enSize = PIC_CUSTOMIZE;
	s32Ret = SAMPLE_COMM_VENC_Start(
			&pIc[1],
			VencChn,
			enPayLoad,
			enSize,
			enRcMode,
			u32Profile,
			CVI_FALSE,
			&stGopAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("[Err]Venc%d Start failed for %#x!\n", 1, s32Ret);
		return CVI_FAILURE;
	}
    return s32Ret; 
}
#endif
#endif