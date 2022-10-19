#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>

#include "cvi_hal_wifi_common.h"
#include "cvi_hal_wifi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define HAL_WIFI_HOSTAPD_CONFIG_DIR   "/dev/wifi/hostapd"
#define HAL_WIFI_HOSTAPD_CONFIG_FILE  "/dev/wifi/hostapd/hostapd.conf"
#define HAL_WIFI_HOSTAPD_EXECUTE_FILE "hostapd"


#define HAL_WIFI_DHCPD_CONFIG_FILE    "/etc/network/udhcpd.conf"
#define HAL_WIFI_DHCPD_EXECUTE_FILE   "/usr/sbin/udhcpd"
#define HAL_WIFI_IP "192.168.1.1"



static int HAL_WIFI_AP_UpdateConfig(char *ifname,  const CVI_HAL_WIFI_APMODE_CFG_S *pstApCfg, const char *pszConfigFile)
{
    int s32Ret = 0;
    DIR *pDir;

    /** ensure /dev/wifi exist */
    pDir = opendir(HAL_WIFI_DEV_DIR);
    if (!pDir) {
        if (mkdir(HAL_WIFI_DEV_DIR, 0666) < 0)
        {
            printf("WiFi: Create '%s' fail,errno(%d)", HAL_WIFI_DEV_DIR,errno);
            return -1;
        }
    }
    else
    {
        closedir(pDir);
    }
    /** ensure hostapd configure file directory exist */
    pDir = opendir(HAL_WIFI_HOSTAPD_CONFIG_DIR);
    if (!pDir)
    {
        if (mkdir(HAL_WIFI_HOSTAPD_CONFIG_DIR, 0666) < 0)
        {
            printf("WiFi: Create '%s' fail,errno(%d)", HAL_WIFI_HOSTAPD_CONFIG_DIR,errno);
            return -1;
        }
    }
    else
    {
        closedir(pDir);
    }
    s32Ret = CVI_WIFI_HAL_UpdateApConfig(ifname,pstApCfg,pszConfigFile);
    if(0 != s32Ret )
    {
        printf("hal update config  fail\n");
        return -1;

    }
    if (chmod(pszConfigFile, 0666) < 0)
    {
        printf("WiFi: Failed to change '%s' to 0666 ,errno(%d)\n", pszConfigFile,errno);
        (void)unlink(pszConfigFile);
        s32Ret = -1;
    }
    return s32Ret;
}

int HAL_WIFI_AP_HostapdStart()
{
    int s32Ret;

    char cmd[192]={'\0'};
    snprintf(cmd,sizeof(cmd),"%s -B %s",HAL_WIFI_HOSTAPD_EXECUTE_FILE,HAL_WIFI_HOSTAPD_CONFIG_FILE);
    s32Ret = system(cmd);
    if(s32Ret == -1)
    {
        if(ECHILD != errno)
        {
            printf("system errno[%d]\n",errno);
        }
    }

    return s32Ret;
}

int HAL_WIFI_AP_DhcpdStart()
{
    int s32Ret;

    char cmd[128]={'\0'};
    snprintf(cmd,sizeof(cmd),"%s %s",HAL_WIFI_DHCPD_EXECUTE_FILE,HAL_WIFI_DHCPD_CONFIG_FILE);
    s32Ret = system(cmd);
    if(s32Ret == -1)
    {
        if(ECHILD != errno)
        {
            printf("system errno[%d]\n",errno);
        }
    }

    return s32Ret;
}

int HAL_WIFI_AP_DhcpdStop()
{
    int s32Ret;
	
    char cmd[128]={'\0'};
    snprintf(cmd,sizeof(cmd),"%s %s",HAL_WIFI_KILL_EXECUTE_FILE,"udhcpd");
    sighandler_t SignalPrev;
    SignalPrev = signal(SIGCHLD,SIG_DFL);
    s32Ret = system(cmd);
    if (s32Ret == -1)
    {
        printf("system errno[%d]\n",errno);
    }
    signal(SIGCHLD,SignalPrev);

    return s32Ret;
}

int HAL_WIFI_AP_HostapdStop()
{
    int s32Ret;

    char cmd[128]={'\0'};
    snprintf(cmd,sizeof(cmd),"%s %s",HAL_WIFI_KILL_EXECUTE_FILE,"hostapd");
    sighandler_t SignalPrev;
    SignalPrev = signal(SIGCHLD,SIG_DFL);
    s32Ret = system(cmd);
    if (s32Ret == -1)
    {
        printf("system errno[%d]\n",errno);
    }
    signal(SIGCHLD,SignalPrev);
	
    return s32Ret;
}


int CVI_WIFI_AP_Start(char *ifname,const CVI_HAL_WIFI_APMODE_CFG_S* pstApCfg)
{
    int s32Ret = 0;
    if((NULL == ifname) || (*ifname == '\0') || (pstApCfg == NULL))
    {
        printf("ifname is null or no content or stApCfg is null,error");
        return -1;
    }
    s32Ret = HAL_WIFI_AP_UpdateConfig(ifname,pstApCfg,HAL_WIFI_HOSTAPD_CONFIG_FILE);
    if(0 != s32Ret)
    {
        printf("update config error\n");
        return -1;
    }
    CVI_WIFI_HAL_PowerOnReset();
    s32Ret = CVI_WIFI_UTILS_Ifconfig(HAL_WIFI_INTERFACE_NAME,1);
    if(0 != s32Ret)
    {
        printf("ifconfig error\n");
    }

    s32Ret = CVI_WIFI_HAL_GetInterface(HAL_WIFI_INTERFACE_NAME);
    if (-1 == s32Ret)
    {
        printf("found no wlan0,get wifi interface fail\n");
        return -1;
    }

    s32Ret = CVI_WIFI_UTILS_SetIp(HAL_WIFI_INTERFACE_NAME,HAL_WIFI_IP);
    if(0 != s32Ret)
    {
        printf("ifconfig error\n");
        return -1;
    }
    s32Ret = CVI_WIFI_UTILS_AllFDClosexec();
    if(0 != s32Ret)
    {
        printf("All fd Closexec error\n");
        return -1;
    }
    s32Ret = HAL_WIFI_AP_HostapdStart();
    if(0 != s32Ret)
    {
        printf("Hostapd Start error\n");
        return -1;
    }
    s32Ret = HAL_WIFI_AP_DhcpdStart();
    if(0 != s32Ret)
    {
        HAL_WIFI_AP_HostapdStop();
        printf("Dhcpd Start error");
        return -1;
    }
    return 0;
}

int CVI_WIFI_AP_Stop(void)
{
    int s32Ret = 0;

    s32Ret = HAL_WIFI_AP_DhcpdStop();
    if(s32Ret != 0) {
        printf("Dhcpd Stop error");
        return s32Ret;
    }
    s32Ret = HAL_WIFI_AP_HostapdStop();
    if(s32Ret != 0) {
        printf("hostapd stop failed\n");
        HAL_WIFI_AP_DhcpdStart();
        return s32Ret;
    }
    s32Ret = CVI_WIFI_UTILS_Ifconfig(HAL_WIFI_INTERFACE_NAME,0);
    if(s32Ret != 0) {
        printf("ifconfig error");
        HAL_WIFI_AP_HostapdStart();
        HAL_WIFI_AP_DhcpdStart();
        return -1;
    }
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
