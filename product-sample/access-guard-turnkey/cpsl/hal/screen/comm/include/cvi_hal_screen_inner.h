#ifndef __CVI_HAL_SCREEN_INNER_H__
#define __CVI_HAL_SCREEN_INNER_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_SCREEN */
/** @{ */  /** <!-- [HAL_SCREEN] */

/** rename module name */
#ifdef CVI_MODULE
#undef CVI_MODULE
#endif
#define CVI_MODULE "HALSCREEN"

/** screen state */
typedef struct tagHAL_SCREEN_STATE_S
{
    CVI_HAL_SCREEN_IDX_E   enScreenIndex;
    CVI_HAL_SCREEN_STATE_E enState;
} HAL_SCREEN_STATE_S;

typedef struct tagHAL_SCREEN_NUMBER_S
{
    CVI_HAL_SCREEN_IDX_E enScreenIndex;
    uint32_t u32Number;
} HAL_SCREEN_NUMBER_S;

/** screen context */
typedef struct tagHAL_SCREEN_CTX_S
{
    CVI_HAL_SCREEN_OBJ_S stScreenObj;
    bool bRegister;
    bool bInit;
} HAL_SCREEN_CTX_S;

/** @}*/  /** <!-- ==== HAL_SCREEN End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_HAL_SCREEN_INNER_H__*/

