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
#include <poll.h>
#include <stdbool.h>

#include "cvi_hal_wiegand.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
static int fd_wgn_rx = -1;
static int fd_wgn_tx = -1;

static int _WiegandOpen(char *node)
{
	int fd;

	fd = open(node, O_RDWR);
	if (fd < 0) {
		perror("wiegand open");
		return fd;
	}

	return fd;
}

static int _WiegandClose(int fd)
{
	int ret;

	ret = close(fd);
	if (ret < 0)
		perror("wiegand close");

	return ret;
}

int CVI_HAL_WiegandSetTxCfg(struct wgn_tx_cfg *tx_cfg)
{
	int ret;

	if (tx_cfg == NULL) {
		printf("tx_cfg is NULL, please check!\n");
		return -1;
	}

	ret = ioctl(fd_wgn_tx, IOCTL_WGN_SET_TX_CFG, tx_cfg);
	if (ret < 0)
		perror("IOCTL_WGN_SET_TX_CFG");

	return ret;
}

int CVI_HAL_WiegandSetRxCfg(struct wgn_rx_cfg *rx_cfg)
{
	int ret;

	if (rx_cfg == NULL) {
		printf("rx_cfg is NULL, please check!\n");
		return -1;
	}

	ret = ioctl(fd_wgn_rx, IOCTL_WGN_SET_RX_CFG, rx_cfg);
	if (ret < 0)
		perror("IOCTL_WGN_SET_RX_CFG");

	return ret;
}

#ifndef WIEGAND_GPIO
int CVI_HAL_WiegandGetTxCfg(struct wgn_tx_cfg *tx_cfg)
{
	if (ioctl(fd_wgn_tx, IOCTL_WGN_GET_TX_CFG, tx_cfg) < 0) {
		perror("IOCTL_WGN_GET_TX_CFG");
		return -1;
	}
	return 0;
}

int CVI_HAL_WiegandGetRxCfg(struct wgn_rx_cfg *rx_cfg)
{
	if (ioctl(fd_wgn_rx, IOCTL_WGN_GET_RX_CFG, rx_cfg) < 0) {
		perror("IOCTL_WGN_GET_RX_CFG");
		return -1;
	}
	return 0;
}
#endif

int CVI_HAL_WiegandSend(struct wng_send_data *send_data)
{
	if (send_data == NULL) {
		printf("Error:WGN NULL pointer send_data\n");
		return -1;
	}

	if (ioctl(fd_wgn_tx, IOCTL_WGN_TX, &(send_data->tx_data)) < 0) {
		perror("IOCTL_WGN_TX");
		return -1;
	}

	return 0;
}

int CVI_HAL_WiegandReceive(struct wng_receive_data *recv_data, bool settimeout)
{
	int ret;
	int timeoutflag = (int)settimeout;

	if (recv_data == NULL) {
		printf("Error:NULL pointer recv_data\n");
		return -1;
	}

	#ifndef WIEGAND_GPIO
		ret = ioctl(fd_wgn_rx, IOCTL_WGN_RX, &timeoutflag);
		if (ret < 0) {
			perror("IOCTL_WGN_RX");
			return -1;
		} else if (ret > 0) {
			printf("IOCTL_WGN_RX timeout\n");
			return 1;
		}

		if (ioctl(fd_wgn_rx, IOCTL_WGN_GET_VAL, &recv_data->rx_data) < 0) {
			perror("IOCTL_WGN_GET_VAL");
			return -1;
		}
	#else
		if (ioctl(fd_wgn_rx, IOCTL_WGN_RX, recv_data) < 0) {
			perror("IOCTL_WGN_RX");
			return -1;
		}

		//read(fd, recv_data, sizeof(*recv_data));
	#endif

	return 0;
}

int CVI_HAL_WiegandInit(struct wgn_tx_cfg *tx_cfg, struct wgn_rx_cfg *rx_cfg)
{
	if (tx_cfg == NULL || rx_cfg == NULL) {
		printf("Error:NULL pointer tx_cfg/rx_cfg\n");
		return -1;
	}

	fd_wgn_rx = _WiegandOpen("/dev/cvi-wiegand0");
	if (fd_wgn_rx < 0) {
		printf("open wiegand0 failed.\n");
		return -1;
	}

	fd_wgn_tx = _WiegandOpen("/dev/cvi-wiegand1");
	if (fd_wgn_tx < 0) {
		printf("open wiegand1 failed.\n");
		return -1;
	}

	if (CVI_HAL_WiegandSetRxCfg(rx_cfg) < 0) {
		printf("set wiegand rx cfg failed.\n");
		return -1;
	}

	if (CVI_HAL_WiegandSetTxCfg(tx_cfg)) {
		printf("set wiegand tx cfg failed.\n");
		return -1;
	}

	return 0;
}

int CVI_HAL_WiegandExit(void)
{
	if (_WiegandClose(fd_wgn_rx) < 0) {
		printf("close wiegand0 failed.\n");
		return -1;
	}

	if (_WiegandClose(fd_wgn_tx) < 0) {
		printf("close wiegand1 failed.\n");
		return -1;
	}

	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */