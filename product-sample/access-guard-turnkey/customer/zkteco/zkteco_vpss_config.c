#include <fcntl.h>
#include "app_utils.h"
#include "app_video.h"

void APP_SetVpssAspect(CVI_S32 vpsschn,CVI_S32 x,CVI_S32 y, CVI_S32 width, CVI_S32 height) 
{
	VPSS_CHN_ATTR_S stChnAttr;
	CVI_VPSS_GetChnAttr(0, vpsschn, &stChnAttr);
	stChnAttr.stAspectRatio.stVideoRect.s32X = x;
	stChnAttr.stAspectRatio.stVideoRect.s32Y = y;
	stChnAttr.stAspectRatio.stVideoRect.u32Width = width;
	stChnAttr.stAspectRatio.stVideoRect.u32Height = height;
	CVI_VPSS_SetChnAttr(0, vpsschn, &stChnAttr);
}

CVI_S32 APP_SetVpssConfig(VPSS_GRP VpssGrp,VPSS_GRP_ATTR_S* stVpssGrpAttr)
{
	VPSS_CHN VpssChn = VPSS_CHN0;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = { 0 };
	VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	CVI_S32 s32Ret = CVI_SUCCESS;

	if(VpssGrp == 0)
	{
		//channel0 for retinaface
		abChnEnable[VpssChn] = CVI_TRUE;
		astVpssChnAttr[VpssChn].u32Width = 576;
		astVpssChnAttr[VpssChn].u32Height = 1024;
		astVpssChnAttr[VpssChn].enVideoFormat = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat = PIXEL_FORMAT_BGR_888;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth = 1;
		astVpssChnAttr[VpssChn].bMirror = SUPPORT_MIRROR;
		astVpssChnAttr[VpssChn].bFlip = SUPPORT_FLIP;

		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor  = COLOR_RGB_BLACK;

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

		astVpssChnAttr[VpssChn].bMirror                     = SUPPORT_MIRROR;
		astVpssChnAttr[VpssChn].bFlip                       = SUPPORT_FLIP;
		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode 		= ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor= CVI_TRUE;
		astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor  	= COLOR_RGB_BLACK;

		//channel2 for vo & capture
		abChnEnable[2] = CVI_TRUE;
		VpssChn        = VPSS_CHN2;
		astVpssChnAttr[VpssChn].u32Width                    = stVoPubAttr.stSyncInfo.u16Hact;
		astVpssChnAttr[VpssChn].u32Height                   = stVoPubAttr.stSyncInfo.u16Vact;
		astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_YUV_PLANAR_420;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth                    = 1;
		astVpssChnAttr[VpssChn].bMirror                     = SUPPORT_MIRROR;
		astVpssChnAttr[VpssChn].bFlip                       = SUPPORT_FLIP;
		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode 		= ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor= CVI_TRUE;
		astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor  	= COLOR_RGB_BLACK;
	}
	else if(VpssGrp == 1)
	{
		//grp1_channel0 for ir
		abChnEnable[0] = CVI_TRUE;
		VpssChn        = VPSS_CHN0;
		astVpssChnAttr[VpssChn].u32Width                    = 576;
		astVpssChnAttr[VpssChn].u32Height                   = 1024;
		astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_BGR_888;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth                    = 1;
		astVpssChnAttr[VpssChn].bMirror                     = SUPPORT_MIRROR;
		astVpssChnAttr[VpssChn].bFlip                       = SUPPORT_FLIP;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;

		//grp1_channel1 for ir
		abChnEnable[1] = CVI_TRUE;
		VpssChn        = VPSS_CHN1;
		astVpssChnAttr[VpssChn].u32Width                    = 1280;
		astVpssChnAttr[VpssChn].u32Height                   = 720;
		astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_YUV_PLANAR_420;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth                    = 1;
		astVpssChnAttr[VpssChn].bMirror                     = SUPPORT_MIRROR;
		astVpssChnAttr[VpssChn].bFlip                       = SUPPORT_FLIP;
		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor  = COLOR_RGB_BLACK;

		//channel2 for vo & capture
		abChnEnable[2] = CVI_TRUE;
		VpssChn        = VPSS_CHN2;
		astVpssChnAttr[VpssChn].u32Width                    = stVoPubAttr.stSyncInfo.u16Hact;
		astVpssChnAttr[VpssChn].u32Height                   = stVoPubAttr.stSyncInfo.u16Vact;
		astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_YUV_PLANAR_420;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth                    = 1;
		astVpssChnAttr[VpssChn].bMirror                     = SUPPORT_MIRROR;
		astVpssChnAttr[VpssChn].bFlip                       = SUPPORT_FLIP;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_MANUAL;
		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode 		= ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor= CVI_TRUE;
		astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor  	= COLOR_RGB_BLACK;
	}
	else if(VpssGrp == 2)//group 2 for img face register
	{
		abChnEnable[0] = CVI_TRUE;
		VpssChn = VPSS_CHN0;
		astVpssChnAttr[VpssChn].u32Width = 1280;
		astVpssChnAttr[VpssChn].u32Height = 720;
		astVpssChnAttr[VpssChn].enVideoFormat = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat = PIXEL_FORMAT_BGR_888;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth = 1;
		astVpssChnAttr[VpssChn].bMirror = SUPPORT_MIRROR;
		astVpssChnAttr[VpssChn].bFlip = SUPPORT_FLIP;

		astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor  = COLOR_RGB_BLACK;
	}
	else if(VpssGrp == 3)//grp3 for factory mode
	{
		abChnEnable[0] = CVI_TRUE;
		VpssChn        = VPSS_CHN0;
		astVpssChnAttr[VpssChn].u32Width                    = 720;
		astVpssChnAttr[VpssChn].u32Height                   = 1280;
		astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_YUV_PLANAR_420;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth                    = 1;
		astVpssChnAttr[VpssChn].bMirror                     = SUPPORT_MIRROR;
		astVpssChnAttr[VpssChn].bFlip                       = SUPPORT_FLIP;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_MANUAL;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_FALSE;
		astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32X = 360;
		astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32Y = 300;
		astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Width = 360;
		astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Height = 640;
		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;
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

CVI_S32 APP_InitVpss(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP_ATTR_S stVpssGrpAttr;
	CVI_S32 vpssgrp_width = 1080;
	CVI_S32 vpssgrp_height = 1920;
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssGrpAttr.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	stVpssGrpAttr.u32MaxW = vpssgrp_width;
	stVpssGrpAttr.u32MaxH = vpssgrp_height;
	// only for test here. u8VpssDev should be decided by VPSS_MODE and usage.
	stVpssGrpAttr.u8VpssDev = 0;
	VPSS_CROP_INFO_S stCropInfo;
	s32Ret = APP_SetVpssConfig(0,&stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Process Grp0 failed with %d\n", s32Ret);
		return s32Ret;
	}
  
	stCropInfo.bEnable = CVI_TRUE;
	stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	stCropInfo.stCropRect.s32X = 0;
	stCropInfo.stCropRect.s32Y = 96;
	stCropInfo.stCropRect.u32Width = 1080;
	stCropInfo.stCropRect.u32Height = 1728;
	CVI_VPSS_SetGrpCrop(0, &stCropInfo);
	#if defined(CONFIG_DUAL_SENSOR_SUPPORT)
	s32Ret = APP_SetVpssConfig(1,&stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Process Grp1 failed with %d\n", s32Ret);
		return s32Ret;
	}
	CVI_VPSS_SetGrpCrop(1, &stCropInfo);
	#endif

	stVpssGrpAttr.enPixelFormat = PIXEL_FORMAT_BGR_888;
	s32Ret = APP_SetVpssConfig(2,&stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Process Grp2 failed with %d\n", s32Ret);
		return s32Ret;
	}
	APP_SetVpssAspect(2,0,0,800,1280);

	stVpssGrpAttr.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	stVpssGrpAttr.u32MaxW = 360;
	stVpssGrpAttr.u32MaxH = 640;
	s32Ret = APP_SetVpssConfig(3,&stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Process Grp2 failed with %d\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

int APP_VpssGetFrame(APP_VIDEO_FRAME_E id, VIDEO_FRAME_INFO_S *stVFrame, int timeout)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch(id) {
		case APP_VIDEO_FRAME_FD_RGB:
			s32Ret = CVI_VPSS_GetChnFrame(0, 0, stVFrame, 1000);
			break;
		case APP_VIDEO_FRAME_FD_IR:
			s32Ret = CVI_VPSS_GetChnFrame(1, 0, stVFrame, 1000);
			break;
		default:
			s32Ret = -1;
			break;
	}

	return s32Ret;
}

int APP_VpssReleaseFrame(APP_VIDEO_FRAME_E id, VIDEO_FRAME_INFO_S *stVFrame)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	switch(id) {
		case APP_VIDEO_FRAME_FD_RGB:
			s32Ret = CVI_VPSS_ReleaseChnFrame(0, 0, stVFrame);
			break;
		case APP_VIDEO_FRAME_FD_IR:
			s32Ret = CVI_VPSS_ReleaseChnFrame(1, 0, stVFrame);
			break;
		default:
			s32Ret = -1;
			break;
	}

	return s32Ret;
}

int APP_VpssBindVo(void)
{
	return SAMPLE_COMM_VPSS_Bind_VO(0, 2, 0, 0); //grp0, chn2, voLayer0, vochn0.
}

int APP_VpssUnBindVo(void)
{
	return SAMPLE_COMM_VPSS_UnBind_VO(0, 2, 0, 0);
}