#ifndef __CVI_HAL__PWM_H__
#define __CVI_HAL__PWM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#include "autoconf.h"

int CVI_HAL_PwmExport(int grp, int chn);
int CVI_HAL_PwmUnExport(int grp, int chn);
int CVI_HAL_PwmSetParm(int grp, int chn, int period, int duty_cycle);
int CVI_HAL_PwmEnable(int grp, int chn);
int CVI_HAL_PwmDisable(int grp, int chn);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __CVI_HAL__PWM_H__ */