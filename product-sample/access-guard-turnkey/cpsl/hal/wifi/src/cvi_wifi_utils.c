/**
* @file    hal_wifi_utils.c
* @brief   hal wifi utils implemention
*
* Copyright (c) 2017 Huawei Tech.Co.,Ltd
*
* @author    HiMobileCam Reference Develop Team
* @date      2017/12/25
* @version

*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/wireless.h>
#include <net/ethernet.h>

#include "cvi_hal_wifi_common.h"
#include "cvi_hal_wifi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif  /* End of #ifdef __cplusplus */
#define HAL_WIFI_HAL_PROC "/proc/net/dev"

int CVI_WIFI_HAL_GetInterface(const char* pszIfname)
{
    int s32Ret = -1;
    char szBuff[1024];
    FILE *pDevicefd = NULL;
    char *pszBegin = NULL, *pszEnd = NULL;
    char szIfName[HAL_WIFI_IFNAMSIZ] = {0,};
    int s32Len;

    /** STA and AP mode - 'wlan0'*/
    snprintf(szIfName, sizeof(szIfName), "%s",pszIfname);
    s32Len = strlen(szIfName);

    pDevicefd = fopen(HAL_WIFI_HAL_PROC, "r");
    if(pDevicefd != NULL)
    {
        /** Eat 2 lines of header */
        (void)fgets(szBuff, sizeof(szBuff), pDevicefd);
        (void)fgets(szBuff, sizeof(szBuff), pDevicefd);

        /** Read each device line */
        while(fgets(szBuff, sizeof(szBuff), pDevicefd))
        {
            /** Skip empty or almost empty lines. It seems that in some
             * cases fgets return a line with only a newline. */
            if((szBuff[0] == '\0') || (szBuff[1] == '\0'))
            {
                continue;
            }
            pszBegin = szBuff;
            while(*pszBegin == ' ')
            {
                pszBegin++;
            }
            pszEnd = strstr(pszBegin, ": ");
            if((pszEnd == NULL) || (((pszEnd - pszBegin) + 1) > (HAL_WIFI_IFNAMSIZ + 1)))/**< not found or pattern not suitable */
            {
                continue;
            }
            if (strncmp(pszBegin, szIfName, s32Len) != 0)
            {
                continue;
            }
            s32Ret = 0;
            break;
        }

    }
    if(NULL != pDevicefd)
    {
        fclose(pDevicefd);
    }
    return s32Ret;
}


int CVI_WIFI_UTILS_Ifconfig(const char* pszIfname, int s32Up)
{
    struct ifreq stIfr;
    int s32fd;

    /* open socket to kernel */
    if ((s32fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("cannot open socket %s", strerror(errno));
        return -1;
    }

    memset(&stIfr, 0, sizeof(struct ifreq));
    strncpy(stIfr.ifr_name, pszIfname, HAL_WIFI_IFNAMSIZ-1);

    if (ioctl(s32fd, SIOCGIFFLAGS, &stIfr) < 0)
    {
        printf(" %s failed!\n", __func__);
        close(s32fd);
        return -1;
    }
    if (s32Up)
    {
        stIfr.ifr_flags = (unsigned int)stIfr.ifr_flags | IFF_UP;
    }
    else
    {
        stIfr.ifr_flags = (unsigned int)stIfr.ifr_flags & (~IFF_UP);
    }

    if (ioctl(s32fd, SIOCSIFFLAGS, &stIfr) < 0)
    {
        printf("  ioctl failed!");
        close(s32fd);
        return -1;
    }
    close(s32fd);
    return 0;
}


int CVI_WIFI_UTILS_SetIp(const char* pszIfname, const char* pszIp)
{
    struct sockaddr_in *pstAddr = NULL;
    struct ifreq stIfr;
    int s32fd;

    /* open socket to kernel */
    if ((s32fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("cannot open socket %s", strerror(errno));
        return -1;
    }

    memset(&stIfr, 0, sizeof(struct ifreq));
    snprintf(stIfr.ifr_name, sizeof(stIfr.ifr_name), "%s", pszIfname);

    pstAddr = (struct sockaddr_in *)&stIfr.ifr_addr;
    pstAddr->sin_family = AF_INET;
    inet_pton(AF_INET, pszIp, (void *)&pstAddr->sin_addr.s_addr);

    if (ioctl(s32fd, SIOCSIFADDR, &stIfr) < 0)
    {
        printf(" ioctl failed!");
        close(s32fd);
        return -1;
    }
    close(s32fd);
    return 0;
}

/**set fd with FD_CLOEXEC,in exec process,fd cannot been used */
int CVI_WIFI_UTILS_AllFDClosexec()
{
    DIR *pDir = NULL;
    int s32fd;
    uint32_t u32Value = 0;
    if(!(pDir = opendir("/proc/self/fd/")))
    {
        printf("[%s]: open fd dir faild !", __func__);
        return -1;
    }
    struct dirent *pDirent = NULL;
    while((pDirent = readdir(pDir)) != NULL){
        if(strncmp(pDirent->d_name, ".", 1) == 0) continue;
        s32fd = atoi(pDirent->d_name);
        if(s32fd >= 3)
        {
            u32Value =(uint32_t)fcntl(s32fd,F_GETFD);
            u32Value |= FD_CLOEXEC;
            uint32_t s32Ret = fcntl(s32fd,F_SETFD,u32Value);
            if(s32Ret != 0) {
                printf("F_SETFD fail,errno[%d]\n",errno);
            }
        }
    }
    closedir(pDir);
    return 0;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */