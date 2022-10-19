/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_hal_watchdog.h
 * Description:
 */

#ifndef __CVI_HAL_WATCHDOG_H__
#define __CVI_HAL_WATCHDOG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#include "autoconf.h"

#define WATCHDOG_REBOOT  5
#define WATCHDOG_TIMEOUT 9

int CVI_HAL_WatchdogOpen(char *node);
int CVI_HAL_WatchdogClose(int fd);
int CVI_HAL_WatchdogKeepalive(void);
int CVI_HAL_WatchdogEnable(void);
int CVI_HAL_WatchdogSetTimeout(int timeout_seconds);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __CVI_HAL_WATCHDOG_H__ */