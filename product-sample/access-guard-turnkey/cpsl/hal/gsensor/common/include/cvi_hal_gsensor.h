/**
 * @file    cvi_hal_gsensor.h
 * @brief   product hal gsensor interface
 *
 * Copyright (c) 2020 CVITEK Co.,Ltd
 *
 *
 */

#ifndef __CVI_HAL_GSENSOR_H__
#define _CVI_HAL_GSENSOR_H__
#include <stdint.h>
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


typedef struct cvHAL_GSENSOR_OBJ_S {
	int32_t (*i2c_bus_init)(int busname);
	int32_t (*i2c_bus_deinit)(void);
	int32_t (*init)(void);
	int32_t (*deinit)(void);
	int32_t (*read_data)(short *x, short *y, short *z);
	int32_t (*set_sensitity)(unsigned char sensitity);
	int32_t (*read_int_status)(unsigned char *int_status);
} CVI_HAL_GSENSOR_OBJ_S;

/** screen context */
typedef struct tagHAL_GSESOR_CTX_S {
	CVI_HAL_GSENSOR_OBJ_S stGsensorObj;
	uint8_t bRegister;
	uint8_t bInit;
} HAL_GSENSOR_CTX_S;

/* @brief sensitity enum*/
typedef enum cvHAL_GSENSOR_SENSITITY_E {
	CVI_HAL_GSENSOR_SENSITITY_OFF = 0, /**<gsensor off*/
	CVI_HAL_GSENSOR_SENSITITY_LOW,     /**<low sensitity*/
	CVI_HAL_GSENSOR_SENSITITY_MIDDLE,  /**<middle sensitity*/
	CVI_HAL_GSENSOR_SENSITITY_HIGH,    /**<high sensitity*/
	CVI_HAL_GSENSOR_SENSITITY_BUIT
} CVI_HAL_GSENSOR_SENSITITY_E;

/* @brief gesensor value*/
typedef struct cvHAL_GSENSOR_VALUE_S {
	int16_t s16XDirValue; /**<x direction value,unit acceleration of gravity g*/
	int16_t s16YDirValue; /**<y direction value,unit acceleration of gravity g*/
	int16_t s16ZDirValue; /**<z direction value,unit acceleration of gravity g*/
} CVI_HAL_GSENSOR_VALUE_S;

/* @brief gesensor chip work attr*/
typedef struct cvHAL_GSENSOR_ATTR_S {
	uint32_t u32SampleRate; /**<sample rate,0 mean Adopt default,not config,unit
                               kps*/
} CVI_HAL_GSENSOR_ATTR_S;

typedef int32_t (*CVI_HAL_GSENSOR_ON_COLLISION_PFN)(void *pvPrivData);

/** hal gsensor Configure */
typedef struct cvHAL_GSENSOR_CFG_S {
	CVI_HAL_GSENSOR_SENSITITY_E enSensitity;
	CVI_HAL_GSENSOR_ATTR_S stAttr;
	int busnum;
} CVI_HAL_GSENSOR_CFG_S;

/**
 * @brief    hal gsensor obj register, copy sensor callback
 * threshold
 * @param[in] pstGsensorObj: gsensor callback obj
 * @return 0 success,non-zero error code.
 * @exception    None
 */

int32_t CVI_HAL_GSENSOR_Register(const CVI_HAL_GSENSOR_OBJ_S *pstGsensorObj);

/**
 * @brief    hal gsensor initialization, insmod gesnor driver and init gsensor
 * threshold
 * @param[in] stCfg: include Collision  work attribute��threshold value  and
 * collision callback function
 * @return 0 success,non-zero error code.
 * @exception    None
 */
int32_t CVI_HAL_GSENSOR_Init(const CVI_HAL_GSENSOR_CFG_S *pstCfg);

/**
 * @brief    dynamic set Collision threshold value
 * @param[in] enSensitity: Collision enSensitity level
 * @return 0 success,non-zero error code.
 * @exception    None
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/03/05
 */
int32_t CVI_HAL_GSENSOR_SetSensitity(CVI_HAL_GSENSOR_SENSITITY_E enSensitity);


/**
 * @brief    dynamic set Collision  work attribute
 * @param[in] pstAttr: Collision work attribute
 * @return 0 success,non-zero error code.
 * @exception    None
 */
int32_t CVI_HAL_GSENSOR_SetAttr(const CVI_HAL_GSENSOR_ATTR_S *pstAttr);

/**
 * @brief    dynamic get current sample value
 * @param[out] pstCurValue: current sample value
 * @return 0 success,non-zero error code.
 * @exception    None
 */
int32_t CVI_HAL_GSENSOR_GetCurValue(CVI_HAL_GSENSOR_VALUE_S *pstCurValue);

/**
 * @brief    dynamic get collision occur or not
 * @param[out] pbOnCollison: current sample value
 * @return 0 success,non-zero error code.
 * @exception    None
 */
int32_t CVI_HAL_GSENSOR_GetCollisionStatus(uint8_t *pbOnCollison);

/**
 * @brief    hal gsensor deinitialization, rmmod driver
 * @return 0 success,non-zero error code.
 * @exception    None
 */
int32_t CVI_HAL_GSENSOR_DeInit(void);

/** @}*/ /** <!-- ==== HAL_GSENSOR End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_HAL_GSENSOR_H__*/
