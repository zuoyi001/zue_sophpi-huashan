#ifndef __CVI_PWM_H__
#define __CVI_PWM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

int CVI_PWM_Export(int grp, int chn);
int CVI_PWM_UnExport(int grp, int chn);
int CVI_PWM_Set_Parm(int grp, int chn, int period, int duty_cycle);
int CVI_PWM_Enable(int grp, int chn);
int CVI_PWM_Disable(int grp, int chn);
int CVI_PWM_Init(void);
int CVI_PWM_Deinit(void);

int CVI_BL_PWM_Init(void);
int CVI_BL_PWM_Get_Percent(void);
int CVI_BL_PWM_Set_Percent(int percentage);
int CVI_BL_PWM_Set_Param(int duty_cycle);
int CVI_BL_PWM_Deinit(void);
int CVI_BL_PWM_PwrOff(void);
int CVI_BL_PWM_PwrOn(void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __CVI_PWM_H__ */
