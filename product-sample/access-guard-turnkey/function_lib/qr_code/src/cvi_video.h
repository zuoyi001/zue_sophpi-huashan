#ifndef _VIDEO_H_  
#define _VIDEO_H_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#include "cvi_buffer.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_comm_isp.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vi.h"
#include "cvi_isp.h"
#include "cvi_ae.h"
#include <cvi_venc.h>
#include "sample_comm.h"
#include <cvi_tracer.h>

void CVI_Set_VI_Config_dual(SAMPLE_VI_CONFIG_S* stViConfig);
CVI_S32 CVI_Init_Vpss(void);
CVI_S32  CVI_Video_Init(void);
CVI_S32 CVI_Video_Release(void);

void set_vpss_aspect(CVI_S32 vpsschn,CVI_S32 x,CVI_S32 y, CVI_S32 width, CVI_S32 height);

void set_ir_vpss_aspect(CVI_S32 vpsschn,CVI_S32 x,CVI_S32 y, CVI_S32 width, CVI_S32 height, PIXEL_FORMAT_E pixelFormat);

bool get_freeze_status();

void set_freeze_status(bool state);

#endif
