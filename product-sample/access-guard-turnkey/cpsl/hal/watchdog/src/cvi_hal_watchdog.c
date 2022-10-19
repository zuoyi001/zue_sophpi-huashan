/*
 ********************************************************************
 * Demo program on CviTek cv183x
 *
 * Copyright CviTek Technologies. All Rights Reserved.
 *
 * Author:  jialuo.zhong@cvitek.com
 *
 ********************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>

#include "cvi_hal_watchdog.h"

static int fd_wdt = -1;

int CVI_HAL_WatchdogOpen(char *node)
{
	fd_wdt = open(node, O_WRONLY);
	if (fd_wdt < 0) {
		perror("watchdog open");
		return -1;
	}

	return 0;
}

int CVI_HAL_WatchdogClose(int fd)
{
	int ret;

	ret = close(fd);
	if (ret < 0)
		perror("watchdog close");

	return ret;
}

int CVI_HAL_WatchdogKeepalive(void)
{
	if (ioctl(fd_wdt, WDIOC_KEEPALIVE) < 0) {
		perror("WDIOC_KEEPALIVE");
		return -1;
	}

	return 0;
}

int CVI_HAL_WatchdogEnable(void)
{
	int flag;

	flag = WDIOS_ENABLECARD;
	if (ioctl(fd_wdt, WDIOC_SETOPTIONS, &flag) < 0) {
		perror("WDIOC_SETOPTIONS");
		return -1;
	}

	return 0;
}

int CVI_HAL_WatchdogSetTimeout(int timeout_seconds)
{
	int timeout = timeout_seconds;

	if (ioctl(fd_wdt, WDIOC_SETTIMEOUT, &timeout) < 0) {
		perror("WDIOC_SETTIMEOUT");
		return -1;
	}

	return 0;
}

