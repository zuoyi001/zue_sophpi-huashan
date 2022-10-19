#ifndef __CVI_CONFIG_WEBSERVER_UTILS_H__
#define __CVI_CONFIG_WEBSERVER_UTILS_H__

#include <stddef.h>

#ifdef  __cplusplus
extern  "C" {
#endif

#define CONFIG_WEBSERVER_PATH "./webserver.json"

// These keys are used by web server. Don't modify these keys!
#define CONF_KEY_CLOUD_SERVER "cloud_server"


typedef struct {
    char cloud_server[256];
} cvi_device_config_webserver_t;

extern cvi_device_config_webserver_t device_cfg_webserver;


int cvi_get_device_config_webserver(cvi_device_config_webserver_t *config);
int cvi_set_device_config_webserver(cvi_device_config_webserver_t *config);

int cvi_get_device_config_webserver_int(const char *key, int *val);
int cvi_get_device_config_webserver_str(const char *key, char *buf, size_t buf_size);
int cvi_get_device_config_webserver_float(const char *key, float *val);

int cvi_set_device_config_webserver_int(const char *key, int val);
int cvi_set_device_config_webserver_str(const char *key, char *buf);
int cvi_set_device_config_webserver_float(const char *key, float val);

#ifdef  __cplusplus
}
#endif

#endif
