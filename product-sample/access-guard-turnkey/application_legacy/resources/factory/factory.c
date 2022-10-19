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
#include <linux/rtc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "app_utils.h"
#include "factory.h"
#include "app_init.h"
#include "../audio/cvi_audio_main.h"

#define msleep(x) usleep(x*1000)

#define __SIGRTMIN 		32
#define SIG_WGN			(__SIGRTMIN + 10)
#define SIG_RS485		(__SIGRTMIN + 11)
#define SIG_REBOOT		(__SIGRTMIN + 12)

/*********************** Production test ***********************/

int test_mode_a = CVI_TEST_INVALID; //production test mode
int test_mode_b = CVI_TEST_INVALID;

/* wiegand data */
struct wng_send_data wgn_send_data;
struct wng_receive_data wgn_recv_data;

/* rs485 data */
char uart_send_buf[4];
char uart_recv_buf[4];

/* rs485 data */
bool wgn_timeout_flag = 0;
bool rs485_timeout_flag = 0;

/* duty_cycle:0~100 */
int CVI_Test_Led(int duty_cycle)
{
	int per;

	CVI_HAL_PwmDisable(0, 3);
	CVI_HAL_PwmUnExport(0, 3);

	if (duty_cycle <= 0) {
		return 0;
	} else if (duty_cycle >= 100) {
		per = 1000000;
	} else {
		per = duty_cycle * 10000;
	}

	CVI_HAL_PwmExport(0, 3);
	CVI_HAL_PwmSetParm(0, 3, 1000000, per);
	CVI_HAL_PwmEnable(0, 3);

	return 0;
}

int CVI_Test_Relay(bool state)
{
	if (state)
		CVI_HAL_GpioSetValue(RELAY_GPIO, CVI_HAL_GPIO_VALUE_H);
	else
		CVI_HAL_GpioSetValue(RELAY_GPIO, CVI_HAL_GPIO_VALUE_L);

	return 0;
}

void CVI_Signal_Func(int signo)
{
	switch (signo) {
	case SIG_WGN:
		break;

	case SIG_RS485:
		break;

	case SIG_REBOOT:
		if (CVI_HAL_WatchdogSetTimeout(1) < 0)
			CVI_SYSLOG(CVI_DBG_ERR, "watchdog set timeout failed");
		break;
	}
}

int CVI_Wiegand_To_Rs485(bool flag, int data)
{
	int i;
	int bitcount = 0;
	int data_high, data_low, data_tmp;

	data_low = data & 0x0FFF;
	data_high = data >> 12;

	//parity should be set
	for (i = 0; i < 12; i++) {
		data_tmp = data_high & 0x01;
		if (data_tmp == 1)
			bitcount++;
		data_high >>= 1;
	}
	wgn_send_data.startParity = (bitcount % 2 == 0) ? 0 : 1;

	bitcount = 0;
	for (i = 0; i < 12; i++) {
		data_tmp = data_low & 0x01;
		if (data_tmp == 1)
			bitcount++;
		data_low >>= 1;
	}
	wgn_send_data.endParity = (bitcount % 2 == 0) ? 1 : 0;

	wgn_send_data.tx_data = ((data << 1) | wgn_send_data.endParity) | (wgn_send_data.startParity << 25);

	if (flag) {
		test_mode_b = CVI_TEST_WIEGAND;
	} else {
		if (CVI_HAL_WiegandSend(&wgn_send_data) < 0) {
			CVI_SYSLOG(CVI_DBG_ERR, "WGN send data failed");
			return -1;
		}
	}

	return 0;
}

/*
 * this function should be used with wiegand and rs485 converter
 * flag: 0:self send other receive
 *       1:self send self receive
 * return: 0:recv successfully
 *         -1:timeout
 */
int CVI_Test_Wiegand(bool flag, int tx_data, int *rx_data)
{
	int *p_s4;
	wgn_timeout_flag = 0;
	//CVI_SYSLOG(CVI_DBG_INFO, "????:%d", tx_data);
	CVI_Wiegand_To_Rs485(flag, tx_data);
	if (flag) {
		//signal(SIG_WGN, CVI_Signal_Func);
		//signal(SIG_WGN_ITMEOUT, CVI_Signal_Func);
		//pause();
		sleep(1);

		p_s4 = (int *)uart_recv_buf;
		*rx_data = *p_s4;
		//CVI_SYSLOG(CVI_DBG_INFO, "????:%d", *rx_data);
	}
	if (wgn_timeout_flag)
		return -1;

	return 0;
}

int CVI_Rs485_To_Wiegand(int flag, int data)
{
	int swap;
	int *p_s4;
	memset(uart_send_buf, 0, 4);
	p_s4 = (int *)uart_recv_buf;
	*p_s4 = data;
	swap = uart_send_buf[0];
	uart_send_buf[0] = uart_send_buf[2];
	uart_send_buf[2] = swap;

	if (flag) {
		test_mode_b = CVI_TEST_RS485;
	} else {
		if (CVI_HAL_UartSend(uart_send_buf, 3) < 0) {
			CVI_SYSLOG(CVI_DBG_ERR, "RS485 send data failed");
			return -1;
		}
	}

	return 0;
}

/*
 * this function should be used with wiegand and rs485 converter
 * flag: 0:self send other receive
 *       1:self send self receive
 */
int CVI_Test_Rs485(bool flag, int tx_data, int *rx_data)
{
	//int i;
	rs485_timeout_flag = 0;
	//CVI_SYSLOG(CVI_DBG_INFO, "????:%d", tx_data);
	CVI_Rs485_To_Wiegand(flag, tx_data);

	if (flag) {
		//signal(SIG_RS485, CVI_Signal_Func);
		//signal(SIG_RS485_ITMEOUT, CVI_Signal_Func);
		//pause();
		sleep(1);

		*rx_data = (wgn_recv_data.rx_data >> 1) & 0xFFFFFF;
		//CVI_SYSLOG(CVI_DBG_INFO, "????:%d", *rx_data);
	}

	if (rs485_timeout_flag)
		return -1;

	return 0;
}

static void *factory_test_thread(void *p)
{
	//int data;
	//int count = 0;
	char szThreadName[20] = "factory_test_thread";
	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);
	//CVI_HAL_GpioSetValue(RS485_GPIO, CVI_HAL_GPIO_VALUE_H);
	//CVI_HAL_GpioSetValue(RS485_GPIO, CVI_HAL_GPIO_VALUE_L);

	while (1) {
		switch (test_mode_a) {
		case CVI_TEST_WIEGAND:
			if (CVI_HAL_WiegandSend(&wgn_send_data) < 0)
				CVI_SYSLOG(CVI_DBG_ERR, "WGN send data failed");

			test_mode_a = CVI_TEST_INVALID;
			break;

		case CVI_TEST_RS485:
			CVI_HAL_GpioSetValue(RS485_GPIO, CVI_HAL_GPIO_VALUE_H);

			if (CVI_HAL_UartSend(uart_send_buf, 3) < 0)
				CVI_SYSLOG(CVI_DBG_ERR, "RS485 send data failed");

			test_mode_a = CVI_TEST_INVALID;
			break;

		case CVI_TEST_INVALID:
		default:
			msleep(500);
			break;
		}
	}

	return NULL;
}

static void *uart_wgn_rx_thread(void *p)
{
	int ret;
	int swap;
	//int count = 0;
	char szThreadName[20] = "uart_wgn_rx_thread";

	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);

	while (1) {
		switch (test_mode_b) {
		case CVI_TEST_WIEGAND:
			CVI_HAL_GpioSetValue(RS485_GPIO, CVI_HAL_GPIO_VALUE_L);

			memset(uart_recv_buf, 0, 4);

			test_mode_a = CVI_TEST_WIEGAND;

			ret = CVI_HAL_UartReceive(uart_recv_buf, 3, 1);
			if (ret > 0) {
				swap = uart_recv_buf[0];
				uart_recv_buf[0] = uart_recv_buf[2];
				uart_recv_buf[2] = swap;

				//kill(getpid(), SIG_WGN);
				test_mode_b = CVI_TEST_INVALID;
			} else if (ret == 0) {
				CVI_SYSLOG(CVI_DBG_ERR, "RS485 receive timeout");
				//kill(getpid(), SIG_WGN_ITMEOUT);
				test_mode_b = CVI_TEST_INVALID;
			} else {
				CVI_SYSLOG(CVI_DBG_ERR, "RS485 receive data failed");
			}

			break;

		case CVI_TEST_RS485:
			memset(&wgn_recv_data, 0, sizeof(wgn_recv_data));

			test_mode_a = CVI_TEST_RS485;

			ret = CVI_HAL_WiegandReceive(&wgn_recv_data, 1);
			//printf("[getpid():%d ret:%d %s,%d]\n",getpid(),ret,__FUNCTION__,__LINE__);
			if (ret < 0) {
				CVI_SYSLOG(CVI_DBG_ERR, "WGN receive data failed");
			} else if (ret > 0) {
				CVI_SYSLOG(CVI_DBG_ERR, "WGN receive timeout");
				//kill(getpid(), SIG_RS485_ITMEOUT);
				test_mode_b = CVI_TEST_INVALID;
			} else {
				//kill(getpid(), SIG_RS485);
				test_mode_b = CVI_TEST_INVALID;
			}
			break;

		case CVI_TEST_INVALID:
		default:
			msleep(500);
			break;
		}
	}

	return NULL;
}

static pthread_t fac_id, uw_id;

int CVI_Test_Production_Init(void)
{
	int res;

	/* Production test thread */
	res = pthread_create(&fac_id, 0, factory_test_thread, 0);
	if (res)
		CVI_SYSLOG(CVI_DBG_ERR, "%s", strerror(res));
	pthread_detach(fac_id);

	res = pthread_create(&uw_id, 0, uart_wgn_rx_thread, 0);
	if (res)
		CVI_SYSLOG(CVI_DBG_ERR, "%s", strerror(res));
	pthread_detach(uw_id);

	return 0;
}

int CVI_Test_Production_Exit(void)
{
	pthread_cancel(fac_id);
	pthread_cancel(uw_id);

	return 0;
}

/************************* Aging test *************************/
int aging_test_enable = 0;

int CVI_Test_Aging_Enable(bool state)
{
	aging_test_enable = state ? 1 : 0;

	return 0;
}

static void *led_switch_thread(void *p)
{
	char szThreadName[20] = "led_switch_thread";

	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);

	while (1) {
		if (aging_test_enable) {
			CVI_Test_Led(100);
			sleep(5);
			CVI_Test_Led(0);
			sleep(5);
		} else {
			sleep(1);
		}
	}

	return NULL;
}

static void *relay_switch_thread(void *p)
{
	char szThreadName[20] = "relay_switch_thread";

	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);

	while (1) {
		if (aging_test_enable) {
			CVI_HAL_GpioSetValue(RELAY_GPIO, CVI_HAL_GPIO_VALUE_H);
			sleep(60);
			CVI_HAL_GpioSetValue(RELAY_GPIO, CVI_HAL_GPIO_VALUE_L);
			sleep(60);
		} else {
			sleep(1);
		}
	}

	return NULL;
}

static pthread_t led_id, relay_id;

int CVI_Test_Aging_Init(void)
{
	int res;
	
	/* Aging test thread */
	res = pthread_create(&led_id, 0, led_switch_thread, 0);
	if (res)
		CVI_SYSLOG(CVI_DBG_ERR, "%s", strerror(res));
	pthread_detach(led_id);

	res = pthread_create(&relay_id, 0, relay_switch_thread, 0);
	if (res)
		CVI_SYSLOG(CVI_DBG_ERR, "%s", strerror(res));
	pthread_detach(relay_id);

	return 0;
}

int CVI_Test_Aging_Exit(void)
{
	pthread_cancel(led_id);
	pthread_cancel(relay_id);

	return 0;
}

/*
 * interval_time:The time interval between two reboots
 * reboot_times:Number of reboots required
 */
int CVI_Test_Reboot_Config(int interval_time, int reboot_times)
{
	int fd;
	char buf[10];
	int counts;
	int reboot_times_old;

	//Record the counts expect to reboot
	if (access("/mnt/data/reboot_times", F_OK) == -1) {
		fd = open("/mnt/data/reboot_times", O_RDWR|O_CREAT, 0664);

		sprintf(buf, "%d", reboot_times);
		write(fd, buf, strlen(buf));

		close(fd);
	} else {
		fd = open("/mnt/data/reboot_times", O_RDWR);

		read(fd, buf, sizeof(buf));
		sscanf(buf, "%d", &reboot_times_old);

		if (reboot_times != reboot_times_old) {
			sprintf(buf, "%d", reboot_times);
			lseek(fd, 0, SEEK_SET);
			system("echo 0 > /mnt/data/reboot_times");
			write(fd, buf, strlen(buf));
		}

		close(fd);
	}

	//record and update counts before reboot
	if (access("/mnt/data/reboot_counts", F_OK) == -1) {
		system("echo 0 > /mnt/data/reboot_counts");
	} else {
		fd = open("/mnt/data/reboot_counts", O_RDWR);

		read(fd, buf, sizeof(buf));
		sscanf(buf, "%d", &counts);
		counts++;
		sprintf(buf, "%d", counts);
		lseek(fd, 0, SEEK_SET);
		write(fd, buf, strlen(buf));

		close(fd);
	}

	system("sync && sync");

	if (counts <= reboot_times) {
		signal(SIG_REBOOT, CVI_Signal_Func);
		sleep(interval_time - 1);
		kill(getpid(), SIG_REBOOT);
	} else {
		system("echo 0 > /mnt/data/reboot_counts");
	}

	return 0;
}

int CVI_Test_RGB_IR_POP(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame,stIrFrame,stDisplayFrame;

	VPSS_CHN_ATTR_S stChnAttr;
	VPSS_CHN_ATTR_S stChnAttr_ir,stChnAttr_rgb;

	int tmp_cnt=0;

	s32Ret = CVI_VPSS_GetChnAttr(1, 0, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnAttr is fail\n");
		return s32Ret;
	}
	memcpy(&stChnAttr_ir,&stChnAttr,sizeof(VPSS_CHN_ATTR_S));
	s32Ret = CVI_VPSS_DisableChn(1, 0);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_DisableChn is fail\n");
		return s32Ret;
	}
	stChnAttr.u32Width                    = 360;
	stChnAttr.u32Height                   = 640;
	stChnAttr.enVideoFormat               = VIDEO_FORMAT_LINEAR;
	stChnAttr.enPixelFormat               = PIXEL_FORMAT_YUV_PLANAR_420;
	stChnAttr.stAspectRatio.enMode        = ASPECT_RATIO_MANUAL;
	stChnAttr.stAspectRatio.stVideoRect.s32X = 0;
	stChnAttr.stAspectRatio.stVideoRect.s32Y = 0;
	stChnAttr.stAspectRatio.stVideoRect.u32Width = 360;
	stChnAttr.stAspectRatio.stVideoRect.u32Height = 640;
	stChnAttr.stAspectRatio.bEnableBgColor = CVI_TRUE;
	stChnAttr.bFlip = CVI_FALSE;
	s32Ret = CVI_VPSS_SetChnAttr(1, 0, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
	}
	s32Ret = CVI_VPSS_EnableChn(1, 0);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_EnableChn is fail\n");
		return s32Ret;
	}

	s32Ret = CVI_VPSS_GetChnAttr(0, 2, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnAttr is fail\n");
		return s32Ret;
	}
	memcpy(&stChnAttr_rgb,&stChnAttr,sizeof(VPSS_CHN_ATTR_S));
	s32Ret = CVI_VPSS_DisableChn(0, 2);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_DisableChn is fail\n");
		return s32Ret;
	}
	stChnAttr.u32Width                    = DISP_WIDTH;
	stChnAttr.u32Height                   = DISP_HEIGHT;
	stChnAttr.enVideoFormat               = VIDEO_FORMAT_LINEAR;
	stChnAttr.enPixelFormat               = PIXEL_FORMAT_YUV_PLANAR_420;
	stChnAttr.stAspectRatio.stVideoRect.s32X = 0;
	stChnAttr.stAspectRatio.stVideoRect.s32Y = 300;
	stChnAttr.stAspectRatio.stVideoRect.u32Width = 360;
	stChnAttr.stAspectRatio.stVideoRect.u32Height = 640;
	stChnAttr.stAspectRatio.bEnableBgColor = CVI_TRUE;
	s32Ret = CVI_VPSS_SetChnAttr(0, 2, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
	}
	s32Ret = CVI_VPSS_EnableChn(0, 2);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_EnableChn is fail\n");
		return s32Ret;
	}

	while (true)
	{
		tmp_cnt++;
		s32Ret = CVI_VPSS_GetChnFrame(0, 2, &stVideoFrame, 1000);//stvideoframe:720*1280
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_GetChnFrame grp0 chn2 failed with %#x\n", s32Ret);
			break;
		}

		s32Ret = CVI_VPSS_SendChnFrame(3, 0, &stVideoFrame, 1000);//grp3 for factory mode
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_SendChnFrame grp3 chn0 failed with %#x\n", s32Ret);
			break;
		}
		
		s32Ret = CVI_VPSS_GetChnFrame(1, 0, &stIrFrame, 1000);/*&stIrFrame*/
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_GetChnFrame grp1 chn0 failed with %#x\n", s32Ret);
			break;
		}

		s32Ret = CVI_VPSS_SendFrame(3, &stIrFrame, 1000);/*&stIrFrame*/
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_SendFrame grp3 failed with %#x\n", s32Ret);
			break;
		}

		s32Ret = CVI_VPSS_GetChnFrame(3, 0, &stDisplayFrame, 1000);/*&stDisplayFrame*/
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_GetChnFrame grp3 chn0 failed with %#x\n", s32Ret);
			break;
		}

		CVI_VO_ShowChn(0,0);
		s32Ret = CVI_VO_SendFrame(0, 0, &stDisplayFrame, -1);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SendFrame failed with %#x\n", s32Ret);
			break;
		}
		
		s32Ret = CVI_VPSS_ReleaseChnFrame(0, 2, &stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_ReleaseChnFrame grp0 chn2 NG\n");
			break;
		}
		s32Ret = CVI_VPSS_ReleaseChnFrame(1, 0, &stIrFrame);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_ReleaseChnFrame grp0 chn2 NG\n");
			break;
		}
		s32Ret = CVI_VPSS_ReleaseChnFrame(3, 0, &stDisplayFrame);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_ReleaseChnFrame grp3 chn0 NG\n");
			break;
		}

		if(tmp_cnt >=100)
			break;

	}
	//reset ChnAttr when exit.
	s32Ret = CVI_VPSS_DisableChn(1, 0);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_DisableChn is fail\n");
	}
	s32Ret = CVI_VPSS_SetChnAttr(1, 0, &stChnAttr_ir);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
	}
	s32Ret = CVI_VPSS_EnableChn(1, 0);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_EnableChn is fail\n");
	}

	s32Ret = CVI_VPSS_DisableChn(0, 2);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_DisableChn is fail\n");
	}
	s32Ret = CVI_VPSS_SetChnAttr(0, 2, &stChnAttr_rgb);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
	}
	s32Ret = CVI_VPSS_EnableChn(0, 2);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_EnableChn is fail\n");
	}
	return s32Ret;
}

int CVI_Messages_Log_Enable(bool state)
{
	if (state)
		system("echo on > /mnt/data/log_enable");
	else
		system("echo off > /mnt/data/log_enable");

	return 0;
}

int CVI_Init_Hotplug_Sock(void)
{
	struct sockaddr_nl snl;
	const int buffersize = 16 * 1024 * 1024;
	int retval;
	int sock_fd;

	memset(&snl, 0, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 1;

	sock_fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (sock_fd == -1) {
		printf("error getting socket: %s", strerror(errno));
		return -1;
	}

	/* set receive buffersize */
	setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
	retval = bind(sock_fd, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));

	if (retval < 0) {
		printf("bind failed: %s", strerror(errno));
		close(sock_fd);

		return -1;
	}

	return sock_fd;
}

static void *get_log_thread(void *p)
{
	char *sd_dev;
	char cmd[110];
	char buf[2048 * 2] = {0};
	char szThreadName[20] = "get_log_thread";
	int sock_fd = CVI_Init_Hotplug_Sock();

	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);

	while (1) {
		memset(buf, 0, sizeof(buf));
		recv(sock_fd, &buf, sizeof(buf), 0);

		if (strstr(buf, "add")) {
			sd_dev = strstr(buf, "sd");
			if (sd_dev != NULL) {
				sd_dev += 4;
				if (strstr(sd_dev, "sd") != NULL) {
					sleep(1);
					sprintf(cmd, "mount /dev/%s /mnt/data/tmp && cp /mnt/data/log/messages* \
						/mnt/data/tmp/ && umount /mnt/data/tmp", sd_dev);
					system(cmd);
					printf("get log successfully!\n");
				}
			}
		}
	}

	return NULL;
}

int CVI_Copy_Log_To_Udisk(void)
{
	int res;
	pthread_t id;

	if (access("/mnt/data/tmp", F_OK) == -1)
		system("mkdir /mnt/data/tmp");

	res = pthread_create(&id, 0, get_log_thread, 0);
	if (res)
		CVI_SYSLOG(CVI_DBG_ERR, "%s", strerror(res));
	pthread_detach(id);

	return 0;
}

void compress_log_file(int signo)
{
	int fd;
	char cmd[120];
	struct rtc_time rtc_tm;

	fd = open("/dev/rtc0", O_RDONLY);
	if (fd < 0)
		perror("open /dev/rtc0");

	if (ioctl(fd, RTC_RD_TIME, &rtc_tm) < 0)
		perror("RTC_RD_TIME");

	sprintf(cmd, "tar -zcvf /mnt/data/log/messages_%d%02d%02d%02d%02d%02d.tar.gz \
		/mnt/data/log/messages > /dev/null 2>&1", rtc_tm.tm_year + 1900, rtc_tm.tm_mon + 1, \
		rtc_tm.tm_mday, rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

	system(cmd);
	system("find /mnt/data/log/ -mtime +2 -name \"messages_*.tar.gz\" | xargs rm -rf");

	close(fd);
}

int CVI_Compress_Log_File(void)
{
	struct itimerval ti;

	signal(SIGALRM, compress_log_file);

	ti.it_interval.tv_sec = 86400;//one day
	ti.it_interval.tv_usec = 0;
	ti.it_value.tv_sec = 86400;
	ti.it_value.tv_usec = 0;

	setitimer(ITIMER_REAL, &ti, 0);

	return 0;
}
//mic²âÊÔ
int CVI_Test_audio_local_loopback()
{
	//return audio_local_loopback();
	return 0;
}
//²¥·ÅÒôÀÖ²âÊÔÀ®°ÈµÄ½Ó¿Ú
int CVI_Test_speaker_loopaudio()
{
	//return audio_play_loop_audio();
	return 0;
}

void CVI_Test_audio_stop()
{
	//audio_stop_working();
}

void CVI_Test_audio_playfile(const char * filename,int bLoop)
{
	//audio_play_filewav(filename,bLoop);
}

int CVI_Test_record_micdata(bool bswitch)
{
	//return audio_record_micdata_to_wav(bswitch);
	return 0;
}

int CVI_Test_play_micdata()
{
	//return audio_play_wav_micdata();
	return 0;
}

#if 0 //Vpss chn out of range in dual mode.
static int test_vpss_resize(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, SIZE_S *stSize,
									PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	VIDEO_FRAME_INFO_S stVideoFrame;
	VPSS_GRP_ATTR_S stVpssGrpAttr;
	VPSS_CHN_ATTR_S stChnAttr, astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {1, 0, 0, 0};

	CVI_U32 u32MinWidth = DEFAULT_ALIGN;
	CVI_U32 u32MinHeight = DEFAULT_ALIGN;
	CVI_U32 u32TimeCnt = 50;

	stVpssGrpAttr.u32MaxW = stSize->u32Width;
	stVpssGrpAttr.u32MaxH = stSize->u32Height;
	stVpssGrpAttr.enPixelFormat = enPixelFormat;
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssGrpAttr.u8VpssDev = 0;

	abChnEnable[VpssChn] = CVI_TRUE;
	astVpssChnAttr[VpssChn].u32Width								 = 720;
	astVpssChnAttr[VpssChn].u32Height								 = 1280;
	astVpssChnAttr[VpssChn].enVideoFormat							 = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat							 = PIXEL_FORMAT_YUV_PLANAR_420;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate				 = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate				 = 30;
	astVpssChnAttr[VpssChn].u32Depth								 = 1;
	astVpssChnAttr[VpssChn].bMirror 								 = SUPPORT_MIRROR;
	astVpssChnAttr[VpssChn].bFlip									 = SUPPORT_FLIP;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode					 = ASPECT_RATIO_MANUAL;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor			 = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor				 = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32X			 = 0;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32Y			 = 0;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Width		 = 720;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Height		 = 1280;
	astVpssChnAttr[VpssChn].stNormalize.bEnable						 = CVI_FALSE;

	memcpy(&stChnAttr, &astVpssChnAttr[VpssChn], sizeof(VPSS_CHN_ATTR_S));

	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}
	
	do {
		s32Ret = SAMPLE_COMM_VPSS_SendFrame(VpssGrp, stSize, enPixelFormat, filename);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_VPSS_SendFrame failed with %#x\n", s32Ret);
			break;
		}

		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_GetChnFrame for grp%d chn%d. s32Ret: 0x%x !\n", VpssGrp, VpssChn, s32Ret);
			break;
		}

		CVI_VO_ShowChn(0,0);
		s32Ret = CVI_VO_SendFrame(0, 0, &stVideoFrame, -1);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SendFrame failed with %#x\n", s32Ret);
			break;
		}

		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssGrp, &stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_ReleaseChnFrame for grp%d chn%d. s32Ret: 0x%x !\n", VpssGrp, VpssChn, s32Ret);
			break;
		}

		if (0 == u32TimeCnt--) {
			u32TimeCnt = 50;

			stChnAttr.stAspectRatio.stVideoRect.u32Width =
				 (ALIGN(stChnAttr.stAspectRatio.stVideoRect.u32Width >> 1, DEFAULT_ALIGN));
			stChnAttr.stAspectRatio.stVideoRect.u32Height =
				 (ALIGN(stChnAttr.stAspectRatio.stVideoRect.u32Height >> 1, DEFAULT_ALIGN));

			if ((u32MinWidth >= stChnAttr.stAspectRatio.stVideoRect.u32Width) ||
				(u32MinHeight >= stChnAttr.stAspectRatio.stVideoRect.u32Height)) {
				break;
			} else {
				s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stChnAttr);
				if (s32Ret != CVI_SUCCESS) {
					SAMPLE_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
					break;
				}
				SAMPLE_PRT("CVI_Test_VPSS_Resize u32Width(%d) u32Height(%d)\n", stChnAttr.u32Width, stChnAttr.u32Height);
			}
		}
	}while(1);

	s32Ret = SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("stop vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}

int CVI_Test_VPSS_Resize(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	VPSS_GRP VpssGrp = 3; // for factory test
	VPSS_CHN VpssChn = 0; // for factory test

	VPSS_GRP_ATTR_S stGrpAttr;
	VPSS_CHN_ATTR_S stChnAttr[VPSS_MAX_PHY_CHN_NUM];
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {1, 0, 0, 0};
	
	SIZE_S stSize;
	CVI_CHAR filename[64];
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_RGB_888;

	//save vpss attr.
	s32Ret = CVI_VPSS_GetGrpAttr(VpssGrp, &stGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("get vpss group attr failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}
	for (int i = 0; i < VPSS_MAX_PHY_CHN_NUM; i++) {
		if (0 == abChnEnable[i]) {
			continue;
		}
		s32Ret = CVI_VPSS_GetChnAttr(VpssGrp, VpssChn, &stChnAttr[i]);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("get vpss chn %d attr failed. s32Ret: 0x%x !\n", i, s32Ret);
			return s32Ret;
		}
	}

	s32Ret = SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("stop vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	stSize.u32Width = 1080;
	stSize.u32Height = 1920;
	enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	sprintf(filename, "sample_0.yuv");
	s32Ret = test_vpss_resize(VpssGrp, VpssChn, &stSize, enPixelFormat, filename);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vpss resize %s fail.\n", filename);
	} else {
		SAMPLE_PRT("vpss resize %s pass.\n", filename);
	}

	stSize.u32Width = 1080;
	stSize.u32Height = 1920;
	enPixelFormat = PIXEL_FORMAT_RGB_888;
	sprintf(filename, "sample_0.rgb");
	s32Ret = test_vpss_resize(VpssGrp, VpssChn, &stSize, enPixelFormat, filename);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vpss resize %s fail.\n", filename);
	} else {
		SAMPLE_PRT("vpss resize %s pass.\n", filename);
	}

	//reset vpss attr when exit.
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stGrpAttr, stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stGrpAttr, stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}
	return s32Ret;
}
#endif