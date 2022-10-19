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
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
#include <poll.h>
#include <termios.h>

#include "cvi_hal_uart.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
static int fd_uart = -1;
static int speed_arr[] = {B115200, B19200, B9600, B4800, B2400, B1200, B300};
static int buad_rate[] = {115200, 19200, 9600, 4800, 2400, 1200, 300};

int CVI_HAL_UartOpen(char *node)
{
	int fd;

	fd = open(node, O_RDWR);
	if (fd < 0) {
		perror("uart open");
		return fd;
	}

	return fd;
}

int CVI_HAL_UartClose(int fd)
{
	int ret;

	ret = close(fd);
	if (ret < 0)
		perror("uart close");

	return ret;
}

int CVI_HAL_UartSetParam(int speed, int flow_ctrl, int databits, int stopbits, char parity)
{
	unsigned int i;
	struct termios options;

	(void)flow_ctrl;
	if (tcgetattr(fd_uart, &options) != 0) {
		perror("tcgetattr");
		return -1;
	}

	for (i = 0; i < sizeof(speed_arr) / sizeof(int); i++) {
		if (speed == buad_rate[i]) {
			cfsetispeed(&options, speed_arr[i]);
			cfsetospeed(&options, speed_arr[i]);
		}
	}

	bzero(&options, sizeof(options));
	options.c_cflag |= CLOCAL | CREAD;
	options.c_cflag &= ~CSIZE;

	switch (databits) {
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	}

	switch (parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;
		options.c_iflag &= ~INPCK;
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);
		options.c_iflag |= INPCK;
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;
		break;
	case 's':
	case 'S':
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported parity\n");
		return -1;
	}

	if (stopbits == 1)
		options.c_cflag &= ~CSTOPB;
	else if (stopbits == 2)
		options.c_cflag |= CSTOPB;

	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 0;

	tcflush(fd_uart, TCIFLUSH);

	options.c_oflag = ~ICANON;

	if ((tcsetattr(fd_uart, TCSANOW, &options)) != 0) {
		perror("tcsetattr");
		return -1;
	}

	return 0;
}

int CVI_HAL_UartReceive(char *rcv_buf, int data_len, int settimeout)
{
	int len = 0;
	int ret;
	struct timeval tv;
	fd_set fs_read;

	FD_ZERO(&fs_read);
	FD_SET(fd_uart, &fs_read);

	tv.tv_sec = 0;
	tv.tv_usec = 200;

	if (settimeout)
		ret = select(fd_uart + 1, &fs_read, NULL, NULL, &tv);
	else
		ret = select(fd_uart + 1, &fs_read, NULL, NULL, NULL);
	if (ret == -1) {
		perror("select fail");
		return -1;
	} else if (ret == 0) {
		printf("select timeout\n");
	} else if (ret > 0) {
		if (FD_ISSET(fd_uart, &fs_read))
			len = read(fd_uart, rcv_buf, data_len);
	}

	return len;
}

int CVI_HAL_UartSend(char *send_buf, int data_len)
{
	int len;

	len = write(fd_uart, send_buf, data_len);

	return len;
}

int CVI_HAL_UartInit(char *node)
{
	fd_uart = CVI_HAL_UartOpen(node);
	if (fd_uart < 0) {
		printf("open uart failed\n");
		return -1;
	}

	return 0;
}

int CVI_HAL_UartExit(void)
{
	if (CVI_HAL_UartClose(fd_uart) < 0) {
		printf("close rs485 failed.\n");
		return -1;
	}

	return 0;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
