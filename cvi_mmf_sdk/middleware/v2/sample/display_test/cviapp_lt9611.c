/**
 * File Name: cviapp_lt9611.c
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

#include "cviapp_lt9611.h"
#include "cvi_mipi_tx.h"
#include "lt9611.h"

void CVIAPP_Lt9611Init(CVIAPP_LT9611_VIDEO_FORMAT_E enVideoFormat)
{
    switch(enVideoFormat)
    {
    case E_CVIAPP_LT9611_VIDEO_1280x720_60HZ:
        cvi_mipi_tx_set(E_CVI_MIPI_TX_VIDEO_1280x720_60HZ);
        lt9611_init(E_LT9611_VIDEO_1280x720_60HZ);
        break;
    case E_CVIAPP_LT9611_VIDEO_1280x1024_60HZ:
        cvi_mipi_tx_set(E_CVI_MIPI_TX_VIDEO_1280x1024_60HZ);
        lt9611_init(E_LT9611_VIDEO_1280x1024_60HZ);
        break;
    case E_CVIAPP_LT9611_VIDEO_1600x1200_60HZ:
        cvi_mipi_tx_set(E_CVI_MIPI_TX_VIDEO_1600x1200_60HZ);
        lt9611_init(E_LT9611_VIDEO_1600x1200_60HZ);
        break;
//    case E_CVIAPP_LT9611_VIDEO_1920x1080_30HZ:
//         cvi_mipi_tx_set(E_CVI_MIPI_TX_VIDEO_1920x1080_30HZ);
//        lt9611_init(E_LT9611_VIDEO_1920x1080_30HZ);
//        break;
    case E_CVIAPP_LT9611_VIDEO_1920x1080_60HZ:
        cvi_mipi_tx_set(E_CVI_MIPI_TX_VIDEO_1920x1080_60HZ);
        lt9611_init(E_LT9611_VIDEO_1920x1080_60HZ);
        break;
    default:
        printf("ERROR: unknown video format");
        break;
    }
}

void CVIAPP_Lt9611SetVideoFormat(CVIAPP_LT9611_VIDEO_FORMAT_E enVideoFormat)
{
    switch(enVideoFormat)
    {
    case E_CVIAPP_LT9611_VIDEO_1280x720_60HZ:
        cvi_mipi_tx_set(E_CVI_MIPI_TX_VIDEO_1280x720_60HZ);
        lt9611_set_video_format(E_LT9611_VIDEO_1280x720_60HZ);
        break;
    case E_CVIAPP_LT9611_VIDEO_1280x1024_60HZ:
        cvi_mipi_tx_set(E_CVI_MIPI_TX_VIDEO_1280x1024_60HZ);
        lt9611_set_video_format(E_LT9611_VIDEO_1280x1024_60HZ);
        break;
    case E_CVIAPP_LT9611_VIDEO_1600x1200_60HZ:
        cvi_mipi_tx_set(E_CVI_MIPI_TX_VIDEO_1600x1200_60HZ);
        lt9611_set_video_format(E_LT9611_VIDEO_1600x1200_60HZ);
        break;
//    case E_CVIAPP_LT9611_VIDEO_1920x1080_30HZ:
//        cvi_mipi_tx_set(E_CVI_MIPI_TX_VIDEO_1920x1080_30HZ);
//        lt9611_set_video_format(E_LT9611_VIDEO_1920x1080_30HZ);
//        break;
    case E_CVIAPP_LT9611_VIDEO_1920x1080_60HZ:
        cvi_mipi_tx_set(E_CVI_MIPI_TX_VIDEO_1920x1080_60HZ);
        lt9611_set_video_format(E_LT9611_VIDEO_1920x1080_60HZ);
        break;
    default:
        printf("ERROR: unknown video format");
        break;
    }
}
