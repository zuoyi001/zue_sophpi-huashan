#ifndef __CVI_NET_H__
#define __CVI_NET_H__

#define ETH0_PORT	"eth0"
#define WLAN0_PORT	"wlan0"
#define WLAN1_PORT	"wlan1"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef enum net_states {
	DISCONNECTED,
	CONNECTED
} NET_STATES_E;

typedef enum wlan_mode {
	STATION,
	ACCESSPOINT,
	MONITOR
} WLAN_MODE_E;

typedef struct wlan_cfg {
	char ssid[64];
	char passwd[64];
} WLAN_CFG_S;

typedef struct ap_info {
	char bssid[32];
	int freq;
	int signal_level;
	char flag[64];
	char ssid[64];
} AP_INTO_S;

typedef struct wlan_ap_info {
	int count;
	AP_INTO_S ap_arr[0];
} WLAN_AP_INFO_S;

int CVI_NET_Wlan_Init(char *port);
WLAN_AP_INFO_S *CVI_NET_Get_Ap_Info(void);
WLAN_AP_INFO_S *CVI_NET_Flash_Ap_Info(void);
int CVI_NET_Connect_State(char *port, int *state);
int CVI_NET_Set_Wlan_Config(char *port, int mode, struct wlan_cfg *pcfg);
int CVI_NET_Wlan_Signal_Strength(char *port, int *strength);
void CheckNetState();
int getEthState();
int getWlanState();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __CVI_NET_H__ */