
#include "cvi_video.h"
#include "mipi_tx.h"
#include <fcntl.h>
#include "cvi_comm_vpss.h"
#include "sample_comm.h"

#define ENABLE_REFINE_VB
#define DUAL_SENSOR

static	SAMPLE_VO_CONFIG_S stVoConfig;

static 	SAMPLE_VI_CONFIG_S stViConfig;

static pthread_mutex_t mutex_video = PTHREAD_MUTEX_INITIALIZER;
bool freeze_status = false;
static VI_PIPE ViPipe = 0;
#define MAX(a,b) (((a)>(b))?(a):(b))

void CVI_Set_VI_Config_dual(SAMPLE_VI_CONFIG_S* stViConfig)
{
#if defined(CUSTOMIZED_PCBA) || defined(CUSTOMIZED_PCBB)
	SAMPLE_SNS_TYPE_E  enSnsType	    = SOI_F35_MIPI_2M_30FPS_10BIT;
	SAMPLE_SNS_TYPE_E  enSnsType2	    = SOI_F35_SLAVE_MIPI_2M_30FPS_10BIT;
#else
	#if defined(WEDB_0001C_IMX307)
		SAMPLE_SNS_TYPE_E  enSnsType	    = SONY_IMX307_2L_MIPI_2M_30FPS_12BIT;
		SAMPLE_SNS_TYPE_E  enSnsType2	    = SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT;
	#else
		SAMPLE_SNS_TYPE_E  enSnsType	    = SONY_IMX327_2L_MIPI_2M_30FPS_12BIT;
		SAMPLE_SNS_TYPE_E  enSnsType2	    = SONY_IMX327_SLAVE_MIPI_2M_30FPS_12BIT;
	#endif
#endif
	WDR_MODE_E	   enWDRMode	    = WDR_MODE_NONE;
	DYNAMIC_RANGE_E    enDynamicRange   = DYNAMIC_RANGE_SDR8;
	PIXEL_FORMAT_E     enPixFormat	    = PIXEL_FORMAT_YUV_PLANAR_420;
	VIDEO_FORMAT_E     enVideoFormat    = VIDEO_FORMAT_LINEAR;
	COMPRESS_MODE_E    enCompressMode   = COMPRESS_MODE_TILE;
	VI_VPSS_MODE_E	   enMastPipeMode   = VI_OFFLINE_VPSS_OFFLINE;

	SAMPLE_COMM_VI_GetSensorInfo(stViConfig);

#ifdef ENABLE_WDR
#if defined(CUSTOMIZED_PCBA) || defined(CUSTOMIZED_PCBB)
	enSnsType	    = SOI_F35_MIPI_2M_30FPS_10BIT_WDR2TO1;
	enSnsType2	    = SOI_F35_SLAVE_MIPI_2M_30FPS_10BIT_WDR2TO1;
#else
	#if defined(WEDB_0001C_IMX307)
		enSnsType	    = SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1;
		enSnsType2	    = SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT_WDR2TO1;
	#else
		enSnsType	    = SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1;
		enSnsType2	    = SONY_IMX327_SLAVE_MIPI_2M_30FPS_12BIT_WDR2TO1;
	#endif
#endif
	enWDRMode	    = WDR_MODE_2To1_LINE;
#endif
	stViConfig->s32WorkingViNum			     = 2;
	stViConfig->bViRotation = true;

	stViConfig->as32WorkingViId[0]			     = 0;
	stViConfig->astViInfo[0].stSnsInfo.enSnsType	     = enSnsType;
#if defined(CUSTOMIZED_PCBA) || defined(CUSTOMIZED_PCBB)
	stViConfig->astViInfo[0].stSnsInfo.MipiDev	     = 0;
#else
	#if defined(WEDB_0001C_IMX307)
		stViConfig->astViInfo[0].stSnsInfo.MipiDev	     = 0;
	#else
		stViConfig->astViInfo[0].stSnsInfo.MipiDev	     = 0xFF;
	#endif
#endif
	stViConfig->astViInfo[0].stSnsInfo.s32BusId	     = 3;
	stViConfig->astViInfo[0].stDevInfo.ViDev	     = 0;
	stViConfig->astViInfo[0].stDevInfo.enWDRMode	     = enWDRMode;
	stViConfig->astViInfo[0].stPipeInfo.enMastPipeMode   = enMastPipeMode;
	stViConfig->astViInfo[0].stPipeInfo.aPipe[0]	     = 0;
	stViConfig->astViInfo[0].stPipeInfo.aPipe[1]	     = -1;
	stViConfig->astViInfo[0].stPipeInfo.aPipe[2]	     = -1;
	stViConfig->astViInfo[0].stPipeInfo.aPipe[3]	     = -1;
	stViConfig->astViInfo[0].stChnInfo.ViChn	     = 0;
	stViConfig->astViInfo[0].stChnInfo.enPixFormat       = enPixFormat;
	stViConfig->astViInfo[0].stChnInfo.enDynamicRange    = enDynamicRange;
	stViConfig->astViInfo[0].stChnInfo.enVideoFormat     = enVideoFormat;
	stViConfig->astViInfo[0].stChnInfo.enCompressMode    = enCompressMode;
#if defined(CUSTOMIZED_PCBA)
	stViConfig->astViInfo[0].stSnsInfo.as16LaneId[0]     = 1;
	stViConfig->astViInfo[0].stSnsInfo.as16LaneId[1]     = 2;
	stViConfig->astViInfo[0].stSnsInfo.as16LaneId[2]     = 3;
	stViConfig->astViInfo[0].stSnsInfo.as16LaneId[3]     = -1;
	stViConfig->astViInfo[0].stSnsInfo.as16LaneId[4]     = -1;
	stViConfig->astViInfo[0].stSnsInfo.as8PNSwap[0]      = 1;
	stViConfig->astViInfo[0].stSnsInfo.as8PNSwap[1]      = 1;
	stViConfig->astViInfo[0].stSnsInfo.as8PNSwap[2]      = 1;
	stViConfig->astViInfo[0].stSnsInfo.as8PNSwap[3]      = 0;
	stViConfig->astViInfo[0].stSnsInfo.as8PNSwap[4]      = 0;
#elif defined(CUSTOMIZED_PCBB)
	stViConfig->astViInfo[0].stSnsInfo.s32BusId	     = 0;
	stViConfig->astViInfo[0].stSnsInfo.as16LaneId[0]     = 3;
	stViConfig->astViInfo[0].stSnsInfo.as16LaneId[1]     = 2;
	stViConfig->astViInfo[0].stSnsInfo.as16LaneId[2]     = 1;
	stViConfig->astViInfo[0].stSnsInfo.as16LaneId[3]     = -1;
	stViConfig->astViInfo[0].stSnsInfo.as16LaneId[4]     = -1;
#else
	#if defined(WEDB_0001C_IMX307)
		stViConfig->astViInfo[0].stSnsInfo.as16LaneId[0]     = 3;
		stViConfig->astViInfo[0].stSnsInfo.as16LaneId[1]     = 4;
		stViConfig->astViInfo[0].stSnsInfo.as16LaneId[2]     = 0;
		stViConfig->astViInfo[0].stSnsInfo.as16LaneId[3]     = -1;
		stViConfig->astViInfo[0].stSnsInfo.as16LaneId[4]     = -1;
	#endif
#endif

	stViConfig->as32WorkingViId[1]			     = 1;
	stViConfig->astViInfo[1].stSnsInfo.enSnsType	     = enSnsType2;
#if defined(CUSTOMIZED_PCBA) || defined(CUSTOMIZED_PCBB)
	stViConfig->astViInfo[1].stSnsInfo.MipiDev	     = 1;
#else
	#if defined(WEDB_0001C_IMX307)
		stViConfig->astViInfo[1].stSnsInfo.MipiDev	     = 1;
	#else
		stViConfig->astViInfo[1].stSnsInfo.MipiDev	     = 0xFF;
	#endif
#endif
	stViConfig->astViInfo[1].stSnsInfo.s32BusId	     = 0;
	stViConfig->astViInfo[1].stDevInfo.ViDev	     = 1;
	stViConfig->astViInfo[1].stDevInfo.enWDRMode	     = enWDRMode;
	stViConfig->astViInfo[1].stPipeInfo.enMastPipeMode   = enMastPipeMode;
	stViConfig->astViInfo[1].stPipeInfo.aPipe[0]	     = 1;
	stViConfig->astViInfo[1].stPipeInfo.aPipe[1]	     = -1;
	stViConfig->astViInfo[1].stPipeInfo.aPipe[2]	     = -1;
	stViConfig->astViInfo[1].stPipeInfo.aPipe[3]	     = -1;
	stViConfig->astViInfo[1].stChnInfo.ViChn	     = 1;
	stViConfig->astViInfo[1].stChnInfo.enPixFormat       = enPixFormat;
	stViConfig->astViInfo[1].stChnInfo.enDynamicRange    = enDynamicRange;
	stViConfig->astViInfo[1].stChnInfo.enVideoFormat     = enVideoFormat;
	stViConfig->astViInfo[1].stChnInfo.enCompressMode    = enCompressMode;
#if defined(CUSTOMIZED_PCBA)
	stViConfig->astViInfo[1].stSnsInfo.as16LaneId[0]     = 4;
	stViConfig->astViInfo[1].stSnsInfo.as16LaneId[1]     = 2;
	stViConfig->astViInfo[1].stSnsInfo.as16LaneId[2]     = 0;
	stViConfig->astViInfo[1].stSnsInfo.as16LaneId[3]     = -1;
	stViConfig->astViInfo[1].stSnsInfo.as16LaneId[4]     = -1;
	stViConfig->astViInfo[1].stSnsInfo.as8PNSwap[0]      = 1;
	stViConfig->astViInfo[1].stSnsInfo.as8PNSwap[1]      = 1;
	stViConfig->astViInfo[1].stSnsInfo.as8PNSwap[2]      = 1;
	stViConfig->astViInfo[1].stSnsInfo.as8PNSwap[3]      = 0;
	stViConfig->astViInfo[1].stSnsInfo.as8PNSwap[4]      = 0;
#elif defined(CUSTOMIZED_PCBB)
	stViConfig->astViInfo[1].stSnsInfo.s32BusId	     = 0;
	stViConfig->astViInfo[1].stSnsInfo.as16LaneId[0]     = 0;
	stViConfig->astViInfo[1].stSnsInfo.as16LaneId[1]     = 2;
	stViConfig->astViInfo[1].stSnsInfo.as16LaneId[2]     = 4;
	stViConfig->astViInfo[1].stSnsInfo.as16LaneId[3]     = -1;
	stViConfig->astViInfo[1].stSnsInfo.as16LaneId[4]     = -1;
#else
	#if defined(WEDB_0001C_IMX307)
		stViConfig->astViInfo[1].stSnsInfo.as16LaneId[0]     = 4;
		stViConfig->astViInfo[1].stSnsInfo.as16LaneId[1]     = 3;
		stViConfig->astViInfo[1].stSnsInfo.as16LaneId[2]     = 2;
		stViConfig->astViInfo[1].stSnsInfo.as16LaneId[3]     = -1;
		stViConfig->astViInfo[1].stSnsInfo.as16LaneId[4]     = -1;
	#endif
#endif
}

void CVI_Set_VI_Config_Ini(SAMPLE_VI_CONFIG_S* stViConfig)
{
	SAMPLE_INI_CFG_S	   stIniCfg = {
		.enSource  = VI_PIPE_FRAME_SOURCE_DEV,
		.devNum    = 1,
		.enSnsType = SONY_IMX327_MIPI_2M_30FPS_12BIT,
		.enWDRMode = WDR_MODE_NONE,
		.s32BusId  = 3,
		.MipiDev   = 0xFF,
		.u8UseDualSns = 0,
		.enSns2Type = SONY_IMX327_SLAVE_MIPI_2M_30FPS_12BIT,
		.s32Sns2BusId = 0,
		.Sns2MipiDev = 0xFF,
	};
	DYNAMIC_RANGE_E    enDynamicRange   = DYNAMIC_RANGE_SDR8;
	PIXEL_FORMAT_E	   enPixFormat	    = PIXEL_FORMAT_YUV_PLANAR_420;
	VIDEO_FORMAT_E	   enVideoFormat    = VIDEO_FORMAT_LINEAR;
	COMPRESS_MODE_E    enCompressMode   = COMPRESS_MODE_TILE;
	VI_VPSS_MODE_E	   enMastPipeMode   = VI_OFFLINE_VPSS_OFFLINE;

	CVI_S32 s32WorkSnsId = 0;
	VI_CHN ViChn = 0;

	// Get config from ini if found.
	if (SAMPLE_COMM_VI_ParseIni(&stIniCfg)) {
		SAMPLE_PRT("Parse complete\n");
	}

	/************************************************
	 * step1:  Config VI
	 ************************************************/
	SAMPLE_COMM_VI_GetSensorInfo(stViConfig);
	//stViConfig->bViRotation = true;
	for (; s32WorkSnsId < stIniCfg.devNum; s32WorkSnsId++) {
		stViConfig->s32WorkingViNum				     = 1 + s32WorkSnsId;
		stViConfig->as32WorkingViId[s32WorkSnsId]		     = s32WorkSnsId;
		stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.enSnsType	     =
			(s32WorkSnsId == 0) ? stIniCfg.enSnsType : stIniCfg.enSns2Type;
		stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.MipiDev	     =
			(s32WorkSnsId == 0) ? stIniCfg.MipiDev : stIniCfg.Sns2MipiDev;
		stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.s32BusId	     =
			(s32WorkSnsId == 0) ? stIniCfg.s32BusId : stIniCfg.s32Sns2BusId;
		stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[0]   =
			(s32WorkSnsId == 0) ? stIniCfg.as16LaneId[0] : stIniCfg.as16Sns2LaneId[0];
		stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[1]   =
			(s32WorkSnsId == 0) ? stIniCfg.as16LaneId[1] : stIniCfg.as16Sns2LaneId[1];
		stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[2]   =
			(s32WorkSnsId == 0) ? stIniCfg.as16LaneId[2] : stIniCfg.as16Sns2LaneId[2];
		stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[3]   =
			(s32WorkSnsId == 0) ? stIniCfg.as16LaneId[3] : stIniCfg.as16Sns2LaneId[3];
		stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[4]   =
			(s32WorkSnsId == 0) ? stIniCfg.as16LaneId[4] : stIniCfg.as16Sns2LaneId[4];
		stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[0]   =
			(s32WorkSnsId == 0) ? stIniCfg.as8PNSwap[0] : stIniCfg.as8Sns2PNSwap[0];
		stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[1]   =
			(s32WorkSnsId == 0) ? stIniCfg.as8PNSwap[1] : stIniCfg.as8Sns2PNSwap[1];
		stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[2]   =
			(s32WorkSnsId == 0) ? stIniCfg.as8PNSwap[2] : stIniCfg.as8Sns2PNSwap[2];
		stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[3]   =
			(s32WorkSnsId == 0) ? stIniCfg.as8PNSwap[3] : stIniCfg.as8Sns2PNSwap[3];
		stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[4]   =
			(s32WorkSnsId == 0) ? stIniCfg.as8PNSwap[4] : stIniCfg.as8Sns2PNSwap[4];

		stViConfig->astViInfo[s32WorkSnsId].stDevInfo.ViDev	     = 0;
		stViConfig->astViInfo[s32WorkSnsId].stDevInfo.enWDRMode	     =
			(s32WorkSnsId == 0) ? stIniCfg.enWDRMode : stIniCfg.enSns2WDRMode;

		stViConfig->astViInfo[s32WorkSnsId].stPipeInfo.enMastPipeMode = enMastPipeMode;
		stViConfig->astViInfo[s32WorkSnsId].stPipeInfo.aPipe[0]	     = s32WorkSnsId;
		stViConfig->astViInfo[s32WorkSnsId].stPipeInfo.aPipe[1]	     = -1;
		stViConfig->astViInfo[s32WorkSnsId].stPipeInfo.aPipe[2]	     = -1;
		stViConfig->astViInfo[s32WorkSnsId].stPipeInfo.aPipe[3]	     = -1;

		stViConfig->astViInfo[s32WorkSnsId].stChnInfo.ViChn	     = ViChn;
		stViConfig->astViInfo[s32WorkSnsId].stChnInfo.enPixFormat     = enPixFormat;
		stViConfig->astViInfo[s32WorkSnsId].stChnInfo.enDynamicRange  = enDynamicRange;
		stViConfig->astViInfo[s32WorkSnsId].stChnInfo.enVideoFormat   = enVideoFormat;
		stViConfig->astViInfo[s32WorkSnsId].stChnInfo.enCompressMode  = enCompressMode;
	}

}

CVI_S32 Set_Vpss_config(VPSS_GRP VpssGrp,VPSS_GRP_ATTR_S* stVpssGrpAttr)
{
	VPSS_CHN VpssChn = VPSS_CHN0;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = { 0 };
	VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	CVI_S32 s32Ret = CVI_SUCCESS;

	if(VpssGrp == 0)
	{
		//channel0 for vo & capture
		abChnEnable[0] = CVI_TRUE;
		VpssChn        = VPSS_CHN0;
		astVpssChnAttr[VpssChn].u32Width                    = 1280;
		astVpssChnAttr[VpssChn].u32Height                   = 720;
		astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_YUV_PLANAR_420;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth                    = 1;
		astVpssChnAttr[VpssChn].bMirror                     = 1;
		astVpssChnAttr[VpssChn].bFlip                       = 1;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_MANUAL;
		astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32X = 0;
		astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32Y = 0;
		astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Width = 1280;
		astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Height = 720;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;

		//channel1 for rtsp
		abChnEnable[1] = CVI_TRUE;
		VpssChn        = VPSS_CHN1;
		astVpssChnAttr[VpssChn].u32Width                    = 1280;
		astVpssChnAttr[VpssChn].u32Height                   = 720;
		astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_YUV_PLANAR_420;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth                    = 1;
		astVpssChnAttr[VpssChn].bMirror                     = 1;
		astVpssChnAttr[VpssChn].bFlip                       = 1;
		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor  = COLOR_RGB_BLACK;

		abChnEnable[2] = CVI_TRUE;
		VpssChn        = VPSS_CHN2;
		astVpssChnAttr[VpssChn].u32Width                    = 640;
		astVpssChnAttr[VpssChn].u32Height                   = 360;
		astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_YUV_PLANAR_420;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth                    = 1;
		astVpssChnAttr[VpssChn].bMirror                     = 1;
		astVpssChnAttr[VpssChn].bFlip                       = 1;
		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor  = COLOR_RGB_BLACK;

	}
	else if(VpssGrp == 1)
	{
		//grp1_channel1 for rtsp
		abChnEnable[1] = CVI_TRUE;
		VpssChn        = VPSS_CHN1;
		astVpssChnAttr[VpssChn].u32Width                    = 1280;
		astVpssChnAttr[VpssChn].u32Height                   = 720;
		astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_YUV_PLANAR_420;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth                    = 1;
		astVpssChnAttr[VpssChn].bMirror                     = 1;
		astVpssChnAttr[VpssChn].bFlip                       = 1;
		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor  = COLOR_RGB_BLACK;
	}

	CVI_SYS_SetVPSSMode(VPSS_MODE_SINGLE);

	/*start vpss*/
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	if (VpssGrp == 0) {
		s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 0, VpssGrp);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
 	}

	if (VpssGrp == 1) {
		s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 1, VpssGrp);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	return s32Ret;
}


static void _initInputCfg(chnInputCfg *ipIc)
{
    ipIc->rcMode = -1;
    ipIc->iqp = -1;
    ipIc->pqp = -1;
    ipIc->gop = -1;
    ipIc->bitrate = -1;
    ipIc->firstFrmstartQp = -1;
    ipIc->num_frames = -1;
    ipIc->framerate = 30;
    ipIc->maxQp = -1;
    ipIc->minQp = -1;
    ipIc->maxIqp = -1;
    ipIc->minIqp = -1;
}
chnInputCfg pIc[2]; //for venc

static CVI_S32 InitEnc(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VENC_CHN VencChn[2] = {0, 1};
    PAYLOAD_TYPE_E enPayLoad = PT_H264;
	PIC_SIZE_E enSize = PIC_720P;
    VENC_GOP_MODE_E enGopMode = VENC_GOPMODE_NORMALP;
    VENC_GOP_ATTR_S stGopAttr;
    SAMPLE_RC_E enRcMode;
    CVI_U32 u32Profile = 0;
	CVI_U32 i;

	for(i = 0; i < sizeof(pIc)/sizeof(chnInputCfg); i++){
		memset(&pIc[i],0,sizeof(chnInputCfg));
		_initInputCfg(&pIc[i]);
		strcpy(pIc[i].codec, "264");
		pIc[i].rcMode = 0; // cbr
		pIc[i].iqp = 38;
		pIc[i].pqp = 38;
		pIc[i].gop = 60;
		pIc[i].bitrate = 1800; // if fps = 20
		pIc[i].firstFrmstartQp = 50;
		pIc[i].num_frames = -1;
		pIc[i].framerate = 25;
		pIc[i].maxQp = 42;
		pIc[i].minQp = 26;
		pIc[i].maxIqp = 42;
		pIc[i].minIqp = 26;
		pIc[i].single_LumaBuf = 1;
		pIc[i].bitstreamBufSize = 2764800;
	}

    enRcMode = (SAMPLE_RC_E) pIc[0].rcMode;

    s32Ret = SAMPLE_COMM_VENC_GetGopAttr(enGopMode, &stGopAttr);
    if (s32Ret != CVI_SUCCESS) {
        printf("[Err]Venc Get GopAttr for %#x!\n", s32Ret);
        return CVI_FAILURE;
    }

	for(i = 0; i < sizeof(VencChn)/sizeof(VENC_CHN); i++) {
		pIc[i].bind_mode = VENC_BIND_VPSS,
		pIc[i].vpssGrp = i,
		pIc[i].vpssChn = 1,
		s32Ret = SAMPLE_COMM_VENC_Start(
				&pIc[i],
				VencChn[i],
				enPayLoad,
				enSize,
				enRcMode,
				u32Profile,
				CVI_FALSE,
				&stGopAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("[Err]Venc%d Start failed for %#x!\n", i, s32Ret);
			return CVI_FAILURE;
		}
	}

    return s32Ret;
}


CVI_S32 CVI_Init_Vpss(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP_ATTR_S stVpssGrpAttr;
	CVI_S32 vpssgrp_width = 1920;
	CVI_S32 vpssgrp_height = 1080;
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssGrpAttr.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	stVpssGrpAttr.u32MaxW = vpssgrp_width;
	stVpssGrpAttr.u32MaxH = vpssgrp_height;
	// only for test here. u8VpssDev should be decided by VPSS_MODE and usage.
	stVpssGrpAttr.u8VpssDev = 0;
	s32Ret = Set_Vpss_config(0,&stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Process Grp0 failed with %d\n", s32Ret);
		return s32Ret;
	}

	#ifdef DUAL_SENSOR
	s32Ret = Set_Vpss_config(1,&stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Process Grp1 failed with %d\n", s32Ret);
		return s32Ret;
	}
	#endif
	set_vpss_aspect(0,0,0,1280,720);

	return s32Ret;
}



void CVI_set_VI_config(SAMPLE_VI_CONFIG_S* stViConfig)
{
	CVI_S32 s32WorkSnsId = 0;
	VI_PIPE ViPipe = 0;
#if (defined CUSTOMIZED_PCBA) || (defined CUSTOMIZED_PCBB)
	SAMPLE_SNS_TYPE_E  enSnsType	    = SOI_F35_MIPI_2M_30FPS_10BIT;
#else
	SAMPLE_SNS_TYPE_E  enSnsType        = SONY_IMX307_MIPI_2M_30FPS_12BIT;
#endif
	WDR_MODE_E	   enWDRMode	    = WDR_MODE_NONE;
	DYNAMIC_RANGE_E    enDynamicRange   = DYNAMIC_RANGE_SDR8;
	PIXEL_FORMAT_E     enPixFormat	    = PIXEL_FORMAT_YUV_PLANAR_420;
	VIDEO_FORMAT_E     enVideoFormat    = VIDEO_FORMAT_LINEAR;
	COMPRESS_MODE_E    enCompressMode   = COMPRESS_MODE_NONE;
	VI_VPSS_MODE_E	   enMastPipeMode   = VI_OFFLINE_VPSS_OFFLINE;

	SAMPLE_COMM_VI_GetSensorInfo(stViConfig);

	stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.enSnsType	     = enSnsType;
	stViConfig->s32WorkingViNum				     = 1;
	stViConfig->as32WorkingViId[0]				     = 0;
	stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.MipiDev	     = 0xFF;
	stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.s32BusId	     = 3;
	stViConfig->astViInfo[s32WorkSnsId].stDevInfo.ViDev	     = 0;
	stViConfig->astViInfo[s32WorkSnsId].stDevInfo.enWDRMode	     = enWDRMode;
	stViConfig->astViInfo[s32WorkSnsId].stPipeInfo.enMastPipeMode = enMastPipeMode;
	stViConfig->astViInfo[s32WorkSnsId].stPipeInfo.aPipe[0]	     = ViPipe;
	stViConfig->astViInfo[s32WorkSnsId].stPipeInfo.aPipe[1]	     = -1;
	stViConfig->astViInfo[s32WorkSnsId].stPipeInfo.aPipe[2]	     = -1;
	stViConfig->astViInfo[s32WorkSnsId].stPipeInfo.aPipe[3]	     = -1;
	stViConfig->astViInfo[s32WorkSnsId].stChnInfo.ViChn	     = 0;
	stViConfig->astViInfo[s32WorkSnsId].stChnInfo.enPixFormat     = enPixFormat;
	stViConfig->astViInfo[s32WorkSnsId].stChnInfo.enDynamicRange  = enDynamicRange;
	stViConfig->astViInfo[s32WorkSnsId].stChnInfo.enVideoFormat   = enVideoFormat;
	stViConfig->astViInfo[s32WorkSnsId].stChnInfo.enCompressMode  = enCompressMode;
	stViConfig->bViRotation = true;
}


int set_Vo_config(SAMPLE_VO_CONFIG_S* stVoConfig)
{
	RECT_S stDefDispRect  = {0, 0, 720, 1280};
	SIZE_S stDefImageSize = {720, 1280};
	CVI_S32 s32Ret = CVI_SUCCESS;
	s32Ret = SAMPLE_COMM_VO_GetDefConfig(stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VO_GetDefConfig failed with %#x\n", s32Ret);
		return s32Ret;
	}

	stVoConfig->VoDev	 = 0;
	stVoConfig->stVoPubAttr.enIntfType  = VO_INTF_MIPI;
	stVoConfig->stVoPubAttr.enIntfSync  = VO_OUTPUT_720x1280_60;
	stVoConfig->stDispRect	 = stDefDispRect;
	stVoConfig->stImageSize	 = stDefImageSize;
	stVoConfig->enPixFormat	 = PIXEL_FORMAT_YUV_PLANAR_420;
	stVoConfig->enVoMode	 = VO_MODE_1MUX;
	return s32Ret;
}

int CVI_Init_Video_Input(SAMPLE_VI_CONFIG_S* stViConfig)
{
	VB_CONFIG_S        stVbConf;
	PIC_SIZE_E         enPicSize;
	CVI_U32            u32BlkSize, u32BlkRotSize, u32BlkRGBSize;
	SIZE_S stSize;
	CVI_S32 s32Ret = CVI_SUCCESS;

	VI_DEV ViDev = 0;
	VI_CHN ViChn = 0;
	CVI_S32 s32WorkSnsId = 0;
	ISP_BIND_ATTR_S stBindAttr;

	VI_DEV_ATTR_S      stViDevAttr;
	VI_CHN_ATTR_S      stChnAttr;
	VI_PIPE_ATTR_S     stPipeAttr;

	/************************************************
	 * step1:  Config VI
	 ************************************************/

	/************************************************
	 * step2:  Get input size
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.enSnsType, &enPicSize);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
		return s32Ret;
	}
	/************************************************
	 * step3:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
#ifdef ENABLE_REFINE_VB
	CVI_U32 u32BlkWidth, u32BlkHeight, u32BlkCnt, u32BlkIdx = 0;
	COMPRESS_MODE_E enCompressMode = COMPRESS_MODE_NONE;//stViConfig->astViInfo[s32WorkSnsId].stChnInfo.enCompressMode;
	PIXEL_FORMAT_E enPixelFormat = SAMPLE_PIXEL_FORMAT;
	do {
		switch (u32BlkIdx) {
			case 0:
				u32BlkWidth 	 = 1920;
				u32BlkHeight	 = 1080;
				enPixelFormat	 = PIXEL_FORMAT_YUV_PLANAR_420;
				u32BlkCnt	 = 12;
				break;
			case 1:
				u32BlkWidth 	 = 720;
				u32BlkHeight	 = 1280;
				enPixelFormat	 = PIXEL_FORMAT_YUV_PLANAR_420;
				u32BlkCnt	 = 12;
				break;
			case 2:
				u32BlkWidth 	 = 608;
				u32BlkHeight	 = 608;
				enPixelFormat	 = PIXEL_FORMAT_RGB_888_PLANAR;
				u32BlkCnt	 = 8;
				break;
			case 3:
				u32BlkWidth 	 = 480;
				u32BlkHeight	 = 640;
				enPixelFormat	 = PIXEL_FORMAT_BGR_888;
				u32BlkCnt	 = 8;
				break;
			case 4:
				u32BlkWidth 	 = 240;
				u32BlkHeight	 = 320;
				enPixelFormat	 = PIXEL_FORMAT_BGR_888;
				u32BlkCnt	 = 8;
				break;
			default:
				break;
		}
		u32BlkSize = COMMON_GetPicBufferSize(u32BlkWidth, u32BlkHeight, enPixelFormat,
						DATA_BITWIDTH_8, enCompressMode, DEFAULT_ALIGN);
		u32BlkRotSize = COMMON_GetPicBufferSize(u32BlkHeight, u32BlkWidth, enPixelFormat,
						DATA_BITWIDTH_8, enCompressMode, DEFAULT_ALIGN);
		u32BlkSize = MAX(u32BlkSize, u32BlkRotSize);
		stVbConf.astCommPool[u32BlkIdx].u32BlkSize	= u32BlkSize;
		stVbConf.astCommPool[u32BlkIdx].u32BlkCnt	= u32BlkCnt;
		stVbConf.astCommPool[u32BlkIdx].enRemapMode = VB_REMAP_MODE_CACHED;
		u32BlkIdx++;
	} while (u32BlkIdx != 5);
	stVbConf.u32MaxPoolCnt = u32BlkIdx;
#else
	stVbConf.u32MaxPoolCnt		= 2;

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height, SAMPLE_PIXEL_FORMAT,
		DATA_BITWIDTH_8, stViConfig->astViInfo[s32WorkSnsId].stChnInfo.enCompressMode, DEFAULT_ALIGN);
	u32BlkRotSize = COMMON_GetPicBufferSize(stSize.u32Height, stSize.u32Width, SAMPLE_PIXEL_FORMAT,
		DATA_BITWIDTH_8, stViConfig->astViInfo[s32WorkSnsId].stChnInfo.enCompressMode, DEFAULT_ALIGN);
	u32BlkSize = MAX(u32BlkSize, u32BlkRotSize);
	u32BlkRGBSize = COMMON_GetPicBufferSize(1080, 1920, PIXEL_FORMAT_RGB_888,
		DATA_BITWIDTH_8, stViConfig->astViInfo[s32WorkSnsId].stChnInfo.enCompressMode, DEFAULT_ALIGN);
		printf("=========u32BlkRGBSize:%d========\n",u32BlkRGBSize);
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 35;
	stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkRGBSize;
	stVbConf.astCommPool[1].u32BlkCnt	= 2;
	stVbConf.astCommPool[1].enRemapMode = VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[0] BlkSize %d\n", u32BlkSize);
#endif
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("system init failed with %#x\n", s32Ret);
		return -1;
	}

	#if 0
	LOG_LEVEL_CONF_S log_conf;
	for (CVI_U8 i = 0; i < CVI_ID_BUTT; ++i) {
		log_conf.enModId = i;
		//if(i == 7) //vpss
		log_conf.s32Level = 7;
		CVI_LOG_SetLevelConf(&log_conf);
	}
	#endif
	/************************************************
	 * step4:  Init VI ISP
	 ************************************************/
#if USE_USER_SEN_DRIVER
	s32Ret = SAMPLE_COMM_VI_StartSensor(stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("system start sensor failed with %#x\n", s32Ret);
		return s32Ret;
	}
#endif
	SAMPLE_COMM_VI_StartDev(&stViConfig->astViInfo[ViDev]);
#if USE_USER_SEN_DRIVER
	SAMPLE_COMM_VI_StartMIPI(stViConfig);
#endif
	memset(&stPipeAttr, 0, sizeof(VI_PIPE_ATTR_S));
	stPipeAttr.bYuvSkip = CVI_FALSE;
	stPipeAttr.u32MaxW = stSize.u32Width;
	stPipeAttr.u32MaxH = stSize.u32Height;
	stPipeAttr.enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stPipeAttr.enBitWidth = DATA_BITWIDTH_12;
	stPipeAttr.stFrameRate.s32SrcFrameRate = -1;
	stPipeAttr.stFrameRate.s32DstFrameRate = -1;
	stPipeAttr.bNrEn = CVI_TRUE;
	stPipeAttr.enCompressMode = stViConfig->astViInfo[0].stChnInfo.enCompressMode;

	CVI_S32 i = 0, j = 0;
	CVI_S32 s32DevNum;
	for (i = 0; i < stViConfig->s32WorkingViNum; i++) {
		SAMPLE_VI_INFO_S *pstViInfo = NULL;

		s32DevNum  = stViConfig->as32WorkingViId[i];
		pstViInfo = &stViConfig->astViInfo[s32DevNum];
		for (j = 0; j < WDR_MAX_PIPE_NUM; j++) {
			if (pstViInfo->stPipeInfo.aPipe[j] >= 0 && pstViInfo->stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
				ViPipe = pstViInfo->stPipeInfo.aPipe[j];
				s32Ret = CVI_VI_CreatePipe(ViPipe, &stPipeAttr);
				if (s32Ret != CVI_SUCCESS) {
					CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_CreatePipe failed with %#x!\n", s32Ret);
					return s32Ret;
				}

				s32Ret = CVI_VI_StartPipe(ViPipe);
				if (s32Ret != CVI_SUCCESS) {
					CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_StartPipe failed with %#x!\n", s32Ret);
					return s32Ret;
				}

				s32Ret = CVI_VI_GetPipeAttr(ViPipe, &stPipeAttr);
				if (s32Ret != CVI_SUCCESS) {
					CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetPipeAttr failed with %#x!\n", s32Ret);
					return s32Ret;
				}
			}
		}
	}
	s32Ret = SAMPLE_COMM_VI_CreateIsp(stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "VI_CreateIsp failed with %#x!\n", s32Ret);
		return s32Ret;
	}
	SAMPLE_COMM_VI_StartViChn(stViConfig);
	if(stViConfig->bViRotation)
	{
		ROTATION_E enRotation;
		#if defined(WEDB_0001C_IMX307)
			enRotation = ROTATION_270;
		#else
			enRotation = ROTATION_90;
		#endif

		s32Ret = CVI_VI_SetChnRotation(0, 0, enRotation);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VI_SetChnRotation failed with %d\n", s32Ret);
			return s32Ret;
		}
		s32Ret = CVI_VI_SetChnRotation(0, 1, enRotation);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VI_SetChnRotation failed with %d\n", s32Ret);
			return s32Ret;
		}
	}
	return s32Ret;
}

int CVI_Init_Video_Output(SAMPLE_VO_CONFIG_S* stVoConfig)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = SAMPLE_COMM_VO_StartVO(stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StartVO failed with %#x\n", s32Ret);
	}
	SAMPLE_PRT("SAMPLE_COMM_VO_StartVO done\n");
	CVI_VO_SetChnRotation(stVoConfig->VoDev, 0, ROTATION_90);
	return s32Ret;
}

CVI_S32 CVI_Video_Release()
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	SAMPLE_COMM_ISP_Stop(0);

	SAMPLE_COMM_VI_DestroyVi(&stViConfig);

	SAMPLE_COMM_VENC_Stop(0);

	//unbind vpss grp0 chn2->vo
	SAMPLE_COMM_VPSS_UnBind_VO(0, 0, 0, 0);
	//unbind vi ch0->vpss grp0
	SAMPLE_COMM_VI_UnBind_VPSS(0, 0, 0);
	//unbind vi ch1->vpss grp1
	SAMPLE_COMM_VI_UnBind_VPSS(0, 1, 1);

	//stop vpss. notice:vpss channel should be the same with Set_Vpss_config()
	for (int i = 0; i < 3; i++)
		abChnEnable[i] = true;
	s32Ret = SAMPLE_COMM_VPSS_Stop(0, abChnEnable);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VPSS_Stop 0 failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}
	for (int i = 2; i < 4; i++)
		abChnEnable[i] = false;
	s32Ret = SAMPLE_COMM_VPSS_Stop(1, abChnEnable);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VPSS_Stop 1 failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}
	abChnEnable[0] = true;
	s32Ret = SAMPLE_COMM_VPSS_Stop(2, abChnEnable);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VPSS_Stop 2 failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	// panel_off();
	SAMPLE_COMM_VO_Exit();
	CVI_SYS_Exit();
	CVI_VB_ExitModCommPool(VB_UID_VDEC);
	CVI_VB_Exit();

	return s32Ret;
}


CVI_S32 CVI_Video_Init(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 VoLayer = 0;
	CVI_U32 VoChn = 0;
	//start sys & isp(vi)
	CVI_Set_VI_Config_Ini(&stViConfig);

	s32Ret = CVI_Init_Video_Input(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Input failed with %d\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_Init_Vpss();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Process Grp1 failed with %d\n", s32Ret);
		return s32Ret;
	}

    // s32Ret = InitEnc();
	// if (s32Ret != CVI_SUCCESS) {
	// 	SAMPLE_PRT("InitEnc  failed with %d\n", s32Ret);
	// 	return s32Ret;
	// }

	//vo config & start vo
	s32Ret = set_Vo_config(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("set_Vo_config failed with %d\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_Init_Video_Output(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Output failed with %d\n", s32Ret);
		return s32Ret;
	}

	//CVI_VO_HideChn(VoLayer,VoChn);

	return s32Ret;
}

void set_vpss_aspect(CVI_S32 vpsschn,CVI_S32 x,CVI_S32 y, CVI_S32 width, CVI_S32 height)
{
	VPSS_CHN_ATTR_S stChnAttr;
	CVI_VPSS_GetChnAttr(0, vpsschn, &stChnAttr);
	stChnAttr.stAspectRatio.stVideoRect.s32X = x;
	stChnAttr.stAspectRatio.stVideoRect.s32Y = y;
	stChnAttr.stAspectRatio.stVideoRect.u32Width = width;
	stChnAttr.stAspectRatio.stVideoRect.u32Height = height;
	CVI_VPSS_SetChnAttr(0, vpsschn, &stChnAttr);
}

void set_ir_vpss_aspect(CVI_S32 vpsschn,CVI_S32 x,CVI_S32 y, CVI_S32 width, CVI_S32 height, PIXEL_FORMAT_E pixelFormat)
{
	VPSS_CHN_ATTR_S stChnAttr;
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_VPSS_DisableChn(1, vpsschn);

	CVI_VPSS_GetChnAttr(1, vpsschn, &stChnAttr);
	stChnAttr.stAspectRatio.stVideoRect.s32X = x;
	stChnAttr.stAspectRatio.stVideoRect.s32Y = y;
	stChnAttr.stAspectRatio.stVideoRect.u32Width = width;
	stChnAttr.stAspectRatio.stVideoRect.u32Height = height;

	stChnAttr.u32Width                    = width;
	stChnAttr.u32Height                   = height;
	stChnAttr.enVideoFormat               = VIDEO_FORMAT_LINEAR;
	stChnAttr.enPixelFormat               = pixelFormat;
	stChnAttr.stAspectRatio.enMode        = ASPECT_RATIO_MANUAL;


	CVI_VPSS_SetChnAttr(1, vpsschn, &stChnAttr);
	s32Ret = CVI_VPSS_EnableChn(1, vpsschn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_EnableChn is fail\n");
	}
}

bool get_freeze_status()
{
	return freeze_status;
}

void set_freeze_status(bool state)
{
	pthread_mutex_lock(&mutex_video);
	freeze_status = state;
	pthread_mutex_unlock(&mutex_video);
}
