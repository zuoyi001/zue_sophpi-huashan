
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <unistd.h>
#include "cvi_math.h"
#include "cvi_vb.h"
#include "cvi_vpss.h"
#include "app_ipcam_venc.h"
#include "cvi_type.h"
#include "app_ipcam_paramparse.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define H26X_MAX_NUM_PACKS      8
#define JPEG_MAX_NUM_PACKS      1

#define VIDEO_SIZE_MAX          2
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct APP_PARAM_VIDEO_SIZE_T {
    CVI_U32 u32Width;
    CVI_U32 u32Height;
} APP_PARAM_VIDEO_SIZE_S;

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
#if 0
APP_PARAM_VENC_CTX_S g_stVencCtx = {
    .u32VencChnCnt = 2,
    .astVencChnCfg = {
        [0] = {
            .bEnable = 	CVI_TRUE,
            .enCodecType = PT_H265,
            .enSize = PIC_2304x1296,
            .VencChn = 0,
            .vpssGrp = 0,
            .vpssChn = 0,
            .RtspEn = CVI_TRUE,
            .posX = 0,
            .posY= 0,
            .width = 2304,
            .height = 1296,
            .srcFramerate = 15,
            .framerate = 15,
            .totalFrames = -1,
            .pixel_format = PIXEL_FORMAT_NV21,
            .bind_mode = VENC_BIND_DISABLE,
            .enRcMode = APP_RC_AVBR,
            .u32Profile = 0,
            .enGopMode = VENC_GOPMODE_NORMALP,
            .iqp = 30,
            .pqp = 30,
            .gop = 60,
            .bitrate = 2000,
            .maxbitrate = 2000,
            .bitstreamBufSize = 0,
            .single_core = 0,
            .s32IPQpDelta =2 ,
            .bRcnRefShareBuf = CVI_FALSE,
            .single_LumaBuf = 1,
            .statTime = 2,
            .bgInterval = 0,
            .bVariFpsEn = CVI_FALSE,
            .quality = -1,
            .stRcParam = {
                .rcMode = APP_RC_AVBR,
                .u32ThrdLv = 2,
                .firstFrmstartQp = 30,
                .initialDelay = 1000,
                .minIprop = 1,
                .maxIprop = 100,
                .minIqp = 12,
                .maxIqp = 36,
                .minQp = 12,
                .maxQp = 36,
                .s32ChangePos = 90,
                .s32MinStillPercent = 10,
                .u32MaxStillQP = 31,
                .u32MotionSensitivity = 24,
                .s32AvbrFrmLostOpen = 0,
                .s32AvbrFrmGap = 1,
                .s32AvbrPureStillThr = 4,
            },
            .stRefParam = {
                .tempLayer = 1,
            },
            .stCuPredParam = {
                .u32IntraCost = 0,
            },
            .stFrameLostParam = {
                .frameLost = 0,
                .frameLostBspThr = 0,
                .frameLostGap = 0,
            },
            .stH264EntropyParam = {
                .h264EntropyMode = 0,
            },
            .stH264TransParam = {
                .h264ChromaQpOffset = 0,
            },
            .stH264VuiParam = {
                .videoSignalTypePresentFlag = 0,
            },
            .stH265TransParam = {
                .h265CbQpOffset = 0,
                .h265CrQpOffset = 0,
            },
            .stH265VuiParam = {
                .videoFullRangeFlag = 0,
            },
            .stJpegCodeParam = {
                .MCUPerECS = 0,
                .quality = 0,
            },
        },
        [1] = {
            .bEnable = 	CVI_TRUE,
            .enCodecType = PT_H265,
            .enSize = PIC_640x480,
            .VencChn = 1,
            .vpssGrp = 2,
            .vpssChn = 0,
            .RtspEn = CVI_TRUE,
            .posX = 0,
            .posY= 0,
            .width = 640,
            .height = 480,
            .srcFramerate = 15,
            .framerate = 15,
            .totalFrames = -1,
            .pixel_format = PIXEL_FORMAT_NV21,
            .bind_mode = VENC_BIND_VPSS,
            .enRcMode = APP_RC_AVBR,
            .u32Profile = 0,
            .enGopMode = VENC_GOPMODE_NORMALP,
            .iqp = 30,
            .pqp = 30,
            .gop = 60,
            .bitrate = 2000,
            .maxbitrate = 2000,
            .bitstreamBufSize = 0,
            .single_core = 0,
            .s32IPQpDelta =2 ,
            .bRcnRefShareBuf = CVI_FALSE,
            .single_LumaBuf = 1,
            .statTime = 2,
            .bgInterval = 0,
            .bVariFpsEn = CVI_FALSE,
            .quality = -1,
            .stRcParam = {
                .rcMode = APP_RC_AVBR,
                .u32ThrdLv = 2,
                .firstFrmstartQp = 30,
                .initialDelay = 1000,
                .minIprop = 1,
                .maxIprop = 100,
                .minIqp = 12,
                .maxIqp = 36,
                .minQp = 12,
                .maxQp = 36,
                .s32ChangePos = 90,
                .s32MinStillPercent = 10,
                .u32MaxStillQP = 31,
                .u32MotionSensitivity = 24,
                .s32AvbrFrmLostOpen = 0,
                .s32AvbrFrmGap = 1,
                .s32AvbrPureStillThr = 4,
            },
            .stRefParam = {
                .tempLayer = 1,
            },
            .stCuPredParam = {
                .u32IntraCost = 0,
            },
            .stFrameLostParam = {
                .frameLost = 0,
                .frameLostBspThr = 0,
                .frameLostGap = 0,
            },
            .stH264EntropyParam = {
                .h264EntropyMode = 0,
            },
            .stH264TransParam = {
                .h264ChromaQpOffset = 0,
            },
            .stH264VuiParam = {
                .videoSignalTypePresentFlag = 0,
            },
            .stH265TransParam = {
                .h265CbQpOffset = 0,
                .h265CrQpOffset = 0,
            },
            .stH265VuiParam = {
                .videoFullRangeFlag = 0,
            },
            .stJpegCodeParam = {
                .MCUPerECS = 0,
                .quality = 0,
            },
        },
    },
};
#else
APP_PARAM_VENC_CTX_S g_stVencCtx;
#endif

APP_PARAM_VENC_CTX_S *g_pstVencCtx = &g_stVencCtx;


APP_PARAM_VIDEO_SIZE_S stVideoSize[VIDEO_SIZE_MAX]={
    [0] = {
        .u32Width       = 2560,
        .u32Height      = 1440,
    },
    [1] = {
        .u32Width       = 960,
        .u32Height      = 540,
    },
};

static pthread_t g_Venc_pthread[VENC_MAX_CHN_NUM];

static CVI_BOOL bJpgCapFlag = CVI_FALSE;
pthread_cond_t JpgCapCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t VencCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t JpgCapMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t VencMutex = PTHREAD_MUTEX_INITIALIZER;
static VB_POOL gVencPicVbPool[VB_MAX_COMM_POOLS] = {[0 ...(VB_MAX_COMM_POOLS - 1)] = VB_INVALID_POOLID };

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_VENC_CTX_S *app_ipcam_Venc_Param_Get(void)
{
    return g_pstVencCtx;
}

APP_VENC_CHN_CFG_S *app_ipcam_VencChnCfg_Get(VENC_CHN VencChn)
{
    APP_VENC_CHN_CFG_S *pstVencChnCfg = &g_pstVencCtx->astVencChnCfg[VencChn];

    return pstVencChnCfg;
}

int app_ipcam_Venc_RefPara_Set(VENC_CHN VencChn, APP_REF_PARAM_S *pstRefCfg)
{
    VENC_REF_PARAM_S stRefParam, *pstRefParam = &stRefParam;

    APP_CHK_RET(CVI_VENC_GetRefParam(VencChn, pstRefParam), "get ref param");

    if (pstRefCfg->tempLayer == 2) {
        pstRefParam->u32Base = 1;
        pstRefParam->u32Enhance = 1;
        pstRefParam->bEnablePred = CVI_TRUE;
    } else if (pstRefCfg->tempLayer == 3) {
        pstRefParam->u32Base = 2;
        pstRefParam->u32Enhance = 1;
        pstRefParam->bEnablePred = CVI_TRUE;
    } else {
        pstRefParam->u32Base = 0;
        pstRefParam->u32Enhance = 0;
        pstRefParam->bEnablePred = CVI_TRUE;
    }

    APP_CHK_RET(CVI_VENC_SetRefParam(VencChn, pstRefParam), "set ref param");

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Venc_CuPrediction_Set(VENC_CHN VencChn, APP_CU_PREDI_PARAM_S *pstCuPrediCfg)
{
    VENC_CU_PREDICTION_S stCuPrediction, *pstCuPrediction = &stCuPrediction;	
    
    APP_CHK_RET(CVI_VENC_GetCuPrediction(VencChn, pstCuPrediction), "get CU Prediction");
    
    pstCuPrediction->u32IntraCost = pstCuPrediCfg->u32IntraCost;

    APP_CHK_RET(CVI_VENC_SetCuPrediction(VencChn, pstCuPrediction), "set CU Prediction");

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Venc_ReEncode_Close(void)
{
    return 0;
}

CVI_S32 app_ipcam_Venc_FrameLost_Set(VENC_CHN VencChn, APP_FRAMELOST_PARAM_S *pFrameLostCfg)
{
    VENC_FRAMELOST_S stFL, *pstFL = &stFL;

    APP_CHK_RET(CVI_VENC_GetFrameLostStrategy(VencChn, pstFL), "get FrameLost Strategy");

    pstFL->bFrmLostOpen = (pFrameLostCfg->frameLost) == 1 ? CVI_TRUE : CVI_FALSE;
    pstFL->enFrmLostMode = FRMLOST_PSKIP;
    pstFL->u32EncFrmGaps = pFrameLostCfg->frameLostGap;
    pstFL->u32FrmLostBpsThr = pFrameLostCfg->frameLostBspThr;

    APP_CHK_RET(CVI_VENC_SetFrameLostStrategy(VencChn, pstFL), "set FrameLost Strategy");

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Venc_H264Trans_Set(VENC_CHN VencChn, APP_H264_TRANS_PARAM_S *pstH264TransParam)
{
    VENC_H264_TRANS_S h264Trans = { 0 };

    APP_CHK_RET(CVI_VENC_GetH264Trans(VencChn, &h264Trans), "get H264 trans");

    h264Trans.chroma_qp_index_offset = pstH264TransParam->h264ChromaQpOffset;

    APP_CHK_RET(CVI_VENC_SetH264Trans(VencChn, &h264Trans), "set H264 trans");

    return CVI_SUCCESS;
}

#ifdef ARCH_CV182X
CVI_S32 app_ipcam_Venc_H264Entropy_Set(VENC_CHN VencChn, APP_H264_ENTROPY_PARAM_S *pstH264EntropyParam)
{
    VENC_H264_ENTROPY_S h264Entropy = { 0 };

    switch (pstH264EntropyParam->h264EntropyMode) {
        case 0:
            h264Entropy.u32EntropyEncModeI = H264E_ENTROPY_CAVLC;
            h264Entropy.u32EntropyEncModeP = H264E_ENTROPY_CAVLC;
            break;
        case 1:
            h264Entropy.u32EntropyEncModeI = H264E_ENTROPY_CABAC;
            h264Entropy.u32EntropyEncModeP = H264E_ENTROPY_CABAC;
            break;
        default:
            h264Entropy.u32EntropyEncModeI = H264E_ENTROPY_CABAC;
            h264Entropy.u32EntropyEncModeP = H264E_ENTROPY_CABAC;
            break;
    }

    APP_CHK_RET(CVI_VENC_SetH264Entropy(VencChn, &h264Entropy), "set H264 entropy");

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Venc_H264Vui_Set(VENC_CHN VencChn, APP_H264_VUI_PARAM_S *pstH264VuiParam)
{
    VENC_H264_VUI_S h264Vui = { 0 };
    APP_VENC_CHN_CFG_S *pstVencChnCfg = &g_pstVencCtx->astVencChnCfg[VencChn];

    APP_CHK_RET(CVI_VENC_GetH264Vui(VencChn, &h264Vui), "get H264 Vui");

    h264Vui.stVuiAspectRatio.aspect_ratio_info_present_flag = pstH264VuiParam->aspectRatioInfoPresentFlag;
    if (h264Vui.stVuiAspectRatio.aspect_ratio_info_present_flag) {
        h264Vui.stVuiAspectRatio.aspect_ratio_idc = pstH264VuiParam->aspectRatioIdc;
        h264Vui.stVuiAspectRatio.sar_width = pstH264VuiParam->sarWidth;
        h264Vui.stVuiAspectRatio.sar_height = pstH264VuiParam->sarHeight;
    }

    h264Vui.stVuiAspectRatio.overscan_info_present_flag = pstH264VuiParam->overscanInfoPresentFlag;
    if (h264Vui.stVuiAspectRatio.overscan_info_present_flag) {
        h264Vui.stVuiAspectRatio.overscan_appropriate_flag = pstH264VuiParam->overscanAppropriateFlag;
    }

    h264Vui.stVuiTimeInfo.timing_info_present_flag = pstH264VuiParam->timingInfoPresentFlag;
    if (h264Vui.stVuiTimeInfo.timing_info_present_flag) {
        h264Vui.stVuiTimeInfo.fixed_frame_rate_flag = pstH264VuiParam->fixedFrameRateFlag;
        h264Vui.stVuiTimeInfo.num_units_in_tick = pstH264VuiParam->numUnitsInTick;
        //264 fps = time_scale / (2 * num_units_in_tick) 
        h264Vui.stVuiTimeInfo.time_scale = pstVencChnCfg->fr32DstFrameRate * 2 * pstH264VuiParam->numUnitsInTick;
    }

    h264Vui.stVuiVideoSignal.video_signal_type_present_flag = pstH264VuiParam->videoSignalTypePresentFlag;
    if (h264Vui.stVuiVideoSignal.video_signal_type_present_flag) {
        h264Vui.stVuiVideoSignal.video_format = pstH264VuiParam->videoFormat;
        h264Vui.stVuiVideoSignal.video_full_range_flag = pstH264VuiParam->videoFullRangeFlag;
        h264Vui.stVuiVideoSignal.colour_description_present_flag = pstH264VuiParam->colourDescriptionPresentFlag;
        if (h264Vui.stVuiVideoSignal.colour_description_present_flag) {
            h264Vui.stVuiVideoSignal.colour_primaries = pstH264VuiParam->colourPrimaries;
            h264Vui.stVuiVideoSignal.transfer_characteristics = pstH264VuiParam->transferCharacteristics;
            h264Vui.stVuiVideoSignal.matrix_coefficients = pstH264VuiParam->matrixCoefficients;
        }
    }

    APP_CHK_RET(CVI_VENC_SetH264Vui(VencChn, &h264Vui), "set H264 Vui");

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Venc_H265Trans_Set(VENC_CHN VencChn, APP_H265_TRANS_PARAM_S *pstH265TransParam)
{
    VENC_H265_TRANS_S h265Trans = { 0 };

    APP_CHK_RET(CVI_VENC_GetH265Trans(VencChn, &h265Trans), "get H265 Trans");

    h265Trans.cb_qp_offset = pstH265TransParam->h265CbQpOffset;
    h265Trans.cr_qp_offset = pstH265TransParam->h265CrQpOffset;

    APP_CHK_RET(CVI_VENC_SetH265Trans(VencChn, &h265Trans), "set H265 Trans");

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Venc_H265Vui_Set(VENC_CHN VencChn, APP_H265_VUI_PARAM_S *pstH265VuiParam)
{
    VENC_H265_VUI_S h265Vui = { 0 };
    APP_VENC_CHN_CFG_S *pstVencChnCfg = &g_pstVencCtx->astVencChnCfg[VencChn];

    APP_CHK_RET(CVI_VENC_GetH265Vui(VencChn, &h265Vui), "get H265 Vui");

    h265Vui.stVuiAspectRatio.aspect_ratio_info_present_flag = pstH265VuiParam->aspectRatioInfoPresentFlag;
    if (h265Vui.stVuiAspectRatio.aspect_ratio_info_present_flag) {
        h265Vui.stVuiAspectRatio.aspect_ratio_idc = pstH265VuiParam->aspectRatioIdc;
        h265Vui.stVuiAspectRatio.sar_width = pstH265VuiParam->sarWidth;
        h265Vui.stVuiAspectRatio.sar_height = pstH265VuiParam->sarHeight;
    }

    h265Vui.stVuiAspectRatio.overscan_info_present_flag = pstH265VuiParam->overscanInfoPresentFlag;
    if (h265Vui.stVuiAspectRatio.overscan_info_present_flag) {
        h265Vui.stVuiAspectRatio.overscan_appropriate_flag = pstH265VuiParam->overscanAppropriateFlag;
    }

    h265Vui.stVuiTimeInfo.timing_info_present_flag = pstH265VuiParam->timingInfoPresentFlag;
    if (h265Vui.stVuiTimeInfo.timing_info_present_flag) {
        h265Vui.stVuiTimeInfo.num_units_in_tick = pstH265VuiParam->numUnitsInTick;
        //265 fps = time_scale / num_units_in_tick
        h265Vui.stVuiTimeInfo.time_scale = pstVencChnCfg->fr32DstFrameRate * pstH265VuiParam->numUnitsInTick;
    }

    h265Vui.stVuiVideoSignal.video_signal_type_present_flag = pstH265VuiParam->videoSignalTypePresentFlag;
    if (h265Vui.stVuiVideoSignal.video_signal_type_present_flag) {
        h265Vui.stVuiVideoSignal.video_format = pstH265VuiParam->videoFormat;
        h265Vui.stVuiVideoSignal.video_full_range_flag = pstH265VuiParam->videoFullRangeFlag;
        h265Vui.stVuiVideoSignal.colour_description_present_flag = pstH265VuiParam->colourDescriptionPresentFlag;
        if (h265Vui.stVuiVideoSignal.colour_description_present_flag) {
            h265Vui.stVuiVideoSignal.colour_primaries = pstH265VuiParam->colourPrimaries;
            h265Vui.stVuiVideoSignal.transfer_characteristics = pstH265VuiParam->transferCharacteristics;
            h265Vui.stVuiVideoSignal.matrix_coefficients = pstH265VuiParam->matrixCoefficients;
        }
    }

    APP_CHK_RET(CVI_VENC_SetH265Vui(VencChn, &h265Vui), "set H265 Vui");

    return CVI_SUCCESS;
}
#endif

CVI_S32 app_ipcam_Venc_Jpeg_Param_Set(VENC_CHN VencChn, APP_JPEG_CODEC_PARAM_S *pstJpegCodecCfg)
{
    VENC_JPEG_PARAM_S stJpegParam, *pstJpegParam = &stJpegParam;

    APP_CHK_RET(CVI_VENC_GetJpegParam(VencChn, pstJpegParam), "get jpeg param");

    if (pstJpegCodecCfg->quality <= 0)
        pstJpegCodecCfg->quality = 1;
    else if (pstJpegCodecCfg->quality >= 100)
        pstJpegCodecCfg->quality = 99;

    pstJpegParam->u32Qfactor = pstJpegCodecCfg->quality;
    pstJpegParam->u32MCUPerECS = pstJpegCodecCfg->MCUPerECS;

    APP_CHK_RET(CVI_VENC_SetJpegParam(VencChn, pstJpegParam), "set jpeg param");

    return CVI_SUCCESS;
}

int app_ipcam_Venc_Chn_Attr_Set(VENC_ATTR_S *pstVencAttr, APP_VENC_CHN_CFG_S *pstVencChnCfg)
{
    pstVencAttr->enType = pstVencChnCfg->enType;
    pstVencAttr->u32MaxPicWidth = pstVencChnCfg->u32Width;
    pstVencAttr->u32MaxPicHeight = pstVencChnCfg->u32Height;
    pstVencAttr->u32PicWidth = pstVencChnCfg->u32Width;
    pstVencAttr->u32PicHeight = pstVencChnCfg->u32Height;
    pstVencAttr->u32Profile = pstVencChnCfg->u32Profile;
    pstVencAttr->bSingleCore = pstVencChnCfg->bSingleCore;
    pstVencAttr->bByFrame = CVI_TRUE;

    APP_PROF_LOG_PRINT(LEVEL_TRACE,"enType=%d u32Profile=%d bSingleCore=%d\n",
        pstVencAttr->enType, pstVencAttr->u32Profile, pstVencAttr->bSingleCore);
    APP_PROF_LOG_PRINT(LEVEL_TRACE,"u32MaxPicWidth=%d u32MaxPicHeight=%d u32PicWidth=%d u32PicHeight=%d\n",
        pstVencAttr->u32MaxPicWidth, pstVencAttr->u32MaxPicHeight, pstVencAttr->u32PicWidth, pstVencAttr->u32PicHeight);

    /* Venc encode type validity check */
    if ((pstVencAttr->enType != PT_H265) && (pstVencAttr->enType != PT_H264) 
        && (pstVencAttr->enType != PT_JPEG) && (pstVencAttr->enType != PT_MJPEG)) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"encode type = %d invalid\n", pstVencAttr->enType);
        return CVI_FAILURE;
    }

    if (pstVencAttr->enType == PT_H264) {
        pstVencAttr->stAttrH264e.bSingleLumaBuf = 1;
    }

    if (PT_JPEG == pstVencChnCfg->enType || PT_MJPEG == pstVencChnCfg->enType) {
        VENC_ATTR_JPEG_S *pstJpegAttr = &pstVencAttr->stAttrJpege;

        pstJpegAttr->bSupportDCF = CVI_FALSE;
        pstJpegAttr->stMPFCfg.u8LargeThumbNailNum = 0;
        pstJpegAttr->enReceiveMode = VENC_PIC_RECEIVE_SINGLE;
    }

    return CVI_SUCCESS;
}

int app_ipcam_Venc_Gop_Attr_Set(VENC_GOP_ATTR_S *pstGopAttr, APP_VENC_CHN_CFG_S *pstVencChnCfg)
{
    VENC_GOP_MODE_E enGopMode = pstVencChnCfg->enGopMode;

    /* Venc gop mode validity check */
    if ((enGopMode != VENC_GOPMODE_NORMALP) && (enGopMode != VENC_GOPMODE_SMARTP) 
        && (enGopMode != VENC_GOPMODE_DUALP) && (enGopMode != VENC_GOPMODE_BIPREDB)) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"gop mode = %d invalid\n", enGopMode);
        return CVI_FAILURE;
    }

    switch (enGopMode) {
        case VENC_GOPMODE_NORMALP:
            pstGopAttr->stNormalP.s32IPQpDelta = pstVencChnCfg->unGopParam.stNormalP.s32IPQpDelta;

            APP_PROF_LOG_PRINT(LEVEL_TRACE,"stNormalP -> s32IPQpDelta=%d\n",
                pstGopAttr->stNormalP.s32IPQpDelta);
            break;
        case VENC_GOPMODE_SMARTP:
            pstGopAttr->stSmartP.s32BgQpDelta = pstVencChnCfg->unGopParam.stSmartP.s32BgQpDelta;
            pstGopAttr->stSmartP.s32ViQpDelta = pstVencChnCfg->unGopParam.stSmartP.s32ViQpDelta;
            pstGopAttr->stSmartP.u32BgInterval = pstVencChnCfg->unGopParam.stSmartP.u32BgInterval;

            APP_PROF_LOG_PRINT(LEVEL_TRACE,"stSmartP -> s32BgQpDelta=%d s32ViQpDelta=%d u32BgInterval=%d\n",
                pstGopAttr->stSmartP.s32BgQpDelta, pstGopAttr->stSmartP.s32ViQpDelta, pstGopAttr->stSmartP.u32BgInterval);
            break;

        case VENC_GOPMODE_DUALP:
            pstGopAttr->stDualP.s32IPQpDelta = pstVencChnCfg->unGopParam.stDualP.s32IPQpDelta;
            pstGopAttr->stDualP.s32SPQpDelta = pstVencChnCfg->unGopParam.stDualP.s32SPQpDelta;
            pstGopAttr->stDualP.u32SPInterval = pstVencChnCfg->unGopParam.stDualP.u32SPInterval;

            APP_PROF_LOG_PRINT(LEVEL_TRACE,"stDualP -> s32IPQpDelta=%d s32SPQpDelta=%d u32SPInterval=%d\n",
                pstGopAttr->stDualP.s32IPQpDelta, pstGopAttr->stDualP.s32SPQpDelta, pstGopAttr->stDualP.u32SPInterval);
            break;

        case VENC_GOPMODE_BIPREDB:
            pstGopAttr->stBipredB.s32BQpDelta = pstVencChnCfg->unGopParam.stBipredB.s32BQpDelta;
            pstGopAttr->stBipredB.s32IPQpDelta = pstVencChnCfg->unGopParam.stBipredB.s32IPQpDelta;
            pstGopAttr->stBipredB.u32BFrmNum = pstVencChnCfg->unGopParam.stBipredB.u32BFrmNum;

            APP_PROF_LOG_PRINT(LEVEL_TRACE,"stBipredB -> s32BQpDelta=%d s32IPQpDelta=%d u32BFrmNum=%d\n",
                pstGopAttr->stBipredB.s32BQpDelta, pstGopAttr->stBipredB.s32IPQpDelta, pstGopAttr->stBipredB.u32BFrmNum);
            break;

        default:
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"not support the gop mode !\n");
            return CVI_FAILURE;
    }

    pstGopAttr->enGopMode = enGopMode;
    if (PT_MJPEG == pstVencChnCfg->enType || PT_JPEG == pstVencChnCfg->enType) {
        pstGopAttr->enGopMode = VENC_GOPMODE_NORMALP;
        pstGopAttr->stNormalP.s32IPQpDelta = pstVencChnCfg->unGopParam.stNormalP.s32IPQpDelta;
    }

    return CVI_SUCCESS;
}

int app_ipcam_Venc_Rc_Attr_Set(VENC_RC_ATTR_S *pstRCAttr, APP_VENC_CHN_CFG_S *pstVencChnCfg)
{
    int SrcFrmRate = pstVencChnCfg->u32SrcFrameRate;
    int DstFrmRate = pstVencChnCfg->fr32DstFrameRate;
    int BitRate    = pstVencChnCfg->u32BitRate;
    int MaxBitrate = pstVencChnCfg->u32MaxBitRate;
    int StatTime   = pstVencChnCfg->statTime;
    int Gop        = pstVencChnCfg->u32Gop;
    int IQP        = pstVencChnCfg->u32IQp;
    int PQP        = pstVencChnCfg->u32PQp;

    APP_PROF_LOG_PRINT(LEVEL_TRACE,"RcMode=%d EncType=%d SrcFR=%d DstFR=%d\n", 
        pstVencChnCfg->enRcMode, pstVencChnCfg->enType, 
        pstVencChnCfg->u32SrcFrameRate, pstVencChnCfg->fr32DstFrameRate);
    APP_PROF_LOG_PRINT(LEVEL_TRACE,"BR=%d MaxBR=%d statTime=%d gop=%d IQP=%d PQP=%d\n", 
        pstVencChnCfg->u32BitRate, pstVencChnCfg->u32MaxBitRate, 
        pstVencChnCfg->statTime, pstVencChnCfg->u32Gop, pstVencChnCfg->u32IQp, pstVencChnCfg->u32PQp);

    pstRCAttr->enRcMode = pstVencChnCfg->enRcMode;
    switch (pstVencChnCfg->enType) {
    case PT_H265: {
        if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H265CBR) {
            VENC_H265_CBR_S *pstH265Cbr = &pstRCAttr->stH265Cbr;

            pstH265Cbr->u32Gop = Gop;
            pstH265Cbr->u32StatTime = StatTime;
            pstH265Cbr->u32SrcFrameRate = SrcFrmRate;
            pstH265Cbr->fr32DstFrameRate = DstFrmRate;
            pstH265Cbr->u32BitRate = BitRate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H265FIXQP) {
            VENC_H265_FIXQP_S *pstH265FixQp = &pstRCAttr->stH265FixQp;

            pstH265FixQp->u32Gop = Gop;
            pstH265FixQp->u32SrcFrameRate = SrcFrmRate;
            pstH265FixQp->fr32DstFrameRate = DstFrmRate;
            pstH265FixQp->u32IQp = IQP;
            pstH265FixQp->u32PQp = PQP;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H265VBR) {
            VENC_H265_VBR_S *pstH265Vbr = &pstRCAttr->stH265Vbr;

            pstH265Vbr->u32Gop = Gop;
            pstH265Vbr->u32StatTime = StatTime;
            pstH265Vbr->u32SrcFrameRate = SrcFrmRate;
            pstH265Vbr->fr32DstFrameRate = DstFrmRate;
            pstH265Vbr->u32MaxBitRate = MaxBitrate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H265AVBR) {
            VENC_H265_AVBR_S *pstH265AVbr = &pstRCAttr->stH265AVbr;

            pstH265AVbr->u32Gop = Gop;
            pstH265AVbr->u32StatTime = StatTime;
            pstH265AVbr->u32SrcFrameRate = SrcFrmRate;
            pstH265AVbr->fr32DstFrameRate = DstFrmRate;
            pstH265AVbr->u32MaxBitRate = MaxBitrate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H265QVBR) {
            VENC_H265_QVBR_S *pstH265QVbr = &pstRCAttr->stH265QVbr;

            pstH265QVbr->u32Gop = Gop;
            pstH265QVbr->u32StatTime = StatTime;
            pstH265QVbr->u32SrcFrameRate = SrcFrmRate;
            pstH265QVbr->fr32DstFrameRate = DstFrmRate;
            pstH265QVbr->u32TargetBitRate = BitRate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H265QPMAP) {
            VENC_H265_QPMAP_S *pstH265QpMap = &pstRCAttr->stH265QpMap;

            pstH265QpMap->u32Gop = Gop;
            pstH265QpMap->u32StatTime = StatTime;
            pstH265QpMap->u32SrcFrameRate = SrcFrmRate;
            pstH265QpMap->fr32DstFrameRate = DstFrmRate;
            pstH265QpMap->enQpMapMode = VENC_RC_QPMAP_MODE_MEANQP;
        } else {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"enRcMode(%d) not support\n", pstVencChnCfg->enRcMode);
            return CVI_FAILURE;
        }
    }
    break;
    case PT_H264: {
        if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H264CBR) {
            VENC_H264_CBR_S *pstH264Cbr = &pstRCAttr->stH264Cbr;

            pstH264Cbr->u32Gop = Gop;
            pstH264Cbr->u32StatTime = StatTime;
            pstH264Cbr->u32SrcFrameRate = SrcFrmRate;
            pstH264Cbr->fr32DstFrameRate = DstFrmRate;
            pstH264Cbr->u32BitRate = BitRate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H264FIXQP) {
            VENC_H264_FIXQP_S *pstH264FixQp = &pstRCAttr->stH264FixQp;

            pstH264FixQp->u32Gop = Gop;
            pstH264FixQp->u32SrcFrameRate = SrcFrmRate;
            pstH264FixQp->fr32DstFrameRate = DstFrmRate;
            pstH264FixQp->u32IQp = IQP;
            pstH264FixQp->u32PQp = PQP;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H264VBR) {
            VENC_H264_VBR_S *pstH264Vbr = &pstRCAttr->stH264Vbr;

            pstH264Vbr->u32Gop = Gop;
            pstH264Vbr->u32StatTime = StatTime;
            pstH264Vbr->u32SrcFrameRate = SrcFrmRate;
            pstH264Vbr->fr32DstFrameRate = DstFrmRate;
            pstH264Vbr->u32MaxBitRate = MaxBitrate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H264AVBR) {
            VENC_H264_AVBR_S *pstH264AVbr = &pstRCAttr->stH264AVbr;

            pstH264AVbr->u32Gop = Gop;
            pstH264AVbr->u32StatTime = StatTime;
            pstH264AVbr->u32SrcFrameRate = SrcFrmRate;
            pstH264AVbr->fr32DstFrameRate = DstFrmRate;
            pstH264AVbr->u32MaxBitRate = MaxBitrate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H264QVBR) {
            VENC_H264_QVBR_S *pstH264QVbr = &pstRCAttr->stH264QVbr;

            pstH264QVbr->u32Gop = Gop;
            pstH264QVbr->u32StatTime = StatTime;
            pstH264QVbr->u32SrcFrameRate = SrcFrmRate;
            pstH264QVbr->fr32DstFrameRate = DstFrmRate;
            pstH264QVbr->u32TargetBitRate = BitRate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H264QPMAP) {
            VENC_H264_QPMAP_S *pstH264QpMap = &pstRCAttr->stH264QpMap;

            pstH264QpMap->u32Gop = Gop;
            pstH264QpMap->u32StatTime = StatTime;
            pstH264QpMap->u32SrcFrameRate = SrcFrmRate;
            pstH264QpMap->fr32DstFrameRate = DstFrmRate;
        } else {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"enRcMode(%d) not support\n", pstVencChnCfg->enRcMode);
            return CVI_FAILURE;
        }
    }
    break;
    case PT_MJPEG: {
        if (pstVencChnCfg->enRcMode == VENC_RC_MODE_MJPEGFIXQP) {
            VENC_MJPEG_FIXQP_S *pstMjpegeFixQp = &pstRCAttr->stMjpegFixQp;

            // 0 use old q-table for forward compatible.
            pstMjpegeFixQp->u32Qfactor = 0;
            pstMjpegeFixQp->u32SrcFrameRate = SrcFrmRate;
            pstMjpegeFixQp->fr32DstFrameRate = DstFrmRate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_MJPEGCBR) {
            VENC_MJPEG_CBR_S *pstMjpegeCbr = &pstRCAttr->stMjpegCbr;

            pstMjpegeCbr->u32StatTime = StatTime;
            pstMjpegeCbr->u32SrcFrameRate = SrcFrmRate;
            pstMjpegeCbr->fr32DstFrameRate = DstFrmRate;
            pstMjpegeCbr->u32BitRate = BitRate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_MJPEGVBR) {
            VENC_MJPEG_VBR_S *pstMjpegeVbr = &pstRCAttr->stMjpegVbr;

            pstMjpegeVbr->u32StatTime = StatTime;
            pstMjpegeVbr->u32SrcFrameRate = SrcFrmRate;
            pstMjpegeVbr->fr32DstFrameRate = DstFrmRate;
            pstMjpegeVbr->u32MaxBitRate = MaxBitrate;
        } else {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"cann't support other mode(%d) in this version!\n", pstVencChnCfg->enRcMode);
            return CVI_FAILURE;
        }
    }
    break;

    case PT_JPEG:
        break;

    default:
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"cann't support this enType (%d) in this version!\n", pstVencChnCfg->enType);
        return CVI_ERR_VENC_NOT_SUPPORT;
    }

    return CVI_SUCCESS;
}

void app_ipcam_Venc_Attr_Check(VENC_CHN_ATTR_S *pstVencChnAttr)
{
    if (pstVencChnAttr->stVencAttr.enType == PT_H264) {
        pstVencChnAttr->stVencAttr.stAttrH264e.bSingleLumaBuf = 1;
    }

    if ((pstVencChnAttr->stGopAttr.enGopMode == VENC_GOPMODE_BIPREDB) &&
        (pstVencChnAttr->stVencAttr.enType == PT_H264)) {
        if (pstVencChnAttr->stVencAttr.u32Profile == 0) {
            pstVencChnAttr->stVencAttr.u32Profile = 1;
            APP_PROF_LOG_PRINT(LEVEL_WARN,"H.264 base not support BIPREDB, change to main\n");
        }
    }

    if ((pstVencChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264QPMAP) ||
        (pstVencChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265QPMAP)) {
        if (pstVencChnAttr->stGopAttr.enGopMode == VENC_GOPMODE_ADVSMARTP) {
            pstVencChnAttr->stGopAttr.enGopMode = VENC_GOPMODE_SMARTP;
            APP_PROF_LOG_PRINT(LEVEL_WARN,"advsmartp not support QPMAP, so change gopmode to smartp!\n");
        }
    }

    if ((pstVencChnAttr->stGopAttr.enGopMode == VENC_GOPMODE_BIPREDB) &&
        (pstVencChnAttr->stVencAttr.enType == PT_H264)) {
        if (pstVencChnAttr->stVencAttr.u32Profile == 0) {
            pstVencChnAttr->stVencAttr.u32Profile = 1;
            APP_PROF_LOG_PRINT(LEVEL_WARN,"H.264 base not support BIPREDB, change to main\n");
        }
    }
    if ((pstVencChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264QPMAP) ||
        (pstVencChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265QPMAP)) {
        if (pstVencChnAttr->stGopAttr.enGopMode == VENC_GOPMODE_ADVSMARTP) {
            pstVencChnAttr->stGopAttr.enGopMode = VENC_GOPMODE_SMARTP;
            APP_PROF_LOG_PRINT(LEVEL_WARN,"advsmartp not support QPMAP, so change gopmode to smartp!\n");
        }
    }
}

int app_ipcam_Venc_Rc_Param_Set(
    VENC_CHN VencChn,
    PAYLOAD_TYPE_E enCodecType,
    VENC_RC_MODE_E enRcMode,
    APP_RC_PARAM_S *pstRcParamCfg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VENC_RC_PARAM_S stRcParam, *pstRcParam = &stRcParam;

    s32Ret = CVI_VENC_GetRcParam(VencChn, pstRcParam);
    if (s32Ret != CVI_SUCCESS) {
        CVI_VENC_ERR("GetRcParam, 0x%X\n", s32Ret);
        return s32Ret;
    }

    CVI_U32 u32MaxIprop          = pstRcParamCfg->u32MaxIprop;
    CVI_U32 u32MinIprop          = pstRcParamCfg->u32MinIprop;
    CVI_U32 u32MaxQp             = pstRcParamCfg->u32MaxQp;
    CVI_U32 u32MinQp             = pstRcParamCfg->u32MinQp;
    CVI_U32 u32MaxIQp            = pstRcParamCfg->u32MaxIQp;
    CVI_U32 u32MinIQp            = pstRcParamCfg->u32MinIQp;
    CVI_S32 s32ChangePos         = pstRcParamCfg->s32ChangePos;
    CVI_S32 s32MinStillPercent   = pstRcParamCfg->s32MinStillPercent;
    CVI_U32 u32MaxStillQP        = pstRcParamCfg->u32MaxStillQP;
    CVI_U32 u32MotionSensitivity = pstRcParamCfg->u32MotionSensitivity;
    CVI_S32 s32AvbrFrmLostOpen   = pstRcParamCfg->s32AvbrFrmLostOpen;
    CVI_S32 s32AvbrFrmGap        = pstRcParamCfg->s32AvbrFrmGap;
    CVI_S32 s32AvbrPureStillThr  = pstRcParamCfg->s32AvbrPureStillThr;

    CVI_U32 u32ThrdLv            = pstRcParamCfg->u32ThrdLv;
    CVI_S32 s32FirstFrameStartQp = pstRcParamCfg->s32FirstFrameStartQp;
    CVI_S32 s32InitialDelay      = pstRcParamCfg->s32InitialDelay;

    APP_PROF_LOG_PRINT(LEVEL_TRACE,"MaxIprop=%d MinIprop=%d MaxQp=%d MinQp=%d MaxIQp=%d MinIQp=%d\n",
        u32MaxIprop, u32MinIprop, u32MaxQp, u32MinQp, u32MaxIQp, u32MinIQp);
    APP_PROF_LOG_PRINT(LEVEL_TRACE,"ChangePos=%d MinStillPercent=%d MaxStillQP=%d MotionSensitivity=%d AvbrFrmLostOpen=%d\n",
        s32ChangePos, s32MinStillPercent, u32MaxStillQP, u32MotionSensitivity, s32AvbrFrmLostOpen);
    APP_PROF_LOG_PRINT(LEVEL_TRACE,"AvbrFrmGap=%d AvbrPureStillThr=%d ThrdLv=%d FirstFrameStartQp=%d InitialDelay=%d\n",
        s32AvbrFrmGap, s32AvbrPureStillThr, u32ThrdLv, s32FirstFrameStartQp, s32InitialDelay);

    pstRcParam->u32ThrdLv = u32ThrdLv;
    pstRcParam->s32FirstFrameStartQp = s32FirstFrameStartQp;
    pstRcParam->s32InitialDelay = s32InitialDelay;

    switch (enCodecType) {
    case PT_H265: {
        if (enRcMode == VENC_RC_MODE_H265CBR) {
            pstRcParam->stParamH265Cbr.u32MaxIprop = u32MaxIprop;
            pstRcParam->stParamH265Cbr.u32MinIprop = u32MinIprop;
            pstRcParam->stParamH265Cbr.u32MaxIQp = u32MaxIQp;
            pstRcParam->stParamH265Cbr.u32MinIQp = u32MinIQp;
            pstRcParam->stParamH265Cbr.u32MaxQp = u32MaxQp;
            pstRcParam->stParamH265Cbr.u32MinQp = u32MinQp;
        } else if (enRcMode == VENC_RC_MODE_H265VBR) {
            pstRcParam->stParamH265Vbr.u32MaxIprop = u32MaxIprop;
            pstRcParam->stParamH265Vbr.u32MinIprop = u32MinIprop;
            pstRcParam->stParamH265Vbr.u32MaxIQp = u32MaxIQp;
            pstRcParam->stParamH265Vbr.u32MinIQp = u32MinIQp;
            pstRcParam->stParamH265Vbr.u32MaxQp = u32MaxQp;
            pstRcParam->stParamH265Vbr.u32MinQp = u32MinQp;
            pstRcParam->stParamH265Vbr.s32ChangePos = s32ChangePos;
        } else if (enRcMode == VENC_RC_MODE_H265AVBR) {
            pstRcParam->stParamH265AVbr.u32MaxIprop = u32MaxIprop;
            pstRcParam->stParamH265AVbr.u32MinIprop = u32MinIprop;
            pstRcParam->stParamH265AVbr.u32MaxIQp = u32MaxIQp;
            pstRcParam->stParamH265AVbr.u32MinIQp = u32MinIQp;
            pstRcParam->stParamH265AVbr.u32MaxQp = u32MaxQp;
            pstRcParam->stParamH265AVbr.u32MinQp = u32MinQp;
            pstRcParam->stParamH265AVbr.s32ChangePos = s32ChangePos;
            pstRcParam->stParamH265AVbr.s32MinStillPercent = s32MinStillPercent;
            pstRcParam->stParamH265AVbr.u32MaxStillQP = u32MaxStillQP;
            pstRcParam->stParamH265AVbr.u32MotionSensitivity = u32MotionSensitivity;
            pstRcParam->stParamH265AVbr.s32AvbrFrmLostOpen = s32AvbrFrmLostOpen;
            pstRcParam->stParamH265AVbr.s32AvbrFrmGap = s32AvbrFrmGap;
            pstRcParam->stParamH265AVbr.s32AvbrPureStillThr = s32AvbrPureStillThr;
        } else {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"enRcMode(%d) not support\n", enRcMode);
            return CVI_FAILURE;
        }
    }
    break;
    case PT_H264: {
        if (enRcMode == VENC_RC_MODE_H264CBR) {
            pstRcParam->stParamH264Cbr.u32MaxIprop = u32MaxIprop;
            pstRcParam->stParamH264Cbr.u32MinIprop = u32MinIprop;
            pstRcParam->stParamH264Cbr.u32MaxIQp = u32MaxIQp;
            pstRcParam->stParamH264Cbr.u32MinIQp = u32MinIQp;
            pstRcParam->stParamH264Cbr.u32MaxQp = u32MaxQp;
            pstRcParam->stParamH264Cbr.u32MinQp = u32MinQp;
        } else if (enRcMode == VENC_RC_MODE_H264VBR) {
            pstRcParam->stParamH264Vbr.u32MaxIprop = u32MaxIprop;
            pstRcParam->stParamH264Vbr.u32MinIprop = u32MinIprop;
            pstRcParam->stParamH264Vbr.u32MaxIQp = u32MaxIQp;
            pstRcParam->stParamH264Vbr.u32MinIQp = u32MinIQp;
            pstRcParam->stParamH264Vbr.u32MaxQp = u32MaxQp;
            pstRcParam->stParamH264Vbr.u32MinQp = u32MinQp;
            pstRcParam->stParamH264Vbr.s32ChangePos = s32ChangePos;
        } else if (enRcMode == VENC_RC_MODE_H264AVBR) {
            pstRcParam->stParamH264AVbr.u32MaxIprop = u32MaxIprop;
            pstRcParam->stParamH264AVbr.u32MinIprop = u32MinIprop;
            pstRcParam->stParamH264AVbr.u32MaxIQp = u32MaxIQp;
            pstRcParam->stParamH264AVbr.u32MinIQp = u32MinIQp;
            pstRcParam->stParamH264AVbr.u32MaxQp = u32MaxQp;
            pstRcParam->stParamH264AVbr.u32MinQp = u32MinQp;
            pstRcParam->stParamH264AVbr.s32ChangePos = s32ChangePos;
            pstRcParam->stParamH264AVbr.s32MinStillPercent = s32MinStillPercent;
            pstRcParam->stParamH264AVbr.u32MaxStillQP = u32MaxStillQP;
            pstRcParam->stParamH264AVbr.u32MotionSensitivity = u32MotionSensitivity;
            pstRcParam->stParamH264AVbr.s32AvbrFrmLostOpen = s32AvbrFrmLostOpen;
            pstRcParam->stParamH264AVbr.s32AvbrFrmGap = s32AvbrFrmGap;
            pstRcParam->stParamH264AVbr.s32AvbrPureStillThr = s32AvbrPureStillThr;
        } else {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"enRcMode(%d) not support\n", enRcMode);
            return CVI_FAILURE;
        }
    }
    break;
    default:
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"cann't support this enType (%d) in this version!\n", enCodecType);
        return CVI_ERR_VENC_NOT_SUPPORT;
    }

    s32Ret = CVI_VENC_SetRcParam(VencChn, pstRcParam);
    if (s32Ret != CVI_SUCCESS) {
        CVI_VENC_ERR("SetRcParam, 0x%X\n", s32Ret);
        return s32Ret;
    }

    return CVI_SUCCESS;
}


static CVI_S32 app_ipcam_Venc_SingleEsBuff_Set(VB_SOURCE_E eVbSource)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    for (VENC_MODTYPE_E modtype = MODTYPE_H264E; modtype <= MODTYPE_JPEGE; modtype++) {
        VENC_PARAM_MOD_S stModParam;

        stModParam.enVencModType = modtype;
        s32Ret = CVI_VENC_GetModParam(&stModParam);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VENC_GetModParam type %d failure\n", modtype);
            return s32Ret;
        }

        switch (modtype) {
        case MODTYPE_H264E:
            stModParam.stH264eModParam.bSingleEsBuf = true;
            stModParam.stH264eModParam.u32SingleEsBufSize = 0x0100000;  // 1.0MB
            stModParam.stH264eModParam.enH264eVBSource = eVbSource;
            break;
        case MODTYPE_H265E:
            stModParam.stH265eModParam.bSingleEsBuf = true;
            stModParam.stH265eModParam.u32SingleEsBufSize = 0x0100000;  // 1.0MB
            stModParam.stH265eModParam.enH265eVBSource = eVbSource;
            break;
        case MODTYPE_JPEGE:
            stModParam.stJpegeModParam.bSingleEsBuf = true;
            stModParam.stJpegeModParam.u32SingleEsBufSize = 0x0100000;  // 1.0MB
            break;
        default:
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Venc_SingleEsBuff_Set invalid type %d failure\n", modtype);
            break;
        }

        s32Ret = CVI_VENC_SetModParam(&stModParam);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "VENC PARAM_MODE type %d failure\n", modtype);
            return s32Ret;
        }
    }

    return CVI_SUCCESS;
}

int _streaming_send_to_rtsp(VENC_CHN VencChn, VENC_STREAM_S *pstStream)
{
    APP_PARAM_RTSP_T *prtspCtx = app_ipcam_Rtsp_Param_Get();

    if ((!prtspCtx->bStart[VencChn])) {
        return CVI_SUCCESS;
    }

    CVI_S32 s32Ret = CVI_SUCCESS;
    VENC_PACK_S *ppack;
    CVI_RTSP_DATA data;

    memset(&data, 0, sizeof(CVI_RTSP_DATA));

    data.blockCnt = pstStream->u32PackCount;
    for (CVI_U32 i = 0; i < pstStream->u32PackCount; i++) {
        ppack = &pstStream->pstPack[i];
        data.dataPtr[i] = ppack->pu8Addr + ppack->u32Offset;
        data.dataLen[i] = ppack->u32Len - ppack->u32Offset;

        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "pack[%d], PTS = %"PRId64", Addr = %p, Len = 0x%X, Offset = 0x%X DataType=%d\n",
            i, ppack->u64PTS, ppack->pu8Addr, ppack->u32Len, ppack->u32Offset, ppack->DataType.enH265EType);
    }

    s32Ret = CVI_RTSP_WriteFrame(prtspCtx->pstServerCtx, prtspCtx->pstSession[VencChn]->video, &data);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RTSP_WriteFrame failed\n");
    }

    return s32Ret;
}

static CVI_S32 app_ipcam_Postfix_Get(PAYLOAD_TYPE_E enPayload, char *szPostfix)
{

    _NULL_POINTER_CHECK_(szPostfix, -1);

    if (enPayload == PT_H264)
        strcpy(szPostfix, ".h264");
    else if (enPayload == PT_H265)
        strcpy(szPostfix, ".h265");
    else if (enPayload == PT_JPEG)
        strcpy(szPostfix, ".jpg");
    else if (enPayload == PT_MJPEG)
        strcpy(szPostfix, ".mjp");
    else {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "payload type(%d) err!\n", enPayload);
        return CVI_FAILURE;
    }
    
    return CVI_SUCCESS;
}

static int _streaming_save_to_flash(APP_VENC_CHN_CFG_S *pstVencChnCfg, VENC_STREAM_S *pstStream)
{
    if (pstVencChnCfg->pFile == NULL) {
        char szPostfix[8] = {0};
        char szFilePath[64] = {0};
        app_ipcam_Postfix_Get(pstVencChnCfg->enType, szPostfix);
        snprintf(szFilePath, 64, "%s/Venc%d_idx_%d%s", pstVencChnCfg->SavePath, pstVencChnCfg->VencChn, pstVencChnCfg->frameNum++, szPostfix);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "update new file name: %s\n", szFilePath);
        pstVencChnCfg->pFile = fopen(szFilePath, "wb");
        if (pstVencChnCfg->pFile == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "open file err, %s\n", szFilePath);
            return CVI_FAILURE;
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_DEBUG, "u32PackCount = %d\n", pstStream->u32PackCount);

    VENC_PACK_S *ppack;

    for (CVI_U32 i = 0; i < pstStream->u32PackCount; i++) {
        ppack = &pstStream->pstPack[i];
        fwrite(ppack->pu8Addr + ppack->u32Offset,
                ppack->u32Len - ppack->u32Offset, 1, pstVencChnCfg->pFile);

        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "pack[%d], PTS = %"PRId64", Addr = %p, Len = 0x%X, Offset = 0x%X DataType=%d\n",
                i, ppack->u64PTS, ppack->pu8Addr, ppack->u32Len, ppack->u32Offset, ppack->DataType.enH265EType);
    }

    if (pstVencChnCfg->enType == PT_JPEG) {
        fclose(pstVencChnCfg->pFile);
        pstVencChnCfg->pFile = NULL;
    } else {
        if (++pstVencChnCfg->fileNum > pstVencChnCfg->u32Duration) {
            pstVencChnCfg->fileNum = 0;
            fclose(pstVencChnCfg->pFile);
            pstVencChnCfg->pFile = NULL;
        }
    }

    return CVI_SUCCESS;
}

int app_ipcam_Streaming_Send(APP_VENC_CHN_CFG_S *pastVencChnCfg, VENC_STREAM_S *pstStream)
{
    CVI_U32 StreamTo = pastVencChnCfg->StreamTo;

    if ((StreamTo & TO_RTSP) == TO_RTSP) {
        APP_CHK_RET(_streaming_send_to_rtsp(pastVencChnCfg->VencChn, pstStream), "streaming send to Rtsp");
    }

    if ((StreamTo & TO_FLASH) == TO_FLASH) {
        APP_CHK_RET(_streaming_save_to_flash(pastVencChnCfg, pstStream), "streaming save to Flash");
    }

    return CVI_SUCCESS;
}

void app_ipcam_JpgCapFlag_Set(CVI_BOOL bEnable)
{
    bJpgCapFlag = bEnable;
}

CVI_BOOL app_ipcam_JpgCapFlag_Get(void)
{
    return bJpgCapFlag;
}

static void *Thread_Streaming_Proc(void *pArgs)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_VENC_CHN_CFG_S *pastVencChnCfg = (APP_VENC_CHN_CFG_S *)pArgs;
    VENC_CHN VencChn = pastVencChnCfg->VencChn;
    CVI_S32 vpssGrp = pastVencChnCfg->VpssGrp;
    CVI_S32 vpssChn = pastVencChnCfg->VpssChn;
    CVI_S32 iTime = GetCurTimeInMsec();
    
    CVI_CHAR TaskName[64] = {'\0'};
    sprintf(TaskName, "Thread_Venc%d_Proc", VencChn);
    prctl(PR_SET_NAME, TaskName, 0, 0, 0);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "Venc channel_%d start running\n", VencChn);

    struct timespec ts;

    usleep(1000);

    pastVencChnCfg->bStart = CVI_TRUE;

    while (pastVencChnCfg->bStart) {
        // pthread_mutex_lock(&pastVencChnCfg->SwitchMutex);

        if (app_ipcam_JpgCapFlag_Get() && (vpssChn == 0)) {
            pthread_mutex_lock(&JpgCapMutex);
            pthread_cond_signal(&JpgCapCond);
            pthread_mutex_unlock(&JpgCapMutex);

            pthread_mutex_lock(&VencMutex);

            while (clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
                continue;
            ts.tv_nsec += 200 * 1000 * 1000;
            ts.tv_sec += ts.tv_nsec / 1000000000;
            ts.tv_nsec %= 1000000000;
            pthread_cond_timedwait(&VencCond, &VencMutex, &ts);

            pthread_mutex_unlock(&VencMutex);
        }

        VIDEO_FRAME_INFO_S stVencFrame = {0};
        iTime = GetCurTimeInMsec();

        if (pastVencChnCfg->enBindMode == VENC_BIND_DISABLE) {
            if (CVI_VPSS_GetChnFrame(vpssGrp, vpssChn, &stVencFrame, 3000) != CVI_SUCCESS) {
                continue;
            }
            
            APP_PROF_LOG_PRINT(LEVEL_DEBUG, "VencChn-%d Get Frame takes %u ms \n", 
                                            VencChn, (GetCurTimeInMsec() - iTime));

            #ifdef AI_SUPPORT
            /* draw AI rect to main YUV (streaming) */
            if (VencChn == 0) {
                app_ipcam_Ai_PD_Rect_Draw(&stVencFrame);
                app_ipcam_Ai_MD_Rect_Draw(&stVencFrame);
                app_ipcam_Ai_FD_Rect_Draw(&stVencFrame);
            }
            #endif
        
            if (CVI_VENC_SendFrame(VencChn, &stVencFrame, 3000) != CVI_SUCCESS) {   /* takes 0~1ms */
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "Venc send frame failed with %#x\n", s32Ret);
                s32Ret = CVI_VPSS_ReleaseChnFrame(vpssGrp, vpssChn, &stVencFrame);
                if (s32Ret != CVI_SUCCESS)
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "vpss release Chn-frame failed with:0x%x\n", s32Ret);
                continue;
            }
        }

        VENC_STREAM_S stStream = {0};
        stStream.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * H26X_MAX_NUM_PACKS);
        if (stStream.pstPack == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "streaming malloc memory failed!\n");
            break;
        }

        s32Ret = CVI_VENC_GetStream(VencChn, &stStream, 2000);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VENC_GetStream, VencChn = %d, s32Ret = 0x%X\n", VencChn, s32Ret);
            free(stStream.pstPack);
            stStream.pstPack = NULL;
            if (pastVencChnCfg->enBindMode == VENC_BIND_DISABLE) {
                CVI_VPSS_ReleaseChnFrame(vpssGrp, vpssChn, &stVencFrame);
            }
            continue;
        } else {
            if (pastVencChnCfg->enBindMode == VENC_BIND_DISABLE) {
                CVI_VPSS_ReleaseChnFrame(vpssGrp, vpssChn, &stVencFrame);
            }
        }

#ifdef RECORD_SUPPORT
        /* process streaming ; save to SD Card */
        if (VencChn == 0)
        {
            static CVI_S32 stFrameNum = 0;
            stFrameNum++;
            app_ipcam_Record_VideoInput(pastVencChnCfg->enType, &stStream, stFrameNum);
        }
#endif

        /* process streaming ; send to RTSP or save to flash or both all */
        s32Ret = app_ipcam_Streaming_Send(pastVencChnCfg, &stStream);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_WARN, "app_ipcam_Streaming_Send fail with s32Ret = %d\n", s32Ret);
        }

        s32Ret = CVI_VENC_ReleaseStream(VencChn, &stStream);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VENC_ReleaseStream, s32Ret = %d\n", s32Ret);
            free(stStream.pstPack);
            stStream.pstPack = NULL;
            // pthread_mutex_unlock(&pastVencChnCfg->SwitchMutex);
            continue;
        }
        free(stStream.pstPack);
        stStream.pstPack = NULL;
        // pthread_mutex_unlock(&pastVencChnCfg->SwitchMutex);
    }

    if ((pastVencChnCfg->StreamTo & TO_FLASH) == TO_FLASH) {
        if (pastVencChnCfg->pFile != NULL) {
            fclose(pastVencChnCfg->pFile);
            pastVencChnCfg->pFile = NULL;
        }
    }

    return (CVI_VOID *) CVI_SUCCESS;

}

static void *Thread_Jpg_Proc(void *pArgs)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    APP_VENC_CHN_CFG_S *pstVencChnCfg = (APP_VENC_CHN_CFG_S *)pArgs;
    VENC_CHN VencChn = pstVencChnCfg->VencChn;

    CVI_CHAR TaskName[64];
    sprintf(TaskName, "Thread_JpegEnc");
    prctl(PR_SET_NAME, TaskName, 0, 0, 0);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "jpg encode task start \n");
    
    CVI_S32 vpssGrp = pstVencChnCfg->VpssGrp;
    CVI_S32 vpssChn = pstVencChnCfg->VpssChn;
    VIDEO_FRAME_INFO_S stVencFrame = {0};

    pstVencChnCfg->bStart = CVI_TRUE;

    while (pstVencChnCfg->bStart) {
        pthread_mutex_lock(&JpgCapMutex);
        pthread_cond_wait(&JpgCapCond, &JpgCapMutex);
        pthread_mutex_unlock(&JpgCapMutex);

        if (!pstVencChnCfg->bStart) {
            break;
        }

        s32Ret = CVI_VPSS_GetChnFrame(vpssGrp, vpssChn, &stVencFrame, 2000);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "CVI_VPSS_GetChnFrame failed! %d\n", s32Ret);
            goto rls_lock;
        }
        
        s32Ret = CVI_VENC_SendFrame(VencChn, &stVencFrame, 2000);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "CVI_VENC_SendFrame failed! %d\n", s32Ret);
            goto rls_frame;
        }

        VENC_STREAM_S stStream = {0};
        stStream.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * JPEG_MAX_NUM_PACKS);
        if (stStream.pstPack == NULL) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "streaming malloc memory failed!\n");
            goto rls_frame;
        }

        s32Ret = CVI_VENC_GetStream(VencChn, &stStream, 2000);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VENC_GetStream, VencChn = %d, s32Ret = 0x%X\n", VencChn, s32Ret);
            free(stStream.pstPack);
            stStream.pstPack = NULL;
            goto rls_frame;
        } else {
            CVI_VPSS_ReleaseChnFrame(vpssGrp, vpssChn, &stVencFrame);
        }

        app_ipcam_JpgCapFlag_Set(CVI_FALSE);

        pthread_mutex_lock(&VencMutex);
        pthread_cond_signal(&VencCond);
        pthread_mutex_unlock(&VencMutex);

        /* save jpg to flash */
        _streaming_save_to_flash(pstVencChnCfg, &stStream);

        CVI_VENC_ReleaseStream(VencChn, &stStream);
        free(stStream.pstPack);
        stStream.pstPack = NULL;

        continue;

    rls_frame:
        CVI_VPSS_ReleaseChnFrame(vpssGrp, vpssChn, &stVencFrame);

    rls_lock:
        pthread_mutex_lock(&VencMutex);
        pthread_cond_signal(&VencCond);
        pthread_mutex_unlock(&VencMutex);
    }

    return (void *) CVI_SUCCESS;
}

static int app_ipcam_VencSize_Update(VENC_CHN VencChn, int index)
{
    APP_VENC_CHN_CFG_S *pstVencChnCfg = &g_pstVencCtx->astVencChnCfg[VencChn];

    pstVencChnCfg->u32Width  = stVideoSize[index].u32Width;
    pstVencChnCfg->u32Height = stVideoSize[index].u32Height;

    return CVI_SUCCESS;
}

int app_ipcam_VencSize_Set(void)
{
    /******************************
     *  tmp : video size switch
     *****************************/
    static int index = 0;
    index = index ? 0 : 1;

    app_ipcam_VencSize_Update(0, index);

    app_ipcam_Venc_Stop(APP_VENC_1ST);
    
    app_ipcam_Venc_Init(APP_VENC_1ST);

    app_ipcam_Venc_Start(APP_VENC_1ST);

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Venc_InitVBPool(APP_VENC_CHN_CFG_S *pstVencChnCfg, VENC_CHN VencChnIdx)
{
    SIZE_S stSize = {0};
    VB_CONFIG_S stVbConf = {0};
    VB_POOL_CONFIG_S stVbPoolCfg = {0};
    APP_PARAM_VPSS_CFG_T *pstVpssCfg = app_ipcam_Vpss_Param_Get();

    if ((pstVencChnCfg->enType != PT_H265) && (pstVencChnCfg->enType != PT_H264))
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"enType:%d unsupport init venc vb\n", pstVencChnCfg->enType);
        return CVI_SUCCESS;
    }

    if (gVencPicVbPool[VencChnIdx] != VB_INVALID_POOLID)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"venc vb already init!\n");
        return CVI_SUCCESS;
    }

    if (VencChnIdx == 0)
    {
        stSize.u32Width = pstVpssCfg->astVpssGrpCfg[0].stVpssGrpAttr.u32MaxW;
        stSize.u32Height = pstVpssCfg->astVpssGrpCfg[0].stVpssGrpAttr.u32MaxH;
    }
    else
    {
        stSize.u32Width = pstVpssCfg->astVpssGrpCfg[2].stVpssGrpAttr.u32MaxW;
        stSize.u32Height = pstVpssCfg->astVpssGrpCfg[2].stVpssGrpAttr.u32MaxH;
    }

    if (pstVencChnCfg->enType == PT_H264)
    {
        CVI_U32 u32WidthAlign = (ALIGN(stSize.u32Width, DEFAULT_ALIGN));
        CVI_U32 u32HeightAlign = (ALIGN(stSize.u32Height, DEFAULT_ALIGN));
        stVbPoolCfg.u32BlkSize = (u32WidthAlign * u32HeightAlign * 2 * 3) / 2;
    }
    else
    {
        stVbPoolCfg.u32BlkSize = ((stSize.u32Width * stSize.u32Height * 3) / 2);
        stVbPoolCfg.u32BlkSize = (ALIGN(stVbPoolCfg.u32BlkSize, 4096));
    }
    stVbPoolCfg.u32BlkCnt   = 2;
    stVbPoolCfg.enRemapMode = VB_REMAP_MODE_CACHED;
    gVencPicVbPool[VencChnIdx] = CVI_VB_CreatePool(&stVbPoolCfg);
    if (gVencPicVbPool[VencChnIdx] == VB_INVALID_POOLID)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"venc:%d create pool failed!!\n", VencChnIdx);
        return CVI_FAILURE;
    }

    CVI_VB_GetConfig(&stVbConf);
    stVbConf.astCommPool[gVencPicVbPool[VencChnIdx]].u32BlkSize = stVbPoolCfg.u32BlkSize;
    stVbConf.astCommPool[gVencPicVbPool[VencChnIdx]].u32BlkCnt = stVbPoolCfg.u32BlkCnt;
    CVI_VB_SetConfig(&stVbConf);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "CVI_VB_CreatePool : id:%d, u32BlkSize=0x%x, u32BlkCnt=%d chn[%d]\n",
                        gVencPicVbPool[VencChnIdx], stVbPoolCfg.u32BlkSize, stVbPoolCfg.u32BlkCnt, VencChnIdx);

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Venc_DeInitVBPool(VENC_CHN VencChnIdx)
{
    VB_CONFIG_S stVbConf = {0};

    if (gVencPicVbPool[VencChnIdx] == VB_INVALID_POOLID)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"venc vb no init!\n");
        return CVI_SUCCESS;
    }

    if (CVI_VB_DestroyPool(gVencPicVbPool[VencChnIdx]) != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"venc:%d destroy pool failed!\n", VencChnIdx);
        return CVI_SUCCESS;
    }

    CVI_VB_GetConfig(&stVbConf);
    stVbConf.astCommPool[gVencPicVbPool[VencChnIdx]].u32BlkSize = 0;
    stVbConf.astCommPool[gVencPicVbPool[VencChnIdx]].u32BlkCnt = 0;
    CVI_VB_SetConfig(&stVbConf);

    gVencPicVbPool[VencChnIdx] = VB_INVALID_POOLID;

    return CVI_SUCCESS;
}

int app_ipcam_Venc_Init(APP_VENC_CHN_E VencIdx)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    APP_PROF_LOG_PRINT(LEVEL_INFO, "Ven init ------------------> start \n");

    VB_SOURCE_E eVbSource = VB_SOURCE_USER;//VB_SOURCE_COMMON
    app_ipcam_Venc_SingleEsBuff_Set(eVbSource);

    for (VENC_CHN s32ChnIdx = 0; s32ChnIdx < g_pstVencCtx->s32VencChnCnt; s32ChnIdx++) {
        APP_VENC_CHN_CFG_S *pstVencChnCfg = &g_pstVencCtx->astVencChnCfg[s32ChnIdx];
        VENC_CHN VencChn = pstVencChnCfg->VencChn;
        if ((!pstVencChnCfg->bEnable) || (pstVencChnCfg->bStart))
            continue;

        if (!((VencIdx >> s32ChnIdx) & 0x01))
            continue;

        APP_PROF_LOG_PRINT(LEVEL_TRACE, "Ven_%d init info\n", VencChn);
        APP_PROF_LOG_PRINT(LEVEL_TRACE, "VpssGrp=%d VpssChn=%d size_W=%d size_H=%d CodecType=%d save_path=%s\n", 
            pstVencChnCfg->VpssGrp, pstVencChnCfg->VpssChn, pstVencChnCfg->u32Width, pstVencChnCfg->u32Height,
            pstVencChnCfg->enType, pstVencChnCfg->SavePath);

        PAYLOAD_TYPE_E enCodecType = pstVencChnCfg->enType;
        VENC_CHN_ATTR_S stVencChnAttr, *pstVencChnAttr = &stVencChnAttr;
        memset(&stVencChnAttr, 0, sizeof(stVencChnAttr));

        /* Venc channel validity check */
        if (VencChn != pstVencChnCfg->VencChn) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"VencChn error %d\n", VencChn);
            goto VENC_EXIT0;
        }

        if (VB_SOURCE_USER == eVbSource)
        {
            s32Ret = app_ipcam_Venc_InitVBPool(pstVencChnCfg, VencChn);
            if (s32Ret != CVI_SUCCESS)
            {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"media_venc_init_vb [%d] failed with %d\n", VencChn, s32Ret);
                goto VENC_EXIT0;
            }
        }

        s32Ret = app_ipcam_Venc_Chn_Attr_Set(&pstVencChnAttr->stVencAttr, pstVencChnCfg);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"media_venc_set_attr [%d] failed with %d\n", VencChn, s32Ret);
            goto VENC_EXIT0;
        }

        s32Ret = app_ipcam_Venc_Gop_Attr_Set(&pstVencChnAttr->stGopAttr, pstVencChnCfg);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"media_venc_set_gop [%d] failed with %d\n", VencChn, s32Ret);
            goto VENC_EXIT0;
        }

        s32Ret = app_ipcam_Venc_Rc_Attr_Set(&pstVencChnAttr->stRcAttr, pstVencChnCfg);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"media_venc_set_rc_attr [%d] failed with %d\n", VencChn, s32Ret);
            goto VENC_EXIT0;
        }

        app_ipcam_Venc_Attr_Check(pstVencChnAttr);

        s32Ret = CVI_VENC_CreateChn(VencChn, pstVencChnAttr);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VENC_CreateChn [%d] failed with %d\n", VencChn, s32Ret);
            goto VENC_EXIT1;
        }

        if ((enCodecType == PT_H265) || (enCodecType == PT_H264)) {
            if (gVencPicVbPool[VencChn] != VB_INVALID_POOLID)
            {
                VENC_CHN_POOL_S stPool = {0};
                stPool.hPicVbPool = gVencPicVbPool[VencChn];
                stPool.hPicInfoVbPool = VB_INVALID_POOLID;
                s32Ret = CVI_VENC_AttachVbPool(VencChn, &stPool);
                if (s32Ret != CVI_SUCCESS)
                {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VENC_AttachVbPool venc:%d failed, %d\n", VencChn, s32Ret);
                    goto VENC_EXIT1;
                }
            }
            s32Ret = app_ipcam_Venc_Rc_Param_Set(VencChn, enCodecType, pstVencChnCfg->enRcMode, &pstVencChnCfg->stRcParam);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"Venc_%d RC Param Set failed with %d\n", VencChn, s32Ret);
                goto VENC_EXIT1;
            }
        } else if (enCodecType == PT_JPEG) {
            s32Ret = app_ipcam_Venc_Jpeg_Param_Set(VencChn, &pstVencChnCfg->stJpegCodecParam);
                if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"Venc_%d JPG Param Set failed with %d\n", VencChn, s32Ret);
                goto VENC_EXIT1;
            }
        }

        if (pstVencChnCfg->enBindMode != VENC_BIND_DISABLE) {
            s32Ret = CVI_SYS_Bind(&pstVencChnCfg->astChn[0], &pstVencChnCfg->astChn[1]);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_SYS_Bind failed with %#x\n", s32Ret);
                goto VENC_EXIT1;
            }
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "Ven init ------------------> done \n");

    return CVI_SUCCESS;

VENC_EXIT1:
    for (VENC_CHN s32ChnIdx = 0; s32ChnIdx < g_pstVencCtx->s32VencChnCnt; s32ChnIdx++) {
        CVI_VENC_ResetChn(g_pstVencCtx->astVencChnCfg[s32ChnIdx].VencChn);
        CVI_VENC_DestroyChn(g_pstVencCtx->astVencChnCfg[s32ChnIdx].VencChn);
        app_ipcam_Venc_DeInitVBPool(g_pstVencCtx->astVencChnCfg[s32ChnIdx].VencChn);
    }

VENC_EXIT0:
    app_ipcam_Vpss_DeInit();
    app_ipcam_Vi_DeInit();
    app_ipcam_Sys_DeInit();

    return s32Ret;
}

int app_ipcam_Venc_Stop(APP_VENC_CHN_E VencIdx)
{
    CVI_S32 s32Ret;

    APP_PROF_LOG_PRINT(LEVEL_INFO, "Venc Count=%d and will stop VencChn=0x%x\n", g_pstVencCtx->s32VencChnCnt, VencIdx);

    for (VENC_CHN s32ChnIdx = 0; s32ChnIdx < g_pstVencCtx->s32VencChnCnt; s32ChnIdx++) {
        APP_VENC_CHN_CFG_S *pstVencChnCfg = &g_pstVencCtx->astVencChnCfg[s32ChnIdx];
        VENC_CHN VencChn = pstVencChnCfg->VencChn;

        if (!pstVencChnCfg->bStart) {
            APP_PROF_LOG_PRINT(LEVEL_WARN, "Venc_%d not start \n", VencChn);
            continue;
        }
        
        if (!((VencIdx >> s32ChnIdx) & 0x01))
            continue;

        pstVencChnCfg->bStart = CVI_FALSE;
        
        /* for jpg capture */
        if (pstVencChnCfg->enType == PT_JPEG) {
            pthread_mutex_lock(&JpgCapMutex);
            pthread_cond_signal(&JpgCapCond);
            pthread_mutex_unlock(&JpgCapMutex);
        }

        if (g_Venc_pthread[VencChn] != 0) {
            pthread_join(g_Venc_pthread[VencChn], CVI_NULL);
            APP_PROF_LOG_PRINT(LEVEL_WARN, "Venc_%d Streaming Proc done \n", VencChn);
            g_Venc_pthread[VencChn] = 0;
        }

        if (pstVencChnCfg->enBindMode != VENC_BIND_DISABLE) {
            s32Ret = CVI_SYS_UnBind(&pstVencChnCfg->astChn[0], &pstVencChnCfg->astChn[1]);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_SYS_UnBind failed with %#x\n", s32Ret);
                return s32Ret;
            }
        }

        s32Ret = CVI_VENC_StopRecvFrame(VencChn);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VENC_StopRecvFrame vechn[%d] failed with %#x\n", VencChn, s32Ret);
            return s32Ret;
        }

        s32Ret = CVI_VENC_ResetChn(VencChn);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VENC_ResetChn vechn[%d] failed with %#x\n", VencChn, s32Ret);
            return s32Ret;
        }

        s32Ret = CVI_VENC_DestroyChn(VencChn);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_VENC_DestroyChn vechn[%d] failed with %#x\n", VencChn, s32Ret);
            return s32Ret;
        }
        app_ipcam_Venc_DeInitVBPool(VencChn);
    }

    return CVI_SUCCESS;
}

int app_ipcam_Venc_Start(APP_VENC_CHN_E VencIdx)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    for (VENC_CHN s32ChnIdx = 0; s32ChnIdx < g_pstVencCtx->s32VencChnCnt; s32ChnIdx++) {
        APP_VENC_CHN_CFG_S *pstVencChnCfg = &g_pstVencCtx->astVencChnCfg[s32ChnIdx];
        VENC_CHN VencChn = pstVencChnCfg->VencChn;
        if ((!pstVencChnCfg->bEnable) || (pstVencChnCfg->bStart))
            continue;

        if (!((VencIdx >> s32ChnIdx) & 0x01))
            continue;

        VENC_RECV_PIC_PARAM_S stRecvParam = {0};
        stRecvParam.s32RecvPicNum = -1;

        APP_CHK_RET(CVI_VENC_StartRecvFrame(VencChn, &stRecvParam), "Start recv frame");

        pthread_attr_t pthread_attr;
        pthread_attr_init(&pthread_attr);
        #if 1
        struct sched_param param;
        param.sched_priority = 80;
        pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
        pthread_attr_setschedparam(&pthread_attr, &param);
        pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);
        #endif

        // pthread_mutex_init(&pastVencChnCfg->SwitchMutex, NULL);
        pfp_task_entry fun_entry = NULL;
        if (pstVencChnCfg->enType == PT_JPEG) {
            fun_entry = Thread_Jpg_Proc;
        } else {
            fun_entry = Thread_Streaming_Proc;
        }

        g_Venc_pthread[VencChn] = 0;
        s32Ret = pthread_create(
                        &g_Venc_pthread[VencChn],
                        &pthread_attr,
                        fun_entry,
                        (CVI_VOID *)pstVencChnCfg);
        if (s32Ret) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "[Chn %d]pthread_create failed:0x%x\n", VencChn, s32Ret);
            return CVI_FAILURE;
        }
    }
    return CVI_SUCCESS;
}



/*****************************************************************
 *  The following API for command test used             Front
 * **************************************************************/

static int app_ipcam_CmdTask_VideoAttr_Parse(
    CVI_MQ_MSG_t *msg, 
    CVI_VOID *userdate, 
    VENC_CHN *pVencChn,
    APP_NEED_STOP_MODULE_E *penStopModule)
{

    CVI_CHAR param[256] = {0};
    snprintf(param, sizeof(param), "%s", msg->payload);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s param:%s Venc=%d\n", __FUNCTION__, param, msg->arg2);

    char *pc = strchr(param, 'i');
    if (pc != NULL) {
        *pVencChn = atoi(pc+2);
    } else {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "not specify which channel you want change!!!\n");
        return CVI_FAILURE;
    }

    APP_VENC_CHN_CFG_S *pstVencChnCfg = app_ipcam_VencChnCfg_Get(*pVencChn);

    CVI_CHAR *temp = strtok(param, ":");
    while(NULL != temp) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "%s switch case -> %c \n", __FUNCTION__, *temp);
        switch (*temp) {
            case 'i': 
                temp = strtok(NULL, "/");
                *pVencChn = atoi(temp);
                break;
            case 'c': 
                {
                    PAYLOAD_TYPE_E enType = pstVencChnCfg->enType;
                    temp = strtok(NULL, "/");
                    APP_IPCAM_CODEC_CHECK(temp, pstVencChnCfg->enType);
                    if (enType != pstVencChnCfg->enType) {
                        *penStopModule = APP_NEED_STOP_RTSP;
                    }
                }
                break;
            case 'w':
                temp = strtok(NULL, "/");
                pstVencChnCfg->u32Width = atoi(temp);
                break;
            case 'h':
                temp = strtok(NULL, "/");
                pstVencChnCfg->u32Height = atoi(temp);
                break;
            case 'r':
                temp = strtok(NULL, "/");
                APP_IPCAM_RCMODE_CHECK(temp, pstVencChnCfg->enRcMode);
                break;
            case 'b':
                temp = strtok(NULL, "/");
                pstVencChnCfg->u32BitRate = atoi(temp);
                break;
            case 'g':
                temp = strtok(NULL, "/");
                pstVencChnCfg->u32Gop = atoi(temp);
                break;
            case 'f':
                temp = strtok(NULL, "/");
                pstVencChnCfg->fr32DstFrameRate = atoi(temp);
                break;
            default:
                return 0;
                break;
        }
        
        temp = strtok(NULL, ":");
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "enType=%d Width=%d Height=%d RcMode=%d BitRate=%d Gop=%d fps=%d\n", 
        pstVencChnCfg->enType, pstVencChnCfg->u32Width, pstVencChnCfg->u32Height,
        pstVencChnCfg->enRcMode, pstVencChnCfg->u32BitRate, pstVencChnCfg->u32Gop, pstVencChnCfg->fr32DstFrameRate);

    return CVI_SUCCESS;
}

int app_ipcam_CmdTask_VideoAttr_Set(CVI_MQ_MSG_t *msg, CVI_VOID *userdate)
{
    VENC_CHN VencChn = 0;
    APP_NEED_STOP_MODULE_E enStopModule = APP_NEED_STOP_NULL;

    APP_CHK_RET(app_ipcam_CmdTask_VideoAttr_Parse(msg, userdate, &VencChn, &enStopModule), "video Attr Parse");

    APP_PROF_LOG_PRINT(LEVEL_INFO, "VencChn = %d\n", VencChn);

    APP_IPCAM_STREAM_ID_TO_CHN(VencChn);

    if (enStopModule & APP_NEED_STOP_RTSP) {
        APP_CHK_RET(app_ipcam_rtsp_Server_Destroy(), "Destory RTSP Server");
    }

    APP_CHK_RET(app_ipcam_Venc_Stop(VencChn), "Stop Venc");
    
    if (enStopModule & APP_NEED_STOP_RTSP) {
        APP_CHK_RET(app_ipcam_Rtsp_Server_Create(), "Create RTSP Server");
    }
    
    APP_CHK_RET(app_ipcam_Venc_Init(VencChn), "Venc Init");

    APP_CHK_RET(app_ipcam_Venc_Start(VencChn), "Venc Start");

    return CVI_SUCCESS;
}

/*****************************************************************
 *  The above API for command test used                 End
 * **************************************************************/
