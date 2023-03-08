
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <math.h>
#include "cvi_osdc.h"
#include "linux/cvi_comm_video.h"
#include "app_ipcam_osd.h"
#include "app_ipcam_paramparse.h"
#include "app_ipcam_loadbmp.h"
#include "app_ipcam_fontmod.h"
#include "app_ipcam_vpss.h"
#include "errno.h"
#ifdef AI_SUPPORT
#include "app_ipcam_ai.h"
#endif
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define OSD_LIB_FONT_W      24
#define OSD_LIB_FONT_H      24

#define NOASCII_CHARACTER_BYTES 2
#define BYTE_BITS               8
#define ISASCII(a)              (((a) >= 0x00 && (a) <= 0x7F) ? 1 : 0)

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef enum APP_AI_RECT_SHOW_T {
    APP_AI_ALL_RECT_HIDDEN = 0x0,
    APP_AI_PD_RECT_SHOW = 0x01,
    APP_AI_MD_RECT_SHOW = 0x02,
    APP_AI_FD_RECT_SHOW = 0x04,
    APP_AI_ALL_RECT_SHOW = 0x07
} APP_AI_RECT_SHOW_E;

typedef struct APP_OSDC_CANVAS_CFG_T {
    CVI_BOOL createCanvas;
    CVI_U16 drawFlag;
    BITMAP_S rgnBitmap;
    OSDC_Canvas_Attr_S stCanvasAttr;
} APP_OSDC_CANVAS_CFG_S;

#ifdef AI_SUPPORT
typedef struct APP_OSDC_AI_RECT_RATIO_T {
    CVI_S32 VpssChn_W;
    CVI_S32 VpssChn_H;
    float ScaleX;
    float ScaleY;
} APP_OSDC_AI_RECT_RATIO_S;
#endif
/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_OSD_CFG_T g_stOsdCfg, *g_pstOsdCfg = &g_stOsdCfg;
static APP_PARAM_COVER_CFG_T g_stCoverCfg, *g_pstCoverCfg = &g_stCoverCfg;
static APP_PARAM_OSDC_CFG_S g_stOsdcCfg, *g_pstOsdcCfg = &g_stOsdcCfg;

static CVI_BOOL g_bTStamp_Running;
static pthread_t g_TStamp_Thread;

static CVI_BOOL g_bOsdcThreadRun;
static pthread_t g_pthOsdcRgn;
static pthread_mutex_t OsdcMutex = PTHREAD_MUTEX_INITIALIZER;

static APP_OSDC_CANVAS_CFG_S g_stOsdcCanvasCfg = {0};
// static OSDC_DRAW_OBJ_S g_ObjsVec[OSDC_OBJS_MAX] = {0};

#ifdef AI_SUPPORT
APP_OSDC_AI_RECT_RATIO_S g_stPdRectRatio = {0};
APP_OSDC_AI_RECT_RATIO_S g_stMdRectRatio = {0};
APP_OSDC_AI_RECT_RATIO_S g_stFdRectRatio = {0};

static cvai_object_t g_objMetaPd = {0};
static cvai_object_t g_objMetaMd = {0};
#ifdef FACE_SUPPORT
static cvai_face_t g_objMetaFd = {0};
#endif
#endif

#ifdef WEB_SOCKET
static APP_OSDC_OBJS_INFO_S g_stOsdcPrivacy[4];
static APP_OSDC_OBJS_INFO_S *g_pstOsdcPrivacy = &g_stOsdcPrivacy[0];
#endif
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

APP_PARAM_OSD_CFG_T *app_ipcam_Osd_Param_Get(void)
{
    return g_pstOsdCfg;
}

#ifdef WEB_SOCKET
APP_OSDC_OBJS_INFO_S *app_ipcam_OsdcPrivacy_Param_Get(void)
{
    return g_pstOsdcPrivacy;
}
#endif

static CVI_S32 GetNonASCNum(char *string, CVI_S32 len)
{
    CVI_S32 i;
    CVI_S32 n = 0;

    for (i = 0; i < len; i++) {
        if (string[i] == '\0')
            break;
        if (!ISASCII(string[i])) {
            i++;
            n++;
        }
    }

    return n;
}

static CVI_S32 GetFontMod(char *Character, uint8_t **FontMod, CVI_S32 *FontModLen)
{
    CVI_U32 offset = 0;
    CVI_U32 areacode = 0;
    CVI_U32 bitcode = 0;

    if (ISASCII(Character[0])) {
        areacode = 3;
        bitcode = (CVI_U32)((uint8_t)Character[0] - 0x20);
    } else {
        areacode = (CVI_U32)((uint8_t)Character[0] - 0xA0);
        bitcode = (CVI_U32)((uint8_t)Character[1] - 0xA0);
    }
    offset = (94 * (areacode - 1) + (bitcode - 1)) * (OSD_LIB_FONT_W * OSD_LIB_FONT_H / 8);
    *FontMod = (uint8_t *)g_fontLib + offset;
    *FontModLen = OSD_LIB_FONT_W*OSD_LIB_FONT_H / 8;
    return CVI_SUCCESS;
}

static CVI_VOID GetTimeStr(const struct tm *pstTime, char *pazStr, CVI_S32 s32MaxLen)
{
    time_t nowTime;
    struct tm stTime = {0};

    if (!pstTime) {
        time(&nowTime);
        localtime_r(&nowTime, &stTime);
        pstTime = &stTime;
    }

    snprintf(pazStr, s32MaxLen, "%04d-%02d-%02d %02d:%02d:%02d",
        pstTime->tm_year + 1900, pstTime->tm_mon + 1, pstTime->tm_mday,
        pstTime->tm_hour, pstTime->tm_min, pstTime->tm_sec);
}

static CVI_VOID GetDebugStr(char *pazStr, CVI_S32 s32MaxLen)
{
    #ifdef AI_SUPPORT
    if (NULL == pazStr || 0 >= s32MaxLen) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "szStr is NULL or s32MaxLen:%d invalid!\n", s32MaxLen);
        return ;
    }

#ifdef FACE_SUPPORT
    snprintf(pazStr, s32MaxLen, "MD[%02dFPS]PD[%02dFPS]FD[%02dFPS]", 
            app_ipcam_Ai_MD_ProcFps_Get(), app_ipcam_Ai_PD_ProcFps_Get(), app_ipcam_Ai_FD_ProcFps_Get());
#else
    snprintf(pazStr, s32MaxLen, "MD[%02dFPS]PD[%02dFPS]", 
            app_ipcam_Ai_MD_ProcFps_Get(), app_ipcam_Ai_PD_ProcFps_Get());
#endif
    #endif
}

int app_ipcam_Osd_Bitmap_Update(char *szStr, BITMAP_S *pstBitmap)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    if (NULL == szStr || NULL == pstBitmap)
    {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "szStr/pstBitmap is NULL\n");
        return s32Ret;
    }
    CVI_U32 u32CanvasWidth, u32CanvasHeight, u32BgColor, u32Color;
    SIZE_S stFontSize;
    CVI_S32 s32StrLen = strnlen(szStr, APP_OSD_STR_LEN_MAX);
    CVI_S32 NonASCNum = GetNonASCNum(szStr, s32StrLen);

    u32CanvasWidth = OSD_LIB_FONT_W * (s32StrLen - NonASCNum * (NOASCII_CHARACTER_BYTES - 1));
    u32CanvasHeight = OSD_LIB_FONT_H;
    stFontSize.u32Width = OSD_LIB_FONT_W;
    stFontSize.u32Height = OSD_LIB_FONT_H;
    u32BgColor = 0x7fff;
    u32Color = 0xffff;

    if (szStr == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "szStr NULL pointer!\n");
        return CVI_FAILURE;
    }

    uint16_t *puBmData = (uint16_t *)pstBitmap->pData;
    CVI_U32 u32BmRow, u32BmCol;

    for (u32BmRow = 0; u32BmRow < u32CanvasHeight; ++u32BmRow) {
        CVI_S32 NonASCShow = 0;

        for (u32BmCol = 0; u32BmCol < u32CanvasWidth; ++u32BmCol) {
            CVI_S32 s32BmDataIdx = u32BmRow * pstBitmap->u32Width + u32BmCol;
            CVI_S32 s32CharIdx = u32BmCol / stFontSize.u32Width;
            CVI_S32 s32StringIdx = s32CharIdx + NonASCShow * (NOASCII_CHARACTER_BYTES - 1);

            if (NonASCNum > 0 && s32CharIdx > 0) {
                NonASCShow = GetNonASCNum(szStr, s32StringIdx);
                s32StringIdx = s32CharIdx + NonASCShow * (NOASCII_CHARACTER_BYTES - 1);
            }
            CVI_S32 s32CharCol = (u32BmCol - (stFontSize.u32Width * s32CharIdx)) * OSD_LIB_FONT_W /
                            stFontSize.u32Width;
            CVI_S32 s32CharRow = u32BmRow * OSD_LIB_FONT_H / stFontSize.u32Height;
            CVI_S32 s32HexOffset = s32CharRow * OSD_LIB_FONT_W / BYTE_BITS + s32CharCol / BYTE_BITS;
            CVI_S32 s32BitOffset = s32CharCol % BYTE_BITS;
            uint8_t *FontMod = NULL;
            CVI_S32 FontModLen = 0;

            if (GetFontMod(&szStr[s32StringIdx], &FontMod, &FontModLen) == CVI_SUCCESS) {
                if (FontMod != NULL && s32HexOffset < FontModLen) {
                    uint8_t temp = FontMod[s32HexOffset];

                    if ((temp >> ((BYTE_BITS - 1) - s32BitOffset)) & 0x1)
                        puBmData[s32BmDataIdx] = (uint16_t)u32Color;
                    else
                        puBmData[s32BmDataIdx] = (uint16_t)u32BgColor;
                    continue;
                }
            }
            APP_PROF_LOG_PRINT(LEVEL_INFO, "GetFontMod Fail\n");
            return CVI_FAILURE;
        }
    }

    return s32Ret;
}

static CVI_VOID *Thread_TimestampUpdate(CVI_VOID *arg)
{
    CVI_S32 s32Ret;
    APP_PARAM_OSD_CFG_T *pstOsdCfg = (APP_PARAM_OSD_CFG_T *)arg;
    RGN_ATTR_S stRegion;
    BITMAP_S stBitmap;
    char szStr[APP_OSD_STR_LEN_MAX];
    CVI_CHAR TaskName[64] = {'\0'};

    sprintf(TaskName, "T-Stamp_Update");
    prctl(PR_SET_NAME, TaskName, 0, 0, 0);

    CVI_U32 SleepCnt = 0;

    APP_PROF_LOG_PRINT(LEVEL_INFO, "TimestampUpdate g_bTStamp_Running = %d\n", g_bTStamp_Running);

    while (g_bTStamp_Running) {
        if (SleepCnt != 100) {
            SleepCnt++;
            usleep(10*1000);
            continue;
        }
        for (CVI_S32 i = 0; i < pstOsdCfg->osd_cnt; i++) {
            if ((pstOsdCfg->astOsdAttr[i].enType == TYPE_TIME) ||
                (pstOsdCfg->astOsdAttr[i].enType == TYPE_DEBUG)) {

                memset(&stRegion, 0, sizeof(RGN_ATTR_S));
                memset(&stBitmap, 0, sizeof(BITMAP_S));
                memset(szStr, 0, APP_OSD_STR_LEN_MAX);
                if (CVI_RGN_GetAttr(pstOsdCfg->astOsdAttr[i].Handle, &stRegion) != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_GetAttr failed!\n");
                    return NULL;
                }
                stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
                stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
                stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
                if (pstOsdCfg->astOsdAttr[i].enType == TYPE_DEBUG) {
                    GetDebugStr(szStr, DEBUG_STR_LEN);
                } else {
                    GetTimeStr(NULL, szStr, APP_OSD_STR_LEN_MAX);
                }

                stBitmap.pData = malloc(2 * (stBitmap.u32Width) * (stBitmap.u32Height));
                if (stBitmap.pData == NULL) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc osd memroy err!\n");
                    return NULL;
                }
                memset(stBitmap.pData, 0, (2 * (stBitmap.u32Width) * (stBitmap.u32Height)));

                s32Ret = app_ipcam_Osd_Bitmap_Update(szStr, &stBitmap);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Osd_Bitmap_Update failed!\n");
                    return NULL;
                }

                s32Ret = CVI_RGN_SetBitMap(pstOsdCfg->astOsdAttr[i].Handle, &stBitmap);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
                    free(stBitmap.pData);
                    return NULL;
                }

                free(stBitmap.pData);
            }
        }
        SleepCnt = 0;
    }
    return NULL;
}

CVI_S32 app_ipcam_Rgn_Mst_LoadBmp(
    const char *filename, 
    BITMAP_S *pstBitmap, 
    CVI_BOOL bFil, 
    CVI_U32 u16FilColor,
    PIXEL_FORMAT_E enPixelFormat)
{
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;
    CVI_S32 Bpp;
    CVI_U32 nColors;
    CVI_U32 u32PdataSize;

    if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0) {
        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "GetBmpInfo err!\n");
        return CVI_FAILURE;
    }

    Bpp = bmpInfo.bmiHeader.biBitCount / 8;
    nColors = 0;
    if (Bpp == 1) {
        if (bmpInfo.bmiHeader.biClrUsed == 0)
            nColors = 1 << bmpInfo.bmiHeader.biBitCount;
        else
            nColors = bmpInfo.bmiHeader.biClrUsed;

        if (nColors > 256) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Number of indexed palette is over 256.");
            return CVI_FAILURE;
        }
    }

    if (enPixelFormat == PIXEL_FORMAT_ARGB_1555) {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;
    } else if (enPixelFormat == PIXEL_FORMAT_ARGB_4444) {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB4444;
    } else if (enPixelFormat == PIXEL_FORMAT_ARGB_8888) {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB8888;
    } else if (enPixelFormat == PIXEL_FORMAT_8BIT_MODE) {
        Surface.enColorFmt = OSD_COLOR_FMT_8BIT_MODE;
    }  else {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Pixel format is not support!\n");
        return CVI_FAILURE;
    }

    u32PdataSize = Bpp * (bmpInfo.bmiHeader.biWidth) * (bmpInfo.bmiHeader.biHeight);
    pstBitmap->pData = malloc(u32PdataSize);
    if (pstBitmap->pData == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc osd memory err!\n");
        return CVI_FAILURE;
    }

   if (0 != CreateSurfaceByBitMap(filename, &Surface, (CVI_U8 *)(pstBitmap->pData))) {
       APP_PROF_LOG_PRINT(LEVEL_ERROR, "CreateSurfaceByBitMap failed!\n");
       return CVI_FAILURE;
   }

    pstBitmap->u32Width = Surface.u16Width;
    pstBitmap->u32Height = Surface.u16Height;
    pstBitmap->enPixelFormat = enPixelFormat;

    // if pixel value match color, make it transparent.
    // Only works for ARGB1555
    if (bFil) {
        CVI_U32 i, j;
        CVI_U16 *pu16Temp;

        pu16Temp = (CVI_U16 *)pstBitmap->pData;
        for (i = 0; i < pstBitmap->u32Height; i++) {
            for (j = 0; j < pstBitmap->u32Width; j++) {
                if (u16FilColor == *pu16Temp)
                    *pu16Temp &= 0x7FFF;

                pu16Temp++;
            }
        }
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Rgn_Mst_Canvas_Update(
    const char *filename, 
    BITMAP_S *pstBitmap, 
    CVI_BOOL bFil, 
    CVI_U32 u16FilColor,
    SIZE_S *pstSize, 
    CVI_U32 u32Stride, 
    PIXEL_FORMAT_E enPixelFormat)
{
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;

    if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0) {
        APP_PROF_LOG_PRINT(LEVEL_WARN,"GetBmpInfo err!\n");
        return CVI_FAILURE;
    }

    if (enPixelFormat == PIXEL_FORMAT_ARGB_1555) {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;
    } else if (enPixelFormat == PIXEL_FORMAT_ARGB_4444) {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB4444;
    } else if (enPixelFormat == PIXEL_FORMAT_ARGB_8888) {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB8888;
    } else {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "Pixel format is not support!\n");
        return CVI_FAILURE;
    }

    if (pstBitmap->pData == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"malloc osd memroy err!\n");
        return CVI_FAILURE;
    }

    CreateSurfaceByCanvas(filename, &Surface, (CVI_U8 *)(pstBitmap->pData)
                , pstSize->u32Width, pstSize->u32Height, u32Stride);

    pstBitmap->u32Width = Surface.u16Width;
    pstBitmap->u32Height = Surface.u16Height;
    pstBitmap->enPixelFormat = enPixelFormat;

    // if pixel value match color, make it transparent.
    // Only works for ARGB1555
    if (bFil) {
        CVI_U32 i, j;
        CVI_U16 *pu16Temp;

        pu16Temp = (CVI_U16 *)pstBitmap->pData;
        for (i = 0; i < pstBitmap->u32Height; i++) {
            for (j = 0; j < pstBitmap->u32Width; j++) {
                if (u16FilColor == *pu16Temp)
                    *pu16Temp &= 0x7FFF;

                pu16Temp++;
            }
        }
    }

    return CVI_SUCCESS;
}

CVI_S32 app_ipcam_Rgn_Canvas_Update(RGN_HANDLE Handle, const char *filename)
{
    CVI_S32 s32Ret;
    SIZE_S stSize;
    BITMAP_S stBitmap;
    RGN_CANVAS_INFO_S stCanvasInfo;

    s32Ret = CVI_RGN_GetCanvasInfo(Handle, &stCanvasInfo);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
        return CVI_FAILURE;
    }

    // stBitmap.pData = stCanvasInfo.pu8VirtAddr;
    stSize.u32Width = stCanvasInfo.stSize.u32Width;
    stSize.u32Height = stCanvasInfo.stSize.u32Height;

    stBitmap.pData = CVI_SYS_Mmap(stCanvasInfo.u64PhyAddr, stCanvasInfo.u32Stride * stCanvasInfo.stSize.u32Height);
    if (stBitmap.pData == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "stBitmap.pData == NULL!\n");
    }

    app_ipcam_Rgn_Mst_Canvas_Update(filename, &stBitmap, CVI_FALSE, 0, 
            &stSize, stCanvasInfo.u32Stride, PIXEL_FORMAT_ARGB_1555);

    s32Ret = CVI_RGN_UpdateCanvas(Handle);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
        CVI_SYS_Munmap(stBitmap.pData, stCanvasInfo.u32Stride * stCanvasInfo.stSize.u32Height);
        return CVI_FAILURE;
    }
    
    CVI_SYS_Munmap(stBitmap.pData, stCanvasInfo.u32Stride * stCanvasInfo.stSize.u32Height);

    return s32Ret;
}

static CVI_S32 app_ipcam_Osd_Destroy(APP_PARAM_OSD_ATTR_S *pstOsdAttr)
{
    CVI_S32 s32Ret;
    RGN_HANDLE Handle = pstOsdAttr->Handle;
    MMF_CHN_S stChn = pstOsdAttr->stChn;

    s32Ret = CVI_RGN_DetachFromChn(Handle, &stChn);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "OSD handle(%d) CVI_RGN_DetachFromChn failed with %#x!\n", Handle, s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_RGN_Destroy(Handle);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "OSD handle(%d) CVI_RGN_Destroy failed with %#x!\n", Handle, s32Ret);
        return s32Ret;
    }

    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_Osd_Create(CVI_BOOL bOsdGloablEnable, APP_PARAM_OSD_ATTR_S *pstOsdAttr)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    RGN_ATTR_S stRegion;
    CVI_S32 s32StrLen;
    char *pszStr = NULL;
    char szStr[APP_OSD_STR_LEN_MAX] = {0};
    BITMAP_S stBitmap;
    RGN_HANDLE Handle = pstOsdAttr->Handle;
    OSD_TYPE_E enType = pstOsdAttr->enType;
    MMF_CHN_S *pstChn = &pstOsdAttr->stChn;

    APP_PROF_LOG_PRINT(LEVEL_DEBUG, "Handle=%d enType=%d stChn=%d DevId=%d ChnId=%d\n", 
        Handle, enType, pstChn->enModId, pstChn->s32DevId, pstChn->s32ChnId);
    switch (enType) {
        case TYPE_PICTURE:
            stRegion.unAttr.stOverlay.stSize.u32Height = pstOsdAttr->stRect.u32Height;
            stRegion.unAttr.stOverlay.stSize.u32Width = pstOsdAttr->stRect.u32Width;
        break;

        case TYPE_STRING:
            s32StrLen = strlen(pstOsdAttr->str) - GetNonASCNum(pstOsdAttr->str, strlen(pstOsdAttr->str));
            pszStr = pstOsdAttr->str;
            stRegion.unAttr.stOverlay.stSize.u32Height = OSD_LIB_FONT_H;
            stRegion.unAttr.stOverlay.stSize.u32Width = OSD_LIB_FONT_W * s32StrLen;
        break;

        case TYPE_TIME:
            GetTimeStr(NULL, szStr, APP_OSD_STR_LEN_MAX);
            s32StrLen = strnlen(szStr, APP_OSD_STR_LEN_MAX);
            pszStr = szStr;
            stRegion.unAttr.stOverlay.stSize.u32Height = OSD_LIB_FONT_H;
            stRegion.unAttr.stOverlay.stSize.u32Width = OSD_LIB_FONT_W * s32StrLen;
        break;

        case TYPE_DEBUG:
            GetDebugStr(szStr, APP_OSD_STR_LEN_MAX);
            s32StrLen = strnlen(szStr, APP_OSD_STR_LEN_MAX);
            stRegion.unAttr.stOverlay.stSize.u32Height = OSD_LIB_FONT_H;
            stRegion.unAttr.stOverlay.stSize.u32Width = OSD_LIB_FONT_W * s32StrLen;
        break;

        default:
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "OSD enType = %d invalid \n", enType);
        break;
    }

    stRegion.enType = OVERLAY_RGN;
    stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
    stRegion.unAttr.stOverlay.u32BgColor = 0x00000000; // ARGB1555 transparent
    stRegion.unAttr.stOverlay.u32CanvasNum = 2;
    stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE; // MARS only
    s32Ret = CVI_RGN_Create(Handle, &stRegion);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_Create failed with %#x!\n", s32Ret);
        return CVI_FAILURE;
    }

    RGN_CHN_ATTR_S stChnAttr = {0};
    stChnAttr.bShow = bOsdGloablEnable ? pstOsdAttr->bShow : CVI_FALSE;
    stChnAttr.enType = OVERLAY_RGN;
    stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = pstOsdAttr->stRect.s32X;
    stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = pstOsdAttr->stRect.s32Y;
    stChnAttr.unChnAttr.stOverlayChn.u32Layer = 0;
    s32Ret = CVI_RGN_AttachToChn(Handle, pstChn, &stChnAttr);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
        goto EXIT0;
    }

    switch (enType) {
        case TYPE_PICTURE:
            s32Ret = app_ipcam_Rgn_Canvas_Update(Handle, pstOsdAttr->filename);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Rgn_Canvas_Update failed!\n");
                goto EXIT1;
            }
        break;

        case TYPE_STRING:
        case TYPE_TIME:
        case TYPE_DEBUG:
            stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
            stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
            stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;

            stBitmap.pData = malloc(2 * (stBitmap.u32Width) * (stBitmap.u32Height));
            if (stBitmap.pData == NULL) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc osd memroy err!\n");
                return CVI_FAILURE;
            }
            memset(stBitmap.pData, 0, (2 * (stBitmap.u32Width) * (stBitmap.u32Height)));

            s32Ret = app_ipcam_Osd_Bitmap_Update(pszStr, &stBitmap);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Osd_Bitmap_Update failed!\n");
                return s32Ret;
            }

            s32Ret = CVI_RGN_SetBitMap(Handle, &stBitmap);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
                free(stBitmap.pData);
                return s32Ret;
            }

            free(stBitmap.pData);
        break;

        default :
        break;
    }

    return CVI_SUCCESS;

EXIT1:
    APP_CHK_RET(CVI_RGN_DetachFromChn(Handle, pstChn), "CVI_RGN_DetachFromChn");

EXIT0:
    APP_CHK_RET(CVI_RGN_Destroy(Handle), "CVI_RGN_Destroy");

    return s32Ret;
}

CVI_S32 app_ipcam_Osd_Init(void)
{
    CVI_S32 s32Ret;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "osd init ------------------> start \n");

    CVI_S32 i = 0;
    CVI_BOOL Timestamp = CVI_FALSE;

    APP_PROF_LOG_PRINT(LEVEL_INFO,"OSD handle count=%d\n", g_pstOsdCfg->osd_cnt);

    for (; i < g_pstOsdCfg->osd_cnt; i++) {
        s32Ret = app_ipcam_Osd_Create(g_pstOsdCfg->bEnable, &g_pstOsdCfg->astOsdAttr[i]);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "OSD handle_%d create failed!\n", i);
            goto EXIT;
        }

        if ((!Timestamp) &&
            (((g_pstOsdCfg->astOsdAttr[i].enType == TYPE_TIME) ||
            (g_pstOsdCfg->astOsdAttr[i].enType == TYPE_DEBUG)))) {
            Timestamp = CVI_TRUE;
        }
    }
    APP_PROF_LOG_PRINT(LEVEL_TRACE,"Timestamp enable = %d\n", Timestamp);
    if (Timestamp) {
        g_bTStamp_Running = CVI_TRUE;
        s32Ret = pthread_create(
                    &g_TStamp_Thread,
                    NULL,
                    Thread_TimestampUpdate,
                    (CVI_VOID *)g_pstOsdCfg);
        if (s32Ret != 0) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "pthread_create failed!\n");
            goto EXIT;
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "osd init ------------------> done \n");

    return CVI_SUCCESS;

EXIT:
    for (; i > 0; i--) {
        app_ipcam_Osd_Destroy(&g_pstOsdCfg->astOsdAttr[i]);
    }

    return s32Ret;
}

int app_ipcam_Osd_DeInit(void)
{
    CVI_S32 s32Ret;

    CVI_BOOL Timestamp = CVI_FALSE;

    APP_PROF_LOG_PRINT(LEVEL_INFO, "osd DeInit handle cnt=(%d)----> start \n", g_pstOsdCfg->osd_cnt);

    for (CVI_S32 i = 0; i < g_pstOsdCfg->osd_cnt; i++) {
        if (((g_pstOsdCfg->astOsdAttr[i].enType == TYPE_TIME) ||
            (g_pstOsdCfg->astOsdAttr[i].enType == TYPE_DEBUG))) {
            Timestamp = CVI_TRUE;
            break;
        }
    }

    if (Timestamp) {
        g_bTStamp_Running = CVI_FALSE;
        if (g_TStamp_Thread > (pthread_t)0) {
            pthread_join(g_TStamp_Thread, NULL);
            g_TStamp_Thread = 0;
        }
    }

    for (CVI_S32 i = 0; i < g_pstOsdCfg->osd_cnt; i++) {
        s32Ret = app_ipcam_Osd_Destroy(&g_pstOsdCfg->astOsdAttr[i]);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Osd_Destroy handle[%d] failed with 0x%x!\n", i, s32Ret);
            return s32Ret;
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "osd DeInit ------------------> done \n");
    return CVI_SUCCESS;
}

APP_PARAM_COVER_CFG_T *app_ipcam_Cover_Param_Get(void)
{
    return g_pstCoverCfg;
}

static int app_ipcam_Cover_Destroy(APP_PARAM_COVER_ATTR_S *pstCoverAttr)
{
    CVI_S32 s32Ret;

    s32Ret = CVI_RGN_DetachFromChn(pstCoverAttr->Handle, &pstCoverAttr->stChn);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_DetachFromChn failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_RGN_Destroy(pstCoverAttr->Handle);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_Destroy failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    return CVI_SUCCESS;
}

static int app_ipcam_Cover_Create(VPSS_MODE_E VpssMode, APP_PARAM_COVER_ATTR_S *pstCoverAttr)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    
    RGN_HANDLE Handle = pstCoverAttr->Handle;

    RGN_ATTR_S stRegion = {0};
    if (VpssMode == VPSS_MODE_RGNEX) {
        stRegion.enType = COVEREX_RGN;
    } else {
        stRegion.enType = COVER_RGN;
    }
    s32Ret = CVI_RGN_Create(Handle, &stRegion);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_Create handle(%d) failed with %#x!\n", Handle, s32Ret);
        return s32Ret;
    }

    RGN_CHN_ATTR_S stChnAttr = {0};
    stChnAttr.bShow = pstCoverAttr->bShow;

    if (VpssMode == VPSS_MODE_RGNEX) {
        stChnAttr.enType = COVEREX_RGN;
        stChnAttr.unChnAttr.stCoverExChn.enCoverType      = AREA_RECT;
        stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = pstCoverAttr->stRect.s32X;
        stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = pstCoverAttr->stRect.s32Y;
        stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = pstCoverAttr->stRect.u32Width;
        stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = pstCoverAttr->stRect.u32Height;
        stChnAttr.unChnAttr.stCoverExChn.u32Color         = pstCoverAttr->u32Color;
        stChnAttr.unChnAttr.stCoverExChn.u32Layer         = 0;
    } else {
        stChnAttr.enType = COVER_RGN;
        stChnAttr.unChnAttr.stCoverChn.enCoverType      = AREA_RECT;
        stChnAttr.unChnAttr.stCoverChn.enCoordinate     = RGN_ABS_COOR;
        stChnAttr.unChnAttr.stCoverChn.stRect.s32X      = pstCoverAttr->stRect.s32X;
        stChnAttr.unChnAttr.stCoverChn.stRect.s32Y      = pstCoverAttr->stRect.s32Y;
        stChnAttr.unChnAttr.stCoverChn.stRect.u32Width  = pstCoverAttr->stRect.u32Width;
        stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = pstCoverAttr->stRect.u32Height;
        stChnAttr.unChnAttr.stCoverChn.u32Color         = pstCoverAttr->u32Color;
        stChnAttr.unChnAttr.stCoverChn.u32Layer         = 0;
    }

    MMF_CHN_S stChn = {0};
    stChn.enModId  = pstCoverAttr->stChn.enModId;
    stChn.s32DevId = pstCoverAttr->stChn.s32DevId;
    stChn.s32ChnId = pstCoverAttr->stChn.s32ChnId;

    s32Ret = CVI_RGN_AttachToChn(Handle, &stChn, &stChnAttr);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
        goto EXIT0;
    }

    return CVI_SUCCESS;

EXIT0:
    s32Ret = CVI_RGN_Destroy(Handle);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_Destroy handle(%d) failed with %#x!\n", Handle, s32Ret);
        return s32Ret;
    }

    return s32Ret;

}

int app_ipcam_Cover_DeInit(void)
{
    CVI_S32 s32Ret;

    for (CVI_S32 i = 0; i < g_pstCoverCfg->Cover_cnt; i++) {
        APP_PARAM_COVER_ATTR_S *pstCoverAttr = &g_pstCoverCfg->astCoverAttr[i];
        s32Ret = CVI_RGN_DetachFromChn(pstCoverAttr->Handle, &pstCoverAttr->stChn);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_DetachFromChn failed with %#x!\n", s32Ret);
            return s32Ret;
        }

        s32Ret = CVI_RGN_Destroy(pstCoverAttr->Handle);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_Destroy failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }

    return CVI_SUCCESS;

}

int app_ipcam_Cover_Init(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "cover init ------------------> start \n");

    CVI_S32 i = 0;

    VPSS_MODE_E VpssMode = CVI_SYS_GetVPSSMode();

    APP_PROF_LOG_PRINT(LEVEL_INFO,"Cover handle count=%d VpssMode=%d\n", g_pstCoverCfg->Cover_cnt, VpssMode);
    for (; i < g_pstCoverCfg->Cover_cnt; i++) {
        s32Ret = app_ipcam_Cover_Create(VpssMode, &g_pstCoverCfg->astCoverAttr[i]);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "Cover handle_%d create failed!\n", i);
            goto EXIT;
        }
    }

    APP_PROF_LOG_PRINT(LEVEL_INFO, "cover init ------------------> done \n");

    return CVI_SUCCESS;

EXIT:
    for (; i > 0; i--) {
        s32Ret = app_ipcam_Cover_Destroy(&g_pstCoverCfg->astCoverAttr[i]);
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Cover_Destroy failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }   

    return s32Ret;

}

APP_PARAM_OSDC_CFG_S *app_ipcam_Osdc_Param_Get(void)
{
    return g_pstOsdcCfg;
}

CVI_S32 app_ipcam_OSDCRgn_Create(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    int iOsdcIndex = 0;
    for (iOsdcIndex = 0; iOsdcIndex < OSDC_NUM_MAX; iOsdcIndex++) {
        if (g_pstOsdcCfg->bShow[iOsdcIndex]) {
            MMF_CHN_S *mmfChn = &g_pstOsdcCfg->mmfChn[iOsdcIndex];
            RGN_HANDLE handle = g_pstOsdcCfg->handle[iOsdcIndex];
            CVI_U32 u32CpsSize = g_pstOsdcCfg->CompressedSize[iOsdcIndex];

            VPSS_CHN_ATTR_S *pVpssChnAttr = &app_ipcam_Vpss_Param_Get()->astVpssGrpCfg[mmfChn->s32DevId].astVpssChnAttr[mmfChn->s32ChnId];
            CVI_U32 u32Width = pVpssChnAttr->u32Width;
            CVI_U32 u32Height = pVpssChnAttr->u32Height;
            APP_PROF_LOG_PRINT(LEVEL_INFO, "OSDC RGN handle(%d) RGN size W:%d H:%d\n", handle, u32Width, u32Height);

            RGN_ATTR_S regAttr;
            memset(&regAttr, 0, sizeof(regAttr));
            regAttr.enType = OVERLAY_RGN;
            regAttr.unAttr.stOverlay.enPixelFormat = (PIXEL_FORMAT_E)g_pstOsdcCfg->format[iOsdcIndex];
            regAttr.unAttr.stOverlay.stSize.u32Width = u32Width;
            regAttr.unAttr.stOverlay.stSize.u32Height = u32Height;
            regAttr.unAttr.stOverlay.u32BgColor = 0x00000000; // ARGB1555 transparent
            regAttr.unAttr.stOverlay.u32CanvasNum = 2;
            regAttr.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_HW;
            regAttr.unAttr.stOverlay.stCompressInfo.u32CompressedSize = u32CpsSize;
            s32Ret = CVI_RGN_Create(handle, &regAttr);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_RGN_Create failed with %#x, hdl(%d)\n", s32Ret, handle);
                return s32Ret;
            }

            CVI_BOOL bShow = g_pstOsdcCfg->bShow[iOsdcIndex];
            RGN_CHN_ATTR_S regChnAttr;
            memset(&regChnAttr, 0, sizeof(regChnAttr));
            regChnAttr.bShow = bShow;
            regChnAttr.enType = OVERLAY_RGN;
            regChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = CVI_FALSE;
            regChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
            regChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
            regChnAttr.unChnAttr.stOverlayChn.u32Layer = 0;
            s32Ret = CVI_RGN_AttachToChn(handle, mmfChn, &regChnAttr);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_RGN_AttachToChn failed with %#x, hdl(%d), chn(%d %d %d)\n",
                           s32Ret, handle, mmfChn->enModId, mmfChn->s32DevId, mmfChn->s32ChnId);
                CVI_RGN_Destroy(handle);
            }
        }
    }

    return s32Ret;
}

CVI_S32 app_ipcam_OSDCRgn_Destory(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    int iOsdcIndex = 0;
    for (iOsdcIndex = 0; iOsdcIndex < OSDC_NUM_MAX; iOsdcIndex++) {
        if (g_pstOsdcCfg->bShow[iOsdcIndex]) {
            RGN_HANDLE handle = g_pstOsdcCfg->handle[iOsdcIndex];
            MMF_CHN_S *mmfChn = &g_pstOsdcCfg->mmfChn[iOsdcIndex];

            s32Ret = CVI_RGN_DetachFromChn(handle, mmfChn);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI_RGN_DetachFromChn failedwith %#x, hdl(%d), chn(%d %d %d)\n",
                           s32Ret, handle, mmfChn->enModId, mmfChn->s32DevId, mmfChn->s32ChnId);
                return CVI_FAILURE;
            }

            s32Ret = CVI_RGN_Destroy(handle);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR,"app_ipcam_OSDCRgn_Destory failed with %#x, hdl(%d)\n", s32Ret, handle);
                return CVI_FAILURE;
            }
        }
    }
    
    return s32Ret;
}

/**
 * color format: RGB8888
 * 0xffffffff   white
 * 0xff000000   black
 * 0xff0000ff   blue
 * 0xff00ff00   green
 * 0xffff0000   red
 * 0xff00ffff   cyan
 * 0xffffff00   yellow
 * 0xffff00ff   carmine
 */
static int app_ipcam_ObjsRectInfo_Update(RGN_HANDLE OsdcHandle, int iOsdcIndex)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_U32 i = 0;
    CVI_U32 OsdcObjsNum = 0;
    CVI_S32 s32StrLen = 0;
    CVI_S32 s32DataLen = 0;
    char *pszStr = NULL;
    char szStr[APP_OSD_STR_LEN_MAX] = {0};

    BITMAP_S stBitmap;
    RGN_CANVAS_INFO_S stCanvasInfo = {0};
    s32Ret = CVI_RGN_GetCanvasInfo(OsdcHandle, &stCanvasInfo);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI RGN GetCanvasInfo failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr = stCanvasInfo.pstCanvasCmprAttr;
    RGN_CMPR_OBJ_ATTR_S *pstObjAttr = stCanvasInfo.pstObjAttr;
    
    #ifdef AI_SUPPORT
    if (iOsdcIndex == 0 && g_pstOsdcCfg->bShowPdRect[iOsdcIndex]) {
        app_ipcam_Ai_PD_ObjDrawInfo_Get(&g_objMetaPd);
        if (g_objMetaPd.size > 0) {
            for (i = 0; i < g_objMetaPd.size; i++) {
                if (OsdcObjsNum >= OSDC_OBJS_MAX) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "OsdcObjsNum(%d) > OSDC_OBJS_MAX(%d)!\n", OsdcObjsNum, OSDC_OBJS_MAX);
                    break;
                }

                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32X = (int)(g_stPdRectRatio.ScaleX * g_objMetaPd.info[i].bbox.x1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32Y = (int)(g_stPdRectRatio.ScaleY * g_objMetaPd.info[i].bbox.y1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Width = g_stPdRectRatio.ScaleX * (g_objMetaPd.info[i].bbox.x2 - g_objMetaPd.info[i].bbox.x1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Height = g_stPdRectRatio.ScaleY * (g_objMetaPd.info[i].bbox.y2 - g_objMetaPd.info[i].bbox.y1);
                pstObjAttr[OsdcObjsNum].stRgnRect.u32Thick = 4;
                if(!strcmp(g_objMetaPd.info[i].name, "Intrusion")){
                    pstObjAttr[OsdcObjsNum].stRgnRect.u32Color = COLOR_RED(0);
                }
                else{
                    pstObjAttr[OsdcObjsNum].stRgnRect.u32Color = COLOR_CYAN(0);
                }
                pstObjAttr[OsdcObjsNum].stRgnRect.u32IsFill = CVI_FALSE;
                pstObjAttr[OsdcObjsNum].enObjType = RGN_CMPR_RECT;

                OsdcObjsNum++;
            }
            CVI_AI_Free(&g_objMetaPd);
        }
    }

    if (iOsdcIndex == 0 && g_pstOsdcCfg->bShowMdRect[iOsdcIndex]) {
        app_ipcam_Ai_MD_ObjDrawInfo_Get(&g_objMetaMd);
        if (g_objMetaMd.size > 0) {
            for (i = 0; i < g_objMetaMd.size; i++) {
                if (OsdcObjsNum >= OSDC_OBJS_MAX) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "OsdcObjsNum(%d) > OSDC_OBJS_MAX(%d)!\n", OsdcObjsNum, OSDC_OBJS_MAX);
                    break;
                }

                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32X = (int)(g_stMdRectRatio.ScaleX * g_objMetaMd.info[i].bbox.x1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32Y = (int)(g_stMdRectRatio.ScaleY * g_objMetaMd.info[i].bbox.y1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Width = g_stMdRectRatio.ScaleX * (g_objMetaMd.info[i].bbox.x2 - g_objMetaMd.info[i].bbox.x1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Height = g_stMdRectRatio.ScaleY * (g_objMetaMd.info[i].bbox.y2 - g_objMetaMd.info[i].bbox.y1);
                pstObjAttr[OsdcObjsNum].stRgnRect.u32Thick = 4;
                pstObjAttr[OsdcObjsNum].stRgnRect.u32Color = COLOR_YELLOW(0);
                pstObjAttr[OsdcObjsNum].stRgnRect.u32IsFill = CVI_FALSE;
                pstObjAttr[OsdcObjsNum].enObjType = RGN_CMPR_RECT;

                OsdcObjsNum++;
            }
            CVI_AI_Free(&g_objMetaMd);
        }
    }

#ifdef FACE_SUPPORT
    if (iOsdcIndex == 0 && g_pstOsdcCfg->bShowFdRect[iOsdcIndex]) {
        app_ipcam_Ai_FD_ObjDrawInfo_Get(&g_objMetaFd);
        if (g_objMetaFd.size > 0) {
            for (i = 0; i < g_objMetaFd.size; i++) {
                if (OsdcObjsNum >= OSDC_OBJS_MAX) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "OsdcObjsNum(%d) > OSDC_OBJS_MAX(%d)!\n", OsdcObjsNum, OSDC_OBJS_MAX);
                    return CVI_FAILURE;
                }

                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32X = (int)(g_stFdRectRatio.ScaleX * g_objMetaFd.info[i].bbox.x1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32Y = (int)(g_stFdRectRatio.ScaleY * g_objMetaFd.info[i].bbox.y1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Width = g_stFdRectRatio.ScaleX * (g_objMetaFd.info[i].bbox.x2 - g_objMetaFd.info[i].bbox.x1);
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Height = g_stFdRectRatio.ScaleY * (g_objMetaFd.info[i].bbox.y2 - g_objMetaFd.info[i].bbox.y1);
                pstObjAttr[OsdcObjsNum].stRgnRect.u32Thick = 4;
                pstObjAttr[OsdcObjsNum].stRgnRect.u32Color = COLOR_RED(0);
                pstObjAttr[OsdcObjsNum].stRgnRect.u32IsFill = CVI_FALSE;
                pstObjAttr[OsdcObjsNum].enObjType = RGN_CMPR_RECT;

                OsdcObjsNum++;
            }
            CVI_AI_Free(&g_objMetaFd);
        }
    }
#endif
    #endif

    for (i = 0; i < g_pstOsdcCfg->osdcObjNum[iOsdcIndex]; i++) {
        if (OsdcObjsNum >= OSDC_OBJS_MAX) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "OsdcObjsNum(%d) > OSDC_OBJS_MAX(%d)!\n", OsdcObjsNum, OSDC_OBJS_MAX);
            break;
        }
        if (!g_pstOsdcCfg->osdcObj[iOsdcIndex][i].bShow) {
            continue;
        }

        pstObjAttr[OsdcObjsNum].enObjType = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].type;
        OSD_TYPE_E enType = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].enType;
        if (pstObjAttr[OsdcObjsNum].enObjType == RGN_CMPR_BIT_MAP) {
            memset(&stBitmap, 0, sizeof(BITMAP_S));
            switch (enType) {
                case TYPE_PICTURE:
                    s32Ret = app_ipcam_Rgn_Mst_LoadBmp(g_pstOsdcCfg->osdcObj[iOsdcIndex][i].filename, &stBitmap, CVI_FALSE, 0, PIXEL_FORMAT_ARGB_1555);
                    if (s32Ret != CVI_SUCCESS) {
                        APP_PROF_LOG_PRINT(LEVEL_DEBUG, "app_ipcam_Rgn_Mst_LoadBmp failed with %#x!\n", s32Ret);
                        continue;
                    }
                break;

                case TYPE_STRING:
                    s32StrLen = strlen(g_pstOsdcCfg->osdcObj[iOsdcIndex][i].str) - GetNonASCNum(g_pstOsdcCfg->osdcObj[iOsdcIndex][i].str, strlen(g_pstOsdcCfg->osdcObj[iOsdcIndex][i].str));
                    pszStr = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].str;
                    stBitmap.u32Width = OSD_LIB_FONT_W * s32StrLen;
                    stBitmap.u32Height = OSD_LIB_FONT_H;
                break;

                case TYPE_TIME:
                    memset(szStr, 0, APP_OSD_STR_LEN_MAX);
                    GetTimeStr(NULL, szStr, APP_OSD_STR_LEN_MAX);
                    s32StrLen = strnlen(szStr, APP_OSD_STR_LEN_MAX);
                    pszStr = szStr;
                    stBitmap.u32Width = OSD_LIB_FONT_W * s32StrLen;
                    stBitmap.u32Height = OSD_LIB_FONT_H;
                break;

                case TYPE_DEBUG:
                    GetDebugStr(szStr, APP_OSD_STR_LEN_MAX);
                    s32StrLen = strnlen(szStr, APP_OSD_STR_LEN_MAX);
                    pszStr = szStr;
                    stBitmap.u32Width = OSD_LIB_FONT_W * s32StrLen;
                    stBitmap.u32Height = OSD_LIB_FONT_H;
                break;

                default:
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "OSDC enType = %d invalid \n", enType);
                break;
            }

            s32DataLen = 2 * (stBitmap.u32Width) * (stBitmap.u32Height);
            if (s32DataLen == 0) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "enType %d s32DataLen invalid!\n", enType);
                continue;
            }

            if (TYPE_PICTURE != enType) {
                stBitmap.pData = malloc(s32DataLen);
                if (stBitmap.pData == NULL) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc osd memroy err!\n");
                    return -1;
                }
                memset(stBitmap.pData, 0, s32DataLen);
                s32Ret = app_ipcam_Osd_Bitmap_Update(pszStr, &stBitmap);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Osd_Bitmap_Update failed!\n");
                    free(stBitmap.pData);
                    return -1;
                }
            }

            s32Ret = CVI_SYS_IonAlloc(&g_pstOsdcCfg->osdcObj[iOsdcIndex][i].u64BitmapPhyAddr, (CVI_VOID **)&g_pstOsdcCfg->osdcObj[iOsdcIndex][i].pBitmapVirAddr,
                "rgn_cmpr_bitmap2", s32DataLen);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_SYS_IonAlloc failed with %#x!\n", s32Ret);
                free(stBitmap.pData);
                return -1;
            }
            memcpy(g_pstOsdcCfg->osdcObj[iOsdcIndex][i].pBitmapVirAddr, stBitmap.pData, s32DataLen);
            pstObjAttr[OsdcObjsNum].stBitmap.stRect.s32X = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].x1;
            pstObjAttr[OsdcObjsNum].stBitmap.stRect.s32Y = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].y1;
            pstObjAttr[OsdcObjsNum].stBitmap.stRect.u32Width = stBitmap.u32Width;
            pstObjAttr[OsdcObjsNum].stBitmap.stRect.u32Height = stBitmap.u32Height;
            pstObjAttr[OsdcObjsNum].stBitmap.u32BitmapPAddr = (CVI_U32)g_pstOsdcCfg->osdcObj[iOsdcIndex][i].u64BitmapPhyAddr;

            free(stBitmap.pData);
        } else {
            pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32X = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].x1;
            pstObjAttr[OsdcObjsNum].stRgnRect.stRect.s32Y = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].y1;
            if (pstObjAttr[OsdcObjsNum].enObjType == RGN_CMPR_LINE) {
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Width = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].x2;
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Height = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].y2;
            } else if (pstObjAttr[OsdcObjsNum].enObjType == RGN_CMPR_RECT) {
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Width = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].width;
                pstObjAttr[OsdcObjsNum].stRgnRect.stRect.u32Height = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].height;
            }
            pstObjAttr[OsdcObjsNum].stRgnRect.u32Thick = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].thickness;
            pstObjAttr[OsdcObjsNum].stRgnRect.u32Color = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].color;
            pstObjAttr[OsdcObjsNum].stRgnRect.u32IsFill = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].filled;
#ifdef WEB_SOCKET
            if (g_pstOsdcCfg->osdcObj[iOsdcIndex][i].filled) {
                g_pstOsdcPrivacy->bShow = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].bShow;
                g_pstOsdcPrivacy->x1 = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].x1;
                g_pstOsdcPrivacy->y1 = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].y1;
                g_pstOsdcPrivacy->width = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].width;
                g_pstOsdcPrivacy->height = g_pstOsdcCfg->osdcObj[iOsdcIndex][i].height;
                COLOR_TO_IDX(0, g_pstOsdcPrivacy->color, g_pstOsdcCfg->osdcObj[iOsdcIndex][i].color);
            }
#endif
        }
        OsdcObjsNum++;
    }

    RGN_ATTR_S stRegion;
    s32Ret = CVI_RGN_GetAttr(OsdcHandle, &stRegion);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI RGN GetAttr failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    pstCanvasCmprAttr->u32Width      = stRegion.unAttr.stOverlay.stSize.u32Width;
    pstCanvasCmprAttr->u32Height     = stRegion.unAttr.stOverlay.stSize.u32Height;
    pstCanvasCmprAttr->u32BgColor    = stRegion.unAttr.stOverlay.u32BgColor;
    pstCanvasCmprAttr->enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
    pstCanvasCmprAttr->u32BsSize     = stRegion.unAttr.stOverlay.stCompressInfo.u32CompressedSize;
    pstCanvasCmprAttr->u32ObjNum     = OsdcObjsNum;

    s32Ret = CVI_RGN_UpdateCanvas(OsdcHandle);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"CVI RGN UpdateCanvas failed with %#x!\n", s32Ret);
    }
    
    for (i = 0; i < g_pstOsdcCfg->osdcObjNum[iOsdcIndex]; i++) {
        if (!g_pstOsdcCfg->osdcObj[iOsdcIndex][i].bShow) {
            continue;
        }
        if (g_pstOsdcCfg->osdcObj[iOsdcIndex][i].type == RGN_CMPR_BIT_MAP) {
            CVI_SYS_IonFree(g_pstOsdcCfg->osdcObj[iOsdcIndex][i].u64BitmapPhyAddr, g_pstOsdcCfg->osdcObj[iOsdcIndex][i].pBitmapVirAddr);
            g_pstOsdcCfg->osdcObj[iOsdcIndex][i].u64BitmapPhyAddr = (CVI_U64)0;
            g_pstOsdcCfg->osdcObj[iOsdcIndex][i].pBitmapVirAddr = NULL;
        }
    }

    return CVI_SUCCESS;
}

static int app_ipcam_ObjRectRatio_Set(void)
{
    #ifdef AI_SUPPORT
    APP_PARAM_AI_PD_CFG_S *pstPdCfg = app_ipcam_Ai_PD_Param_Get();
    APP_PARAM_AI_MD_CFG_S *pstMdCfg = app_ipcam_Ai_MD_Param_Get();
    //APP_PARAM_AI_FD_CFG_S *pstFdCfg = app_ipcam_Ai_FD_Param_Get();
    _NULL_POINTER_CHECK_(pstPdCfg, -1);
    _NULL_POINTER_CHECK_(pstMdCfg, -1);
    //_NULL_POINTER_CHECK_(pstFdCfg, -1);

    /* get main-streaming VPSS Grp0Chn0 size */
    APP_VPSS_GRP_CFG_T *pstVpssCfg = &app_ipcam_Vpss_Param_Get()->astVpssGrpCfg[0];
    _NULL_POINTER_CHECK_(pstVpssCfg, -1);

    /* OSD Codec size form main streaming (vpss group_0 channel_0) */
    SIZE_S stOdecSize;
    stOdecSize.u32Width = pstVpssCfg->astVpssChnAttr[0].u32Width;
    stOdecSize.u32Height = pstVpssCfg->astVpssChnAttr[0].u32Height;
    APP_PROF_LOG_PRINT(LEVEL_INFO, "draw canvas size W=%d H=%d\n", stOdecSize.u32Width, stOdecSize.u32Height);

    /* set AI PD rect-ratio */
    g_stPdRectRatio.VpssChn_W = pstPdCfg->model_size_w;
    g_stPdRectRatio.VpssChn_H = pstPdCfg->model_size_h;
    g_stPdRectRatio.ScaleX = g_stPdRectRatio.ScaleY = 
        fmax(((float)stOdecSize.u32Width / (float)g_stPdRectRatio.VpssChn_W), ((float)stOdecSize.u32Height / (float)g_stPdRectRatio.VpssChn_H));

    /* set AI MD rect-ratio */
    g_stMdRectRatio.VpssChn_W = pstMdCfg->u32GrpWidth;
    g_stMdRectRatio.VpssChn_H = pstMdCfg->u32GrpHeight;
    g_stMdRectRatio.ScaleX = (float)stOdecSize.u32Width / (float)g_stMdRectRatio.VpssChn_W;
    g_stMdRectRatio.ScaleY = (float)stOdecSize.u32Height / (float)g_stMdRectRatio.VpssChn_H;

    /* set AI FD rect-ratio */
    #if 0
    g_stFdRectRatio.VpssChn_W = pstFdCfg->u32GrpWidth;
    g_stFdRectRatio.VpssChn_H = pstFdCfg->u32GrpHeight;
    g_stFdRectRatio.ScaleX = (float)stOdecSize.u32Width / (float)g_stFdRectRatio.VpssChn_W;
    g_stFdRectRatio.ScaleY = (float)stOdecSize.u32Height / (float)g_stFdRectRatio.VpssChn_H;
    #endif
    #endif

    return CVI_SUCCESS;
}

static void app_ipcam_AiRectShow_Set(int status)
{
    int showRect = status;
    if (showRect > APP_AI_ALL_RECT_SHOW) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "showRect(%d) > (%d) not legal and draw all rect default\n", showRect, APP_AI_ALL_RECT_SHOW);
        showRect = APP_AI_ALL_RECT_SHOW;
    }

    int iOsdcIndex = 0;
    for (iOsdcIndex = 0; iOsdcIndex < OSDC_NUM_MAX; iOsdcIndex++) {
        if (g_pstOsdcCfg->bShow[iOsdcIndex]) {
            g_pstOsdcCfg->bShowPdRect[iOsdcIndex] = (APP_AI_PD_RECT_SHOW & showRect);
            g_pstOsdcCfg->bShowMdRect[iOsdcIndex] = (APP_AI_MD_RECT_SHOW & showRect);
            g_pstOsdcCfg->bShowFdRect[iOsdcIndex] = (APP_AI_FD_RECT_SHOW & showRect);
        }
    }
}

void *Thread_Osdc_Draw(void *arg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    // struct timeval now;
    // struct timespec outtime;
    prctl(PR_SET_NAME, "OSDC_DRAW", 0, 0, 0);
    CVI_U32 SleepCnt = 0;
    int iOsdcIndex = 0;
    
    while (g_bOsdcThreadRun) {
        if (SleepCnt != 10) {
            SleepCnt++;
            usleep(10*1000);
            continue;
        }
        pthread_mutex_lock(&OsdcMutex);
        for (iOsdcIndex = 0; iOsdcIndex < OSDC_NUM_MAX; iOsdcIndex++) {
            if (g_pstOsdcCfg->bShow[iOsdcIndex]) {
                RGN_HANDLE OsdcHandle = g_pstOsdcCfg->handle[iOsdcIndex];
                s32Ret = app_ipcam_ObjsRectInfo_Update(OsdcHandle, iOsdcIndex);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR,"app_ipcam_ObjsRectInfo_Update failed with %#x!\n", s32Ret);
                }
            }
            if (!g_bOsdcThreadRun) {
                pthread_mutex_unlock(&OsdcMutex);
                break;
            }
        }
        pthread_mutex_unlock(&OsdcMutex);
        SleepCnt = 0;
    }

    return NULL;
}

int app_ipcam_Osdc_Init(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if (!g_pstOsdcCfg->enable) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "draw Osdc thread not enable!\n");
        return CVI_SUCCESS;
    }

    if (g_stOsdcCanvasCfg.createCanvas == CVI_FALSE) {
        s32Ret = app_ipcam_OSDCRgn_Create();
        if (s32Ret != CVI_SUCCESS) {
            APP_PROF_LOG_PRINT(LEVEL_ERROR,"app_ipcam_OSDCRgn_Create failed with %#x!\n", s32Ret);
            return CVI_FAILURE;
        }
        g_stOsdcCanvasCfg.createCanvas = CVI_TRUE;
        APP_PROF_LOG_PRINT(LEVEL_INFO, "app_ipcam_OSDCRgn_Created!\n");
    }

    /* calculate AI Rect ratio betwen streaming and AI size */
    APP_IPCAM_CHECK_RET(app_ipcam_ObjRectRatio_Set(), "OSDC OBJ RATIO RECT SET");

    g_bOsdcThreadRun = CVI_TRUE;
    s32Ret = pthread_create(
                &g_pthOsdcRgn,
                NULL,
                Thread_Osdc_Draw,
                (CVI_VOID *)g_pstOsdcCfg);
    if (s32Ret != 0) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "pthread_create failed!\n");
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

int app_ipcam_Osdc_DeInit(void)
{
    // CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_S32 iTime = GetCurTimeInMsec();

    if (!g_pstOsdcCfg->enable) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "draw Osdc thread not enable!\n");
        return CVI_SUCCESS;
    }
    
    if (!g_stOsdcCanvasCfg.createCanvas) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "draw Osdc Canvas not create!\n");
        return CVI_SUCCESS;
    }

    g_bOsdcThreadRun = CVI_FALSE;
    if (g_pthOsdcRgn > (pthread_t)0) {
        pthread_join(g_pthOsdcRgn, NULL);
        g_pthOsdcRgn = 0;
    }
    APP_PROF_LOG_PRINT(LEVEL_WARN, "waiting osdc thread exit takes %u ms \n", (GetCurTimeInMsec() - iTime));

    app_ipcam_OSDCRgn_Destory();

    g_stOsdcCanvasCfg.createCanvas = CVI_FALSE;
    
    return CVI_SUCCESS;
}

/*****************************************************************
 *  The following API for command test used             Front
 * **************************************************************/
static CVI_BOOL g_bOsdDynamic_Running;
static pthread_t g_OsdDynamicShow;
int offset_step = 50;

static CVI_VOID *Thread_Osd_Dynamic_Running(CVI_VOID *arg)
{
    // CVI_S32 s32Ret;

    (CVI_VOID)arg;

    char szStr[APP_OSD_STR_LEN_MAX] = {0};
    CVI_CHAR TaskName[64] = {'\0'};

    sprintf(TaskName, "T-Osd_D_Show");
    prctl(PR_SET_NAME, TaskName, 0, 0, 0);

    APP_PROF_LOG_PRINT(LEVEL_INFO, " g_bOsdDynamic_Running = %d\n", g_bOsdDynamic_Running);
    
    MMF_CHN_S *pstChn = NULL;
    RGN_CHN_ATTR_S stChnAttr = {0};
    RGN_HANDLE Handle = 0;
    while (g_bOsdDynamic_Running) {
        GetTimeStr(NULL, szStr, APP_OSD_STR_LEN_MAX);
        for (CVI_S32 i = 0; i < g_pstOsdCfg->osd_cnt; i++) {
            Handle = i;
            memset(&stChnAttr, 0 ,sizeof(stChnAttr));
            pstChn = &g_pstOsdCfg->astOsdAttr[Handle].stChn;

            CVI_RGN_GetDisplayAttr(Handle, pstChn, &stChnAttr);
            APP_PROF_LOG_PRINT(LEVEL_DEBUG, "osd handle(%d) s32X=%d offset=%d\n", Handle, stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X, offset_step); 
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = (stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X + offset_step < 2560) ? 
                                                                (stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X + offset_step) : 0;
            CVI_RGN_SetDisplayAttr(Handle, pstChn, &stChnAttr);
        }
        sleep(1);
    }
    return NULL;
}

void app_ipcam_Osdc_Status(int status)
{
    if ((status == CVI_TRUE) && (g_stOsdcCanvasCfg.createCanvas == CVI_FALSE)) {
        g_stOsdcCfg.enable = CVI_TRUE;
        app_ipcam_Osdc_Init();
    } else if ((g_stOsdcCanvasCfg.createCanvas == CVI_TRUE) && (status == CVI_FALSE)) {
        app_ipcam_Osdc_DeInit();
        g_stOsdcCfg.enable = CVI_FALSE;
    }
}

int app_ipcam_CmdTask_Osd_Switch(CVI_MQ_MSG_t *msg, CVI_VOID *userdate)
{
    CVI_CHAR param[512] = {0};
    snprintf(param, sizeof(param), "%s", msg->payload);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s param:%s arg2=%d\n", __FUNCTION__, param, msg->arg2);

    RGN_HANDLE Handle = 0;

    // you should specify the OSD handle before show or hide it!
    char *pc = strchr(param, 'i');
    if (!pc) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Pls specify which OSD Handle you want to show or hidden!!!\n");
        return CVI_FAILURE;
    }
        
    CVI_CHAR *temp = strtok(param, ":");
    while(NULL != temp) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "%s switch case -> %c \n", __FUNCTION__, *temp);
        switch (*temp) {
            case 'i': 
                temp = strtok(NULL, "/");
                Handle = atoi(temp);
                break;
            case 's': 
                {
                    temp = strtok(NULL, "/");
                    int show = atoi(temp);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "osd handle(%d) set show=%d\n", Handle, show); 
                    const MMF_CHN_S *pstChn = &g_pstOsdCfg->astOsdAttr[Handle].stChn;
                    RGN_CHN_ATTR_S stChnAttr = {0};
                    APP_CHK_RET(CVI_RGN_GetDisplayAttr(Handle, pstChn, &stChnAttr), "Get OSD Attr");
                    stChnAttr.bShow = (CVI_BOOL)show;
                    if (CVI_SUCCESS != CVI_RGN_SetDisplayAttr(Handle, pstChn, &stChnAttr))
                    {
                        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Set OSD Attr failed!\n"); 
                    }
                }
                break;

            case 'o':
                {
                    temp = strtok(NULL, "/");
                    offset_step = atoi(temp);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "offset_step=%d\n", offset_step); 
                }
                break;

            case 'r':
                temp = strtok(NULL, "/");
                g_bOsdDynamic_Running = (CVI_BOOL)atoi(temp);
                APP_PROF_LOG_PRINT(LEVEL_INFO, "g_bOsdDynamic_Running = %d\n", g_bOsdDynamic_Running); 

                if (g_bOsdDynamic_Running) {
                    APP_CHK_RET(pthread_create(
                                    &g_OsdDynamicShow, 
                                    NULL, 
                                    Thread_Osd_Dynamic_Running, 
                                    (CVI_VOID *)g_pstOsdCfg), "create OSD dynamic show task");

                    APP_CHK_RET(pthread_detach(g_OsdDynamicShow), "detach OSD dynamic show task");
                }
                break;

            default:
                return 0;
                break;
        }
        
        temp = strtok(NULL, ":");
    }

    return CVI_SUCCESS;
}


int app_ipcam_CmdTask_Cover_Switch(CVI_MQ_MSG_t *msg, CVI_VOID *userdate)
{
    CVI_CHAR param[512] = {0};
    snprintf(param, sizeof(param), "%s", msg->payload);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s param:%s arg2=%d\n", __FUNCTION__, param, msg->arg2);

    int iCoverIndex = 0;
    char *pc = strchr(param, 'i');
    if (!pc) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Pls specify which Cover Handle you want to show or hidden!!!\n");
        return CVI_FAILURE;
    }
        
    CVI_CHAR *temp = strtok(param, ":");
    while(NULL != temp) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "%s switch case -> %c \n", __FUNCTION__, *temp);
        switch (*temp) {
            case 'i': 
                temp = strtok(NULL, "/");
                iCoverIndex = atoi(temp);
                break;

            case 'c':
                {
                    temp = strtok(NULL, "/");
                    int color = atoi(temp);
                    APP_PROF_LOG_PRINT(LEVEL_INFO, "Cover iCoverIndex(%d) set color=0x%x\n", iCoverIndex, color); 
                    const MMF_CHN_S *pstChn = &g_pstCoverCfg->astCoverAttr[iCoverIndex].stChn;
                    RGN_CHN_ATTR_S stChnAttr = {0};

                    APP_CHK_RET(CVI_RGN_GetDisplayAttr(g_pstCoverCfg->astCoverAttr[iCoverIndex].Handle, pstChn, &stChnAttr), "Get Cover Attr");

                    stChnAttr.unChnAttr.stCoverChn.u32Color         = color;
                    if (CVI_SUCCESS != CVI_RGN_SetDisplayAttr(g_pstCoverCfg->astCoverAttr[iCoverIndex].Handle, pstChn, &stChnAttr))
                    {
                        APP_PROF_LOG_PRINT(LEVEL_ERROR, "Set Cover Attr failed!\n"); 
                    }
                }
                break;

            default:
                return 0;
                break;
        }
        
        temp = strtok(NULL, ":");
    }

    return CVI_SUCCESS;
}

int app_ipcam_CmdTask_Rect_Switch(CVI_MQ_MSG_t *msg, CVI_VOID *userdate)
{
    CVI_CHAR param[512] = {0};
    snprintf(param, sizeof(param), "%s", msg->payload);
    APP_PROF_LOG_PRINT(LEVEL_INFO, "%s param:%s arg2=%d\n", __FUNCTION__, param, msg->arg2);
        
    CVI_CHAR *temp = strtok(param, ":");
    
    APP_PROF_LOG_PRINT(LEVEL_WARN, "%s temp=%c \n", __FUNCTION__, *temp);
    while(NULL != temp) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "%s switch case -> %c \n", __FUNCTION__, *temp);
        switch (*temp) {
            case 's':
                {
                    temp = strtok(NULL, "/");
                    int flag = atoi(temp);
                    APP_PROF_LOG_PRINT(LEVEL_WARN, "%s flag=0x%x \n", __FUNCTION__, flag);
                    app_ipcam_AiRectShow_Set(flag);
                }
                break;

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
