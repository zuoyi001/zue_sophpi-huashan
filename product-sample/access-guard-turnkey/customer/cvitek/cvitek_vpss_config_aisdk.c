#include <fcntl.h>
#include "autoconf.h"

#include "cvi_buffer.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_comm_isp.h"
#include <linux/cvi_comm_vpss.h>
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vi.h"
#include "cvi_isp.h"
#include "cvi_ae.h"

#include "sample_comm.h"
#include "app_video.h"


#if defined(CONFIG_VPSS_VIDEO_FLIP_SUPPORT)
#define SUPPORT_FLIP CVI_TRUE
#else
#define SUPPORT_FLIP CVI_FALSE
#endif

#if defined(CONFIG_VPSS_VIDEO_MIRROR_SUPPORT)
#define SUPPORT_MIRROR CVI_TRUE
#else
#define SUPPORT_MIRROR CVI_FALSE
#endif

extern VO_PUB_ATTR_S stVoPubAttr;

void APP_SetVpssAspect(CVI_S32 vpsschn,CVI_S32 x,CVI_S32 y, CVI_S32 width, CVI_S32 height)
{
	VPSS_CHN_ATTR_S stChnAttr;
	CVI_VPSS_GetChnAttr(5, vpsschn, &stChnAttr);
	stChnAttr.stAspectRatio.stVideoRect.s32X = x;
	stChnAttr.stAspectRatio.stVideoRect.s32Y = y;
	stChnAttr.stAspectRatio.stVideoRect.u32Width = width;
	stChnAttr.stAspectRatio.stVideoRect.u32Height = height;
	CVI_VPSS_SetChnAttr(5, vpsschn, &stChnAttr);
}

CVI_S32 APP_SetVpssConfig(VPSS_GRP VpssGrp,VPSS_GRP_ATTR_S* stVpssGrpAttr)
{
	VPSS_CHN VpssChn = VPSS_CHN0;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = { 0 };
	VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	CVI_S32 s32Ret = CVI_SUCCESS;

	if(VpssGrp == 1)
	{
	//channel0 for stLivenessRGBFrame
		abChnEnable[VpssChn] = CVI_TRUE;
		astVpssChnAttr[VpssChn].u32Width                    = 270;
		astVpssChnAttr[VpssChn].u32Height                   = 480;
		astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_BGR_888;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth                    = 1;
		astVpssChnAttr[VpssChn].bMirror = SUPPORT_MIRROR;
		astVpssChnAttr[VpssChn].bFlip = SUPPORT_FLIP;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[VpssChn].stNormalize.bEnable = CVI_FALSE;

	//channel1 for rtsp
		abChnEnable[1] = CVI_TRUE;
		VpssChn        = VPSS_CHN1;
		astVpssChnAttr[VpssChn].u32Width                    = 1280;
		astVpssChnAttr[VpssChn].u32Height                   = 720;
		astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat               = stVpssGrpAttr->enPixelFormat;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth                    = 1;

		astVpssChnAttr[VpssChn].bMirror                     = SUPPORT_MIRROR;
		astVpssChnAttr[VpssChn].bFlip                       = SUPPORT_FLIP;
		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor  = COLOR_RGB_BLACK;

	}
	else if(VpssGrp == 2)
	{
		//grp2_channel0 for ir
		abChnEnable[0] = CVI_TRUE;
		VpssChn        = VPSS_CHN0;
		astVpssChnAttr[VpssChn].u32Width                    = 270;
		astVpssChnAttr[VpssChn].u32Height                   = 480;
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

		//grp2_channel1 for ir
		abChnEnable[1] = CVI_TRUE;
		VpssChn        = VPSS_CHN1;
		astVpssChnAttr[VpssChn].u32Width                    = 1280;
		astVpssChnAttr[VpssChn].u32Height                   = 720;
		astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat               = stVpssGrpAttr->enPixelFormat;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth                    = 1;
		astVpssChnAttr[VpssChn].bMirror                     = SUPPORT_MIRROR;
		astVpssChnAttr[VpssChn].bFlip                       = SUPPORT_FLIP;
		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor  = COLOR_RGB_BLACK;
	}
	else if(VpssGrp == 3)//group 3 for img face register
	{
		abChnEnable[0] = CVI_TRUE;
		VpssChn = VPSS_CHN0;
		astVpssChnAttr[VpssChn].u32Width = 608;
		astVpssChnAttr[VpssChn].u32Height = 608;
		astVpssChnAttr[VpssChn].enVideoFormat = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat = PIXEL_FORMAT_RGB_888_PLANAR;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth = 1;
		astVpssChnAttr[VpssChn].bMirror = SUPPORT_MIRROR;
		astVpssChnAttr[VpssChn].bFlip = SUPPORT_FLIP;

		astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor  = COLOR_RGB_BLACK;
		astVpssChnAttr[VpssChn].stNormalize.bEnable = CVI_TRUE;
		astVpssChnAttr[VpssChn].stNormalize.factor[0] = (128/ 255.001236);
		astVpssChnAttr[VpssChn].stNormalize.factor[1] = (128/ 255.001236);
		astVpssChnAttr[VpssChn].stNormalize.factor[2] = (128 / 255.001236);
		astVpssChnAttr[VpssChn].stNormalize.mean[0]         = 0;
		astVpssChnAttr[VpssChn].stNormalize.mean[1]         = 0;
		astVpssChnAttr[VpssChn].stNormalize.mean[2]         = 0;
		astVpssChnAttr[VpssChn].stNormalize.rounding = VPSS_ROUNDING_TO_EVEN;


		abChnEnable[1] = CVI_TRUE;
		VpssChn        = VPSS_CHN1;
		astVpssChnAttr[VpssChn].u32Width                    = 480;
		astVpssChnAttr[VpssChn].u32Height                   = 640;
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

	}
	else if(VpssGrp == 4)//grp4 for factory mode
	{
		//IR image for vo & capture
		abChnEnable[0] = CVI_TRUE;
		VpssChn        = VPSS_CHN0;
		astVpssChnAttr[VpssChn].u32Width                    = 720;
		astVpssChnAttr[VpssChn].u32Height                   = 1280;
		astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat               = stVpssGrpAttr->enPixelFormat;//PIXEL_FORMAT_RGB_888_PLANAR;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth                    = 1;
		astVpssChnAttr[VpssChn].bMirror                     = SUPPORT_MIRROR;
		astVpssChnAttr[VpssChn].bFlip                       = SUPPORT_FLIP;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_AUTO;
		//astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32X 		= 0;
        //astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32Y 		= 0;
        //astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Width  = 720;
        //astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Height = 1280;
		astVpssChnAttr[VpssChn].stNormalize.bEnable          		= CVI_FALSE;
        astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor 		= CVI_FALSE;

		//channel1 for retinaface
		abChnEnable[1] = CVI_TRUE;
		VpssChn        = VPSS_CHN1;
		astVpssChnAttr[VpssChn].u32Width = 608;
		astVpssChnAttr[VpssChn].u32Height = 608;
		astVpssChnAttr[VpssChn].enVideoFormat = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat = PIXEL_FORMAT_RGB_888;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth = 1;
		astVpssChnAttr[VpssChn].bMirror = SUPPORT_MIRROR;
		astVpssChnAttr[VpssChn].bFlip = SUPPORT_FLIP;

		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor  = COLOR_RGB_BLACK;
	}
	else if(VpssGrp == 5)
	{
		//for vo & capture
		abChnEnable[0] = CVI_TRUE;
		VpssChn        = VPSS_CHN0;
		astVpssChnAttr[VpssChn].u32Width                    = 720;
		astVpssChnAttr[VpssChn].u32Height                   = 1280;
		astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat               = stVpssGrpAttr->enPixelFormat;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth                    = 1;
		astVpssChnAttr[VpssChn].bMirror                     = SUPPORT_MIRROR;
		astVpssChnAttr[VpssChn].bFlip                       = SUPPORT_FLIP;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_AUTO;
		//astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32X 		= 0;
        //astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32Y 		= 0;
        //astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Width  = 720;
        //astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Height = 1280;
		astVpssChnAttr[VpssChn].stNormalize.bEnable          		= CVI_FALSE;
        astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor 		= CVI_FALSE;

		//channel1 for retinaface
		abChnEnable[1] = CVI_TRUE;
		VpssChn        = VPSS_CHN1;
		astVpssChnAttr[VpssChn].u32Width = 608;
		astVpssChnAttr[VpssChn].u32Height = 608;
		astVpssChnAttr[VpssChn].enVideoFormat = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat = PIXEL_FORMAT_RGB_888;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth = 1;
		astVpssChnAttr[VpssChn].bMirror = SUPPORT_MIRROR;
		astVpssChnAttr[VpssChn].bFlip = SUPPORT_FLIP;

		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_AUTO;
		astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor  = COLOR_RGB_WHITE;

	}

	CVI_SYS_SetVPSSMode(VPSS_MODE_DUAL);

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

	if (VpssGrp == 1) {
		s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 1, VpssGrp);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
 	}

	if (VpssGrp == 2) {
		s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 0, VpssGrp);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	if (VpssGrp == 5) {
		s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 1, VpssGrp);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	if (VpssGrp == 4) {
		s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 0, VpssGrp);
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
	CVI_U32 chip = 0;
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
	CVI_SYS_GetChipId(&chip);
	if (!IS_CHIP_CV183X(chip))
		stVpssGrpAttr.enPixelFormat = PIXEL_FORMAT_NV21;
	else
		stVpssGrpAttr.enPixelFormat = PIXEL_FORMAT_NV21;
	stVpssGrpAttr.u32MaxW = vpssgrp_width;
	stVpssGrpAttr.u32MaxH = vpssgrp_height;
	// only for test here. u8VpssDev should be decided by VPSS_MODE and usage.
	stVpssGrpAttr.u8VpssDev = 1;
	s32Ret = APP_SetVpssConfig(1,&stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Process Grp1 failed with %d\n", s32Ret);
		return s32Ret;
	}

	s32Ret = APP_SetVpssConfig(5,&stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Process Grp5 failed with %d\n", s32Ret);
		return s32Ret;
	}

	s32Ret = APP_SetVpssConfig(4,&stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Process Grp4 failed with %d\n", s32Ret);
		return s32Ret;
	}

	//#ifdef DUAL_SENSOR
	s32Ret = APP_SetVpssConfig(2,&stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Process Grp2 failed with %d\n", s32Ret);
		return s32Ret;
	}
	//#endif

	//set_vpss_aspect(0,0,0,720,1280);
#if 0
	stVpssGrpAttr.enPixelFormat = PIXEL_FORMAT_BGR_888;
	s32Ret = Set_Vpss_config(3,&stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Process Grp3 failed with %d\n", s32Ret);
		return s32Ret;
	}

	stVpssGrpAttr.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	stVpssGrpAttr.u32MaxW = 360;
	stVpssGrpAttr.u32MaxH = 640;
	s32Ret = Set_Vpss_config(4,&stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Process Grp4 failed with %d\n", s32Ret);
		return s32Ret;
	}
#endif
	return s32Ret;
}


int APP_VpssGetFrame(APP_VIDEO_FRAME_E id, VIDEO_FRAME_INFO_S *stVFrame, int timeout)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	//printf("APP_VpssGetFrame id = %d, [%p]\n", id, stVFrame);
	switch(id) {
		case APP_VIDEO_FRAME_FD_RGB:
			s32Ret = CVI_VPSS_GetChnFrame(5, 1, stVFrame, timeout);
			break;
		case APP_VIDEO_FRAME_FD_IR:
			s32Ret = CVI_VPSS_GetChnFrame(2, 0, stVFrame, timeout);
			break;
		case APP_VIDEO_FRAME_LIVENESS_RGB:
			s32Ret = CVI_VPSS_GetChnFrame(1, 0, stVFrame, timeout);
			break;
		case APP_VIDEO_FRAME_LIVENESS_IR:
			s32Ret = -1;
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
	//printf("APP_VpssReleaseFrame id = %d, [%p]\n", id, stVFrame);

	switch(id) {
		case APP_VIDEO_FRAME_FD_RGB:
			s32Ret = CVI_VPSS_ReleaseChnFrame(5, 1, stVFrame);
			break;
		case APP_VIDEO_FRAME_FD_IR:
			s32Ret = CVI_VPSS_ReleaseChnFrame(2, 0, stVFrame);
			break;
		case APP_VIDEO_FRAME_LIVENESS_RGB:
			s32Ret = CVI_VPSS_ReleaseChnFrame(1, 0, stVFrame);
			break;
		case APP_VIDEO_FRAME_LIVENESS_IR:
			s32Ret = -1;
			break;
		default:
			s32Ret = -1;
			break;
	}

	return s32Ret;
}


int APP_VpssBindVo(void)
{
	return SAMPLE_COMM_VPSS_Bind_VO(5, 0, 0, 0); //grp5, chn0, voLayer0, vochn0.
}

int APP_VpssUnBindVo(void)
{
	return SAMPLE_COMM_VPSS_UnBind_VO(5, 0, 0, 0);
}