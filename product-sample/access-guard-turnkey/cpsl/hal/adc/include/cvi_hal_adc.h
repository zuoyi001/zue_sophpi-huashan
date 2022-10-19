#ifndef __CVI_ADC_H__
#define __CVI_ADC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

int CVI_HAL_ADC_Init(void);
int CVI_HAL_ADC_Deinit(void);
int CVI_HAL_ADC_GET_VALUE(int int_adc_channel);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __CVI_ADC_H__ */
