/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_hal_wiegand.h
 * Description:
 */

#ifndef __CVI_HAL_WIEGAND_H__
#define __CVI_HAL_WIEGAND_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#include "autoconf.h"

#define IOCTL_BASE	'W'
#ifdef WIEGAND_GPIO
#define IOCTL_WGN_SET_TX_CFG	_IO(IOCTL_BASE, 1)
#define IOCTL_WGN_SET_RX_CFG	_IO(IOCTL_BASE, 2)
#define IOCTL_WGN_TX			_IO(IOCTL_BASE, 3)
#define IOCTL_WGN_RX			_IO(IOCTL_BASE, 4)
#else
#define IOCTL_WGN_SET_TX_CFG	_IO(IOCTL_BASE, 1)
#define IOCTL_WGN_SET_RX_CFG	_IO(IOCTL_BASE, 2)
#define IOCTL_WGN_GET_TX_CFG	_IO(IOCTL_BASE, 3)
#define IOCTL_WGN_GET_RX_CFG	_IO(IOCTL_BASE, 4)
#define IOCTL_WGN_TX			_IO(IOCTL_BASE, 5)
#define IOCTL_WGN_RX			_IO(IOCTL_BASE, 6)
#define IOCTL_WGN_GET_VAL		_IO(IOCTL_BASE, 7)
#endif

struct wgn_tx_cfg {
	unsigned int tx_lowtime;
	unsigned int tx_hightime;
	unsigned int tx_bitcount;
	unsigned int tx_msb1st;
	#ifndef WIEGAND_GPIO
	unsigned int tx_opendrain;
	#endif
};

struct wgn_rx_cfg {
	#ifndef WIEGAND_GPIO
	unsigned int rx_debounce;
	#endif
	unsigned int rx_idle_timeout;
	unsigned int rx_bitcount;
	unsigned int rx_msb1st;
};

struct wng_send_data {
	unsigned int FacilityCode;
	unsigned int UserCode;
	int startParity;
	int endParity;
	long long tx_data;
};

struct wng_receive_data {
	long long rx_data;
	unsigned int FacilityCode;
	unsigned int UserCode;
	int startParity;
	int endParity;
	#ifdef WIEGAND_GPIO
	char start_parity[5];
	char end_parity[5];
	#endif
};

//int CVI_HAL_WiegandOpen(char *node);
//int CVI_HAL_WiegandClose(int fd);
int CVI_HAL_WiegandSetTxCfg(struct wgn_tx_cfg *tx_cfg);
int CVI_HAL_WiegandSetRxCfg(struct wgn_rx_cfg *rx_cfg);
#ifndef WIEGAND_GPIO
int CVI_HAL_WiegandGetTxCfg(struct wgn_tx_cfg *tx_cfg);
int CVI_HAL_WiegandGetRxCfg(struct wgn_rx_cfg *rx_cfg);
#endif
int CVI_HAL_WiegandSend(struct wng_send_data *send_data);
int CVI_HAL_WiegandReceive(struct wng_receive_data *recv_data, bool settimeout);
int CVI_HAL_WiegandInit(struct wgn_tx_cfg *tx_cfg, struct wgn_rx_cfg *rx_cfg);
int CVI_HAL_WiegandExit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __CVI_HAL_WIEGAND_H__ */