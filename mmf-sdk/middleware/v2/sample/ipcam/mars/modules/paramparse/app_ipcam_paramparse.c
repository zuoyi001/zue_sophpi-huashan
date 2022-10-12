#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "minIni.h"
#include "app_ipcam_paramparse.h"
#include "app_ipcam_comm.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define PARAM_CFG_INI "/mnt/data/param_config.ini"
#define PARAM_STRING_LEN               (32)

static char *input_file;

static char ParamCfgFile[64] = "/mnt/data/param_config.ini";
/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

static int _Load_Param_Sys(const char *file, APP_PARAM_SYS_CFG_S *Sys)
{
    CVI_U32 i = 0;
    CVI_U32 vbpoolnum = 0;
    char tmp_section[16] = {0};

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading systerm config ------------------> start \n");

    Sys->vb_pool_num = ini_getl("vb_config", "vb_pool_cnt", 0, file);

    for (i = 0; i < Sys->vb_pool_num; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vb_pool_%d", i);

        Sys->vb_pool[i].bEnable = ini_getl(tmp_section, "bEnable", 1, file);
        if (!Sys->vb_pool[i].bEnable)
            continue;

        Sys->vb_pool[vbpoolnum].width      = ini_getl(tmp_section, "frame_width", 0, file);
        Sys->vb_pool[vbpoolnum].height     = ini_getl(tmp_section, "frame_height", 0, file);
        Sys->vb_pool[vbpoolnum].fmt        = ini_getl(tmp_section, "frame_fmt", 0, file);
        Sys->vb_pool[vbpoolnum].enBitWidth = ini_getl(tmp_section, "data_bitwidth", 0, file);
        Sys->vb_pool[vbpoolnum].enCmpMode  = ini_getl(tmp_section, "compress_mode", 0, file);
        Sys->vb_pool[vbpoolnum].vb_blk_num = ini_getl(tmp_section, "blk_cnt", 0, file);

        APP_PROF_LOG_PRINT(LEVEL_INFO, "vb_pool[%d] w=%4d h=%4d count=%d fmt=%d\n", vbpoolnum, Sys->vb_pool[vbpoolnum].width, 
            Sys->vb_pool[vbpoolnum].height, Sys->vb_pool[vbpoolnum].vb_blk_num, Sys->vb_pool[vbpoolnum].fmt);
        
        vbpoolnum++;
    }

    Sys->vb_pool_num = vbpoolnum;
    Sys->stVIVPSSMode.aenMode[0] = ini_getl("vi_vpss_mode", "enMode", 0, file);
    Sys->stVPSSMode.enMode       = ini_getl("vpss_mode", "enMode", 0, file);
    CVI_U32 dev_cnt = ini_getl("vpss_dev", "dev_cnt", 0, file);
    for (i = 0; i < dev_cnt; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vpss_dev%d", i);
        Sys->stVPSSMode.aenInput[i] = ini_getl(tmp_section, "aenInput", 0, file);
        Sys->stVPSSMode.ViPipe[i]   = ini_getl(tmp_section, "ViPipe", 0, file);
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading systerm config ------------------>done \n\n");

    return CVI_SUCCESS;
}

static int _Load_Param_Vi(const char *file, APP_PARAM_VI_CTX_S *pViIniCfg)
{
    int i = 0;
    long int work_sns_cnt = 0;
    char tmp[16] = {0};
    char tmp_section[16] = {0};

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading vi config ------------------> start \n");

    memset(tmp_section, 0, sizeof(tmp_section));
    snprintf(tmp_section, sizeof(tmp_section), "vi_config");
    work_sns_cnt = ini_getl(tmp_section, "sensor_cnt", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "work_sns_cnt = %ld\n", work_sns_cnt);   
    if(work_sns_cnt <= (long int)(sizeof(pViIniCfg->astSensorCfg) / sizeof(pViIniCfg->astSensorCfg[0]))) {
        pViIniCfg->u32WorkSnsCnt = work_sns_cnt;
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "work_sns_cnt error (%ld)\n", work_sns_cnt);
        return CVI_FAILURE;
    }

    // APP_PROF_LOG_PRINT(LEVEL_INFO, "sensor cfg path = %s\n", DEF_INI_PATH);
    for (i = 0; i< (int)pViIniCfg->u32WorkSnsCnt; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "sensor_config%d", i);

        pViIniCfg->astSensorCfg[i].s32SnsId = i;
        /* framerate here is right ?? */
        pViIniCfg->astSensorCfg[i].s32Framerate = ini_getl(tmp_section, "framerate", 0, file);
        /* used sensor name instead of enum ?? */
        pViIniCfg->astSensorCfg[i].enSnsType = ini_getl(tmp_section, "sns_type", 0, file);
        pViIniCfg->astSensorCfg[i].MipiDev = ini_getl(tmp_section, "mipi_dev", 0, file);
        pViIniCfg->astSensorCfg[i].s32BusId = ini_getl(tmp_section, "bus_id", 0, file);
        pViIniCfg->astSensorCfg[i].s32I2cAddr = ini_getl(tmp_section, "sns_i2c_addr", -1, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "sensor_ID=%d enSnsType=%d MipiDev=%d s32BusId=%d s32I2cAddr=0x%x\n", i, pViIniCfg->astSensorCfg[i].enSnsType, 
            pViIniCfg->astSensorCfg[i].MipiDev, pViIniCfg->astSensorCfg[i].s32BusId, pViIniCfg->astSensorCfg[i].s32I2cAddr);
        for (int j = 0; j < 5; j++) {
            memset(tmp, 0, sizeof(tmp));
            sprintf(tmp, "laneid%d", j);
            pViIniCfg->astSensorCfg[i].as16LaneId[j] = ini_getl(tmp_section, tmp, 0, file);

            memset(tmp, 0, sizeof(tmp));
            sprintf(tmp, "swap%d", j);
            pViIniCfg->astSensorCfg[i].as8PNSwap[j] = ini_getl(tmp_section, tmp, 0, file);
        }
        pViIniCfg->astSensorCfg[i].bMclkEn = ini_getl(tmp_section, "mclk_en", 1, file);
        pViIniCfg->astSensorCfg[i].u8Mclk = ini_getl(tmp_section, "mclk", 0, file);
        pViIniCfg->astSensorCfg[i].u8Orien = ini_getl(tmp_section, "orien", 0, file);
        pViIniCfg->astSensorCfg[i].bHwSync = ini_getl(tmp_section, "hw_sync", 0, file);
        pViIniCfg->astSensorCfg[i].u8UseDualSns = ini_getl(tmp_section, "use_dual_Sns", 0, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "sensor_ID=%d bMclkEn=%d u8Mclk=%d u8Orien=%d bHwSync=%d\n", i, pViIniCfg->astSensorCfg[i].bMclkEn, 
            pViIniCfg->astSensorCfg[i].u8Mclk, pViIniCfg->astSensorCfg[i].u8Orien, pViIniCfg->astSensorCfg[i].bHwSync);
    }

    for (i = 0; i< (int)pViIniCfg->u32WorkSnsCnt; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vi_cfg_dev%d", i);
        pViIniCfg->astDevInfo[i].ViDev = ini_getl(tmp_section, "videv", 0, file);
        pViIniCfg->astDevInfo[i].enWDRMode = ini_getl(tmp_section, "wdrmode", 0, file);

        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vi_cfg_pipe%d", i);
        for (int j = 0; j< WDR_MAX_PIPE_NUM; j++) {
            memset(tmp, 0, sizeof(tmp));
            sprintf(tmp, "apipe%d", j);
            pViIniCfg->astPipeInfo[i].aPipe[j] = ini_getl(tmp_section, tmp, 0, file);
        }

        pViIniCfg->astPipeInfo[i].enMastPipeMode = ini_getl(tmp_section, "pipe_mode", 0, file);

        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vi_cfg_chn%d", i);
        pViIniCfg->astChnInfo[i].s32ChnId = i;
        pViIniCfg->astChnInfo[i].u32Width = ini_getl(tmp_section, "width", 0, file);
        pViIniCfg->astChnInfo[i].u32Height = ini_getl(tmp_section, "height", 0, file);
        pViIniCfg->astChnInfo[i].f32Fps = ini_getl(tmp_section, "fps", 0, file);
        pViIniCfg->astChnInfo[i].enPixFormat = ini_getl(tmp_section, "pixFormat", 0, file);
        pViIniCfg->astChnInfo[i].enDynamicRange = ini_getl(tmp_section, "dynamic_range", 0, file);
        pViIniCfg->astChnInfo[i].enVideoFormat = ini_getl(tmp_section, "video_format", 0, file);
        pViIniCfg->astChnInfo[i].enCompressMode = ini_getl(tmp_section, "compress_mode", 0, file);

        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vi_cfg_isp%d", i);
        pViIniCfg->astIspCfg[i].bAfFliter = ini_getl(tmp_section, "af_filter", 0, file);
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading vi config ------------------> done \n\n");
    return CVI_SUCCESS;
}


static int _Load_Param_Vpss(const char *file, APP_PARAM_VPSS_CFG_T *Vpss)
{
    CVI_U32 grp_idx = 0;
    CVI_U32 chn_idx = 0;
    char tmp_section[16] = {0};

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading vpss config ------------------> start \n");

    Vpss->u32GrpCnt = ini_getl("vpss_config", "vpss_grp", 0, file);

    for (grp_idx = 0; grp_idx < Vpss->u32GrpCnt; grp_idx++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vpssgrp%d", grp_idx);
        Vpss->astVpssGrpCfg[grp_idx].bEnable = ini_getl(tmp_section, "grp_enable", 1, file);
        if (!Vpss->astVpssGrpCfg[grp_idx].bEnable)
            continue;
        Vpss->astVpssGrpCfg[grp_idx].VpssGrp = grp_idx;
        Vpss->astVpssGrpCfg[grp_idx].bBindMode                = ini_getl(tmp_section, "bind_mode", 0, file);
        if (Vpss->astVpssGrpCfg[grp_idx].bBindMode) {
            Vpss->astVpssGrpCfg[grp_idx].astChn[0].enModId    = CVI_ID_VPSS;
            Vpss->astVpssGrpCfg[grp_idx].astChn[0].s32DevId   = ini_getl(tmp_section, "src_dev_id", 0, file);
            Vpss->astVpssGrpCfg[grp_idx].astChn[0].s32ChnId   = ini_getl(tmp_section, "src_chn_id", 0, file);
            Vpss->astVpssGrpCfg[grp_idx].astChn[1].enModId    = CVI_ID_VPSS;
            Vpss->astVpssGrpCfg[grp_idx].astChn[1].s32DevId   = ini_getl(tmp_section, "dst_dev_id", 0, file);
            Vpss->astVpssGrpCfg[grp_idx].astChn[1].s32ChnId   = 0;
        }
        APP_PROF_LOG_PRINT(LEVEL_INFO, "vpss grp_idx=%d\n", grp_idx);
        VPSS_GRP_ATTR_S *pstVpssGrpAttr = &Vpss->astVpssGrpCfg[grp_idx].stVpssGrpAttr;
        pstVpssGrpAttr->enPixelFormat               = ini_getl(tmp_section, "pixel_fmt", 0, file);
        pstVpssGrpAttr->stFrameRate.s32SrcFrameRate = ini_getl(tmp_section, "src_framerate", 0, file);
        pstVpssGrpAttr->stFrameRate.s32DstFrameRate = ini_getl(tmp_section, "dst_framerate", 0, file);
        pstVpssGrpAttr->u8VpssDev                   = ini_getl(tmp_section, "vpss_dev", 0, file);
        pstVpssGrpAttr->u32MaxW                     = ini_getl(tmp_section, "max_w", 0, file);
        pstVpssGrpAttr->u32MaxH                     = ini_getl(tmp_section, "max_h", 0, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "Group_ID_%d Config: pix_fmt=%2d sfr=%2d dfr=%2d Dev=%d W=%4d H=%4d\n", 
            grp_idx, pstVpssGrpAttr->enPixelFormat, pstVpssGrpAttr->stFrameRate.s32SrcFrameRate, pstVpssGrpAttr->stFrameRate.s32DstFrameRate,
            pstVpssGrpAttr->u8VpssDev, pstVpssGrpAttr->u32MaxW, pstVpssGrpAttr->u32MaxH);

        VPSS_CROP_INFO_S *pstVpssGrpCropInfo = &Vpss->astVpssGrpCfg[grp_idx].stVpssGrpCropInfo;
        pstVpssGrpCropInfo->bEnable = ini_getl(tmp_section, "crop_en", 0, file);
        if (pstVpssGrpCropInfo->bEnable){
            pstVpssGrpCropInfo->enCropCoordinate     = ini_getl(tmp_section, "crop_coor", 0, file);
            pstVpssGrpCropInfo->stCropRect.s32X      = ini_getl(tmp_section, "crop_rect_x", 0, file);
            pstVpssGrpCropInfo->stCropRect.s32Y      = ini_getl(tmp_section, "crop_rect_y", 0, file);
            pstVpssGrpCropInfo->stCropRect.u32Width  = ini_getl(tmp_section, "crop_rect_w", 0, file);
            pstVpssGrpCropInfo->stCropRect.u32Height = ini_getl(tmp_section, "crop_rect_h", 0, file);
        }

        CVI_U32 grp_chn_cnt = ini_getl(tmp_section, "chn_cnt", 0, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "Group_ID_%d channel count = %d\n", grp_idx, grp_chn_cnt);
        /* load vpss group channel config */
        for (chn_idx = 0; chn_idx < grp_chn_cnt; chn_idx++) {
            memset(tmp_section, 0, sizeof(tmp_section));
            snprintf(tmp_section, sizeof(tmp_section), "vpssgrp%d.chn%d", grp_idx, chn_idx);
            Vpss->astVpssGrpCfg[grp_idx].abChnEnable[chn_idx] = ini_getl(tmp_section, "chn_enable", 0, file);
            if (!Vpss->astVpssGrpCfg[grp_idx].abChnEnable[chn_idx])
                continue;

            VPSS_CHN_ATTR_S *pastVpssChnAttr = &Vpss->astVpssGrpCfg[grp_idx].astVpssChnAttr[chn_idx];
            pastVpssChnAttr->u32Width                    = ini_getl(tmp_section, "width", 0, file);
            pastVpssChnAttr->u32Height                   = ini_getl(tmp_section, "height", 0, file);
            pastVpssChnAttr->enVideoFormat               = ini_getl(tmp_section, "video_fmt", 0, file);
            pastVpssChnAttr->enPixelFormat               = ini_getl(tmp_section, "chn_pixel_fmt", 0, file);
            pastVpssChnAttr->stFrameRate.s32SrcFrameRate = ini_getl(tmp_section, "src_framerate", 0, file);
            pastVpssChnAttr->stFrameRate.s32DstFrameRate = ini_getl(tmp_section, "dst_framerate", 0, file);
            pastVpssChnAttr->u32Depth                    = ini_getl(tmp_section, "depth", 0, file);
            pastVpssChnAttr->bMirror                     = ini_getl(tmp_section, "mirror", 0, file);
            pastVpssChnAttr->bFlip                       = ini_getl(tmp_section, "flip", 0, file);
            
            pastVpssChnAttr->stAspectRatio.enMode = ini_getl(tmp_section, "aspectratio", 0, file);
            if (pastVpssChnAttr->stAspectRatio.enMode == ASPECT_RATIO_MANUAL) { /*ASPECT_RATIO_MANUAL*/
                pastVpssChnAttr->stAspectRatio.stVideoRect.s32X      = ini_getl(tmp_section, "s32x", 0, file);
                pastVpssChnAttr->stAspectRatio.stVideoRect.s32Y      = ini_getl(tmp_section, "s32y", 0, file);
                pastVpssChnAttr->stAspectRatio.stVideoRect.u32Width  = ini_getl(tmp_section, "rec_width", 0, file);
                pastVpssChnAttr->stAspectRatio.stVideoRect.u32Height = ini_getl(tmp_section, "rec_heigh", 0, file);
                pastVpssChnAttr->stAspectRatio.bEnableBgColor        = ini_getl(tmp_section, "en_color", 0, file);
                if (pastVpssChnAttr->stAspectRatio.bEnableBgColor != 0)
                    pastVpssChnAttr->stAspectRatio.u32BgColor        = ini_getl(tmp_section, "color", 0, file);
            }
            pastVpssChnAttr->stNormalize.bEnable = ini_getl(tmp_section, "normalize", 0, file);

            VPSS_CROP_INFO_S *pstVpssChnCropInfo = &Vpss->astVpssGrpCfg[grp_idx].stVpssChnCropInfo[chn_idx];
            pstVpssChnCropInfo->bEnable = ini_getl(tmp_section, "crop_en", 0, file);
            if (pstVpssChnCropInfo->bEnable) {
                pstVpssChnCropInfo->enCropCoordinate     = ini_getl(tmp_section, "crop_coor", 0, file);
                pstVpssChnCropInfo->stCropRect.s32X      = ini_getl(tmp_section, "crop_rect_x", 0, file);
                pstVpssChnCropInfo->stCropRect.s32Y      = ini_getl(tmp_section, "crop_rect_y", 0, file);
                pstVpssChnCropInfo->stCropRect.u32Width  = ini_getl(tmp_section, "crop_rect_w", 0, file);
                pstVpssChnCropInfo->stCropRect.u32Height = ini_getl(tmp_section, "crop_rect_h", 0, file);
            }

            Vpss->astVpssGrpCfg[grp_idx].aAttachEn[chn_idx]        = ini_getl(tmp_section, "attach_en", 0, file);
            if (Vpss->astVpssGrpCfg[grp_idx].aAttachEn[chn_idx]) {
                Vpss->astVpssGrpCfg[grp_idx].aAttachPool[chn_idx] = ini_getl(tmp_section, "attach_pool", 0, file);
            }
            APP_PROF_LOG_PRINT(LEVEL_INFO, "Chn_ID_%d config: sft=%2d dfr=%2d W=%4d H=%4d Depth=%d Mirror=%d Flip=%d V_fmt=%2d P_fmt=%2d\n", 
                chn_idx, pastVpssChnAttr->stFrameRate.s32SrcFrameRate, pastVpssChnAttr->stFrameRate.s32DstFrameRate,
                pastVpssChnAttr->u32Width, pastVpssChnAttr->u32Height, pastVpssChnAttr->u32Depth, pastVpssChnAttr->bMirror, pastVpssChnAttr->bFlip,
                pastVpssChnAttr->enVideoFormat, pastVpssChnAttr->enPixelFormat);
        }
    }
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading vpss config ------------------> done \n\n");
    return CVI_SUCCESS;
}

static int _Load_Param_Venc(const char *file, APP_PARAM_VENC_CTX_S *Venc)
{
    int i = 0;
    long int chn_num = 0;
    int buffSize = 0;
    char tmp_section[16] = {0};
    char tmp_buff[PARAM_STRING_LEN] = {0};

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading venc config ------------------> start \n");

    chn_num = ini_getl("venc_config", "chn_num", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "vpss_chn_num: %ld\n", chn_num);
    Venc->s32VencChnCnt = chn_num;

    for (i = 0; i < chn_num; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vencchn%d", i);
        Venc->astVencChnCfg[i].VencChn = i;

        Venc->astVencChnCfg[i].bEnable          = ini_getl(tmp_section, "bEnable", 0, file);
        if (!Venc->astVencChnCfg[i].bEnable) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "Venc_chn[%d] not enable!\n", i);
            continue;
        }

        Venc->astVencChnCfg[i].enType           = ini_getl(tmp_section, "en_type", 0, file);
        Venc->astVencChnCfg[i].StreamTo         = ini_getl(tmp_section, "send_to", 0, file);
        Venc->astVencChnCfg[i].VpssGrp          = ini_getl(tmp_section, "vpss_grp", 0, file);
        Venc->astVencChnCfg[i].VpssChn          = ini_getl(tmp_section, "vpss_chn", 0, file);
        Venc->astVencChnCfg[i].u32Width         = ini_getl(tmp_section, "width", 0, file);
        Venc->astVencChnCfg[i].u32Height        = ini_getl(tmp_section, "height", 0, file);

        Venc->astVencChnCfg[i].enBindMode       = ini_getl(tmp_section, "bind_mode", 0, file);
        if (Venc->astVencChnCfg[i].enBindMode != VENC_BIND_DISABLE) {
            Venc->astVencChnCfg[i].astChn[0].enModId    = ini_getl(tmp_section, "src_mod_id", 0, file);
            Venc->astVencChnCfg[i].astChn[0].s32DevId   = ini_getl(tmp_section, "src_dev_id", 0, file);
            Venc->astVencChnCfg[i].astChn[0].s32ChnId   = ini_getl(tmp_section, "src_chn_id", 0, file);
            Venc->astVencChnCfg[i].astChn[1].enModId    = ini_getl(tmp_section, "dst_mod_id", 0, file);
            Venc->astVencChnCfg[i].astChn[1].s32DevId   = ini_getl(tmp_section, "dst_dev_id", 0, file);
            Venc->astVencChnCfg[i].astChn[1].s32ChnId   = ini_getl(tmp_section, "dst_chn_id", 0, file);
        }
        ini_gets(tmp_section, "save_path", " ", tmp_buff, PARAM_STRING_LEN, file);
        strncpy(Venc->astVencChnCfg[i].SavePath, tmp_buff, PARAM_STRING_LEN);

        APP_PROF_LOG_PRINT(LEVEL_INFO, "Venc_chn[%d] enType=%d StreamTo=%d VpssGrp=%d VpssChn=%d u32Width=%d u32Height=%d\n save path =%s\n", 
            i, Venc->astVencChnCfg[i].enType, Venc->astVencChnCfg[i].StreamTo, Venc->astVencChnCfg[i].VpssGrp, Venc->astVencChnCfg[i].VpssChn, 
            Venc->astVencChnCfg[i].u32Width, Venc->astVencChnCfg[i].u32Height, Venc->astVencChnCfg[i].SavePath);
        
        Venc->astVencChnCfg[i].enRcMode         = ini_getl(tmp_section, "rc_mode", 0, file);
        buffSize = ini_getl(tmp_section, "bitStreamBuf", 0, file);
        Venc->astVencChnCfg[i].u32StreamBufSize = (buffSize << 10); 

        if (Venc->astVencChnCfg[i].enType != PT_JPEG) {

            Venc->astVencChnCfg[i].u32Profile       = ini_getl(tmp_section, "profile", 0, file);
            Venc->astVencChnCfg[i].u32SrcFrameRate  = ini_getl(tmp_section, "src_framerate", 0, file);
            Venc->astVencChnCfg[i].u32DstFrameRate  = ini_getl(tmp_section, "dst_framerate", 0, file);
            Venc->astVencChnCfg[i].enGopMode        = ini_getl(tmp_section, "gop_mode", 0, file);

            switch (Venc->astVencChnCfg[i].enGopMode) {
                case VENC_GOPMODE_NORMALP:
                    Venc->astVencChnCfg[i].unGopParam.stNormalP.s32IPQpDelta = ini_getl(tmp_section, "NormalP_IPQpDelta", 0, file);
                break;
                case VENC_GOPMODE_SMARTP:
                    Venc->astVencChnCfg[i].unGopParam.stSmartP.s32BgQpDelta  = ini_getl(tmp_section, "SmartP_BgQpDelta", 0, file);
                    Venc->astVencChnCfg[i].unGopParam.stSmartP.s32ViQpDelta  = ini_getl(tmp_section, "SmartP_ViQpDelta", 0, file);
                    Venc->astVencChnCfg[i].unGopParam.stSmartP.u32BgInterval = ini_getl(tmp_section, "SmartP_BgInterval", 0, file);
                break;
                case VENC_GOPMODE_DUALP:
                    Venc->astVencChnCfg[i].unGopParam.stDualP.s32IPQpDelta   = ini_getl(tmp_section, "DualP_IPQpDelta", 0, file);
                    Venc->astVencChnCfg[i].unGopParam.stDualP.s32SPQpDelta   = ini_getl(tmp_section, "DualP_SPQpDelta", 0, file);
                    Venc->astVencChnCfg[i].unGopParam.stDualP.u32SPInterval  = ini_getl(tmp_section, "DualP_SPInterval", 0, file);
                break;
                case VENC_GOPMODE_BIPREDB:
                    Venc->astVencChnCfg[i].unGopParam.stBipredB.s32BQpDelta  = ini_getl(tmp_section, "BipredB_BQpDelta", 0, file);
                    Venc->astVencChnCfg[i].unGopParam.stBipredB.s32IPQpDelta = ini_getl(tmp_section, "BipredB_IPQpDelta", 0, file);
                    Venc->astVencChnCfg[i].unGopParam.stBipredB.u32BFrmNum   = ini_getl(tmp_section, "BipredB_BFrmNum", 0, file);
                break;
                default:
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "venc_chn[%d] gop mode: %d invalid", i, Venc->astVencChnCfg[i].enGopMode);
                break;
            }

            Venc->astVencChnCfg[i].u32BitRate       = ini_getl(tmp_section, "bit_rate", 0, file);
            Venc->astVencChnCfg[i].u32MaxBitRate    = ini_getl(tmp_section, "max_bitrate", 0, file);
            Venc->astVencChnCfg[i].bSingleCore      = ini_getl(tmp_section, "single_core", 0, file);
            Venc->astVencChnCfg[i].u32Gop           = ini_getl(tmp_section, "gop", 0, file);
            Venc->astVencChnCfg[i].u32IQp           = ini_getl(tmp_section, "fixed_IQP", 0, file);
            Venc->astVencChnCfg[i].u32PQp           = ini_getl(tmp_section, "fixed_QPQ", 0, file);
            Venc->astVencChnCfg[i].statTime         = ini_getl(tmp_section, "statTime", 0, file);
            Venc->astVencChnCfg[i].u32Duration      = ini_getl(tmp_section, "file_duration", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32ThrdLv            = ini_getl(tmp_section, "ThrdLv", 0, file);
            Venc->astVencChnCfg[i].stRcParam.s32FirstFrameStartQp = ini_getl(tmp_section, "firstFrmstartQp", 0, file);
            Venc->astVencChnCfg[i].stRcParam.s32InitialDelay      = ini_getl(tmp_section, "initialDelay", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MaxIprop          = ini_getl(tmp_section, "MaxIprop", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MinIprop          = ini_getl(tmp_section, "MinIprop", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MaxQp             = ini_getl(tmp_section, "MaxQp", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MinQp             = ini_getl(tmp_section, "MinQp", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MaxIQp            = ini_getl(tmp_section, "MaxIQp", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MinIQp            = ini_getl(tmp_section, "MinIQp", 0, file);
            Venc->astVencChnCfg[i].stRcParam.s32ChangePos         = ini_getl(tmp_section, "ChangePos", 0, file);
            Venc->astVencChnCfg[i].stRcParam.s32MinStillPercent   = ini_getl(tmp_section, "MinStillPercent", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MaxStillQP        = ini_getl(tmp_section, "MaxStillQP", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MinStillPSNR      = ini_getl(tmp_section, "MinStillPSNR", 0, file);
            Venc->astVencChnCfg[i].stRcParam.u32MotionSensitivity = ini_getl(tmp_section, "MotionSensitivity", 0, file);
            Venc->astVencChnCfg[i].stRcParam.s32AvbrFrmLostOpen   = ini_getl(tmp_section, "AvbrFrmLostOpen", 0, file);
            Venc->astVencChnCfg[i].stRcParam.s32AvbrFrmGap        = ini_getl(tmp_section, "AvbrFrmGap", 0, file);
            Venc->astVencChnCfg[i].stRcParam.s32AvbrPureStillThr  = ini_getl(tmp_section, "AvbrPureStillThr", 0, file);
            Venc->astVencChnCfg[i].stRcParam.s32MaxReEncodeTimes  = ini_getl(tmp_section, "MaxReEncodeTimes", 0, file);

            APP_PROF_LOG_PRINT(LEVEL_INFO, "Venc_chn[%d] enRcMode=%d u32BitRate=%d u32MaxBitRate=%d enBindMode=%d bSingleCore=%d\n", 
                i, Venc->astVencChnCfg[i].enRcMode, Venc->astVencChnCfg[i].u32BitRate, Venc->astVencChnCfg[i].u32MaxBitRate, 
                Venc->astVencChnCfg[i].enBindMode, Venc->astVencChnCfg[i].bSingleCore);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "u32Gop=%d statTime=%d u32ThrdLv=%d\n", 
                Venc->astVencChnCfg[i].u32Gop, Venc->astVencChnCfg[i].statTime, Venc->astVencChnCfg[i].stRcParam.u32ThrdLv);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "u32MaxQp=%d u32MinQp=%d u32MaxIQp=%d u32MinIQp=%d s32ChangePos=%d s32InitialDelay=%d\n", 
                Venc->astVencChnCfg[i].stRcParam.u32MaxQp, Venc->astVencChnCfg[i].stRcParam.u32MinQp, Venc->astVencChnCfg[i].stRcParam.u32MaxIQp, 
                Venc->astVencChnCfg[i].stRcParam.u32MinIQp, Venc->astVencChnCfg[i].stRcParam.s32ChangePos, Venc->astVencChnCfg[i].stRcParam.s32InitialDelay);
        } else {
            Venc->astVencChnCfg[i].stJpegCodecParam.quality       = ini_getl(tmp_section, "quality", 0, file);
            Venc->astVencChnCfg[i].stJpegCodecParam.MCUPerECS     = ini_getl(tmp_section, "MCUPerECS", 0, file);
            APP_PROF_LOG_PRINT(LEVEL_INFO, "Venc_chn[%d] quality=%d\n", i, Venc->astVencChnCfg[i].stJpegCodecParam.quality);
        }
    }

    int roi_num = ini_getl("roi_config", "max_num", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "roi_max_num: %ld\n", roi_num);

    for (i = 0; i < roi_num; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "roi_index%d", i);

        Venc->astRoiCfg[i].u32Index = i;
        Venc->astRoiCfg[i].bEnable          = ini_getl(tmp_section, "bEnable", 0, file);
        if (!Venc->astRoiCfg[i].bEnable) {
            APP_PROF_LOG_PRINT(LEVEL_INFO, "Roi[%d] not enable!\n", i);
            continue;
        }

        Venc->astRoiCfg[i].VencChn          = ini_getl(tmp_section, "venc", 0, file);
        Venc->astRoiCfg[i].bAbsQp           = ini_getl(tmp_section, "absqp", 0, file);
        Venc->astRoiCfg[i].u32Qp            = ini_getl(tmp_section, "qp", 0, file);
        Venc->astRoiCfg[i].u32X             = ini_getl(tmp_section, "x", 0, file);
        Venc->astRoiCfg[i].u32Y             = ini_getl(tmp_section, "y", 0, file);
        Venc->astRoiCfg[i].u32Width         = ini_getl(tmp_section, "width", 0, file);
        Venc->astRoiCfg[i].u32Height        = ini_getl(tmp_section, "height", 0, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "Venc_chn[%d] bAbsQp=%d u32Qp=%d xy=(%d,%d) wd=(%d,%d)\n",
            Venc->astRoiCfg[i].VencChn, Venc->astRoiCfg[i].bAbsQp, Venc->astRoiCfg[i].u32Qp,
            Venc->astRoiCfg[i].u32X, Venc->astRoiCfg[i].u32Y, Venc->astRoiCfg[i].u32Width,
            Venc->astRoiCfg[i].u32Height);
    }
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading Roi config ------------------> done \n\n");

    return CVI_SUCCESS;
}

#ifdef AUDIO_SUPPORT
static int _Load_Param_Audio(const char *file, APP_PARAM_AUDIO_CFG_T *Auido)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading audio config ------------------> start \n");

    Auido->astAudioCfg.enSamplerate     = ini_getl("audio_config", "sample_rate", 0, file);
    Auido->astAudioCfg.enReSamplerate     = ini_getl("audio_config", "resample_rate", 0, file);
    Auido->astAudioCfg.Cal_DB_Enable                = ini_getl("audio_config", "Cal_DB_Enable", 0, file);
    Auido->astAudioCfg.u32ChnCnt        = ini_getl("audio_config", "chn_cnt", 0, file);
    Auido->astAudioCfg.enSoundmode      = ini_getl("audio_config", "sound_mode", 0, file);
    Auido->astAudioCfg.enBitwidth       = ini_getl("audio_config", "bit_width", 0, file);
    Auido->astAudioCfg.enWorkmode       = ini_getl("audio_config", "work_mode", 0, file);
    Auido->astAudioCfg.u32EXFlag        = ini_getl("audio_config", "ex_flag", 0, file);
    Auido->astAudioCfg.u32FrmNum        = ini_getl("audio_config", "frm_num", 0, file);
    Auido->astAudioCfg.u32PtNumPerFrm   = ini_getl("audio_config", "ptnum_per_frm", 0, file);
    Auido->astAudioCfg.u32ClkSel        = ini_getl("audio_config", "clk_sel", 0, file);
    Auido->astAudioCfg.enI2sType        = ini_getl("audio_config", "i2s_type", 0, file);
    Auido->astAudioCfg.u32AiDevId       = ini_getl("audio_config", "ai_dev_id", 0, file);
    Auido->astAudioCfg.u32AiChn         = ini_getl("audio_config", "ai_chn", 0, file);
    Auido->astAudioCfg.u32AoDevId       = ini_getl("audio_config", "ao_dev_id", 0, file);
    Auido->astAudioCfg.u32AoChn         = ini_getl("audio_config", "ao_chn", 0, file);
    Auido->astAudioCfg.u32AeChn         = ini_getl("audio_config", "ae_chn", 0, file);
    Auido->astAudioCfg.u32AdChn         = ini_getl("audio_config", "ad_chn", 0, file);
    Auido->astAudioCfg.enAencType       = ini_getl("audio_config", "en_type", 0, file);

    Auido->astAudioVol.iDacLVol         = ini_getl("audio_config", "daclvol", 0, file);
    Auido->astAudioVol.iDacRVol         = ini_getl("audio_config", "dacrvol", 0, file);
    Auido->astAudioVol.iAdcLVol         = ini_getl("audio_config", "adclvol", 0, file);
    Auido->astAudioVol.iAdcRVol         = ini_getl("audio_config", "adcrvol", 0, file);

    Auido->astAudioVqe.bAiAgcEnable                   = ini_getl("audio_vqe_agc", "ai_bEnable", 0, file);
    Auido->astAudioVqe.mAiAgcCfg.para_agc_max_gain    = ini_getl("audio_vqe_agc", "ai_max_gain", 0, file);
    Auido->astAudioVqe.mAiAgcCfg.para_agc_target_high = ini_getl("audio_vqe_agc", "ai_target_high", 0, file);
    Auido->astAudioVqe.mAiAgcCfg.para_agc_target_low  = ini_getl("audio_vqe_agc", "ai_target_low", 0, file);

    Auido->astAudioVqe.bAiAnrEnable                   = ini_getl("audio_vqe_anr", "ai_bEnable", 0, file);
    Auido->astAudioVqe.mAiAnrCfg.para_nr_snr_coeff    = ini_getl("audio_vqe_anr", "ai_snr_coeff", 0, file);

    Auido->astAudioVqe.bAiAecEnable                   = ini_getl("audio_vqe_aec", "ai_bEnable", 0, file);
    Auido->astAudioVqe.mAiAecCfg.para_aec_filter_len  = ini_getl("audio_vqe_aec", "ai_filter_len", 0, file);
    Auido->astAudioVqe.mAiAecCfg.para_aes_std_thrd    = ini_getl("audio_vqe_aec", "ai_std_thrd", 0, file);
    Auido->astAudioVqe.mAiAecCfg.para_aes_supp_coeff  = ini_getl("audio_vqe_aec", "ai_supp_coeff", 0, file);

    Auido->astAudioVqe.bAoAgcEnable                   = ini_getl("audio_vqe_agc", "ao_bEnable", 0, file);
    Auido->astAudioVqe.mAoAgcCfg.para_agc_max_gain    = ini_getl("audio_vqe_agc", "ao_max_gain", 0, file);
    Auido->astAudioVqe.mAoAgcCfg.para_agc_target_high = ini_getl("audio_vqe_agc", "ao_target_high", 0, file);
    Auido->astAudioVqe.mAoAgcCfg.para_agc_target_low  = ini_getl("audio_vqe_agc", "ao_target_low", 0, file);

    Auido->astAudioVqe.bAoAnrEnable                   = ini_getl("audio_vqe_anr", "ao_bEnable", 0, file);
    Auido->astAudioVqe.mAoAnrCfg.para_nr_snr_coeff    = ini_getl("audio_vqe_anr", "ao_snr_coeff", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading audio config ------------------> done \n\n");

    return CVI_SUCCESS;
}
#endif

static int _Load_Param_Osd(const char *file, APP_PARAM_OSD_CFG_T *Osd)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading OSD config ------------------> start \n");

    int i = 0;
    char tmp_buff[APP_OSD_STR_LEN_MAX] = {0};
    char tmp_section[16] = {0};

    Osd->osd_cnt = ini_getl("osd_config", "osd_cnt", 0, file);
    Osd->bEnable = ini_getl("osd_config", "osd_enable_all", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "osd Enable: %d osd_cnt: %d\n", Osd->bEnable, Osd->osd_cnt);
    if (Osd->bEnable) {
        for (i = 0; i < Osd->osd_cnt; i++) {
            memset(tmp_section, 0, sizeof(tmp_section));
            snprintf(tmp_section, sizeof(tmp_section), "osd%d", i);

            Osd->astOsdAttr[i].bShow            = ini_getl(tmp_section, "show", 1, file);
            Osd->astOsdAttr[i].Handle           = ini_getl(tmp_section, "handle", 0, file);
            Osd->astOsdAttr[i].enType           = ini_getl(tmp_section, "type", 0, file);
            Osd->astOsdAttr[i].stChn.enModId    = ini_getl(tmp_section, "mod_id", 0, file);
            Osd->astOsdAttr[i].stChn.s32DevId   = ini_getl(tmp_section, "dev_id", 0, file);
            Osd->astOsdAttr[i].stChn.s32ChnId   = ini_getl(tmp_section, "chn_id", 0, file);
            Osd->astOsdAttr[i].stRect.s32X      = ini_getl(tmp_section, "s32x", 0, file);
            Osd->astOsdAttr[i].stRect.s32Y      = ini_getl(tmp_section, "s32y", 0, file);
            Osd->astOsdAttr[i].stRect.u32Width  = ini_getl(tmp_section, "rec_width", 0, file);
            Osd->astOsdAttr[i].stRect.u32Height = ini_getl(tmp_section, "rec_heigh", 0, file);

            APP_PROF_LOG_PRINT(LEVEL_INFO, "bShow=%d Handle=%d enType=%d ModeId=%d DevId=%d ChnId=%d X=%4d Y=%4d W=%4d H=%4d\n", 
                Osd->astOsdAttr[i].bShow, Osd->astOsdAttr[i].Handle, Osd->astOsdAttr[i].enType,
                Osd->astOsdAttr[i].stChn.enModId, Osd->astOsdAttr[i].stChn.s32DevId, Osd->astOsdAttr[i].stChn.s32ChnId,
                Osd->astOsdAttr[i].stRect.s32X,Osd->astOsdAttr[i].stRect.s32Y, Osd->astOsdAttr[i].stRect.u32Width, Osd->astOsdAttr[i].stRect.u32Height);

            if (Osd->astOsdAttr[i].enType == TYPE_STRING) {
                ini_gets(tmp_section, "str", " ", tmp_buff, APP_OSD_STR_LEN_MAX, file);
                strncpy(Osd->astOsdAttr[i].str, tmp_buff, APP_OSD_STR_LEN_MAX);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "str[%d]: %s \n", i, Osd->astOsdAttr[i].str);
            }
            else if (Osd->astOsdAttr[i].enType == TYPE_PICTURE) {
                ini_gets(tmp_section, "file_name", " ", tmp_buff, PARAM_STRING_LEN, file);
                strncpy(Osd->astOsdAttr[i].filename, tmp_buff, PARAM_STRING_LEN);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "file_name[%d]: %s \n", i, Osd->astOsdAttr[i].filename);
            }
        }
    } else {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "osd not enable\n");
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading OSD config ------------------> done \n\n");

    return CVI_SUCCESS;
}
static int _Load_Param_Cover(const char *file, APP_PARAM_COVER_CFG_T *Cover)
{
    int i = 0;
    char tmp_section[16] = {0};

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading Cover config ------------------> start \n");

    Cover->Cover_cnt = ini_getl("cover_config", "cover_cnt", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "cover_cnt: %d\n", Cover->Cover_cnt);

    for (i = 0; i < Cover->Cover_cnt; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "Cover%d", i);

        Cover->astCoverAttr[i].Handle           = ini_getl(tmp_section, "handle", 0, file);
        Cover->astCoverAttr[i].bShow            = ini_getl(tmp_section, "show", 1, file);
        Cover->astCoverAttr[i].stChn.enModId    = ini_getl(tmp_section, "mod_id", 0, file);
        Cover->astCoverAttr[i].stChn.s32DevId   = ini_getl(tmp_section, "dev_id", 0, file);
        Cover->astCoverAttr[i].stChn.s32ChnId   = ini_getl(tmp_section, "chn_id", 0, file);
        Cover->astCoverAttr[i].stRect.s32X      = ini_getl(tmp_section, "s32x", 0, file);
        Cover->astCoverAttr[i].stRect.s32Y      = ini_getl(tmp_section, "s32y", 0, file);
        Cover->astCoverAttr[i].stRect.u32Width  = ini_getl(tmp_section, "rec_width", 0, file);
        Cover->astCoverAttr[i].stRect.u32Height = ini_getl(tmp_section, "rec_heigh", 0, file);
        Cover->astCoverAttr[i].u32Color         = ini_getl(tmp_section, "color", 0, file);

        APP_PROF_LOG_PRINT(LEVEL_INFO, "Handle=%d bShow=%d u32Color=0x%x ModeId=%d DevId=%d ChnId=%d X=%4d Y=%4d W=%4d H=%4d \n", 
            Cover->astCoverAttr[i].Handle, Cover->astCoverAttr[i].bShow, Cover->astCoverAttr[i].u32Color, 
            Cover->astCoverAttr[i].stChn.enModId, Cover->astCoverAttr[i].stChn.s32DevId, Cover->astCoverAttr[i].stChn.s32ChnId,
            Cover->astCoverAttr[i].stRect.s32X, Cover->astCoverAttr[i].stRect.s32Y, Cover->astCoverAttr[i].stRect.u32Width, Cover->astCoverAttr[i].stRect.u32Height);
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading Cover config ------------------> done \n\n");

    return CVI_SUCCESS;
}

static int _Load_Param_Osdc(const char *file, APP_PARAM_OSDC_CFG_S *Osdc)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading Osdc config ------------------> start \n");

    unsigned int i = 0;
    char tmp_section[16] = {0};

    Osdc->enable = ini_getl("osdc_config", "enable", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "osdc enable: %d\n", Osdc->enable);

    if (Osdc->enable) {
        Osdc->bShow           = ini_getl("osdc_config", "bShow", 0, file);
        Osdc->handle          = ini_getl("osdc_config", "handle", 0, file);
        Osdc->VpssGrp         = ini_getl("osdc_config", "vpss_grp", 0, file);
        Osdc->VpssChn         = ini_getl("osdc_config", "vpss_chn", 0, file);
        Osdc->CompressedSize  = ini_getl("osdc_config", "compressedsize", 0, file);
        Osdc->format          = ini_getl("osdc_config", "format", 0, file);
        Osdc->mmfChn.enModId  = ini_getl("osdc_config", "mod_id", 0, file);
        Osdc->mmfChn.s32DevId = ini_getl("osdc_config", "dev_id", 0, file);
        Osdc->mmfChn.s32ChnId = ini_getl("osdc_config", "chn_id", 0, file);
        Osdc->bShowPdRect     = ini_getl("osdc_config", "show_pd_rect", 0, file);
        Osdc->bShowMdRect     = ini_getl("osdc_config", "show_md_rect", 0, file);
        Osdc->bShowFdRect     = ini_getl("osdc_config", "show_fd_rect", 0, file);

        APP_PROF_LOG_PRINT(LEVEL_INFO, "handle=%d bShow=%d Format=0x%x cpsSize=%d ModeId=%d DevId=%d ChnId=%d PdRect=%d MdRect=%d FdRect=%d\n", 
            Osdc->handle, Osdc->bShow, Osdc->format, Osdc->CompressedSize, Osdc->mmfChn.enModId, 
            Osdc->mmfChn.s32DevId, Osdc->mmfChn.s32ChnId, Osdc->bShowPdRect, Osdc->bShowMdRect, Osdc->bShowFdRect);

        Osdc->osdcObjNum = ini_getl("osdc_objs_info", "cnt", 0, file);
        APP_PROF_LOG_PRINT(LEVEL_INFO, "osdc info cnt: %d\n", Osdc->osdcObjNum);

        for (i = 0; i < Osdc->osdcObjNum; i++) {
            memset(tmp_section, 0, sizeof(tmp_section));
            snprintf(tmp_section, sizeof(tmp_section), "osdc_obj_info%d", i);

            Osdc->osdcObj[i].bShow      = ini_getl(tmp_section, "bShow", 0, file);
            Osdc->osdcObj[i].type       = ini_getl(tmp_section, "type", 0, file);
            Osdc->osdcObj[i].color      = ini_getl(tmp_section, "color", 0, file);
            Osdc->osdcObj[i].x1         = ini_getl(tmp_section, "x1", 0, file);
            Osdc->osdcObj[i].y1         = ini_getl(tmp_section, "y1", 0, file);
            if (RGN_CMPR_LINE == Osdc->osdcObj[i].type) {
                Osdc->osdcObj[i].x2     = ini_getl(tmp_section, "x2", 0, file);
                Osdc->osdcObj[i].y2     = ini_getl(tmp_section, "y2", 0, file);
            } else if (RGN_CMPR_RECT == Osdc->osdcObj[i].type) {
                Osdc->osdcObj[i].width  = ini_getl(tmp_section, "width", 0, file);
                Osdc->osdcObj[i].height = ini_getl(tmp_section, "height", 0, file);
            }
            Osdc->osdcObj[i].filled     = ini_getl(tmp_section, "filled", 0, file);
            Osdc->osdcObj[i].thickness  = ini_getl(tmp_section, "thickness", 0, file);

            APP_PROF_LOG_PRINT(LEVEL_INFO, "type=%d color=0x%x x1=%d y1=%d x2=%d y2=%d width=%d height=%d filled=%d thickness=%d \n", 
                Osdc->osdcObj[i].type, Osdc->osdcObj[i].color, Osdc->osdcObj[i].x1, Osdc->osdcObj[i].y1,
                Osdc->osdcObj[i].x2, Osdc->osdcObj[i].y2, Osdc->osdcObj[i].width, Osdc->osdcObj[i].height,
                Osdc->osdcObj[i].filled, Osdc->osdcObj[i].thickness);
        }
    } else {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "Osdc not enable!\n");
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading Osdc config ------------------> done \n\n");

    return CVI_SUCCESS;
}

static int _Load_Param_Rtsp(const char *file, APP_PARAM_RTSP_T *Rtsp)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading RTSP config ------------------> start \n");

    char tmp_section[16] = {0};

    Rtsp->session_cnt = ini_getl("rtsp_config", "rtsp_cnt", 0, file);
    Rtsp->port = ini_getl("rtsp_config", "port", 0, file);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "rtsp session cnt=%d port:%d\n", Rtsp->session_cnt, Rtsp->port);
    for (int i = 0; i < Rtsp->session_cnt; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "session%d", i);

        Rtsp->VencChn[i] = ini_getl(tmp_section, "venc_chn", 0, file);
        Rtsp->SessionAttr[i].video.bitrate = ini_getl(tmp_section, "bitrate", 0, file);

        APP_PROF_LOG_PRINT(LEVEL_INFO, "Vecn Chn=%d Vbitrate=%d\n", 
            Rtsp->VencChn[i], Rtsp->SessionAttr[i].video.bitrate);
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading RTSP config ------------------> done \n\n");

    return CVI_SUCCESS;
}

#ifdef AI_SUPPORT
static int _Load_Param_Ai_FD(const char *file, APP_PARAM_AI_FD_CFG_S *Ai)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI PD config ------------------> start \n");

    char tmp_buff[128] = {0};
    float color_t;

    Ai->FD_bEnable        = ini_getl("ai_fd_config", "fd_enable", 0, file);
    Ai->FR_bEnable        = ini_getl("ai_fd_config", "fr_enable", 0, file);
    Ai->MASK_bEnable      = ini_getl("ai_fd_config", "mask_enable", 0, file);
    Ai->CAPURE_bEnable    = ini_getl("ai_fd_config", "capure_enable", 0, file);
    Ai->FACE_AE_bEnable   = ini_getl("ai_fd_config", "face_ae_enable", 0, file);
    Ai->VpssGrp           = ini_getl("ai_fd_config", "vpss_grp", 0, file);
    Ai->VpssChn           = ini_getl("ai_fd_config", "vpss_chn", 0, file);
    Ai->FdPoolId          = ini_getl("ai_fd_config", "fd_poolid", 0, file);
    Ai->u32GrpWidth       = ini_getl("ai_fd_config", "grp_width", 0, file);
    Ai->u32GrpHeight      = ini_getl("ai_fd_config", "grp_height", 0, file);
    Ai->model_size_w      = ini_getl("ai_fd_config", "model_width", 0, file);
    Ai->model_size_h      = ini_getl("ai_fd_config", "model_height", 0, file);
    Ai->bVpssPreProcSkip  = ini_getl("ai_fd_config", "vpssPreProcSkip", 0, file);
    Ai->threshold_fd      = ini_getf("ai_fd_config", "threshold_fd", 0, file);
    Ai->threshold_fr      = ini_getf("ai_fd_config", "threshold_fr", 0, file);
    Ai->threshold_mask    = ini_getf("ai_fd_config", "threshold_mask", 0, file);
    Ai->model_id_fd       = ini_getl("ai_fd_config", "model_id_fd", 0, file);
    Ai->model_id_fr       = ini_getl("ai_fd_config", "model_id_fr", 0, file);
    Ai->model_id_mask     = ini_getl("ai_fd_config", "model_id_mask", 0, file);

    color_t = ini_getf("ai_fd_config", "color_r", 0, file);
    Ai->rect_brush.color.r = color_t*255;
    color_t = ini_getf("ai_fd_config", "color_g", 0, file);
    Ai->rect_brush.color.g = color_t*255;
    color_t = ini_getf("ai_fd_config", "color_b", 0, file);
    Ai->rect_brush.color.b = color_t*255;
    Ai->rect_brush.size = ini_getl("ai_fd_config", "color_size", 0, file);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "bEnable=%d Grp=%d Chn=%d GrpW=%d GrpH=%d\n", 
        Ai->FD_bEnable, Ai->VpssGrp, Ai->VpssChn, Ai->u32GrpWidth, Ai->u32GrpHeight);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_w=%d model_h=%d bSkip=%d FdPoolId=%d threshold=%f\n",
        Ai->model_size_w, Ai->model_size_h, Ai->bVpssPreProcSkip, Ai->FdPoolId ,Ai->threshold_fd);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "color r=%f g=%f b=%f size=%d\n",
        Ai->rect_brush.color.r, Ai->rect_brush.color.g, 
        Ai->rect_brush.color.b, Ai->rect_brush.size);

    ini_gets("ai_fd_config", "model_path_fd", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_fd, tmp_buff, 128);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id=%d model_path=%s\n",
        Ai->model_id_fd, Ai->model_path_fd);
    
    ini_gets("ai_fd_config", "model_path_fr", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_fr, tmp_buff, 128);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id=%d model_path=%s\n",
        Ai->model_id_fr, Ai->model_path_fr);
    
    ini_gets("ai_fd_config", "model_path_mask", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path_mask, tmp_buff, 128);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id=%d model_path=%s\n",
        Ai->model_id_mask, Ai->model_path_mask);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI PD config ------------------> done \n\n");

    return CVI_SUCCESS;
}

static int _Load_Param_Ai_PD(const char *file, APP_PARAM_AI_PD_CFG_S *Ai)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI PD config ------------------> start \n");

    char tmp_buff[128] = {0};
    float color_t;

    Ai->bEnable      = ini_getl("ai_pd_config", "pd_enable", 0, file);
    Ai->Intrusion_bEnable = ini_getl("ai_pd_config", "intrusion_enable", 0, file);
    Ai->capture_enable    = ini_getl("ai_pd_config", "capture_enable", 0, file);
    Ai->capture_frames    = ini_getl("ai_pd_config", "capture_frames", 0, file);
    Ai->VpssGrp      = ini_getl("ai_pd_config", "vpss_grp", 0, file);
    Ai->VpssChn      = ini_getl("ai_pd_config", "vpss_chn", 0, file);
    Ai->u32GrpWidth  = ini_getl("ai_pd_config", "grp_width", 0, file);
    Ai->u32GrpHeight = ini_getl("ai_pd_config", "grp_height", 0, file);
    Ai->model_size_w = ini_getl("ai_pd_config", "model_width", 0, file);
    Ai->model_size_h = ini_getl("ai_pd_config", "model_height", 0, file);
    Ai->bVpssPreProcSkip = ini_getl("ai_pd_config", "vpssPreProcSkip", 0, file);
    Ai->model_id     = ini_getl("ai_pd_config", "model_id", 0, file);
    Ai->region_stRect_x1    = ini_getf("ai_pd_config", "region_stRect_x1", 0, file);
    Ai->region_stRect_y1    = ini_getf("ai_pd_config", "region_stRect_y1", 0, file);
    Ai->region_stRect_x2    = ini_getf("ai_pd_config", "region_stRect_x2", 0, file);
    Ai->region_stRect_y2    = ini_getf("ai_pd_config", "region_stRect_y2", 0, file);
    Ai->region_stRect_x3    = ini_getf("ai_pd_config", "region_stRect_x3", 0, file);
    Ai->region_stRect_y3    = ini_getf("ai_pd_config", "region_stRect_y3", 0, file);
    Ai->region_stRect_x4    = ini_getf("ai_pd_config", "region_stRect_x4", 0, file);
    Ai->region_stRect_y4    = ini_getf("ai_pd_config", "region_stRect_y4", 0, file);
    Ai->region_stRect_x5    = ini_getf("ai_pd_config", "region_stRect_x5", 0, file);
    Ai->region_stRect_y5    = ini_getf("ai_pd_config", "region_stRect_y5", 0, file);
    Ai->region_stRect_x6    = ini_getf("ai_pd_config", "region_stRect_x6", 0, file);
    Ai->region_stRect_y6    = ini_getf("ai_pd_config", "region_stRect_y6", 0, file);
    Ai->threshold           = ini_getf("ai_pd_config", "threshold", 0, file);

    color_t = ini_getf("ai_pd_config", "color_r", 0, file);
    Ai->rect_brush.color.r = color_t*255;
    color_t = ini_getf("ai_pd_config", "color_g", 0, file);
    Ai->rect_brush.color.g = color_t*255;
    color_t = ini_getf("ai_pd_config", "color_b", 0, file);
    Ai->rect_brush.color.b = color_t*255;

    Ai->rect_brush.size = ini_getl("ai_pd_config", "color_size", 0, file);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "bEnable=%d Grp=%d Chn=%d GrpW=%d GrpH=%d\n", 
        Ai->bEnable, Ai->VpssGrp, Ai->VpssChn, Ai->u32GrpWidth, Ai->u32GrpHeight);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_w=%d model_h=%d bSkip=%d threshold=%f\n",
        Ai->model_size_w, Ai->model_size_h, Ai->bVpssPreProcSkip, Ai->threshold);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "color r=%f g=%f b=%f size=%d\n",
        Ai->rect_brush.color.r, Ai->rect_brush.color.g, 
        Ai->rect_brush.color.b, Ai->rect_brush.size);

    ini_gets("ai_pd_config", "model_path", " ", tmp_buff, 128, file);
    strncpy(Ai->model_path, tmp_buff, 128);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_id=%d model_path=%s\n",
        Ai->model_id, Ai->model_path);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI PD config ------------------> done \n\n");

    return CVI_SUCCESS;
}

static int _Load_Param_Ai_MD(const char *file, APP_PARAM_AI_MD_CFG_S *Ai)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI MD config ------------------> start \n");

    float color_t;

    Ai->bEnable       = ini_getl("ai_md_config", "md_enable", 0, file);
    Ai->VpssGrp       = ini_getl("ai_md_config", "vpss_grp", 0, file);
    Ai->VpssChn       = ini_getl("ai_md_config", "vpss_chn", 0, file);
    Ai->u32GrpWidth   = ini_getl("ai_md_config", "grp_width", 0, file);
    Ai->u32GrpHeight  = ini_getl("ai_md_config", "grp_height", 0, file);
    Ai->threshold     = ini_getl("ai_md_config", "threshold", 0, file);
    Ai->miniArea      = ini_getl("ai_md_config", "miniArea", 0, file);
    Ai->u32BgUpPeriod = ini_getl("ai_md_config", "bgUpPeriod", 0, file);

    color_t = ini_getf("ai_md_config", "color_r", 0, file);
    Ai->rect_brush.color.r = color_t*255;
    color_t = ini_getf("ai_md_config", "color_g", 0, file);
    Ai->rect_brush.color.g = color_t*255;
    color_t = ini_getf("ai_md_config", "color_b", 0, file);
    Ai->rect_brush.color.b = color_t*255;

    Ai->rect_brush.size = ini_getl("ai_md_config", "color_size", 0, file);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "bEnable=%d Grp=%d Chn=%d GrpW=%d GrpH=%d\n", 
        Ai->bEnable, Ai->VpssGrp, Ai->VpssChn, Ai->u32GrpWidth, Ai->u32GrpHeight);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "threshold=%d miniArea=%d u32BgUpPeriod=%d\n",
        Ai->threshold, Ai->miniArea, Ai->u32BgUpPeriod);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "color r=%f g=%f b=%f size=%d\n",
        Ai->rect_brush.color.r, Ai->rect_brush.color.g, 
        Ai->rect_brush.color.b, Ai->rect_brush.size);

    APP_PROF_LOG_PRINT(LEVEL_INFO, "loading AI MD config ------------------> done \n\n");

    return CVI_SUCCESS;
}
#endif

static void print_usage(const char *prog)
{
    printf("Usage: %s [-i]\n", prog);
    puts("  -i --input    input param config ini file (e.g. \"/mnt/data/param_config.ini\")\n");
    exit(1);
}

int app_ipcam_Opts_Parse(int argc, char *argv[])
{
    while (1) {
        static const struct option lopts[] = {
            { "input",   1, 0, 'i' },
            { NULL, 0, 0, 0 },
        };
        int c;

        c = getopt_long(argc, argv, "i:",
                lopts, NULL);

        if (c == -1)
            break;

        switch (c) {
        case 'i':
            input_file = optarg;
            APP_CHK_RET(access(input_file, F_OK), "param config ini file access");
            strncpy(ParamCfgFile, input_file, 64);
            break;
        default:
            print_usage(argv[0]);
            break;
        }
    }

    return CVI_SUCCESS;
}

int app_ipcam_Param_Load(void)
{
    APP_CHK_RET(access(ParamCfgFile, F_OK), "param_config.ini access");

    APP_CHK_RET(_Load_Param_Sys(ParamCfgFile, app_ipcam_Sys_Param_Get()),     "load SysVb Param");
    APP_CHK_RET(_Load_Param_Vi(ParamCfgFile, app_ipcam_Vi_Param_Get()),       "load Vi Param");
    APP_CHK_RET(_Load_Param_Vpss(ParamCfgFile, app_ipcam_Vpss_Param_Get()),   "load Vpss Param");
    APP_CHK_RET(_Load_Param_Venc(ParamCfgFile, app_ipcam_Venc_Param_Get()),   "load Venc Param");
    #ifdef AUDIO_SUPPORT
    APP_CHK_RET(_Load_Param_Audio(ParamCfgFile, app_ipcam_Audio_Param_Get()), "load Audio Param");
    #endif
    APP_CHK_RET(_Load_Param_Osd(ParamCfgFile, app_ipcam_Osd_Param_Get()),     "load Osd Param");
    APP_CHK_RET(_Load_Param_Cover(ParamCfgFile, app_ipcam_Cover_Param_Get()), "load Cover Param");
    APP_CHK_RET(_Load_Param_Osdc(ParamCfgFile, app_ipcam_Osdc_Param_Get()),   "load Osdc Param");
    APP_CHK_RET(_Load_Param_Rtsp(ParamCfgFile, app_ipcam_Rtsp_Param_Get()),   "load Rtsp Param");
    #ifdef AI_SUPPORT
    APP_CHK_RET(_Load_Param_Ai_PD(ParamCfgFile, app_ipcam_Ai_PD_Param_Get()), "load Ai PD Param");
    APP_CHK_RET(_Load_Param_Ai_MD(ParamCfgFile, app_ipcam_Ai_MD_Param_Get()), "load Ai MD Param");
    APP_CHK_RET(_Load_Param_Ai_FD(ParamCfgFile, app_ipcam_Ai_FD_Param_Get()), "load Ai FD Param");
    #endif

    return CVI_SUCCESS;
}

