#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "cvi_hal_ahd.h"
#include "cvi_hal_ahd_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     AHD */
/** @{ */  /** <!-- [AHD] */

#define CHECK_HAL_AHD_REG_RET(idx)                                          \
    do {                                                                    \
        if (s_astHalAhdCtx[idx].bRegister == false) {                       \
            printf("AHD[%d] has no registered\n", idx);                     \
            return -1;                                                      \
        }                                                                   \
    } while (0)

#define CHECK_HAL_AHD_INIT_RET(idx)                                         \
    do {                                                                    \
        if (s_astHalAhdCtx[idx].bInit == false) {                           \
            printf("AHD[%d] has no initialized\n", idx);                    \
            return -1;                                                      \
        }                                                                   \
    } while (0)

#define CHECK_HAL_AHD_NULL_PTR_RET(ptr)                                     \
    do {                                                                    \
        if (ptr == NULL) {                                                  \
            printf("%s is NULL pointer\n", #ptr);                           \
            return -1;                                                      \
        }                                                                   \
    } while (0)

#define CHECK_HAL_AHD_IDX_RET(idx)                                          \
    do {                                                                    \
        if (idx >= CVI_HAL_AHD_IDX_BUTT) {                                  \
            printf("AHD index[%s] is invalid\n", #idx);                     \
            return -1;                                                      \
        }                                                                   \
    } while (0)


static HAL_AHD_CTX_S s_astHalAhdCtx[CVI_HAL_AHD_IDX_BUTT];

int32_t CVI_HAL_AHD_Register(CVI_HAL_AHD_IDX_E enAhdIndex, const CVI_HAL_AHD_OBJ_S *pstAhdObj)
{
    CHECK_HAL_AHD_IDX_RET(enAhdIndex);

    if (s_astHalAhdCtx[enAhdIndex].bRegister == true) {
        printf("AHD[%d] has been registered\n", enAhdIndex);
        return -1;
    }

    if (pstAhdObj->u32ChnMax == 0 || pstAhdObj->u32ChnMax > CVI_HAL_AHD_CHN_MAX) {
        printf("AHD[%d] u32ChnMax:%d should not be (0,%d] \n", enAhdIndex,
                       pstAhdObj->u32ChnMax, CVI_HAL_AHD_CHN_MAX);
        return -1;
    }

    memcpy(&s_astHalAhdCtx[enAhdIndex].stAhdObj, pstAhdObj, sizeof(CVI_HAL_AHD_OBJ_S));
    s_astHalAhdCtx[enAhdIndex].bRegister = true;

    return 0;
}

int32_t CVI_HAL_AHD_UnRegister(CVI_HAL_AHD_IDX_E enAhdIndex)
{
    CHECK_HAL_AHD_IDX_RET(enAhdIndex);

    if (s_astHalAhdCtx[enAhdIndex].bInit == true) {
        printf("AHD[%d] should deinit before unregistered\n", enAhdIndex);
        return -1;
    }

    memset(&s_astHalAhdCtx[enAhdIndex].stAhdObj, 0, sizeof(CVI_HAL_AHD_OBJ_S));
    s_astHalAhdCtx[enAhdIndex].bRegister = false;

    return 0;
}

int32_t CVI_HAL_AHD_Init(CVI_HAL_AHD_IDX_E enAhdIndex)
{
    CHECK_HAL_AHD_IDX_RET(enAhdIndex);
    CHECK_HAL_AHD_REG_RET(enAhdIndex);

    if (s_astHalAhdCtx[enAhdIndex].bInit == true) {
        printf("AHD[%d] has been initialized\n", enAhdIndex);
        return -1;
    }

    CHECK_HAL_AHD_NULL_PTR_RET(s_astHalAhdCtx[enAhdIndex].stAhdObj.pfnInit);

    if (s_astHalAhdCtx[enAhdIndex].stAhdObj.pfnInit() != 0) {
        printf("AHD[%d] pfnInit fail\n", enAhdIndex);
        return -1;
    }

    s_astHalAhdCtx[enAhdIndex].bInit = true;

    return 0;
}

int32_t CVI_HAL_AHD_Deinit(CVI_HAL_AHD_IDX_E enAhdIndex)
{
    CHECK_HAL_AHD_IDX_RET(enAhdIndex);
    CHECK_HAL_AHD_REG_RET(enAhdIndex);
    CHECK_HAL_AHD_INIT_RET(enAhdIndex);
    CHECK_HAL_AHD_NULL_PTR_RET(s_astHalAhdCtx[enAhdIndex].stAhdObj.pfnDeinit);

    if (s_astHalAhdCtx[enAhdIndex].stAhdObj.pfnDeinit() != 0) {
        printf("AHD[%d] pfnDeinit fail\n", enAhdIndex);
        return -1;
    }

    s_astHalAhdCtx[enAhdIndex].bInit = false;

    return 0;
}

int32_t CVI_HAL_AHD_StartChn(CVI_HAL_AHD_IDX_E enAhdIndex, uint32_t u32Chn)
{
    CHECK_HAL_AHD_IDX_RET(enAhdIndex);
    CHECK_HAL_AHD_REG_RET(enAhdIndex);
    CHECK_HAL_AHD_INIT_RET(enAhdIndex);
    CHECK_HAL_AHD_NULL_PTR_RET(s_astHalAhdCtx[enAhdIndex].stAhdObj.pfnStartChn);

    if (s_astHalAhdCtx[enAhdIndex].stAhdObj.pfnStartChn(u32Chn) != 0) {
        printf("AHD[%d] pfnStartChn[%d] fail\n", enAhdIndex, u32Chn);
        return -1;
    }

    return 0;
}

int32_t CVI_HAL_AHD_StopChn(CVI_HAL_AHD_IDX_E enAhdIndex, uint32_t u32Chn)
{
    CHECK_HAL_AHD_IDX_RET(enAhdIndex);
    CHECK_HAL_AHD_REG_RET(enAhdIndex);
    CHECK_HAL_AHD_INIT_RET(enAhdIndex);
    CHECK_HAL_AHD_NULL_PTR_RET(s_astHalAhdCtx[enAhdIndex].stAhdObj.pfnStopChn);

    if (s_astHalAhdCtx[enAhdIndex].stAhdObj.pfnStopChn(u32Chn) != 0) {
        printf("AHD[%d] pfnStopChn[%d] fail\n", enAhdIndex, u32Chn);
        return -1;
    }

    return 0;
}

int32_t CVI_HAL_AHD_SetAttr(CVI_HAL_AHD_IDX_E enAhdIndex, const CVI_HAL_AHD_ATTR_S *pstAttr)
{
    CHECK_HAL_AHD_IDX_RET(enAhdIndex);
    CHECK_HAL_AHD_REG_RET(enAhdIndex);
    CHECK_HAL_AHD_INIT_RET(enAhdIndex);
    CHECK_HAL_AHD_NULL_PTR_RET(s_astHalAhdCtx[enAhdIndex].stAhdObj.pfnSetAttr);

    if (s_astHalAhdCtx[enAhdIndex].stAhdObj.pfnSetAttr(pstAttr) != 0) {
        printf("AHD[%d] pfnSetAttr fail\n", enAhdIndex);
        return -1;
    }

    return 0;
}

int32_t CVI_HAL_AHD_GetAttr(CVI_HAL_AHD_IDX_E enAhdIndex, CVI_HAL_AHD_ATTR_S *pstAttr)
{
    CHECK_HAL_AHD_IDX_RET(enAhdIndex);
    CHECK_HAL_AHD_REG_RET(enAhdIndex);
    CHECK_HAL_AHD_INIT_RET(enAhdIndex);
    CHECK_HAL_AHD_NULL_PTR_RET(s_astHalAhdCtx[enAhdIndex].stAhdObj.pfnGetAttr);

    if (s_astHalAhdCtx[enAhdIndex].stAhdObj.pfnGetAttr(pstAttr) != 0) {
        printf("AHD[%d] pfnGetAttr fail\n", enAhdIndex);
        return -1;
    }

    return 0;
}

int32_t CVI_HAL_AHD_GetStatus(CVI_HAL_AHD_IDX_E enAhdIndex, CVI_HAL_AHD_STATUS_E astAhdStatus[], uint32_t u32Cnt)
{
    CHECK_HAL_AHD_IDX_RET(enAhdIndex);
    CHECK_HAL_AHD_REG_RET(enAhdIndex);
    CHECK_HAL_AHD_INIT_RET(enAhdIndex);
    CHECK_HAL_AHD_NULL_PTR_RET(s_astHalAhdCtx[enAhdIndex].stAhdObj.pfnGetStatus);

    CVI_HAL_AHD_STATUS_E astAhdStatusTmp[CVI_HAL_AHD_CHN_MAX];

    if (u32Cnt == 0 || u32Cnt > CVI_HAL_AHD_CHN_MAX) {
        printf("enAhdIndex:%d, u32Cnt:%u is illegal \n", enAhdIndex, u32Cnt);
        return -1;
    }

    if (s_astHalAhdCtx[enAhdIndex].stAhdObj.pfnGetStatus(astAhdStatusTmp, CVI_HAL_AHD_CHN_MAX) != 0) {
        printf("AHD[%d] pfnGetAttr fail\n", enAhdIndex);
        return -1;
    }

    for (uint32_t i = 0; i < u32Cnt; i++) {
        astAhdStatus[i] = astAhdStatusTmp[i];
    }

    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

