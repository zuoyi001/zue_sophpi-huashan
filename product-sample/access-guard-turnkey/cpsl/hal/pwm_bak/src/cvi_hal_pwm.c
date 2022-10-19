/*
 ********************************************************************
 * Demo program on CviTek cv183x
 *
 * Copyright CviTek Techanelologies. All Rights Reserved.
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
#include "cvi_sysutils.h"
#define MAX_BUF 64
#define PWM_KO_PATH "/mnt/system/ko/cv183x_pwm.ko"
#define SYSFS_PWM_DIR "/sys/class/pwm/pwmchip0"

#define DEFAULT_PERIOD 20000
#define DEFAULT_DUTY_CYCLE 10000
static  int pwm_init = 0;
#define BL_PWM   1   //maybe 1 2 3
int check_grp_chn_value(int grp, int chn)
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

int CVI_PWM_Export(int grp, int chn)
{
	int fd;
	char buf[MAX_BUF];
	if(pwm_init != 1) {
		CVI_PWM_Init();
		cvi_system("devmem 0x030010A4 32 0x0");
		pwm_init = 1;
	}
	if (check_grp_chn_value(grp, chn) != 0)
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

int CVI_PWM_UnExport(int grp, int chn)
{
	int fd;
	char buf[MAX_BUF];

	if (check_grp_chn_value(grp, chn) != 0)
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

int CVI_PWM_Set_Parm(int grp, int chn, int period, int duty_cycle)
{
	int fd;
	char buf[MAX_BUF], buf1[MAX_BUF];

	if (check_grp_chn_value(grp, chn) != 0)
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

static void pwm_save_value(int duty_cycle)
{
	char str_pwm[16] = {0};
	char shellcmd[32] = {0};
    int pwmstatus = (((DEFAULT_PERIOD & 0xFFFF) << 16) | ((duty_cycle & 0xFFFF) << 0));
    memset(str_pwm, 0 ,sizeof(str_pwm));
    sprintf(str_pwm, "%X", pwmstatus);
    snprintf(shellcmd, sizeof(shellcmd), "%s %s %s",
            "fw_setenv", "pwmstatus", str_pwm);
    system(shellcmd);
}

static int get_pwm_value_sum(void)
{
    char *shellcmd = "fw_printenv pwmstatus";
    char *buf = NULL;
    char val[32] = {0};
    uint32_t pwm_env = 0;
    FILE* fp = NULL;

    fp = popen(shellcmd, "r");
    if (fp == NULL) {
        printf("get_pwm_value_sum popen failed\n");
        return false;
    }

    fgets(val, sizeof(val), fp);
    pclose(fp);

    buf = strtok(val, "=");
    buf = strtok(NULL, "=");
    printf("pwmstatus buf=%s\n", buf);

    if (buf != NULL) {
        pwm_env = strtoul(buf, NULL, 16);
        return pwm_env;
    }
    return false;
}

int CVI_BL_PWM_Get_Percent(void)
{
	if(pwm_init == 0)
	{
		CVI_BL_PWM_Init();
	}

	uint32_t pwm_value = get_pwm_value_sum();
	uint32_t pwm_period = 0xFFFF & (pwm_value >> 16);
	uint32_t pwm_duty_cycle = 0xFFFF & (pwm_value >> 0);
	return (pwm_duty_cycle * 100 / pwm_period);
}

/*the input parm: percentage should between 0 - 100*/
int CVI_BL_PWM_Set_Percent(int percentage)
{
	if(pwm_init == 0)
	{
		CVI_BL_PWM_Init();
	}

	if((percentage < 0) || (percentage > 100))
	{
		printf("input error parm\n");
		return -1;
	}

	int duty_cycle = DEFAULT_PERIOD * percentage / 100;
	if(duty_cycle < 10)
		duty_cycle = 10;
	CVI_PWM_Set_Parm(0, BL_PWM, DEFAULT_PERIOD, duty_cycle);

	pwm_save_value(duty_cycle);
	return 0;
}

int CVI_BL_PWM_Set_Param(int duty_cycle)
{
	if(pwm_init == 0) {
		CVI_BL_PWM_Init();
	}
    if(duty_cycle < 10) {
        duty_cycle = 10;
    } else {
        if(duty_cycle > DEFAULT_PERIOD)
            duty_cycle = DEFAULT_PERIOD;
    }
    return CVI_PWM_Set_Parm(0, BL_PWM, DEFAULT_PERIOD, duty_cycle);
}


int CVI_PWM_Enable(int grp, int chn)
{
	int fd;
	char buf[MAX_BUF];

	if (check_grp_chn_value(grp, chn) != 0)
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

int CVI_PWM_Disable(int grp, int chn)
{
	int fd;
	char buf[MAX_BUF];

	if (check_grp_chn_value(grp, chn) != 0)
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

int CVI_PWM_Init(void)
{
	return cvi_insmod(PWM_KO_PATH, NULL);
}

int CVI_PWM_Deinit(void)
{
	return cvi_rmmod(PWM_KO_PATH);
}

int CVI_BL_PWM_Init(void)
{
    int ret = 0;
	if(pwm_init == 0) {
		ret |= CVI_PWM_Init();
		cvi_system("devmem 0x030010A4 32 0x0");
		ret |= CVI_PWM_Export(0, BL_PWM);
		uint32_t pwm_value = get_pwm_value_sum();
		uint32_t pwm_period = 0xFFFF & (pwm_value >> 16);
        uint32_t pwm_duty_cycle = 0xFFFF & (pwm_value >> 0);
		if(pwm_duty_cycle < 10)
			pwm_duty_cycle = 10;
		ret |= CVI_PWM_Set_Parm(0, BL_PWM, pwm_period, pwm_duty_cycle);
		ret |= CVI_PWM_Enable(0, BL_PWM);
		pwm_init = 1;
	}
    return ret;
}


int CVI_BL_PWM_Deinit(void)
{
	if(pwm_init == 0)
		return 0;
	pwm_init = 0;
    return CVI_PWM_Deinit();
}

int CVI_BL_PWM_PwrOn(void) {
	if(pwm_init == 0) {
		CVI_BL_PWM_Init();
	}
	return CVI_PWM_Set_Parm(0, BL_PWM, DEFAULT_PERIOD, DEFAULT_DUTY_CYCLE);
}

int CVI_BL_PWM_PwrOff(void) {
	if(pwm_init == 0) {
		CVI_BL_PWM_Init();
	}
	return CVI_PWM_Set_Parm(0, BL_PWM, DEFAULT_PERIOD, 10);
}
