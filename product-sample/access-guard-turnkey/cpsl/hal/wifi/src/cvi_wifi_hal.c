#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "cvi_hal_wifi_common.h"
#include "cvi_hal_wifi.h"
#include "cvi_hal_gpio.h"
#include "cvi_sysutils.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif  /* End of #ifdef __cplusplus */

void CVI_WIFI_HAL_PowerOnReset(void)
{
    return;    /** 8189 need not power on reset*/
}

void CVI_WIFI_HAL_SysPreInit(void)
{
    static bool bPinOutInit = false;
    if(true == bPinOutInit)
    {
        printf("hal_system_preinit already init\n");
        return;
    }
	bPinOutInit = true;
    /** SDIO1-MUX*/

}

int CVI_WIFI_HAL_LoadAPDriver(void)
{
    int s32Ret = 0;

    s32Ret = cvi_insmod("/mnt/system/ko/3rd/8189fs.ko", NULL);
    if(0 != s32Ret)
    {
        printf("insmod ap: failed, errno(%d)\n", errno);
        return -1;
    }
    sleep(1);

    return 0;
}


int CVI_WIFI_HAL_LoadSTADriver(void)
{
    printf("insmod sta mode driver not support,failed\n");
    return -1;
}

int CVI_WIFI_HAL_RemoveDriver(void)
{
    int s32Ret = 0;

    s32Ret = cvi_rmmod("/mnt/system/ko/3rd/8189fs.ko");
    if(0 != s32Ret)
    {
        printf("insmod ap: failed, errno(%d)\n", errno);
        return -1;
    }
    return 0;
}
int CVI_WIFI_HAL_UpdateApConfig(char *ifname,  const CVI_HAL_WIFI_APMODE_CFG_S *pstApCfg, const char *pszConfigFile)
{
    int s32Ret = 0;
    int s32fd;
    char* szWbuf = NULL;
    /** open configure file, if not exist, create it */
    s32fd = open(pszConfigFile, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if(s32fd < 0)
    {
        printf("WiFi: Cann't open configure file '%s',errno(%d)", pszConfigFile,errno);
        return -1;
    }

    asprintf(&szWbuf, "interface=%s\n"
                    "driver=%s\n"
                    "wpa=3\n"
                    "ctrl_interface=/dev/wifi/hostapd\n"
                    "ssid=%s\n"
                    "wpa_passphrase=%s\n"
                    "channel=%d\n"
                    "ignore_broadcast_ssid=%d\n"
                    "hw_mode=g\n"
                    "ieee80211n=1\n"
                    "ht_capab=[SHORT-GI-20][SHORT-GI-40]%s\n"
                    "wpa_key_mgmt=WPA-PSK\n"
                    "wpa_pairwise=CCMP\n"
                    "max_num_sta=1\n",
                    ifname, "nl80211", pstApCfg->stCfg.szWiFiSSID, pstApCfg->stCfg.szWiFiPassWord, pstApCfg->s32Channel,
                    pstApCfg->bHideSSID? 1 : 0,
                    (pstApCfg->s32Channel > 4) ? "[HT40-]" : "[HT40+]");

    if (write(s32fd, szWbuf, strlen(szWbuf)) < 0) {
        printf("WiFi: Cann't write configuration to '%s',errno(%d)\n", pszConfigFile,errno);
        s32Ret = -1;
    }
    close(s32fd);
    free(szWbuf);
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */
