#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <unistd.h>
#include "app_ipcam_ai.h"

// Ai Model info
/*****************************************************************
 * Model Func : Persion , vehicle , non-motor vehicle
 * Model ID   : CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PERSON_VEHICLE
 * Inference  : CVI_AI_MobileDetV2_Person_Vehicle
 * Model file : mobiledetv2-person-vehicle-ls-768.cvimodel (input size 1x3x512x768)
 *              mobiledetv2-person-vehicle-ls.cvimodel (input size 1x3x512x768)
 =================================================================
 * Model Func : Pedestrian detection
 * Model ID   : CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN_D0
 * Inference  : CVI_AI_MobileDetV2_Pedestrian_D0
 * Model file : mobiledetv2-pedestrian-d0-ls-384.cvimodel (input size 1x3x256x384)
 *              mobiledetv2-pedestrian-d0-ls-640.cvimodel (input size 1x3x384x640)
 *              mobiledetv2-pedestrian-d0-ls-768.cvimodel (input size 1x3x512x768)
 *              mobiledetv2-pedestrian-d1-ls.cvimodel (input size 1x3x512x896)
 *              mobiledetv2-pedestrian-d1-ls-1024.cvimodel (input size 1x3x384x640)
 =================================================================
 * Model Func : Vehicle detection
 * Model ID   : CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_VEHICLE_D0
 * Inference  : CVI_AI_MobileDetV2_Vehicle_D0
 * Model file : mobiledetv2-vehicle-d0-ls.cvimodel (input size 1x3x384x512)
****************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

#define SMT_MUTEXAUTOLOCK_INIT(mutex) pthread_mutex_t AUTOLOCK_##mutex = PTHREAD_MUTEX_INITIALIZER;

#define SMT_MutexAutoLock(mutex, lock) __attribute__((cleanup(AutoUnLock))) \
    pthread_mutex_t *lock= &AUTOLOCK_##mutex;\
    pthread_mutex_lock(lock);

__attribute__ ((always_inline)) inline void AutoUnLock(void *mutex) {
    pthread_mutex_unlock( *(pthread_mutex_t**) mutex);
}

SMT_MUTEXAUTOLOCK_INIT(OD_Mutex);

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
#if 0
static APP_PARAM_AI_PD_CFG_S stPdCfg = {
    .bEnable = CVI_TRUE,
    .VpssGrp = 3,
    .VpssChn = 0,
    .u32GrpWidth = 640,
    .u32GrpHeight = 360,
    .model_size_w = 384,
    .model_size_h = 256,
    .bVpssPreProcSkip = CVI_TRUE,
    .threshold = 0.7,
    .model_id = CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN_D0,
    .model_path = "/mnt/data/ai_models/mobiledetv2-pedestrian-d0-ls-384.cvimodel",
    .rect_brush = {
        .color = {
            .r = 3.0*255,
            .g = 8.0*255,
            .b = 247.0*255,
        },
        .size = 4,
    },
};
#else
static APP_PARAM_AI_PD_CFG_S stPdCfg;
#endif

static APP_PARAM_AI_PD_CFG_S *pstPdCfg = &stPdCfg;

static CVI_U32 pd_fps;
static CVI_U32 pd_proc;
static volatile bool bRunning = CVI_FALSE;
static pthread_t Thread_Handle;
static cviai_handle_t Ai_Handle = NULL;
static cviai_service_handle_t Ai_Service_Handle = NULL;
static cvai_object_t stObjDraw;
static pfpInferenceFunc pfpInference;
cvai_pts_t **convex_pts = NULL;
uint32_t convex_num;
cvai_service_brush_t region_brush;
static int capture_counts = 0;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_AI_PD_CFG_S *app_ipcam_Ai_PD_Param_Get(void)
{
    return pstPdCfg;
}

CVI_VOID app_ipcam_Ai_PD_ProcStatus_Set(CVI_BOOL flag)
{
    bRunning = flag;
}

CVI_BOOL app_ipcam_Ai_PD_ProcStatus_Get(void)
{
    return bRunning;
}

CVI_U32 app_ipcam_Ai_PD_ProcFps_Get(void)
{
    return pd_fps;
}

CVI_S32 app_ipcam_Ai_PD_ProcTime_Get(void)
{
    return pd_proc;
}

#if 0
static int getNumDigits(uint64_t num) 
{
    int digits = 0;
    do {
        num /= 10;
        digits++;
    } while (num != 0);

    return digits;
}

static char *uint64ToString(uint64_t number) 
{
    int n = getNumDigits(number);
    int i;
    char *numArray = calloc(n, sizeof(char));
    for (i = n - 1; i >= 0; --i, number /= 10) {
        numArray[i] = (number % 10) + '0';
    }

    return numArray;
}
#endif

static CVI_S32 app_ipcam_Ai_InferenceFunc_Get(CVI_AI_SUPPORTED_MODEL_E model_id)
{
    switch (model_id)
    {
        case CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN_D0:
            pfpInference = CVI_AI_MobileDetV2_Pedestrian_D0;
        break;

        case CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_VEHICLE_D0:
            pfpInference = CVI_AI_MobileDetV2_Vehicle_D0;
        break;

        case CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PERSON_VEHICLE:
            pfpInference = CVI_AI_MobileDetV2_Person_Vehicle;
        break;

         case CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_LITE_PERSON_PETS:
            pfpInference = CVI_AI_MobileDetV2_Lite_Person_Pets;
            break;

        default:
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "model id (%d) invalid!\n", model_id);
            return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

static void app_ipcam_Ai_Param_dump(void)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "bEnable=%d Grp=%d Chn=%d GrpW=%d GrpH=%d\n", 
        pstPdCfg->bEnable, pstPdCfg->VpssGrp, pstPdCfg->VpssChn, pstPdCfg->u32GrpWidth, pstPdCfg->u32GrpHeight);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "model_w=%d model_h=%d bSkip=%d threshold=%f\n",
        pstPdCfg->model_size_w, pstPdCfg->model_size_h, pstPdCfg->bVpssPreProcSkip, pstPdCfg->threshold);
    APP_PROF_LOG_PRINT(LEVEL_INFO, " model_id=%d model_path=%s\n",
        pstPdCfg->model_id, pstPdCfg->model_path);
    APP_PROF_LOG_PRINT(LEVEL_INFO, " color r=%f g=%f b=%f size=%d\n",
        pstPdCfg->rect_brush.color.r, pstPdCfg->rect_brush.color.g, 
        pstPdCfg->rect_brush.color.b, pstPdCfg->rect_brush.size);

}

static CVI_S32 app_ipcam_Ai_VpssChnAttr_Set(CVI_VOID) 
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    CVI_AI_SUPPORTED_MODEL_E model_id = pstPdCfg->model_id;
    VPSS_GRP VpssGrp = pstPdCfg->VpssGrp;
    VPSS_CHN VpssChn = pstPdCfg->VpssChn;

    APP_PARAM_VPSS_CFG_T *pstVpssCfg = app_ipcam_Vpss_Param_Get();
    VPSS_CHN_ATTR_S *pastVpssChnAttr = &pstVpssCfg->astVpssGrpCfg[VpssGrp].astVpssChnAttr[VpssChn];
    CVI_U32 u32Width = pstVpssCfg->astVpssGrpCfg[VpssGrp].stVpssGrpAttr.u32MaxW;
    CVI_U32 u32Height = pstVpssCfg->astVpssGrpCfg[VpssGrp].stVpssGrpAttr.u32MaxH;
    CVI_S32 s32SrcFrameRate = pastVpssChnAttr->stFrameRate.s32SrcFrameRate;
    CVI_S32 s32DstFrameRate = pastVpssChnAttr->stFrameRate.s32DstFrameRate;
    
    cvai_vpssconfig_t vpssConfig;
    memset(&vpssConfig, 0, sizeof(vpssConfig));

    s32Ret = CVI_AI_GetVpssChnConfig(
                            Ai_Handle, 
                            model_id,
                            u32Width,
                            u32Height,
                            0, 
                            &vpssConfig);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_GetVpssChnConfig failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    vpssConfig.chn_attr.stFrameRate.s32SrcFrameRate = s32SrcFrameRate;
    vpssConfig.chn_attr.stFrameRate.s32DstFrameRate = s32DstFrameRate;

    s32Ret = CVI_VPSS_SetChnScaleCoefLevel(VpssGrp, VpssChn, vpssConfig.chn_coeff);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VPSS_SetChnScaleCoefLevel failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "vpssConfig.chn_attr u32Width=%d u32Height=%d SrcFR=%d DstFR=%d\n", 
                    vpssConfig.chn_attr.u32Width, vpssConfig.chn_attr.u32Height, s32SrcFrameRate, s32DstFrameRate);

    s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &vpssConfig.chn_attr);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VPSS_SetChnAttr failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_VPSS_EnableChn failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_AI_SetVpssTimeout(Ai_Handle, 2000);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "set vpss timeout failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_Ai_Proc_Init(CVI_VOID)
{
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI PD init ------------------> start \n");
    
    CVI_S32 s32Ret = CVI_SUCCESS;

    app_ipcam_Ai_Param_dump();

    CVI_AI_SUPPORTED_MODEL_E model_id = pstPdCfg->model_id;

    if (Ai_Handle == NULL)
    {
        s32Ret = CVI_AI_CreateHandle(&Ai_Handle);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_CreateHandle failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    } 
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_CreateHandle has created\n");
        return s32Ret;
    }
    
    if (Ai_Service_Handle == NULL)
    {
        s32Ret = CVI_AI_Service_CreateHandle(&Ai_Service_Handle, Ai_Handle);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_CreateHandle failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }
    else
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_Service_CreateHandle has created\n");
        return s32Ret;
    }

    s32Ret = app_ipcam_Ai_InferenceFunc_Get(model_id);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "unsupported model id: %d \n", model_id);
        return s32Ret;
    }

    s32Ret = CVI_AI_SetModelPath(Ai_Handle, model_id, pstPdCfg->model_path);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_SetModelPath failed with %#x! maybe reset model path\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_AI_SetModelThreshold(Ai_Handle, model_id, pstPdCfg->threshold);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_SetModelThreshold failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    /* Mark this , then draw rectangle by cpu , or by tpu */
    // CVI_AI_Service_EnableTPUDraw(g_AI_Service_Handle, true);

    CVI_BOOL bSkip = pstPdCfg->bVpssPreProcSkip;
    CVI_AI_SetSkipVpssPreprocess(Ai_Handle, model_id, bSkip);
    if (bSkip)
    {
        s32Ret = app_ipcam_Ai_VpssChnAttr_Set();
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "VpssChnAttr Set failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }

    if (pstPdCfg->Intrusion_bEnable)
    {
        float r0[2][4] = {{10, 10, 1400, 1400}, {10, 1400, 1400, 10}};
        cvai_pts_t test_region_0;
        test_region_0.size = (uint32_t)sizeof(r0) / (sizeof(float) * 2);
        test_region_0.x = malloc(sizeof(float) * test_region_0.size);
        test_region_0.y = malloc(sizeof(float) * test_region_0.size);
        memcpy(test_region_0.x, r0[0], sizeof(float) * test_region_0.size);
        memcpy(test_region_0.y, r0[1], sizeof(float) * test_region_0.size);

        s32Ret = CVI_AI_Service_Polygon_SetTarget(Ai_Service_Handle, &test_region_0);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_Service_Polygon_SetTarget failed with %#x!\n", s32Ret);
            return s32Ret;
        }
        s32Ret = CVI_AI_Service_Polygon_GetTarget(Ai_Service_Handle, &convex_pts, &convex_num);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_Service_Polygon_GetTarget failed with %#x!\n", s32Ret);
            return s32Ret;
        }

        region_brush = CVI_AI_Service_GetDefaultBrush();
        region_brush.color.r = 122;
        region_brush.color.g = 255;
        region_brush.color.b = 122;
    }
    s32Ret = CVI_AI_SelectDetectClass(Ai_Handle, model_id, 8, 
                                        CVI_AI_DET_TYPE_PERSON,
                                        CVI_AI_DET_TYPE_BICYCLE,
                                        CVI_AI_DET_TYPE_CAR,
                                        CVI_AI_DET_TYPE_MOTORBIKE,
                                        CVI_AI_DET_TYPE_TRUCK,
                                        CVI_AI_DET_TYPE_BUS,
                                        CVI_AI_DET_TYPE_CAT,
                                        CVI_AI_DET_TYPE_DOG);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_SelectDetectClass failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_AI_OpenModel(Ai_Handle, model_id);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_SelectDetectClass failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI PD init ------------------> done \n");

    return CVI_SUCCESS;
}

static CVI_VOID *Thread_PD_PROC(CVI_VOID *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI PD start running!\n");

    prctl(PR_SET_NAME, "Thread_PD_PROC");

    VPSS_GRP VpssGrp = pstPdCfg->VpssGrp;
    VPSS_CHN VpssChn = pstPdCfg->VpssChn;

    VIDEO_FRAME_INFO_S stfdFrame = {0};

    CVI_U32 pd_frame = 0;
    CVI_S32 iTime_start, iTime_proc,iTime_fps;
    float iTime_gop;
    iTime_start = GetCurTimeInMsec();

    while (app_ipcam_Ai_PD_ProcStatus_Get())
    {
        s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stfdFrame, 2000);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) get frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
            usleep(100*1000);
            continue;
        }
        iTime_proc = GetCurTimeInMsec();
        cvai_object_t obj_meta;
        memset(&obj_meta, 0, sizeof(cvai_object_t));

        pfpInference(Ai_Handle, &stfdFrame, &obj_meta);
        if (pstPdCfg->capture_enable){
            if (obj_meta.size > 0){
                capture_counts ++;
            }else{
                capture_counts = 0;
            }
            if (capture_counts > pstPdCfg->capture_frames){
                app_ipcam_JpgCapFlag_Set(CVI_TRUE);
                capture_counts = 0;
            }
        }
        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "PD obj: %d \n", obj_meta.size);
        pd_proc = GetCurTimeInMsec() - iTime_proc;
        s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stfdFrame);
        if (s32Ret != CVI_SUCCESS)
        {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Grp(%d)-Chn(%d) release frame failed with %#x\n", VpssGrp, VpssChn, s32Ret);
        }

        pd_frame ++;
        iTime_fps = GetCurTimeInMsec();
        iTime_gop = (float)(iTime_fps - iTime_start)/1000;
        if(iTime_gop >= 1)
        {
            pd_fps = pd_frame/iTime_gop;
            pd_frame = 0;
            iTime_start = iTime_fps;
        }

        SMT_MutexAutoLock(OD_Mutex, lock);
        if (stObjDraw.info != NULL) {
            CVI_AI_Free(&stObjDraw);
        }
        memset(&stObjDraw, 0, sizeof(cvai_object_t));
        stObjDraw.size = obj_meta.size;
        stObjDraw.rescale_type = RESCALE_RB;
        stObjDraw.width = obj_meta.width;
        stObjDraw.height = obj_meta.height;
        stObjDraw.info = (cvai_object_info_t *)malloc(obj_meta.size * sizeof(cvai_object_info_t));
        memset(stObjDraw.info, 0, sizeof(cvai_object_info_t) * stObjDraw.size);
        for (uint32_t i = 0; i < obj_meta.size; i++)
        {
            CVI_AI_CopyObjectInfo(&obj_meta.info[i], &stObjDraw.info[i]);
        }

        CVI_AI_Free(&obj_meta);
    }

    pthread_exit(NULL);

    return NULL;
}

int app_ipcam_Ai_PD_Rect_Draw(VIDEO_FRAME_INFO_S *pstVencFrame)
{
    CVI_S32 iTime;

    if (app_ipcam_Ai_PD_ProcStatus_Get())
    {
        SMT_MutexAutoLock(OD_Mutex, lock);
        iTime = GetCurTimeInMsec();
        CVI_AI_RescaleMetaRB(pstVencFrame, &stObjDraw);
        if (pstPdCfg->Intrusion_bEnable)
        {
            for (uint32_t i = 0; i < convex_num; i++)
            {
                CVI_AI_Service_DrawPolygon(Ai_Service_Handle, pstVencFrame, convex_pts[i], region_brush);
            }
            for (uint32_t i = 0; i < stObjDraw.size; i++)
            {
                bool is_intrusion;
                cvai_bbox_t t_bbox = stObjDraw.info[i].bbox;
                t_bbox.x1 *= 1;
                t_bbox.y1 *= 1;
                t_bbox.x2 *= 1;
                t_bbox.y2 *= 1;
                CVI_AI_Service_Polygon_Intersect(Ai_Service_Handle, &t_bbox, &is_intrusion);
                if (is_intrusion)
                {
                    APP_PROF_LOG_PRINT(LEVEL_DEBUG, "[%u] intrusion! (%.1f,%.1f,%.1f,%.1f)\n", i, stObjDraw.info[i].bbox.x1,
                    stObjDraw.info[i].bbox.y1, stObjDraw.info[i].bbox.x2, stObjDraw.info[i].bbox.y2);
                    strcpy(stObjDraw.info[i].name,"Intrusion!");
                }
                else
                {
                    strcpy(stObjDraw.info[i].name,"");
                }
            }
        }

        CVI_AI_Service_ObjectDrawRect(
                            Ai_Service_Handle, 
                            &stObjDraw, 
                            pstVencFrame, 
                            CVI_TRUE, 
                            pstPdCfg->rect_brush);
        #ifdef AI_DRAW_OBJECT_ID
        for (uint32_t i = 0; i < stObjDraw.size; i++)
        {
            char *id_num = uint64ToString(stObjDraw.info[i].unique_id);
            CVI_AI_Service_ObjectWriteText(id_num, stObjDraw.info[i].bbox.x1, stObjDraw.info[i].bbox.y1,
                                pstVencFrame, -1, -1, -1);
            free(id_num);
        }
        #endif
        CVI_AI_Free(&stObjDraw);
        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "PD draw result takes %u ms \n", (GetCurTimeInMsec() - iTime));
    }

    return CVI_SUCCESS;
}

int app_ipcam_Ai_PD_Stop(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!pstPdCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI PD not enable\n");
        return CVI_SUCCESS;
    }

    if (!app_ipcam_Ai_PD_ProcStatus_Get())
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI PD has not running!\n");
        return s32Ret;
    }

    app_ipcam_Ai_PD_ProcStatus_Set(CVI_FALSE);

    CVI_S32 iTime = GetCurTimeInMsec();

    if (Thread_Handle)
    {
        pthread_join(Thread_Handle, NULL);
        Thread_Handle = 0;
    }

    s32Ret = CVI_AI_Service_DestroyHandle(Ai_Service_Handle);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_Service_DestroyHandle failed with 0x%x!\n", s32Ret);
        return s32Ret;
    }
    else
    {
        Ai_Service_Handle = NULL;
    }

    s32Ret = CVI_AI_DestroyHandle(Ai_Handle);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_AI_DestroyHandle failed with 0x%x!\n", s32Ret);
        return s32Ret;
    }
    else
    {
        Ai_Handle = NULL;
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "AI PD Thread exit takes %u ms\n", (GetCurTimeInMsec() - iTime));

    return CVI_SUCCESS;
}

int app_ipcam_Ai_PD_Start(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!pstPdCfg->bEnable)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI PD not enable\n");
        return CVI_SUCCESS;
    }

    if (bRunning)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "AI PD has started\n");
        return CVI_SUCCESS;
    }

    app_ipcam_Ai_Proc_Init();

    app_ipcam_Ai_PD_ProcStatus_Set(CVI_TRUE);

    s32Ret = pthread_create(&Thread_Handle, NULL, Thread_PD_PROC, NULL);
    if (s32Ret != CVI_SUCCESS)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "AI pthread_create failed!\n");
        return s32Ret;
    }
    
    return CVI_SUCCESS;
}

/*****************************************************************
 *  The following API for command test used             Front
 * **************************************************************/

int app_ipcam_CmdTask_Ai_PD_Switch(CVI_MQ_MSG_t *msg, CVI_VOID *userdate)
{
    CVI_CHAR param[256] = {0};
    CVI_S32 s32Ret = CVI_SUCCESS;
    snprintf(param, sizeof(param), "%s", msg->payload);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s param:%s arg2=%d\n", __FUNCTION__, param, msg->arg2);

    CVI_CHAR *temp = strtok(param, ":");
    while (NULL != temp)
    {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "%s switch case -> %c \n", __FUNCTION__, *temp);
        switch (*temp)
        {
            case 's': 
            {
                temp = strtok(NULL, "/");
                CVI_BOOL isEnable = (CVI_BOOL)atoi(temp);
                if (isEnable)
                {
                    app_ipcam_Ai_PD_Start();
                }
                else
                {
                    app_ipcam_Ai_PD_Stop();
                }
                break;
            }
            case 't':
            {
                temp = strtok(NULL, "/");
                float threshold = (float)atoi(temp)/100;
                APP_PROF_LOG_PRINT(LEVEL_INFO, "threshold:%lf\n", threshold);
                s32Ret = CVI_AI_SetModelThreshold(Ai_Handle, pstPdCfg->model_id, threshold);
                if (s32Ret != CVI_SUCCESS)
                {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "%s CVI_AI_SetModelThreshold failed with %#x!\n",pstPdCfg->model_path, s32Ret);
                    return s32Ret;
                }
                break;
            }
            default:
                return 0;
                break;
        }
        
        temp = strtok(NULL, ":");
    }

    return CVI_SUCCESS;
}

/*****************************************************************
 *  The above API for command test used                 End
 * **************************************************************/
