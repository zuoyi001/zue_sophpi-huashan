#ifndef __CVI_HAL_AHD_H__
#define __CVI_HAL_AHD_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define CVI_HAL_AHD_CHN_MAX      2
#define CVI_HAL_AHD_DEV_NAME_LEN 20

/* struct */
typedef enum cviHAL_AHD_IDX_E {
    CVI_HAL_AHD_IDX_0 = 0,
    CVI_HAL_AHD_IDX_1,
    CVI_HAL_AHD_IDX_BUTT
} CVI_HAL_AHD_IDX_E;

typedef enum cviHAL_AHD_RES_E {
    CVI_HAL_AHD_RES_720P_25 = 0,
    CVI_HAL_AHD_RES_720P_30,
    CVI_HAL_AHD_RES_1080P_25,
    CVI_HAL_AHD_RES_1080P_30,
    CVI_HAL_AHD_RES_BUIT
} CVI_HAL_AHD_RES_E;

typedef enum cviHAL_AHD_STATE_E {
    CVI_HAL_AHD_STATUS_CONNECT = 0,
    CVI_HAL_AHD_STATUS_DISCONNECT,
    CVI_HAL_AHD_STATUS_BUIT
} CVI_HAL_AHD_STATUS_E;

typedef struct cviHAL_AHD_ATTR_S {
    CVI_HAL_AHD_STATUS_E enStatus;
    CVI_HAL_AHD_RES_E enRes;
} CVI_HAL_AHD_ATTR_S;

typedef struct cviHAL_AHD_OBJ_S {
    uint32_t id;
    char     name[CVI_HAL_AHD_DEV_NAME_LEN];
    uint32_t u32ChnMax;
    bool     bUsedChn[CVI_HAL_AHD_CHN_MAX]; /* true means used the chn, and only change chn will update it's value */
    int32_t (*pfnInit)(void);
    int32_t (*pfnDeinit)(void);
    int32_t (*pfnStartChn)(uint32_t u32Chn);
    int32_t (*pfnStopChn)(uint32_t u32Chn);
    int32_t (*pfnSetAttr)(const CVI_HAL_AHD_ATTR_S *pstAhdAttr);
    int32_t (*pfnGetAttr)(CVI_HAL_AHD_ATTR_S *pstAhdAttr);
    int32_t (*pfnGetStatus)(CVI_HAL_AHD_STATUS_E astAhdStatus[], uint32_t u32Cnt);
} CVI_HAL_AHD_OBJ_S;

int32_t CVI_HAL_AHD_Register(CVI_HAL_AHD_IDX_E enAhdIndex, const CVI_HAL_AHD_OBJ_S *pstAhdObj);
int32_t CVI_HAL_AHD_UnRegister(CVI_HAL_AHD_IDX_E enAhdIndex);
int32_t CVI_HAL_AHD_Init(CVI_HAL_AHD_IDX_E enAhdIndex);
int32_t CVI_HAL_AHD_Deinit(CVI_HAL_AHD_IDX_E enAhdIndex);
int32_t CVI_HAL_AHD_StartChn(CVI_HAL_AHD_IDX_E enAhdIndex, uint32_t u32Chn);
int32_t CVI_HAL_AHD_StopChn(CVI_HAL_AHD_IDX_E enAhdIndex, uint32_t u32Chn);
int32_t CVI_HAL_AHD_SetAttr(CVI_HAL_AHD_IDX_E enAhdIndex, const CVI_HAL_AHD_ATTR_S *pstAttr);
int32_t CVI_HAL_AHD_GetAttr(CVI_HAL_AHD_IDX_E enAhdIndex, CVI_HAL_AHD_ATTR_S *pstAttr);
int32_t CVI_HAL_AHD_GetStatus(CVI_HAL_AHD_IDX_E enAhdIndex, CVI_HAL_AHD_STATUS_E astAhdStatus[], uint32_t u32Cnt);

/** @}*/  /** <!-- ==== HAL_AHD End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_HAL_AHD_H__*/

