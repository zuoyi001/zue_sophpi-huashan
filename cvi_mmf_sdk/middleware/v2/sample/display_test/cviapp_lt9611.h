/**
 * File Name: cviapp_lt9611.h
 *
 * Version: V1.0
 *
 * Brief:
 *
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * Description:
 *
 * History			:
 * =====================================================================================
   1.Date 			:	2021/01/21
   Author 			:	mason.zou
   Modification		:	Created file
 * ====================================================================================*/

#ifndef CVIAPP_LT9611_H_
#define CVIAPP_LT9611_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


typedef enum
{
    E_CVIAPP_LT9611_VIDEO_1280x720_60HZ,
    E_CVIAPP_LT9611_VIDEO_1280x1024_60HZ,
    E_CVIAPP_LT9611_VIDEO_1600x1200_60HZ,
    E_CVIAPP_LT9611_VIDEO_1920x1080_60HZ,
    E_CVIAPP_LT9611_VIDEO_MAX
} CVIAPP_LT9611_VIDEO_FORMAT_E;

void CVIAPP_Lt9611Init(CVIAPP_LT9611_VIDEO_FORMAT_E enVideoFormat);

void CVIAPP_Lt9611SetVideoFormat(CVIAPP_LT9611_VIDEO_FORMAT_E enVideoFormat);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
