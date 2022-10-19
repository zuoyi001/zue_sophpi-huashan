/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: uart.h
 * Description:
 */

#ifndef __CVI_HAL_UART_H__
#define __CVI_HAL_UART_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

int CVI_HAL_UartOpen(char *node);
int CVI_HAL_UartClose(int fd);
int CVI_HAL_UartSetParam(int speed, int flow_ctrl, int databits, int stopbits, char parity);
int CVI_HAL_UartReceive(char *rcv_buf, int data_len, int settimeout);
int CVI_HAL_UartSend(char *send_buf, int data_len);
int CVI_HAL_UartInit(char *node);
int CVI_HAL_UartExit(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __CVI_HAL_UART_H__ */