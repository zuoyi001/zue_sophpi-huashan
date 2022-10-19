/*
 ********************************************************************
 * Demo program on CviTek cv183x
 *
 * Copyright CviTek Technologies. All Rights Reserved.
 *
 * Author:  liang.wang@cvitek.com
 *
 ********************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <poll.h>
#include "cvi_hal_gpio.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64
#define CHECK_GPIO_NUMBER(x)                                                                                       \
        do {                                                                                                       \
            if ((x < CVI_HAL_GPIO_NUM_MIN) || (x > CVI_HAL_GPIO_NUM_MAX)) {                                                        \
                printf("\033[0;31m GPIO %d is invalid at %s: LINE: %d!\033[0;39m\n", x, __func__, __LINE__);       \
                return -1;                                                                                         \
            }                                                                                                      \
        } while (0)

static int GpioExport(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		printf("gpio %d export error\n", gpio);
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);

	return 0;
}

static int GpioUnexport(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		printf("gpio %d unexport error\n", gpio);
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}

static int GpioSetDirection(unsigned int gpio, unsigned int out_flag)
{
	int fd;
	char buf[MAX_BUF];
	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR"/gpio%d/direction", gpio);
	if (access(buf, 0) == -1) {
		GpioExport(gpio);
	}

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		printf("gpio %d set-direction error\n", gpio);
		return fd;
	}

	//printf("mark %d , %s \n",out_flag, buf);
	if (out_flag)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);

	close(fd);
	return 0;
}

static int GpioSetValue(unsigned int gpio, unsigned int value)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR"/gpio%d/value", gpio);
	if (access(buf, 0) == -1) {
		//BM_LOG(LOG_DEBUG_ERROR, cout << buf << " not exist!" << endl);
		GpioExport(gpio);
	}

	fd = GpioSetDirection(gpio, 1); //output
	if (fd < 0) {
		printf("gpio %d set-value error\n", gpio);
		return fd;
	}

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		printf("gpio %d set-value error\n", gpio);
		return fd;
	}

	if (value != 0) {
		write(fd, "1", 2);
	} else {
		write(fd, "0", 2);
	}

	close(fd);
	return 0;
}

static int GpioGetValue(unsigned int gpio, unsigned int *value)
{
	int fd;
	char buf[MAX_BUF];
	char ch;

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	if (access(buf, 0) == -1) {
		//BM_LOG(LOG_DEBUG_ERROR, cout << buf << " not exist!" << endl);
		GpioExport(gpio);
	}

	fd = GpioSetDirection(gpio, 0); //input
	if (fd < 0) {
		printf("gpio %d get-value error\n", gpio);
		return fd;
	}

	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		printf("gpio %d get-value error\n", gpio);
		return fd;
	}

	read(fd, &ch, 1);
	// printf(" GpioGetValue = %c \n",ch);

	if (ch != '0') {
		*value = 1;
	} else {
		*value = 0;
	}

	close(fd);
	return 0;
}

static int GpioSetEdge(unsigned int gpio, unsigned int edge_flag)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR"/gpio%d/edge", gpio);
	if (access(buf, 0) == -1) {
		GpioExport(gpio);
	}

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		printf("gpio %d set-edge error\n", gpio);
		return fd;
	}

	//printf("mark %d , %s \n",edge_flag, buf);
	switch (edge_flag) {
	case CVI_HAL_GPIO_EDGE_NONE:
		write(fd, "none", 5);
		break;
	case CVI_HAL_GPIO_EDGE_RISING:
		write(fd, "rising", 7);
		break;
	case CVI_HAL_GPIO_EDGE_FALLING:
		write(fd, "falling", 8);
		break;
	case CVI_HAL_GPIO_EDGE_BOTH:
		write(fd, "both", 5);
		break;
	default:
		write(fd, "none", 5);
		break;
	}

	close(fd);
	return 0;
}

static int GpioPoll(unsigned int gpio, unsigned int event, FunType Fp)
{
	struct pollfd pfd;
	int fd;
	char value;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR"/gpio%d/value", gpio);
	fd = open(buf, O_RDWR);
	if (fd < 0) {
		printf("gpio %d open error\n", gpio);
		return -1;
	}
	if (lseek(fd, 0, SEEK_SET) == -1) {
		printf("gpio %d lseek error\n", gpio);
		return -1;
	}
	if (read(fd, &value, 1) == -1) {
		printf("gpio %d read error\n", gpio);
		return -1;
	}

	pfd.fd = fd;
	pfd.events = event;

	while (1) {
		if (poll(&pfd, 1, 1000) == -1) {
			printf("gpio %d poll error\n", gpio);
			return -1;
		}

		if (pfd.revents & POLLPRI) {
			if (lseek(fd, 0, SEEK_SET) == -1) {
				printf("gpio %d lseek error\n", gpio);
				return -1;
			}
			if (read(fd, &value, 1) == -1) {
				printf("gpio %d read error\n", gpio);
				return -1;
			}
			printf("poll value:%c\n", value);
			if (Fp) {
				Fp();
			}

		}
		if (pfd.revents & POLLERR) {
			printf("gpio %d POLLERR\n", gpio);
		}

		usleep(500 * 1000);
	}

	close(fd);

	return 0;
}


int CVI_HAL_GPIO_Export(CVI_HAL_GPIO_NUM_E gpio)
{
	int ret = 0;
	CHECK_GPIO_NUMBER(gpio);
	ret = GpioExport(gpio);
	return ret;
}

int CVI_HAL_GPIO_Unexport(CVI_HAL_GPIO_NUM_E gpio)
{
	int ret = 0;
	CHECK_GPIO_NUMBER(gpio);
	ret = GpioUnexport(gpio);
	return ret;
}

int CVI_HAL_GpioDirectionInput(CVI_HAL_GPIO_NUM_E gpio)
{
	int ret = 0;
	CHECK_GPIO_NUMBER(gpio);
	ret = GpioSetDirection(gpio, 0);
	return ret;
}

int CVI_HAL_GpioDirectionOutput(CVI_HAL_GPIO_NUM_E gpio)
{
	int ret = 0;
	CHECK_GPIO_NUMBER(gpio);
	ret = GpioSetDirection(gpio, 1);
	return ret;
}

int CVI_HAL_GpioSetValue(CVI_HAL_GPIO_NUM_E gpio, CVI_HAL_GPIO_VALUE_E value)
{
	int ret = 0;
	CHECK_GPIO_NUMBER(gpio);
	ret = GpioSetValue(gpio, value);
	return ret;
}

int CVI_HAL_GpioGetValue(CVI_HAL_GPIO_NUM_E gpio, CVI_HAL_GPIO_VALUE_E *value)
{
	int ret = 0;
	CHECK_GPIO_NUMBER(gpio);
	ret = GpioGetValue(gpio, value);
	return ret;
}

int CVI_HAL_GpioPoll(CVI_HAL_GPIO_NUM_E gpio, CVI_HAL_GPIO_EDGE_E edge, FunType Fp)
{
	int ret = 0;
	CHECK_GPIO_NUMBER(gpio);

	//打开gpio
	GpioExport(gpio);

	//设为输入模式
	ret = GpioSetDirection(gpio, 0);

	//设置中断触发
	ret = GpioSetEdge(gpio, edge);

	ret = GpioPoll(gpio, POLLPRI | POLLERR, Fp);

	//操作完毕,释放gpio
	ret = GpioUnexport(gpio);

	return ret;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
