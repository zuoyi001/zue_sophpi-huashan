#ifndef __APP_IPCAM_OSD_H__
#define __APP_IPCAM_OSD_H__

#include <stdbool.h>
#include "cvi_type.h"
#include "cvi_comm_region.h"
#include "app_ipcam_mq.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define OSD_COUNT_MAX   16
#define MAX_FILE_LEN    32
#define APP_OSD_STR_LEN_MAX     64
#define COVER_COUNT_MAX 16

typedef enum OSD_TYPE_T {
    TYPE_PICTURE,
    TYPE_STRING,
    TYPE_TIME,
    TYPE_DEBUG,
    TYPE_END
} OSD_TYPE_E;

typedef struct APP_PARAM_OSD_ATTR_T {
    CVI_BOOL bShow;
    RGN_HANDLE Handle;
    OSD_TYPE_E enType;
    MMF_CHN_S stChn;
    RECT_S stRect;
    union {
        char filename[MAX_FILE_LEN];
        char str[APP_OSD_STR_LEN_MAX];
    };
} APP_PARAM_OSD_ATTR_S;

typedef struct APP_PARAM_OSD_CFG_S {
    bool bInit;
    CVI_S32 osd_cnt;
    APP_PARAM_OSD_ATTR_S astOsdAttr[OSD_COUNT_MAX];
} APP_PARAM_OSD_CFG_T;

typedef struct APP_PARAM_COVER_ATTR_T {
    CVI_BOOL bShow;
    RGN_HANDLE Handle;
    CVI_U32 u32Color;
    MMF_CHN_S stChn;
    RECT_S stRect;
} APP_PARAM_COVER_ATTR_S;

typedef struct APP_PARAM_COVER_CFG_S {
    bool bInit;
    CVI_S32 Cover_cnt;
    APP_PARAM_COVER_ATTR_S astCoverAttr[COVER_COUNT_MAX];
} APP_PARAM_COVER_CFG_T;


APP_PARAM_OSD_CFG_T *app_ipcam_Osd_Param_Get(void);
APP_PARAM_COVER_CFG_T *app_ipcam_Cover_Param_Get(void);
int app_ipcam_Osd_Init(void);
int app_ipcam_Cover_Init(void);
int app_ipcam_Osd_DeInit(void);
int app_ipcam_Cover_DeInit(void);

/*****************************************************************
 *  The following API for command test used             S
 * **************************************************************/
int app_ipcam_CmdTask_Osd_Switch(CVI_MQ_MSG_t *msg, CVI_VOID *userdate);
int app_ipcam_CmdTask_Cover_Switch(CVI_MQ_MSG_t *msg, CVI_VOID *userdate);
/*****************************************************************
 *  The above API for command test used                 E
 * **************************************************************/

#ifdef __cplusplus
}
#endif

#endif