#ifndef __CVI_HAL_AHD_INNER_H__
#define __CVI_HAL_AHD_INNER_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_AHD */
/** @{ */  /** <!-- [HAL_AHD] */

/** rename module name */
#ifdef CVI_MODULE
#undef CVI_MODULE
#endif
#define CVI_MODULE "HALAHD"

/** screen state */
typedef struct tagHAL_AHD_STATE_S
{
    CVI_HAL_AHD_IDX_E    enAhdIndex;
    CVI_HAL_AHD_STATUS_E enState;
} HAL_AHD_STATE_S;

typedef struct tagHAL_AHD_NUMBER_S
{
    CVI_HAL_AHD_IDX_E enAhdIndex;
    uint32_t u32Number;
} HAL_AHD_NUMBER_S;

/** screen context */
typedef struct tagHAL_AHD_CTX_S
{
    CVI_HAL_AHD_OBJ_S stAhdObj;
    bool bRegister;
    bool bInit;
} HAL_AHD_CTX_S;

/** @}*/  /** <!-- ==== HAL_AHD End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_HAL_AHD_INNER_H__*/

