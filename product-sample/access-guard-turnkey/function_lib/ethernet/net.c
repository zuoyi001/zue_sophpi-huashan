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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <stdbool.h>

#include "net.h"

#define ETH0_PORT	"eth0"
#define WLAN0_PORT	"wlan0"
#define WLAN1_PORT	"wlan1"

int ethState = DISCONNECTED;
int wlanState = DISCONNECTED;

int getEthState()
{
	return ethState;
}
int getWlanState()
{
	return wlanState;
}

int CVI_NET_Wlan_Init(char *port)
{
	if (port == NULL) {
		printf("NULL pointer port\n");
		return -1;
	}

	system("echo ctrl_interface=/var/run/wpa_supplicant >/tmp/wpa_supplicant.conf");

	if (strcmp(WLAN0_PORT, port) == 0) {
		printf("wifi test !!!!!!!!!!!!!!\n");
		system("ifconfig wlan0 up");
		system("/usr/bin/wpa_supplicant -iwlan0 -Dnl80211 -c/tmp/wpa_supplicant.conf &");
	}

	return 0;
}

/* remember to free the return memory */
WLAN_AP_INFO_S *CVI_NET_Get_Ap_Info(void)
{
	char buf[256] = {'\0'};
	char szTest[512] = {0};
	FILE *fp = NULL;
	FILE *fc = NULL;
	WLAN_AP_INFO_S *ap_info = NULL;

	system("wpa_cli -iwlan0 scan");
	sleep(3);
	system("wpa_cli -iwlan0 scan_results > /tmp/scan_results");

	fp = fopen("/tmp/scan_results", "r");

	fgets(szTest, sizeof(szTest), fp);
	memset(szTest, 0, sizeof(szTest));

	ap_info = (WLAN_AP_INFO_S *)malloc(sizeof(int) + sizeof(AP_INTO_S));

	ap_info->count = 0;
	while(fgets(szTest, sizeof(szTest), fp) != NULL) {
		fc = fopen("/tmp/apinfo", "w");
		fprintf(fc, "%s", szTest);
		fclose(fc);

		system("awk '{print $5}' /tmp/apinfo > /tmp/ssid");

		fc = fopen("/tmp/ssid", "r");
		memset(buf, '\0', sizeof(buf));
		fscanf(fc, "%s", buf);
		fclose(fc);

		system("rm -f /tmp/apinfo /tmp/ssid");

		if (strlen(buf) < 64) {
			sscanf(szTest, "%s\t%d\t%d\t%s\t%s", ap_info->ap_arr[ap_info->count].bssid, \
				&ap_info->ap_arr[ap_info->count].freq, &ap_info->ap_arr[ap_info->count].signal_level, \
				ap_info->ap_arr[ap_info->count].flag, ap_info->ap_arr[ap_info->count].ssid);

			ap_info = (WLAN_AP_INFO_S *)realloc(ap_info, sizeof(int) + sizeof(AP_INTO_S) * (ap_info->count + 2));

			ap_info->count++;
			memset(szTest, 0, sizeof(szTest));
		}
	}

	fclose(fp);

	return ap_info;
}


WLAN_AP_INFO_S *CVI_NET_Flash_Ap_Info(void)
{
	char buf[256] = {'\0'};
	char szTest[512] = {0};
	FILE *fp = NULL;
	FILE *fc = NULL;
	WLAN_AP_INFO_S *ap_info = NULL;

	system("wpa_cli -iwlan0 scan_results > /tmp/scan_results");

	fp = fopen("/tmp/scan_results", "r");

	fgets(szTest, sizeof(szTest), fp);
	memset(szTest, 0, sizeof(szTest));

	ap_info = (WLAN_AP_INFO_S *)malloc(sizeof(int) + sizeof(AP_INTO_S));

	ap_info->count = 0;
	while(fgets(szTest, sizeof(szTest), fp) != NULL) {
		fc = fopen("/tmp/apinfo", "w");
		fprintf(fc, "%s", szTest);
		fclose(fc);

		system("awk '{print $5}' /tmp/apinfo > /tmp/ssid");

		fc = fopen("/tmp/ssid", "r");
		memset(buf, '\0', sizeof(buf));
		fscanf(fc, "%s", buf);
		fclose(fc);

		system("rm -f /tmp/apinfo /tmp/ssid");

		if (strlen(buf) < 64) {
			sscanf(szTest, "%s\t%d\t%d\t%s\t%s", ap_info->ap_arr[ap_info->count].bssid, \
				&ap_info->ap_arr[ap_info->count].freq, &ap_info->ap_arr[ap_info->count].signal_level, \
				ap_info->ap_arr[ap_info->count].flag, ap_info->ap_arr[ap_info->count].ssid);

			ap_info = (WLAN_AP_INFO_S *)realloc(ap_info, sizeof(int) + sizeof(AP_INTO_S) * (ap_info->count + 2));

			ap_info->count++;
			memset(szTest, 0, sizeof(szTest));
		}
	}

	fclose(fp);

	return ap_info;
}

int CVI_NET_Set_Wlan_Config(char *port, int mode, struct wlan_cfg *pcfg)
{
	FILE *fp = NULL;

	if (port == NULL || pcfg == NULL) {
		printf("NULL pointer, check please\n");
		return -1;
	}

	printf("Setting net port:%s\n", port);

	switch (mode) {
	case STATION:
		system("killall wpa_supplicant");
		
		usleep(1000*1000);
		printf("usleep(1000*1000)\n");

		fp = fopen("/tmp/wpa_supplicant.conf", "w");

		fprintf(fp, "ctrl_interface=/var/run/wpa_supplicant\n");
		fprintf(fp, "network={\n");
		fprintf(fp, "\tssid=\"%s\"\n", pcfg->ssid);
		fprintf(fp, "\tpsk=\"%s\"\n", pcfg->passwd);
		fprintf(fp, "}\n");

		fclose(fp);

		system("ifconfig wlan0 up");
		system("/usr/bin/wpa_supplicant -iwlan0 -Dnl80211 -c/tmp/wpa_supplicant.conf &");
		system("/sbin/udhcpc -b -i wlan0 -R &");

		break;

	case ACCESSPOINT:
#if 0
		fp = fopen("/tmp/hostapd.conf", "w");

		fclose(fp);

		system("/usr/bin/hostapd -f /tmp/hostapd.conf &");
		system("udhcpd /etc/udhcpd.conf &");
#endif
		break;

	case MONITOR:
#if 0
		system("ifconfig wlan0 down");
		system("iwconfig wlan0 mode monitor");
		system("ifconfig wlan0 up");
#endif
		break;

	default:
		printf("not supported wlan mode:%d\n", mode);
		break;
	}

	return 0;
}

int CVI_NET_Connect_State(char *port, int *state)
{
	int fd;
	int len;
	char buf[20] = {'\0'};

	if (strcmp(WLAN0_PORT, port) == 0) {
		system("wpa_cli -iwlan0 status | grep wpa_state | cut -d '=' -f 2 > /tmp/wlan0_state");

		fd = open("/tmp/wlan0_state", O_RDONLY);
		if (fd < 0) {
			perror("Error:open failed");
			return -1;
		}

		read(fd, buf, sizeof(buf));

		close(fd);

		if (strstr(buf, "COMPLETED") != NULL)
			*state = CONNECTED;
		else
			*state = DISCONNECTED;

	} else if (strcmp(ETH0_PORT, port) == 0) {
		system("ifconfig eth0 | grep RUNNING > /dev/null");
		system("echo $? > /tmp/eth0_state");

		fd = open("/tmp/eth0_state", O_RDONLY);
		if (fd < 0) {
			perror("Error:open failed");
			return -1;
		}

		read(fd, buf, sizeof(buf));

		close(fd);

		if (strstr(buf, "0") != NULL)
			*state = CONNECTED;
		else
			*state = DISCONNECTED;
	}

	return 0;
}

int CVI_NET_Wlan_Signal_Strength(char *port, int *signal)
{
	char buf[4] = {'\0'};
	int fd;

	if (strcmp(port, "wlan0") == 0)
		system("cat /proc/net/rtl8189fs/wlan0/rx_signal | grep rssi | cut -d ':' -f 2 > /tmp/wlan_signal");
	else if (strcmp(port, "wlan1") == 0)
		system("cat /proc/net/rtl8189fs/wlan1/rx_signal | grep rssi | cut -d ':' -f 2 > /tmp/wlan_signal");
	else {
		printf("invalid wlan port\n");
		return -1;
	}

	fd = open("/tmp/wlan_signal", O_RDONLY);
	if (fd < 0) {
		perror("Error:open failed");
		return -1;
	}

	read(fd, buf, sizeof(buf));

	sscanf(buf, "%d", signal);

	close(fd);

	return 0;
}
void CheckNetState()
{
	CVI_NET_Connect_State(WLAN0_PORT, &wlanState);
}

#if 0
int main()
{
	int i;
	WLAN_AP_INFO_S *tmp = NULL;
	struct wlan_cfg cfg;

	CVI_NET_Wlan_Init("wlan0");

	sleep(3);

	tmp = CVI_NET_Get_Ap_Info();
	if (NULL != tmp) {
		printf("tmp->count=%d\n", tmp->count);
		for (i = 0; i < tmp->count; i++)
			printf("%s\t%d\t%d\t%s\t%s\n", tmp->ap_arr[i].bssid, tmp->ap_arr[i].freq, \
				tmp->ap_arr[i].signal_level, tmp->ap_arr[i].flag, tmp->ap_arr[i].ssid);
		free(tmp);
	} else {
		printf("tmp is null\n");
	}

	//strcpy(cfg.ssid, "TP-LINK_Zhxjun");
	//strcpy(cfg.passwd, "12345678");
	//CVI_NET_Set_Wlan_Config("wlan0", STATION, &cfg);
	//sleep(6);

	return 0;
}
#endif