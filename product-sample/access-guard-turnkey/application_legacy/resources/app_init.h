#ifndef _APP_INIT_H_
#define _APP_INIT_H_

#include <linux/cvi_comm_vpss.h>
#include "sample_comm.h"

#ifdef __cplusplus
extern "C" {
#endif

int APP_InitVideoInput(SAMPLE_VI_CONFIG_S* stViConfig);
int APP_InitVideoOutput(SAMPLE_VO_CONFIG_S* stVoConfig);
int APP_InitFacelib(hal_facelib_config_t *facelib_config);

#ifdef __cplusplus
}
#endif
#endif