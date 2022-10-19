/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: factory.h
 * Description:
 */

#ifndef __CVI_FACTORY_H__
#define __CVI_FACTORY_H__
#include <stdbool.h>
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef enum factory_mode {
	CVI_TEST_LED = 0,
	CVI_TEST_RELAY,
	CVI_TEST_RS485,
	CVI_TEST_WIEGAND,
	CVI_TEST_INVALID
} FACTORY_MODE_E;


int CVI_Wiegand_To_Rs485(bool flag, int data);
int CVI_Rs485_To_Wiegand(int flag, int data);
int CVI_Test_Led(int duty_cycle);
int CVI_Test_Relay(bool state);
int CVI_Test_Wiegand(bool flag, int tx_data, int *rx_data);
int CVI_Test_Rs485(bool flag, int tx_data, int *rx_data);
int CVI_Test_Production_Init(void);
int CVI_Test_Production_Exit(void);

int CVI_Test_Aging_Enable(bool state);
int CVI_Test_Aging_Init(void);
int CVI_Test_Aging_Exit(void);

int CVI_Test_Reboot_Config(int interval_time, int reboot_times);
int CVI_Test_RGB_IR_POP(void);

int CVI_Messages_Log_Enable(bool state);
int CVI_Init_Hotplug_Sock(void);
int CVI_Copy_Log_To_Udisk(void);
int CVI_Compress_Log_File(void);
int CVI_Test_VPSS_Resize(void);
void CVI_Test_audio_stop();
int CVI_Test_speaker_loopaudio();
int CVI_Test_audio_local_loopback();
void CVI_Test_audio_playfile(const char * filename,int bLoop);
int CVI_Test_record_micdata(bool bswitch);
int CVI_Test_play_micdata();


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __CVI_FACTORY_H__ */
