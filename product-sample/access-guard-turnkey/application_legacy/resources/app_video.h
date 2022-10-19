#ifndef _APP_VIDEO_H_  
#define _APP_VIDEO_H_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#include "cvi_buffer.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_comm_isp.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vi.h"
#include "cvi_isp.h"
#include "cvi_ae.h"

#include "app_utils.h"
#include "app_init.h"
#include <cvi_venc.h>

#include <cvi_sys.h>

#define VENC_CHN_MAX    8
#define MAX_NUM_ROI     8

extern VO_PUB_ATTR_S stVoPubAttr;

typedef enum {
	APP_VIDEO_FRAME_FD_RGB = 0,
	APP_VIDEO_FRAME_FD_IR,
	APP_VIDEO_FRAME_LIVENESS_RGB,
	APP_VIDEO_FRAME_LIVENESS_IR,
	APP_VIDEO_FRAME_RGB,
} APP_VIDEO_FRAME_E;

typedef enum APP_RC_MODE_T {
    APP_RC_CBR = 0,
    APP_RC_VBR,
    APP_RC_AVBR,
    APP_RC_QVBR,
    APP_RC_FIXQP,
    APP_RC_QPMAP,
    APP_RC_MAX
} APP_RC_MODE_E;

typedef struct APP_REF_PARAM_T {
    CVI_S32 tempLayer;
} APP_REF_PARAM_S;

typedef struct APP_CU_PREDI_PARAM_T {
    CVI_U32 u32IntraCost;
} APP_CU_PREDI_PARAM_S;

typedef struct APP_FRAMELOST_PARAM_T {
    CVI_S32 frameLost;
    CVI_U32 frameLostGap;
    CVI_U32 frameLostBspThr;
} APP_FRAMELOST_PARAM_S;

typedef struct APP_H264_ENTROPY_PARAM_T {
    CVI_U32 h264EntropyMode;
} APP_H264_ENTROPY_PARAM_S;

typedef struct APP_H264_TRANS_PARAM_T{
    CVI_S32 h264ChromaQpOffset;
} APP_H264_TRANS_PARAM_S;

typedef struct APP_H264_VUI_PARAM_T{
    CVI_U8	aspectRatioInfoPresentFlag;
    CVI_U8	aspectRatioIdc;
    CVI_U16	sarWidth;
    CVI_U16	sarHeight;
    CVI_U8	overscanInfoPresentFlag;
    CVI_U8	overscanAppropriateFlag;
    CVI_U8	timingInfoPresentFlag;
    CVI_U8	fixedFrameRateFlag;
    CVI_U32	numUnitsInTick;
    CVI_U32	timeScale;
    CVI_U8	videoSignalTypePresentFlag;
    CVI_U8	videoFormat;
    CVI_U8	videoFullRangeFlag;
    CVI_U8	colourDescriptionPresentFlag;
    CVI_U8	colourPrimaries;
    CVI_U8	transferCharacteristics;
    CVI_U8	matrixCoefficients;
} APP_H264_VUI_PARAM_S;

typedef struct APP_H265_TRANS_PARAM_T{
    CVI_S32 h265CbQpOffset;
    CVI_S32 h265CrQpOffset;
} APP_H265_TRANS_PARAM_S;

typedef struct APP_H265_VUI_PARAM_T{
    CVI_U8	aspectRatioInfoPresentFlag;
    CVI_U8	aspectRatioIdc;
    CVI_U16	sarWidth;
    CVI_U16	sarHeight;
    CVI_U8	overscanInfoPresentFlag;
    CVI_U8	overscanAppropriateFlag;
    CVI_U8	timingInfoPresentFlag;
    CVI_U8	fixedFrameRateFlag;
    CVI_U32	numUnitsInTick;
    CVI_U32	timeScale;
    CVI_U8	videoSignalTypePresentFlag;
    CVI_U8	videoFormat;
    CVI_U8	videoFullRangeFlag;
    CVI_U8	colourDescriptionPresentFlag;
    CVI_U8	colourPrimaries;
    CVI_U8	transferCharacteristics;
    CVI_U8	matrixCoefficients;
} APP_H265_VUI_PARAM_S;

typedef struct APP_JPEG_CODEC_PARAM_T {
    CVI_S32 quality;
    CVI_S32 MCUPerECS;
} APP_JPEG_CODEC_PARAM_S;

typedef union APP_GOP_PARAM_N {
    VENC_GOP_NORMALP_S stNormalP; /*attributes of normal P*/
    VENC_GOP_DUALP_S stDualP; /*attributes of dual   P*/
    VENC_GOP_SMARTP_S stSmartP; /*attributes of Smart P*/
    VENC_GOP_ADVSMARTP_S stAdvSmartP; /*attributes of AdvSmart P*/
    VENC_GOP_BIPREDB_S stBipredB; /*attributes of b */
} APP_GOP_PARAM_U;

/* APP_RC_PARAM_S copy from VENC_RC_PARAM_S */
typedef struct APP_RC_PARAM_T {
    CVI_U32 u32ThrdI[RC_TEXTURE_THR_SIZE];
    CVI_U32 u32ThrdP[RC_TEXTURE_THR_SIZE];
    CVI_U32 u32ThrdB[RC_TEXTURE_THR_SIZE];
    CVI_U32 u32DirectionThrd;
    CVI_U32 u32RowQpDelta;
    CVI_S32 s32FirstFrameStartQp;
    CVI_S32 s32InitialDelay;
    CVI_U32 u32ThrdLv;
    CVI_BOOL bBgEnhanceEn;
    CVI_S32 s32BgDeltaQp;
    CVI_S32 s32ChangePos;
    CVI_U32 u32MinIprop;
    CVI_U32 u32MaxIprop;
    CVI_S32 s32MaxReEncodeTimes;
    CVI_S32 s32MinStillPercent;
    CVI_U32 u32MaxStillQP;
    CVI_U32 u32MinStillPSNR;
    CVI_U32 u32MaxQp;
    CVI_U32 u32MinQp;
    CVI_U32 u32MaxIQp;
    CVI_U32 u32MinIQp;
    CVI_U32 u32MinQpDelta;
    CVI_U32 u32MotionSensitivity;
    CVI_S32	s32AvbrFrmLostOpen;
    CVI_S32 s32AvbrFrmGap;
    CVI_S32 s32AvbrPureStillThr;
    CVI_BOOL bQpMapEn;
    VENC_RC_QPMAP_MODE_E enQpMapMode;
} APP_RC_PARAM_S;

typedef struct APP_VENC_CHN_CFG_T {
    CVI_BOOL bEnable;
    CVI_BOOL bStart;
    VENC_CHN VencChn;
    PAYLOAD_TYPE_E enType;
    CVI_U32 u32Duration;
    CVI_BOOL bRtspEn;
    CVI_U32 u32Width;
    CVI_U32 u32Height;
    CVI_U32 u32SrcFrameRate;
    CVI_U32 u32DstFrameRate;
    CVI_U32 u32BitRate;
    CVI_U32 u32MaxBitRate;
    CVI_U32 u32StreamBufSize;
    CVI_U32 VpssGrp;
    CVI_U32 VpssChn;
    CVI_U32 u32Profile;
    CVI_BOOL bSingleCore;
    CVI_U32 u32Gop;
    CVI_U32 u32IQp;
    CVI_U32 u32PQp;
    CVI_U32 statTime;
    CVI_U32 enBindMode;
    MMF_CHN_S astChn[2];
    VENC_GOP_MODE_E enGopMode;
    APP_GOP_PARAM_U unGopParam;
    VENC_RC_MODE_E enRcMode;
    APP_RC_PARAM_S stRcParam;
    APP_JPEG_CODEC_PARAM_S stJpegCodecParam;
    APP_FRAMELOST_PARAM_S stFrameLostCtrl;
    FILE *pFile;
    CVI_U32 frameNum;
    CVI_U32 fileNum;

    volatile CVI_S32 savePic;
} APP_VENC_CHN_CFG_S;

typedef struct APP_VENC_ROI_CFG_T {
    VENC_CHN VencChn;
    CVI_U32 u32Index;
    CVI_BOOL bEnable;
    CVI_BOOL bAbsQp;
    CVI_U32 u32Qp;
    CVI_U32 u32X;
    CVI_U32 u32Y;
    CVI_U32 u32Width;
    CVI_U32 u32Height;
} APP_VENC_ROI_CFG_S;

typedef struct APP_PARAM_VENC_CTX_T {
    CVI_BOOL bInit;
    CVI_S32 s32VencChnCnt;
    APP_VENC_CHN_CFG_S astVencChnCfg[VENC_CHN_MAX];
    APP_VENC_ROI_CFG_S astRoiCfg[MAX_NUM_ROI];
} APP_PARAM_VENC_CTX_S;

CVI_S32 APP_InitVpss(void);
CVI_S32 APP_VideoInit(void);
CVI_S32 APP_VideoRelease(void);
CVI_S32 APP_Venc_Init (void);

void APP_SetVpssAspect(CVI_S32 vpsschn,CVI_S32 x,CVI_S32 y, CVI_S32 width, CVI_S32 height);
void APP_SetIrVpssAspect(CVI_S32 vpsschn,CVI_S32 x,CVI_S32 y, CVI_S32 width, CVI_S32 height, PIXEL_FORMAT_E pixelFormat);
bool APP_GetFreezeStatus();
void APP_SetFreezeStatus(bool state);

int APP_VpssGetFrame(APP_VIDEO_FRAME_E id, VIDEO_FRAME_INFO_S *stVFrame, int timeout);
int APP_VpssReleaseFrame(APP_VIDEO_FRAME_E id, VIDEO_FRAME_INFO_S *stVFrame);
int APP_VpssBindVo(void);
int APP_VpssUnBindVo(void);

CVI_S32 CVI_APP_SetVpssAttr(void);
void CVI_APP_RecoverVpssAttr(void);
#endif
