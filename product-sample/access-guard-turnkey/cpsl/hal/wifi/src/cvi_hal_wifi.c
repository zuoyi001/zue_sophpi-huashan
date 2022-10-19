#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "cvi_hal_wifi_common.h"
#include "cvi_hal_wifi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
/** macro define */
#define SSID_MAX_LEN (31)
#define SSID_MIN_LEN (1)
#define PWD_MAX_LEN (63)
#define PWD_MIN_LEN (8)

#define HAL_WIFI_HOSTAPD_CONFIG_FILE ("/dev/wifi/hostapd/hostapd.conf")

static bool s_bHALWIFIInitState = false;
static CVI_HAL_WIFI_MODE_E s_enHALWIFIMode = CVI_HAL_WIFI_MODE_BUIT;
static bool s_bHALWIFIStartState = false;
static pthread_mutex_t sWifiMutex = PTHREAD_MUTEX_INITIALIZER;

static int HAL_WIFI_ApValidChck(const CVI_HAL_WIFI_APMODE_CFG_S* pstApCfg)
{
    /** cahnnel check */
    if((pstApCfg->s32Channel < 1) ||(pstApCfg->s32Channel > 165))
    {
        printf("[Errot]s32Channel(%d),invalid\n",pstApCfg->s32Channel);
        return -1;
    }
    if ((strlen(pstApCfg->stCfg.szWiFiSSID) < SSID_MIN_LEN) ||
            (strlen(pstApCfg->stCfg.szWiFiSSID) > SSID_MAX_LEN)) { /** < ssid check */
        printf("[Errot]szWiFiSSID len (%lu),invalid\n",(unsigned long)strlen(pstApCfg->stCfg.szWiFiSSID));
        return -1;
    }
	if ((strlen(pstApCfg->stCfg.szWiFiPassWord) < PWD_MIN_LEN) ||
            (strlen(pstApCfg->stCfg.szWiFiPassWord) > PWD_MAX_LEN)) { /** < ssid check */
        printf("[Errot]szWiFiPassWord len (%lu),invalid\n",(unsigned long)strlen(pstApCfg->stCfg.szWiFiPassWord));
        return -1;
    }
    return 0;
}
static int HAL_WIFI_StaValidChck(const CVI_HAL_WIFI_STAMODE_CFG_S* pstStaCfg)
{
    printf("[Errot]wifi STA mode parm validcheck not support %d", pstStaCfg->enStaMode);
    return -1;
}

static int HAL_WIFI_InParmValidChck(const CVI_HAL_WIFI_CFG_S* pstCfg)
{
    int s32Ret = 0;
    switch(pstCfg->enMode)
    {
        case CVI_HAL_WIFI_MODE_STA:
            s32Ret = HAL_WIFI_StaValidChck(&pstCfg->unCfg.stStaCfg);
            break;
        case CVI_HAL_WIFI_MODE_AP:
            s32Ret = HAL_WIFI_ApValidChck(&pstCfg->unCfg.stApCfg);
            break;
        default:
            s32Ret = -1;
            printf("[Errot]enMode(%d),invalid\n",pstCfg->enMode);
            break;

    }
    return s32Ret;
}

int CVI_HAL_WIFI_Init(CVI_HAL_WIFI_MODE_E enMode)
{
    int s32Ret = 0;
    pthread_mutex_lock(&sWifiMutex);
    if (true == s_bHALWIFIInitState)
    {
        pthread_mutex_unlock(&sWifiMutex);
        printf("[Errot]wifi init already\n");
        return -1;
    }
    CVI_WIFI_HAL_SysPreInit();

    switch (enMode)
    {
        case CVI_HAL_WIFI_MODE_AP:
            s32Ret = CVI_WIFI_HAL_LoadAPDriver();
            break;
        default:
            printf("enMode(%d) not support at present or error!\n",enMode);
            s32Ret = -1;
    }
    if (0 == s32Ret)
    {
        s_enHALWIFIMode = enMode;
        s_bHALWIFIInitState = true;
    }
    else
    {
        s32Ret = -1;
    }
    pthread_mutex_unlock(&sWifiMutex);
    return s32Ret;
}

int CVI_HAL_WIFI_CheckeCfgValid(const CVI_HAL_WIFI_CFG_S* pstCfg,bool* pCfgValid)
{
    int s32Ret = 0;
    if ((NULL == pstCfg) || (NULL == pCfgValid))
    {
        printf("[Errot]pstCfg or pCfgValid is null,error\n");
        return -1;
    }

    *pCfgValid = false;

    s32Ret = HAL_WIFI_InParmValidChck(pstCfg);
    if (0 == s32Ret)
    {
        *pCfgValid = true;
    }
    return 0;
}

int CVI_HAL_WIFI_Start(const CVI_HAL_WIFI_CFG_S* pstCfg)
{
    int s32Ret = 0;
    pthread_mutex_lock(&sWifiMutex);
    if (false == s_bHALWIFIInitState)
    {
        pthread_mutex_unlock(&sWifiMutex);
        printf("[Errot]wifi not init\n");
        return -1;
    }
    if (true == s_bHALWIFIStartState)
    {
        pthread_mutex_unlock(&sWifiMutex);
        printf("[Errot]wifi start already\n");
        return -1;
    }
    if(pstCfg->enMode != s_enHALWIFIMode)
    {
        pthread_mutex_unlock(&sWifiMutex);
        printf("[Errot]start mode(%d) and init mode(%d) must be same,error\n",s_enHALWIFIMode,pstCfg->enMode);
        return -1;
    }
    s32Ret = HAL_WIFI_InParmValidChck(pstCfg);
    if (0 != s32Ret)
    {
        pthread_mutex_unlock(&sWifiMutex);
        printf("wifi inparm valid error!\n");
        return -1;
    }
    switch (s_enHALWIFIMode)
    {
        case CVI_HAL_WIFI_MODE_STA:
            printf("wifi sta not support at present!\n");
            s32Ret = -1;
            break;
        case CVI_HAL_WIFI_MODE_AP:
            s32Ret = CVI_WIFI_AP_Start(HAL_WIFI_INTERFACE_NAME,&pstCfg->unCfg.stApCfg);
            if(0 == s32Ret)
            {
                s_bHALWIFIStartState = true;
            }
            else
            {
                s32Ret = -1;
            }
            break;
        default:
            printf("enMode(%d) error!\n",s_enHALWIFIMode);
            s32Ret = -1;
    }
    pthread_mutex_unlock(&sWifiMutex);
    return s32Ret;
}

int CVI_HAL_WIFI_GetStartedStatus(bool* pbEnable)
{
    if (NULL == pbEnable)
    {
        printf("[Errot]pbEnable is null,error\n");
        return -1;
    }
    pthread_mutex_lock(&sWifiMutex);
    *pbEnable = s_bHALWIFIStartState;
    pthread_mutex_unlock(&sWifiMutex);
    return 0;
}

int CVI_HAL_WIFI_Stop(void)
{
    int s32Ret = 0;
    pthread_mutex_lock(&sWifiMutex);
    if (false == s_bHALWIFIInitState)
    {
        pthread_mutex_unlock(&sWifiMutex);
        printf("[Errot]wifi not init,can not stop\n");
        return -1;
    }
    if (false == s_bHALWIFIStartState)
    {
        pthread_mutex_unlock(&sWifiMutex);
        printf("[Errot]wifi stop already\n");
        return -1;
    }
    s32Ret = CVI_WIFI_AP_Stop();
    if(0 == s32Ret)
    {
        s_bHALWIFIStartState = false;
    }
    else
    {
        s32Ret = -1;
    }
    pthread_mutex_unlock(&sWifiMutex);
    return s32Ret;
}

int CVI_HAL_WIFI_Deinit(void)
{
    int s32Ret = 0;
    pthread_mutex_lock(&sWifiMutex);
    if (false == s_bHALWIFIInitState)
    {
        pthread_mutex_unlock(&sWifiMutex);
        printf("[Errot]wifi deinit already\n");
        return -1;
    }
    s32Ret = CVI_WIFI_HAL_RemoveDriver();
    if(0 == s32Ret)
    {
        s_enHALWIFIMode = CVI_HAL_WIFI_MODE_BUIT;
        s_bHALWIFIInitState = false;
    }
    else
    {
        s32Ret = -1;
    }
    pthread_mutex_unlock(&sWifiMutex);
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

