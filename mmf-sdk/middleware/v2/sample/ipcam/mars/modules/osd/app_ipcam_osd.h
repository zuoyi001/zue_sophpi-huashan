#ifndef __APP_IPCAM_OSD_H__
#define __APP_IPCAM_OSD_H__

#include <stdbool.h>
#include "cvi_type.h"
#include "linux/cvi_comm_region.h"
#include "app_ipcam_mq.h"
#include "cvi_region.h"
#include <cvi_osdc.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define OSD_COUNT_MAX   16
#define MAX_FILE_LEN    32
#define DEBUG_STR_LEN   32
#define APP_OSD_STR_LEN_MAX     64
#define COVER_COUNT_MAX 16

#define OSDC_OBJS_MAX 128

typedef enum OSD_TYPE_T {
    TYPE_PICTURE,
    TYPE_STRING,
    TYPE_TIME,
    TYPE_DEBUG,
    TYPE_END
} OSD_TYPE_E;

typedef struct APP_PARAM_OSD_ATTR_T {
    CVI_BOOL bShow;
    CVI_BOOL bChange;
    RGN_HANDLE Handle;
    OSD_TYPE_E enType;
    CVI_S32 ts_format;
    CVI_S32 font_size;
    CVI_S32 color;
    MMF_CHN_S stChn;
    RECT_S stRect;
    union {
        char filename[MAX_FILE_LEN];
        char str[APP_OSD_STR_LEN_MAX];
    };
} APP_PARAM_OSD_ATTR_S;

typedef struct APP_PARAM_OSD_CFG_S {
    bool bInit;
    bool bEnable;
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

typedef struct APP_OSDC_OBJS_INFO_T {
    CVI_BOOL bShow;
    RGN_CMPR_TYPE_E type;
    CVI_U32 color;
    CVI_U32 x1;
    CVI_U32 y1;
    CVI_U32 x2;
    CVI_U32 y2;
    CVI_U32 width;
    CVI_U32 height;
    CVI_BOOL filled;
    CVI_S32 thickness;
} APP_OSDC_OBJS_INFO_S;

typedef struct APP_PARAM_OSDC_CFG_T {
    CVI_BOOL enable;
    RGN_HANDLE handle;
    MMF_CHN_S  mmfChn;
    CVI_BOOL bShow;
    CVI_U32 VpssGrp;
    CVI_U32 VpssChn;
    CVI_U32 CompressedSize;
    OSDC_OSD_FORMAT_E format;
    CVI_BOOL bShowPdRect;
    CVI_BOOL bShowMdRect;
    CVI_BOOL bShowFdRect;
    CVI_U32 osdcObjNum;
    APP_OSDC_OBJS_INFO_S osdcObj[OSDC_OBJS_MAX];
} APP_PARAM_OSDC_CFG_S;

APP_PARAM_OSD_CFG_T *app_ipcam_Osd_Param_Get(void);
APP_PARAM_COVER_CFG_T *app_ipcam_Cover_Param_Get(void);
int app_ipcam_Osd_Init(void);
int app_ipcam_Cover_Init(void);
int app_ipcam_Osd_DeInit(void);
int app_ipcam_Cover_DeInit(void);
int app_ipcam_Osd_Bitmap_Update(char *szStr, BITMAP_S *pstBitmap);

APP_PARAM_OSDC_CFG_S *app_ipcam_Osdc_Param_Get(void);
int app_ipcam_Osdc_Init(void);
int app_ipcam_Osdc_DeInit(void);

#ifdef WEB_SOCKET
APP_OSDC_OBJS_INFO_S *app_ipcam_OsdcPrivacy_Param_Get(void);
#endif
/*****************************************************************
 *  The following API for command test used             S
 * **************************************************************/
int app_ipcam_CmdTask_Osd_Switch(CVI_MQ_MSG_t *msg, CVI_VOID *userdate);
int app_ipcam_CmdTask_Cover_Switch(CVI_MQ_MSG_t *msg, CVI_VOID *userdate);
int app_ipcam_CmdTask_Rect_Switch(CVI_MQ_MSG_t *msg, CVI_VOID *userdate);
/*****************************************************************
 *  The above API for command test used                 E
 * **************************************************************/

#ifdef __cplusplus
}
#endif

#endif