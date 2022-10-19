#ifndef __CVI_CONFIG_SYSTEM_UTILS_H__
#define __CVI_CONFIG_SYSTEM_UTILS_H__

#include <stddef.h>

#ifdef  __cplusplus
extern  "C" {
#endif

#define CONFIG_PATH "./config.json"

// These keys are used by web server. Don't modify these keys!
#define CONF_KEY_DEVICE_ID "deviceId"
#define CONF_KEY_DEVICE_NAME "deviceName"
#define CONF_KEY_SIGN_KEY "signKey"
#define CONF_KEY_SIGN_TYPE "signType"
#define CONF_KEY_CLOUD_SERVER "cloud_server"
#define CONF_KEY_REGIST_ID "regist_id"
#define CONF_KEY_WEB_ONOFF "web_onoff"






typedef struct {
	int deviceId;
    char deviceName[256];
    char signKey[32];
    int signType;
	char cloud_server[256];
	char regist_id[64];
	int web_onoff;
} cvi_device_config_system_t;

extern cvi_device_config_system_t device_cfg_system;


int cvi_get_device_config_system(cvi_device_config_system_t *config);
int cvi_set_device_config_system(cvi_device_config_system_t *config);

int cvi_get_device_config_system_int(const char *key, int *val);
int cvi_get_device_config_system_str(const char *key, char *buf, size_t buf_size);
int cvi_get_device_config_system_float(const char *key, float *val);

int cvi_set_device_config_system_int(const char *key, int val);
int cvi_set_device_config_system_str(const char *key, char *buf);
int cvi_set_device_config_system_float(const char *key, float val);

#ifdef  __cplusplus
}
#endif

#endif
