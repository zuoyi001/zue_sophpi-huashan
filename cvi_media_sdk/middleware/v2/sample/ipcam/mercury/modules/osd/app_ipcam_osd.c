
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include "cvi_region.h"
#include "cvi_comm_video.h"
#include "app_ipcam_osd.h"
#include "app_ipcam_paramparse.h"
#include "app_ipcam_loadbmp.h"
#include "app_ipcam_fontmod.h"
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

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static APP_PARAM_OSD_CFG_T g_stOsdCfg, *g_pstOsdCfg = &g_stOsdCfg;
static APP_PARAM_COVER_CFG_T g_stCoverCfg, *g_pstCoverCfg = &g_stCoverCfg;

static CVI_BOOL g_bTStamp_Running;
static pthread_t g_TStamp_Thread;

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
    if (NULL == pazStr || 0 >= s32MaxLen)
    {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "szStr is NULL or s32MaxLen:%d invalid!\n", s32MaxLen);
        return ;
    }
    snprintf(pazStr, s32MaxLen, "MD[%02d-%d]PD[%02d-%d]FD[%02d-%d]", 
            app_ipcam_Ai_MD_ProcFps_Get(), app_ipcam_Ai_MD_ProcTime_Get(),
            app_ipcam_Ai_PD_ProcFps_Get(), app_ipcam_Ai_PD_ProcTime_Get(),
            app_ipcam_Ai_FD_ProcFps_Get(), app_ipcam_Ai_FD_ProcTime_Get());
#else
#endif
}

static CVI_S32 app_ipcam_Osd_Bitmap_Update(RGN_HANDLE RgnHdl, char *szStr, BITMAP_S *pstBitmap)
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

    pstBitmap->pData = malloc(2 * (pstBitmap->u32Width) * (pstBitmap->u32Height));
    if (pstBitmap->pData == NULL) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "malloc osd memroy err!\n");
        return CVI_FAILURE;
    }
    memset(pstBitmap->pData, 0, (2 * (pstBitmap->u32Width) * (pstBitmap->u32Height)));

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

    s32Ret = CVI_RGN_SetBitMap(RgnHdl, pstBitmap);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
        free(pstBitmap->pData);
        return CVI_FAILURE;
    }

    free(pstBitmap->pData);

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
                if (CVI_SYS_GetVPSSMode() == VPSS_MODE_RGNEX) {
                    stBitmap.u32Width = stRegion.unAttr.stOverlayEx.stSize.u32Width;
                    stBitmap.u32Height = stRegion.unAttr.stOverlayEx.stSize.u32Height;
                    stBitmap.enPixelFormat = stRegion.unAttr.stOverlayEx.enPixelFormat;
                } else {
                    stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
                    stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
                    stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
                }
                if (pstOsdCfg->astOsdAttr[i].enType == TYPE_DEBUG)
                {
                    GetDebugStr(szStr, APP_OSD_STR_LEN_MAX);
                }
                else
                {
                    GetTimeStr(NULL, szStr, APP_OSD_STR_LEN_MAX);
                }
                s32Ret = app_ipcam_Osd_Bitmap_Update(pstOsdCfg->astOsdAttr[i].Handle, szStr, &stBitmap);
                if (s32Ret != CVI_SUCCESS) {
                    APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Osd_Bitmap_Update failed!\n");
                    return NULL;
                }
            }
        }
        SleepCnt = 0;
    }
    return NULL;
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
        APP_PROF_LOG_PRINT(LEVEL_ERROR,"GetBmpInfo err!\n");
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

    stBitmap.pData = stCanvasInfo.pu8VirtAddr;
    stSize.u32Width = stCanvasInfo.stSize.u32Width;
    stSize.u32Height = stCanvasInfo.stSize.u32Height;
    app_ipcam_Rgn_Mst_Canvas_Update(filename, &stBitmap, CVI_FALSE, 0, 
            &stSize, stCanvasInfo.u32Stride, PIXEL_FORMAT_ARGB_1555);

    s32Ret = CVI_RGN_UpdateCanvas(Handle);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
        return CVI_FAILURE;
    }
    return s32Ret;
}

static CVI_S32 app_ipcam_Osd_Destroy(APP_PARAM_OSD_ATTR_S *pstOsdAttr)
{
    CVI_S32 s32Ret;
    RGN_HANDLE Handle = pstOsdAttr->Handle;
    MMF_CHN_S stChn = pstOsdAttr->stChn;

    s32Ret = CVI_RGN_DetachFromChn(Handle, &stChn);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_DetachFromChn failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_RGN_Destroy(Handle);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_Destroy failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    return CVI_SUCCESS;
}

static CVI_S32 app_ipcam_Osd_Create(VPSS_MODE_E VpssMode, APP_PARAM_OSD_ATTR_S *pstOsdAttr)
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
            if (VpssMode == VPSS_MODE_RGNEX) {
                stRegion.unAttr.stOverlayEx.stSize.u32Height = pstOsdAttr->stRect.u32Height;
                stRegion.unAttr.stOverlayEx.stSize.u32Width = pstOsdAttr->stRect.u32Width;
            } else {
                stRegion.unAttr.stOverlay.stSize.u32Height = pstOsdAttr->stRect.u32Height;
                stRegion.unAttr.stOverlay.stSize.u32Width = pstOsdAttr->stRect.u32Width;
            }
        break;

        case TYPE_STRING:
            s32StrLen = strlen(pstOsdAttr->str) - GetNonASCNum(pstOsdAttr->str, strlen(pstOsdAttr->str));
            pszStr = pstOsdAttr->str;
            if (VpssMode == VPSS_MODE_RGNEX) {
                stRegion.unAttr.stOverlayEx.stSize.u32Height = OSD_LIB_FONT_H;
                stRegion.unAttr.stOverlayEx.stSize.u32Width = OSD_LIB_FONT_W * s32StrLen;
            } else {
                stRegion.unAttr.stOverlay.stSize.u32Height = OSD_LIB_FONT_H;
                stRegion.unAttr.stOverlay.stSize.u32Width = OSD_LIB_FONT_W * s32StrLen;
            }
        break;

        case TYPE_TIME:
            GetTimeStr(NULL, szStr, APP_OSD_STR_LEN_MAX);
            s32StrLen = strnlen(szStr, APP_OSD_STR_LEN_MAX);
            pszStr = szStr;
            if (VpssMode == VPSS_MODE_RGNEX) {
                stRegion.unAttr.stOverlayEx.stSize.u32Height = OSD_LIB_FONT_H;
                stRegion.unAttr.stOverlayEx.stSize.u32Width = OSD_LIB_FONT_W * s32StrLen;
            } else {
                stRegion.unAttr.stOverlay.stSize.u32Height = OSD_LIB_FONT_H;
                stRegion.unAttr.stOverlay.stSize.u32Width = OSD_LIB_FONT_W * s32StrLen;
            }
        break;

        case TYPE_DEBUG:
            if (VpssMode == VPSS_MODE_RGNEX) {
                stRegion.unAttr.stOverlayEx.stSize.u32Height = OSD_LIB_FONT_H;
                stRegion.unAttr.stOverlayEx.stSize.u32Width = OSD_LIB_FONT_W * APP_OSD_STR_LEN_MAX;
            } else {
                stRegion.unAttr.stOverlay.stSize.u32Height = OSD_LIB_FONT_H;
                stRegion.unAttr.stOverlay.stSize.u32Width = OSD_LIB_FONT_W * APP_OSD_STR_LEN_MAX;
            }
        break;

        default:
            APP_PROF_LOG_PRINT(LEVEL_ERROR, "OSD enType = %d invalid \n", enType);
        break;
    }

    if (VpssMode == VPSS_MODE_RGNEX) {
        stRegion.enType = OVERLAYEX_RGN;
        stRegion.unAttr.stOverlayEx.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
        stRegion.unAttr.stOverlayEx.u32BgColor = 0x00000000; // ARGB1555 transparent
        stRegion.unAttr.stOverlayEx.u32CanvasNum = 2;
    } else {
        stRegion.enType = OVERLAY_RGN;
        stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
        stRegion.unAttr.stOverlay.u32BgColor = 0x00000000; // ARGB1555 transparent
        stRegion.unAttr.stOverlay.u32CanvasNum = 2;
    }

    s32Ret = CVI_RGN_Create(Handle, &stRegion);
    if (s32Ret != CVI_SUCCESS) {
        APP_PROF_LOG_PRINT(LEVEL_ERROR, "CVI_RGN_Create failed with %#x!\n", s32Ret);
        return CVI_FAILURE;
    }

    RGN_CHN_ATTR_S stChnAttr = {0};
    stChnAttr.bShow = pstOsdAttr->bShow;
    if (VpssMode == VPSS_MODE_RGNEX) {
        stChnAttr.enType = OVERLAYEX_RGN;
        stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = pstOsdAttr->stRect.s32X;
        stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = pstOsdAttr->stRect.s32Y;
        stChnAttr.unChnAttr.stOverlayExChn.u32Layer = 0;
    } else {
        stChnAttr.enType = OVERLAY_RGN;
        stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = pstOsdAttr->stRect.s32X;
        stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = pstOsdAttr->stRect.s32Y;
        stChnAttr.unChnAttr.stOverlayChn.u32Layer = 0;
    }

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
            if (VpssMode == VPSS_MODE_RGNEX) {
                stBitmap.u32Width = stRegion.unAttr.stOverlayEx.stSize.u32Width;
                stBitmap.u32Height = stRegion.unAttr.stOverlayEx.stSize.u32Height;
                stBitmap.enPixelFormat = stRegion.unAttr.stOverlayEx.enPixelFormat;
            } else {
                stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
                stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
                stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
            }
            s32Ret = app_ipcam_Osd_Bitmap_Update(Handle, pszStr, &stBitmap);
            if (s32Ret != CVI_SUCCESS) {
                APP_PROF_LOG_PRINT(LEVEL_ERROR, "app_ipcam_Osd_Bitmap_Update failed!\n");
                goto EXIT1;
            }
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

    VPSS_MODE_E VpssMode = CVI_SYS_GetVPSSMode();

    APP_PROF_LOG_PRINT(LEVEL_INFO,"OSD handle count=%d VpssMode=%d\n", g_pstOsdCfg->osd_cnt, VpssMode);
    for (; i < g_pstOsdCfg->osd_cnt; i++) {
        s32Ret = app_ipcam_Osd_Create(VpssMode, &g_pstOsdCfg->astOsdAttr[i]);
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
    
    VPSS_MODE_E VpssMode = CVI_SYS_GetVPSSMode();
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
            if (VpssMode == VPSS_MODE_RGNEX) {
                APP_PROF_LOG_PRINT(LEVEL_DEBUG, "osd handle(%d) s32X=%d offset=%d\n", Handle, stChnAttr.unChnAttr.stCoverChn.stRect.s32X, offset_step); 
                stChnAttr.unChnAttr.stCoverChn.stRect.s32X = (stChnAttr.unChnAttr.stCoverChn.stRect.s32X + offset_step < 2560) ?
                                                            (stChnAttr.unChnAttr.stCoverChn.stRect.s32X + offset_step) : 0 ;
            } else {
                APP_PROF_LOG_PRINT(LEVEL_DEBUG, "osd handle(%d) s32X=%d offset=%d\n", Handle, stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X, offset_step); 
                stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = (stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X + offset_step < 2560) ? 
                                                                (stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X + offset_step) : 0;
            }
            CVI_RGN_SetDisplayAttr(Handle, pstChn, &stChnAttr);
        }
        sleep(1);
    }
    return NULL;
}


int app_ipcam_CmdTask_Osd_Switch(CVI_MQ_MSG_t *msg, CVI_VOID *userdate)
{
    CVI_CHAR param[256] = {0};
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
    CVI_CHAR param[256] = {0};
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
                    VPSS_MODE_E VpssMode = CVI_SYS_GetVPSSMode();

                    APP_CHK_RET(CVI_RGN_GetDisplayAttr(g_pstCoverCfg->astCoverAttr[iCoverIndex].Handle, pstChn, &stChnAttr), "Get Cover Attr");
                    if (VpssMode == VPSS_MODE_RGNEX) {
                        stChnAttr.unChnAttr.stCoverExChn.u32Color         = color;
                    } else {
                        stChnAttr.unChnAttr.stCoverChn.u32Color         = color;
                    }
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

/*****************************************************************
 *  The above API for command test used                 End
 * **************************************************************/
