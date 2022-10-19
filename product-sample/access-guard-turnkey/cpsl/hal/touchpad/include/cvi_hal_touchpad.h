#ifndef __CVI_HAL_TOUCHPAD_H__
#define __CVI_HAL_TOUCHPAD_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** touchpad touch info*/
typedef struct cviHAL_TOUCHPAD_INPUTINFO_S
{
    int s32ID;/**<input id info, one finger or two fingers*/
    int s32X;/**<x coordinate absolute*/
    int s32Y;/**<y coordinate absolute*/
    uint32_t u32Pressure;/**<is press on screen: 0, 1*/
    uint32_t u32TimeStamp;/**<time stamp*/
} CVI_HAL_TOUCHPAD_INPUTINFO_S;

typedef struct cviHAL_TOUCHPAD_OBJ_S
{
    int (*pfnInit)(void);
    int (*pfnStart)(uint32_t* ps32Fd);
    int (*pfnStop)(void);
    int (*pfnReadInputEvent)(CVI_HAL_TOUCHPAD_INPUTINFO_S* pstInputData);
    int (*pfnSuspend)(void);
    int (*pfnResume)(void);
    int (*pfnDeinit)(void);
} CVI_HAL_TOUCHPAD_OBJ_S;


int CVI_HAL_TOUCHPAD_Init(void);
int CVI_HAL_TOUCHPAD_Suspend(void);
int CVI_HAL_TOUCHPAD_Resume(void);
int CVI_HAL_TOUCHPAD_Start(int* ps32Fd);
int CVI_HAL_TOUCHPAD_Stop(void);
int CVI_HAL_TOUCHPAD_ReadInputEvent(CVI_HAL_TOUCHPAD_INPUTINFO_S* pstInputData);
int CVI_HAL_TOUCHPAD_Deinit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif/* End of #ifdef __cplusplus */

#endif /* End of __CVI_HAL_TOUCHPAD_H__*/
