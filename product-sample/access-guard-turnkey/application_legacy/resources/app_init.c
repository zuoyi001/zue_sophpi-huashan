#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>

#include "cvi_hal_face_api.h"
#include "app_init.h"
#include "app_utils.h"

VB_BLK blk;
extern hal_facelib_handle_t facelib_handle;

#define MAX(a,b) (((a)>(b))?(a):(b))

#define USE_USER_SEN_DRIVER 1

#if 0
static void _vpss_stop_unbind_vi(VPSS_GRP VpssGrp, CVI_U8 chns)
{
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = { 0 };
	CVI_S32 s32Ret = CVI_SUCCESS;

	SAMPLE_COMM_VI_UnBind_VPSS(0, 0, VpssGrp);

	abChnEnable[0] = CVI_TRUE;
	if (chns == 2)
		abChnEnable[1] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_PRT("stop vpss group failed. s32Ret: 0x%x !\n", s32Ret);
}
#endif

#if 0
int CliCmdTestFd(int argc,char *argv[])
{
	return 0;
}

int CliCmdDoFaceRegister(int argc,char *argv[])
{
	return 0;
}

int CliCmdRepoManage(int argc,char *argv[]) {
    if (argc == 1) {
        printf("Usage: repo [list/delete [id]/deleteall]\n");
        return 0;
    } else if (argc == 2) {
        if (!strcmp(argv[1], "list")) {
            HAL_FACE_RepoShowList(facelib_handle);
            return 0;
        } else if (!strcmp(argv[1], "deleteall")) {
            HAL_FACE_RepoRemoveAllIdentities(facelib_handle);
        }
    } else if (argc == 3) {
        if (!strcmp(argv[1], "delete")) {
            int face_id = atoi(argv[2]);
            HAL_FACE_RepoRemoveIdentity(facelib_handle, face_id);
        }
    }
    return 0;
}
#endif
#define INIT_V(attr, param, value) {(attr).param = (value); }
#define INIT_A(attr, param, manual, ...) do {\
	typeof((attr).stManual.param) arr[] = { __VA_ARGS__ };\
	(attr).stManual.param = manual;\
	for (CVI_U32 x = 0; x < ISP_AUTO_ISO_STRENGTH_NUM; x++) {\
		(attr).stAuto.param[x] = arr[x];\
	} \
} while (0)

int APP_InitVideoInput(SAMPLE_VI_CONFIG_S* pstViConfig)
{
	VB_CONFIG_S        	stVbConf;
	PIC_SIZE_E         	enPicSize;
	CVI_U32            	u32BlkSize, u32BlkRotSize; 
	SIZE_S stSize;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32WorkSnsId = 0;

	/************************************************
	 * step1:  Config VI
	 ************************************************/

	/************************************************
	 * step2:  Get input size
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.enSnsType, &enPicSize);
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
	COMPRESS_MODE_E enCompressMode = COMPRESS_MODE_NONE;//pstViConfig->astViInfo[s32WorkSnsId].stChnInfo.enCompressMode;
	PIXEL_FORMAT_E enPixelFormat = SAMPLE_PIXEL_FORMAT;
	do {
		switch (u32BlkIdx) {
			case 0:
				u32BlkWidth 	 = 1920;
				u32BlkHeight	 = 1080;
				enPixelFormat	 = SAMPLE_PIXEL_FORMAT;
				u32BlkCnt	 = 12;
				break;
			case 1:
				u32BlkWidth 	 = 720;
				u32BlkHeight	 = 1280;
				enPixelFormat	 = SAMPLE_PIXEL_FORMAT;
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
				u32BlkWidth 	 = 270;
				u32BlkHeight	 = 480;
				enPixelFormat	 = PIXEL_FORMAT_BGR_888;
				u32BlkCnt	 = 8;
				break;
			case 5:
				u32BlkWidth 	 = 576;
				u32BlkHeight	 = 1024;
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
	} while (u32BlkIdx != 6);
	stVbConf.u32MaxPoolCnt = u32BlkIdx;
#else
	stVbConf.u32MaxPoolCnt		= 2;

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height, SAMPLE_PIXEL_FORMAT,
		DATA_BITWIDTH_8, pstViConfig->astViInfo[s32WorkSnsId].stChnInfo.enCompressMode, DEFAULT_ALIGN);
	u32BlkRotSize = COMMON_GetPicBufferSize(stSize.u32Height, stSize.u32Width, SAMPLE_PIXEL_FORMAT,
		DATA_BITWIDTH_8, pstViConfig->astViInfo[s32WorkSnsId].stChnInfo.enCompressMode, DEFAULT_ALIGN);
	u32BlkSize = MAX(u32BlkSize, u32BlkRotSize);
	u32BlkRGBSize = COMMON_GetPicBufferSize(1080, 1920, PIXEL_FORMAT_RGB_888,
		DATA_BITWIDTH_8, pstViConfig->astViInfo[s32WorkSnsId].stChnInfo.enCompressMode, DEFAULT_ALIGN);
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

	s32Ret = SAMPLE_PLAT_VI_INIT(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vi init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}
	
	if (pstViConfig->bViRotation) {
		#if defined(CONFIG_VI_ROTATION_270)
		ROTATION_E	enRotation = ROTATION_270;
		#elif defined(CONFIG_VI_ROTATION_180)
		ROTATION_E	enRotation = ROTATION_180;
		#elif defined(CONFIG_VI_ROTATION_90)
		ROTATION_E	enRotation = ROTATION_90;
		#elif defined(CONFIG_VI_ROTATION_NONE)
		ROTATION_E	enRotation = ROTATION_0;
		#endif
		s32Ret = CVI_VI_SetChnRotation(0, 0, enRotation);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VI_SetChnRotation failed with 0x%x\n", s32Ret);
			return s32Ret;
		}

		if (pstViConfig->s32WorkingViNum == 2) {
			s32Ret = CVI_VI_SetChnRotation(0, 1, enRotation);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VI_SetChnRotation failed with 0x%x\n", s32Ret);
				return s32Ret;
			}
		}
	}
	return s32Ret;
}

int APP_InitFacelib(hal_facelib_config_t *facelib_config)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = HAL_FACE_LibOpen(facelib_config);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_StopPipe failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret =  HAL_FACE_LibRepoOpen(facelib_config,&facelib_handle);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CviFaceLibRepoOpen failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}