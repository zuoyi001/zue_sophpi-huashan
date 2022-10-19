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
#include <string.h>
#include "cvi_sysutils.h"

static  int adc_init = 0;
#define ADC_KO_PATH "/mnt/system/ko/cvi_saradc.ko"
#define ADC_SYS_PATH "/sys/class/cvi-saradc/cvi-saradc0/device/cv_saradc"

/*adc info*/
#define REFERENCE_VOLTAGE	((float)1500)
#define ADC_RESOLUTION		(4096)
#define STEP_VOLTAGE		(REFERENCE_VOLTAGE/ADC_RESOLUTION)

static int CVI_ADC_Insmod(void)
{
	return cvi_insmod(ADC_KO_PATH, NULL);
}

static int CVI_ADC_Rmmod(void)
{
	return cvi_rmmod(ADC_KO_PATH);
}

static int fd_adc = -1;
static int cvi_adc_init(char *node, char channel)
{
	fd_adc = open(node, O_RDWR);
	if (fd_adc < 0) {
		printf("open adc failed\n");
		return -1;
	}

	write(fd_adc, &channel, 1);
	close(fd_adc);

	return 0;
}

static int cvi_adc_get_value(const char *node)
{
	int val = 0;
	int val_pre = 0;
    char buf[5];
	memset(buf, 0, sizeof(buf));
	ssize_t len;

	fd_adc = open(node, O_RDWR);
	if (fd_adc < 0) {
		printf("open adc failed\n");
		return -1;
	}
	len = read(fd_adc, buf, sizeof(buf));
	buf[len] = 0;
	val_pre = atoi(buf);
	close(fd_adc);
	val = (int)(val_pre * STEP_VOLTAGE);
	return val;
}

int CVI_HAL_ADC_GET_VALUE(int int_adc_channel)
{
	char *adc_path = ADC_SYS_PATH;
	char char_channel;

	if(int_adc_channel != 0 && int_adc_channel != 1 && int_adc_channel != 2)
	{
		printf("channel should between in 1 and 3\n");
		return -1;
	}
	sprintf(&char_channel, "%d", int_adc_channel);
	if(NULL == adc_path)
	{
		printf("point must vailable\n");
		return -1;
	}
	cvi_adc_init(adc_path, char_channel);
	return cvi_adc_get_value(adc_path) * 100 / ADC_RESOLUTION;
}

int CVI_HAL_ADC_Init(void)
{
    int ret = 0;
	if(adc_init == 0) {
		ret |= CVI_ADC_Insmod();
		adc_init = 1;
	}
    return ret;
}

int CVI_HAL_ADC_Deinit(void)
{
	if(adc_init == 0)
		return 0;
	adc_init = 0;
    return CVI_ADC_Rmmod();
}
