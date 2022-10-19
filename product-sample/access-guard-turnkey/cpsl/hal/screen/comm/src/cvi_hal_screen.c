#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "cvi_hal_screen.h"
#include "cvi_hal_screen_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCREEN */
/** @{ */  /** <!-- [SCREEN] */

static HAL_SCREEN_CTX_S s_astHALSCREENCtx[CVI_HAL_SCREEN_IDX_BUTT];


int32_t CVI_HAL_SCREEN_Register(CVI_HAL_SCREEN_IDX_E enScreenIndex, const CVI_HAL_SCREEN_OBJ_S* pstScreenObj)
{
    if (s_astHALSCREENCtx[enScreenIndex].bRegister)
    {
        printf("Screen[%d] has been registered\n", enScreenIndex);
        return -1;
    }

    memcpy(&s_astHALSCREENCtx[enScreenIndex].stScreenObj, pstScreenObj, sizeof(CVI_HAL_SCREEN_OBJ_S));
    s_astHALSCREENCtx[enScreenIndex].bRegister = true;

    return 0;
}

int32_t CVI_HAL_SCREEN_Init(CVI_HAL_SCREEN_IDX_E enScreenIndex)
{
	int32_t s32Ret = 0;
	
    if (s_astHALSCREENCtx[enScreenIndex].bInit)
    {
        printf("Screen[%d] has been inited\n", enScreenIndex);
        return 0;
    }

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnInit)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnInit();
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    s_astHALSCREENCtx[enScreenIndex].bInit = true;
    return s32Ret;
}

int32_t CVI_HAL_SCREEN_GetAttr(CVI_HAL_SCREEN_IDX_E enScreenIndex, CVI_HAL_SCREEN_ATTR_S* pstAttr)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetAttr)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetAttr(pstAttr);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t CVI_HAL_SCREEN_GetDisplayState(CVI_HAL_SCREEN_IDX_E enScreenIndex, CVI_HAL_SCREEN_STATE_E* penDisplayState)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetDisplayState)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetDisplayState(penDisplayState);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t CVI_HAL_SCREEN_SetDisplayState(CVI_HAL_SCREEN_IDX_E enScreenIndex, CVI_HAL_SCREEN_STATE_E enDisplayState)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetDisplayState)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetDisplayState(enDisplayState);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t CVI_HAL_SCREEN_GetBackLightState(CVI_HAL_SCREEN_IDX_E enScreenIndex, CVI_HAL_SCREEN_STATE_E* penBackLightState)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetBackLightState)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetBackLightState(penBackLightState);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t CVI_HAL_SCREEN_SetBackLightState(CVI_HAL_SCREEN_IDX_E enScreenIndex, CVI_HAL_SCREEN_STATE_E enBackLightState)
{
    int32_t s32Ret = 0;
	
    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetBackLightState)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetBackLightState(enBackLightState);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t CVI_HAL_SCREEN_GetLuma(CVI_HAL_SCREEN_IDX_E enScreenIndex, uint32_t* pu32Luma)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetLuma)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetLuma(pu32Luma);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t CVI_HAL_SCREEN_SetLuma(CVI_HAL_SCREEN_IDX_E enScreenIndex, uint32_t u32Luma)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetLuma)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetLuma(u32Luma);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t CVI_HAL_SCREEN_GetSaturature(CVI_HAL_SCREEN_IDX_E enScreenIndex, uint32_t* pu32Saturature)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetSaturature)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetSaturature(pu32Saturature);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t CVI_HAL_SCREEN_SetSaturature(CVI_HAL_SCREEN_IDX_E enScreenIndex, uint32_t u32Saturature)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetSaturature)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetSaturature(u32Saturature);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t CVI_HAL_SCREEN_GetContrast(CVI_HAL_SCREEN_IDX_E enScreenIndex, uint32_t* pu32Contrast)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetContrast)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetContrast(pu32Contrast);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t CVI_HAL_SCREEN_SetContrast(CVI_HAL_SCREEN_IDX_E enScreenIndex, uint32_t u32Contrast)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetContrast)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetContrast(u32Contrast);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t CVI_HAL_SCREEN_Deinit(CVI_HAL_SCREEN_IDX_E enScreenIndex)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnDeinit)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnDeinit();
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    s_astHALSCREENCtx[enScreenIndex].bInit = false;
	
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

