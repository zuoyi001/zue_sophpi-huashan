#ifndef __CVI_HAL_WIFI_COMMON_H__
#define __CVI_HAL_WIFI_COMMON_H__

#include "cvi_hal_wifi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_WIFI_COMMON */
/** @{ */  /** <!-- [HAL_WIFI_COMMON] */

/** macro define */
#define HAL_WIFI_IFNAMSIZ (16)
#define HAL_WIFI_INTERFACE_NAME "wlan0"
#define HAL_WIFI_DEV_DIR  "/dev/wifi"
#define HAL_WIFI_KILL_EXECUTE_FILE  "/usr/bin/killall"

/** @}*/  /** <!-- ==== HAL_WIFI_COMMON End ====*/

int CVI_WIFI_HAL_GetInterface(const char* pszIfname);
int CVI_WIFI_UTILS_Ifconfig (const char* pszIfname, int s32Up);
int CVI_WIFI_UTILS_SetIp(const char* pszIfname, const char* pszIp);
int CVI_WIFI_UTILS_AllFDClosexec(void);
void CVI_WIFI_HAL_PowerOnReset(void);
void CVI_WIFI_HAL_SysPreInit(void);
int CVI_WIFI_HAL_LoadAPDriver(void);
int CVI_WIFI_HAL_LoadSTADriver(void);
int CVI_WIFI_HAL_RemoveDriver(void);
int CVI_WIFI_HAL_UpdateApConfig(char *ifname,  const CVI_HAL_WIFI_APMODE_CFG_S *pstApCfg, const char *pszConfigFile);
int CVI_WIFI_AP_Start(char *ifname,const CVI_HAL_WIFI_APMODE_CFG_S* pstApCfg);
int CVI_WIFI_AP_Stop(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif/* End of #ifdef __cplusplus */

#endif /* End of __CVI_HAL_WIFI_COMMON_H__*/
