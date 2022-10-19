/*
 ********************************************************************
 * Demo program on CviTek cv183x
 *
 * Copyright CviTek Techanelologies. All Rights Reserved.
 * 
 * Author:  liang.wang@cvitek.com
 *
 ********************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <stdbool.h>

#include "cvi_hal_pwm.h"

#define MAX_BUF 64

#define SYSFS_PWM_DIR "/sys/class/pwm/pwmchip0"

static int _CheckGrpChnValue(int grp, int chn)
{
	if (!((chn >= 0) && (chn <= 3))) {
		printf("pwm chanel 0 ~ 3\n");
		return -1;
	}

	if (!((grp >= 0) && (grp <= 3))) {
		printf("pwm chanel 0 ~ 3\n");
		return -1;
	}

	return 0;
}

int CVI_HAL_PwmExport(int grp, int chn)
{
	int fd;
	char buf[MAX_BUF];

	if (_CheckGrpChnValue(grp, chn) != 0)
		return -1;

	snprintf(buf, sizeof(buf), SYSFS_PWM_DIR"/pwm%d", chn);

	if ((access(buf, F_OK)) == -1) {
		fd = open(SYSFS_PWM_DIR"/export", O_WRONLY);
		if (fd < 0) {
			printf("open export error\n");
			return -1;
		}
		if (chn == 0)
			write(fd, "0", strlen("0"));
		else if (chn == 1)
			write(fd, "1", strlen("1"));
		else if (chn == 2)
			write(fd, "2", strlen("2"));
		else if (chn == 3)
			write(fd, "3", strlen("3"));

		close(fd);
	}

	return 0;
}

int CVI_HAL_PwmUnExport(int grp, int chn)
{
	int fd;
	char buf[MAX_BUF];

	if (_CheckGrpChnValue(grp, chn) != 0)
		return -1;

	snprintf(buf, sizeof(buf), SYSFS_PWM_DIR"/pwm%d", chn);

	if ((access(buf, F_OK)) != -1) {
		fd = open(SYSFS_PWM_DIR"/unexport", O_WRONLY);
		if (fd < 0) {
			printf("open unexport error\n");
			return -1;
		}
		if (chn == 0)
			write(fd, "0", strlen("0"));
		else if (chn == 1)
			write(fd, "1", strlen("1"));
		else if (chn == 2)
			write(fd, "2", strlen("2"));
		else if (chn == 3)
			write(fd, "3", strlen("3"));

		close(fd);
	}
	return 0;
}

int CVI_HAL_PwmSetParm(int grp, int chn, int period, int duty_cycle)
{
	int fd;
	int len;
	char buf[MAX_BUF], buf1[MAX_BUF];

	if (_CheckGrpChnValue(grp, chn) != 0)
		return -1;

	snprintf(buf, sizeof(buf), SYSFS_PWM_DIR"/pwm%d/period", chn);
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		printf("open period error\n");
		return -1;
	}
	snprintf(buf1, sizeof(buf1), "%d", period);
	write(fd, buf1, sizeof(buf1));
	close(fd);

	snprintf(buf, sizeof(buf), SYSFS_PWM_DIR"/pwm%d/duty_cycle", chn);
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		printf("open duty_cycle error\n");
		return -1;
	}
	snprintf(buf1, sizeof(buf1), "%d", duty_cycle);
	write(fd, buf1, sizeof(buf1));
	close(fd);

	return 0;
}

int CVI_HAL_PwmEnable(int grp, int chn)
{
	int fd;
	char buf[MAX_BUF];

	if (_CheckGrpChnValue(grp, chn) != 0)
		return -1;

	snprintf(buf, sizeof(buf), SYSFS_PWM_DIR"/pwm%d/enable", chn);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		printf("open period error\n");
		return -1;
	}

	write(fd, "1", strlen("1"));

	close(fd);
	return 0;
}

int CVI_HAL_PwmDisable(int grp, int chn)
{
	int fd;
	char buf[MAX_BUF];

	if (_CheckGrpChnValue(grp, chn) != 0)
		return -1;

	snprintf(buf, sizeof(buf), SYSFS_PWM_DIR"/pwm%d/enable", chn);

	if ((access(buf, F_OK)) != -1) {
		fd = open(buf, O_WRONLY);
		if (fd < 0) {
			printf("open period error\n");
			return -1;
		}

		write(fd, "0", strlen("0"));

		close(fd);
	}

	return 0;
}
