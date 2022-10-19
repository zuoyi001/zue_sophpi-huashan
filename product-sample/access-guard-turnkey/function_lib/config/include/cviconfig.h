#ifndef __CVI_CONFIG_UTILS_H__
#define __CVI_CONFIG_UTILS_H__

#include <stddef.h>

#ifdef  __cplusplus
extern  "C" {
#endif

#define CONFIG_PATH "./config.json"

// These keys are used by web server. Don't modify these keys!
#define CONF_KEY_COMPNAY_NAME "companyName"
#define CONF_KEY_DEVICE_PASSWORD "devicePassword"
#define CONF_KEY_DEVICE_NAME "deviceName"
#define CONF_KEY_THRESHOLD "threshold"
#define CONF_KEY_THRESHOLD_LIVENESS "threshold_liveness"
#define CONF_KEY_OPEN_DELAY "openDelay"
#define CONF_KEY_INTERVAL "interval"
#define CONF_KEY_VOICE_MODE "voiceMode"
#define CONF_KEY_VOICE_SOUND "voiceSound"
#define CONF_KEY_VOICE_CUSTOM "voiceCustom"
#define CONF_KEY_DISPLAY_MODE "displayMode"
#define CONF_KEY_DISPLAY_CUSTOM "displayCustom"
#define CONF_KEY_DISPLAY_BRIGHTNESS "displayBrightness"
#define CONF_KEY_DISPLAY_ENTER_LOCK_TIME "displayEnterLockTime"
#define CONF_KEY_DISPLAY_ENTER_CLOSE_TIME "displayEnterCloseTime"
#define CONF_KEY_STRANGER_MODE "strangerMode"
#define CONF_KEY_STRANGER_CUSTOM "strangerCustom"
#define CONF_KEY_STRANGER_VOICE_MODE "strangerVoiceMode"
#define CONF_KEY_STRANGERVOICE_CUSTOM "strangerVoiceCustom"
#define CONF_KEY_MAX_FACE_SIZE "maxFaceSize"
#define CONF_KEY_LIVENESS_TYPE "livenessType"
#define CONF_KEY_SIGN_DISTANCE "signDistance"
#define CONF_KEY_SUCCESS_RETRY_DELAY "successRetryDelay"
#define CONF_KEY_SUCCESS_RETRY "successRetry"
#define CONF_KEY_UPLOAD_RECORD_IMAGE "uploadRecordImage"
#define CONF_KEY_IR_LIVE_PREVIEW "irLivePreview"
#define CONF_KEY_REBOOT_EVERY_DAY "rebootEveryDay"
#define CONF_KEY_REBOOT_HOUR "rebootHour"
#define CONF_KEY_REBOOT_MIN "rebootMin"
#define CONF_KEY_VERSION_CODE "versionCode"
#define CONF_KEY_PACKAGE_NAME "packageName"
#define CONF_KEY_VERSION_NAME "versionName"
#define CONF_KEY_FACE_QUALITY "faceQuality"
#define CONF_KEY_FACE_QUALITY_THRESHOLD "faceQualityThreshold"
#define CONF_KEY_NET_MODE	 "netMode"
#define CONF_KEY_NET_IP		 "netIp"
#define CONF_KEY_NET_MASK	 "netMask"
#define CONF_KEY_GATEWAY	 "gateway"
#define CONF_KEY_DNS	 	"dns"
#define CONF_KEY_WIFI_ONOFF	 "wifiOnOff"
#define CONF_KEY_WIFI_SSID	 "wifiSSID"
#define CONF_KEY_WIFI_PWD	 "wifiPWD"





typedef struct {
    char company_name[256];
    char device_password[256];
    char device_name[256];
    float threshold;
	float threshold_liveness;
    int open_delay;
    int interval;
    int voice_mode;
    int voice_sound;
    char voice_custom[512];
    int display_mode;
    char display_custom[512];
    int display_brightness;
    int display_enter_lock_time;
    int display_enter_close_time;
    int stranger_mode;
    char stranger_custom[512];
    int stranger_voice_mode;
    char stranger_voice_custom[512];
    int max_face_size;
    int liveness_type;
    int sign_distance;
    int success_retry_delay;
    int success_retry;
    int upload_record_image;
    int ir_live_preview;
    int reboot_every_day;
    int reboot_hour;
    int reboot_min;
    int version_code;
    char package_name[256];
    char version_name[64];
    int face_quality;
    float face_quality_threshold;
	int net_mode;
	char ip_address[32]; 
	char netmask[32];
	char gateway[32];
	char dns[32];
	int wifi_onoff;
	char wifi_ssid[32]; 
	char wifi_pwd[32];
} cvi_device_config_t;

extern cvi_device_config_t device_cfg1;


int cvi_get_device_config(cvi_device_config_t *config);
int cvi_set_device_config(cvi_device_config_t *config);

int cvi_get_device_config_int(const char *key, int *val);
int cvi_get_device_config_str(const char *key, char *buf, size_t buf_size);
int cvi_get_device_config_float(const char *key, float *val);

int cvi_set_device_config_int(const char *key, const int val);
int cvi_set_device_config_str(const char *key, const char *buf);
int cvi_set_device_config_float(const char *key, const float val);

#ifdef  __cplusplus
}
#endif

#endif
