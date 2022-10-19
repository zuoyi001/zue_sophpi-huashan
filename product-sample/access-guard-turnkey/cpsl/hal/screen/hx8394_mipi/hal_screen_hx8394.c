#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "stdlib.h"
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdbool.h>

#include "cvi_hal_screen.h"
#include "screen_mipidsi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCREEN */
/** @{ */  /** <!-- [SCREEN] */

typedef struct tagHAL_SCREEN_Hx8394Ctx
{
    CVI_HAL_SCREEN_STATE_E screenDisplayState;
} HAL_SCREEN_Hx8394Ctx;
static HAL_SCREEN_Hx8394Ctx g_halScreenHx8394Ctx = {CVI_HAL_SCREEN_STATE_BUIT};

static const struct dsc_instr dsi_init_cmds[] = {
		{
			.delay = 0, .data_type = 0x29, .size = 4, .data = {
				0xB9, 0xFF, 0x83, 0x94
			}
		},
		{
			.delay = 0, .data_type = 0x29, .size = 15, .data = {
				0xB1, 0x50, 0x15, 0x75, 0x09, 0x32, 0x44, 0x71,
				0x31, 0x4D, 0x2F, 0x56, 0x73, 0x02, 0x02
			}
		},
		{
			.delay = 0, .data_type = 0x29, .size = 7, .data = {
				0xBA, 0x63, 0x03, 0x68, 0x6B, 0xB2, 0xC0
			}
		},
		{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0xD2, 0x88} },
		{
			.delay = 0, .data_type = 0x29, .size = 6, .data = {
				0xB2, 0x00, 0x80, 0x64, 0x10, 0x07
			}
		},
		{
			.delay = 0, .data_type = 0x29, .size = 22, .data = {
				0xB4, 0x01, 0x75, 0x01, 0x75, 0x01, 0x75, 0x01, 0x0C, 0x86,
				0x75, 0x00, 0x3F, 0x01, 0x75, 0x01, 0x75, 0x01, 0x75, 0x01,
				0x0C, 0x86
			}
		},
		{
			.delay = 0, .data_type = 0x29, .size = 34, .data = {
				0xD3, 0x00, 0x00, 0x06, 0x06, 0x40, 0x1A, 0x08, 0x00, 0x32,
				0x10, 0x08, 0x00, 0x08, 0x54, 0x15, 0x10, 0x05, 0x04, 0x02,
				0x12, 0x10, 0x05, 0x07, 0x23, 0x23, 0x0C, 0x0C, 0x27, 0x10,
				0x07, 0x07, 0x10, 0x40,
			}
		},
		{
			.delay = 0, .data_type = 0x29, .size = 45, .data = {
				0xD5, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x20,
				0x21, 0x22, 0x23, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
				0x18, 0x19, 0x19, 0x18, 0x18, 0x18, 0x18, 0x1B, 0x1B, 0x1A,
				0x1A, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
				0x18, 0x18, 0x18, 0x18, 0x18,
			}
		},
		{
			.delay = 0, .data_type = 0x29, .size = 45, .data = {
				0xD6, 0x03, 0x02, 0x01, 0x00, 0x07, 0x06, 0x05, 0x04, 0x23,
				0x22, 0x21, 0x20, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x58,
				0x58, 0x18, 0x18, 0x19, 0x19, 0x18, 0x18, 0x1B, 0x1B, 0x1A,
				0x1A, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
				0x18, 0x18, 0x18, 0x18, 0x18,
			}
		},
		{
			.delay = 0, .data_type = 0x29, .size = 59, .data = {
				0xE0, 0x00, 0x1A, 0x24, 0x2C, 0x2E, 0x32, 0x34, 0x32, 0x66,
				0x73, 0x82, 0x7F, 0x85, 0x95, 0x97, 0x99, 0xA4, 0xA5, 0xA0,
				0xAB, 0xBA, 0x5A, 0x59, 0x5D, 0x61, 0x63, 0x6C, 0x72, 0x7F,
				0x00, 0x19, 0x24, 0x2C, 0x2E, 0x32, 0x34, 0x32, 0x66, 0x73,
				0x82, 0x7F, 0x85, 0x95, 0x97, 0x99, 0xA4, 0xA5, 0xA0, 0xAB,
				0xBA, 0x5A, 0x59, 0x5D, 0x61, 0x63, 0x6C, 0x72, 0x7F,
			}
		},
		{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0xCC, 0x03} },
		{ .delay = 0, .data_type = 0x29, .size = 3, .data = {0xC0, 0x1F, 0x73} },
		{ .delay = 0, .data_type = 0x29, .size = 3, .data = {0xB6, 0x42, 0x42} },
		{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0xD4, 0x02} },
		{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0xBD, 0x01} },
		{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0xB1, 0x00} },
		{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0xBD, 0x00} },
		{ .delay = 0, .data_type = 0x29, .size = 8, .data = {0xBF, 0x40, 0x81, 0x50, 0x00, 0x1A, 0xFC, 0x01} },
		{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0xC6, 0xEF} },
		{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x36, 0x02} }, // h-flip
		{ .delay = 120, .data_type = 0x05, .size = 1, .data = {0x11} },
		{ .delay = 20, .data_type = 0x05, .size = 1, .data = {0x29} },
};

/**MIPI Screen do reset in HAL, RGB do reset in DRV*/
static void HAL_SCREEN_Reset(void)
{
	
}

/**Use GPIO high_low level must be set GPIO pin reuse*/
/**Use PWM. must set Pin reuse and PWM config*/
static void HAL_SCREEN_LumaInit(void)
{
	
}

static int32_t HAL_SCREEN_CmdInit(int devno)
{
	int ret;

	for (uint32_t i = 0; i < ARRAY_SIZE(dsi_init_cmds); i++) {
		const struct dsc_instr *instr = &(dsi_init_cmds[i]);
		struct cmd_info_s cmd_info = { .devno = devno, .cmd_size = instr->size
			, .data_type = instr->data_type, .cmd = (void *)instr->data };

		ret = MIPIDSI_WriteCmd(&cmd_info);
		if (instr->delay)
			usleep(instr->delay * 1000);
		if (ret) {
			printf("dsi init failed at %d instr.\n", i);
			return ret;
		}
	}
	return ret;
}

static int32_t HAL_SCREEN_HX8394_Init(void)
{
    int32_t ret = 0;
    ret = MIPIDSI_Init();
    if (ret != 0) {
        printf("MIPIDSI_Init failed.\n");
        return -1;
    }
    HAL_SCREEN_Reset();
    HAL_SCREEN_LumaInit();
    const struct combo_dev_cfg_s mipiDevCfg =
    {
		.devno = 0,
		.lane_id = {MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_CLK, MIPI_TX_LANE_2, MIPI_TX_LANE_3},
		.output_mode = OUTPUT_MODE_DSI_VIDEO,
		.video_mode = BURST_MODE,
		.output_format = OUT_FORMAT_RGB_24_BIT,
		.sync_info = {
			.vid_hsa_pixels = 64,
			.vid_hbp_pixels = 36,
			.vid_hfp_pixels = 128,
			.vid_hline_pixels = 720,
			.vid_vsa_lines = 16,
			.vid_vbp_lines = 4,
			.vid_vfp_lines = 6,
			.vid_active_lines = 1280,
			.vid_vsa_pos_polarity = false,
			.vid_hsa_pos_polarity = true,
		},
		.pixel_clk = 74250,
	};
    ret = MIPIDSI_SetDeviceConfig(&mipiDevCfg);
    if (ret != 0) {
        printf("MIPIDSI_SetDeviceConfig failed.\n");
        return -1;
    }
	ret = MIPIDSI_DisableVideoDataTransport();
    if (ret != 0) {
        printf("MIPIDSI_EnableVideoDataTransport failed.\n");
        return -1;
    }
	
	//init screen cmd
	HAL_SCREEN_CmdInit(CVI_HAL_SCREEN_IDX_0);
	//end
	
    ret = MIPIDSI_EnableVideoDataTransport();
    if (ret != 0) {
        printf("MIPIDSI_EnableVideoDataTransport failed.\n");
        return -1;
    }
    g_halScreenHx8394Ctx.screenDisplayState = true;
    return 0;
}

static int32_t HAL_SCREEN_HX8394_GetAttr(CVI_HAL_SCREEN_ATTR_S* pstAttr)
{
    pstAttr->enType = CVI_HAL_SCREEN_INTF_TYPE_MIPI;

    /* these magic value are given from screen attribute */
    pstAttr->stAttr.u32Width = 720;
    pstAttr->stAttr.u32Height = 1280;

    pstAttr->stAttr.u32Framerate = 60;

    pstAttr->stAttr.stSynAttr.u16Vact = 1280;
    pstAttr->stAttr.stSynAttr.u16Vbb = 4;
    pstAttr->stAttr.stSynAttr.u16Vfb = 6;
    pstAttr->stAttr.stSynAttr.u16Hact = 720;
    pstAttr->stAttr.stSynAttr.u16Hbb = 36;
    pstAttr->stAttr.stSynAttr.u16Hfb = 128;
    pstAttr->stAttr.stSynAttr.u16Hpw = 64;
    pstAttr->stAttr.stSynAttr.u16Vpw = 16;
    pstAttr->stAttr.stSynAttr.bIdv = 0;
    pstAttr->stAttr.stSynAttr.bIhs = 0;
    pstAttr->stAttr.stSynAttr.bIvs = 1;

    return 0;
}

static int32_t HAL_SCREEN_HX8394_GetDisplayState(CVI_HAL_SCREEN_STATE_E* penDisplayState)
{
    *penDisplayState = g_halScreenHx8394Ctx.screenDisplayState;
    return 0;
}

static int32_t HAL_SCREEN_HX8394_SetDisplayState(CVI_HAL_SCREEN_STATE_E enDisplayState)
{
    g_halScreenHx8394Ctx.screenDisplayState = enDisplayState;
    return 0;
}

static int32_t HAL_SCREEN_HX8394_GetLuma(uint32_t* pu32Luma)
{
	printf("HAL_SCREEN_HX8394_GetLuma = %d\n", *pu32Luma);
    return 0;
}

static int32_t HAL_SCREEN_HX8394_SetLuma(uint32_t u32Luma)
{
	printf("HAL_SCREEN_HX8394_SetLuma = %d\n", u32Luma);
    return 0;
}

static int32_t HAL_SCREEN_HX8394_GetSatuature(uint32_t* pu32Satuature)
{
	printf("HAL_SCREEN_HX8394_GetSatuature = %d\n", *pu32Satuature);
    return 0;
}

static int32_t HAL_SCREEN_HX8394_SetSatuature(uint32_t u32Satuature)
{
	printf("HAL_SCREEN_HX8394_SetSatuature = %d\n", u32Satuature);
    return 0;
}

static int32_t HAL_SCREEN_HX8394_GetContrast(uint32_t* pu32Contrast)
{
	printf("HAL_SCREEN_HX8394_GetContrast = %d\n", *pu32Contrast);
    return 0;
}

static int32_t HAL_SCREEN_HX8394_SetContrast(uint32_t u32Contrast)
{
	printf("HAL_SCREEN_HX8394_SetContrast = %d\n", u32Contrast);
    return 0;
}

static int32_t HAL_SCREEN_HX8394_SetBackLightState(CVI_HAL_SCREEN_STATE_E enBackLightState)
{
	printf("HAL_SCREEN_HX8394_SetBackLightState = %d\n", enBackLightState);
    return 0;
}

static int32_t HAL_SCREEN_HX8394_GetBackLightState(CVI_HAL_SCREEN_STATE_E* penBackLightState)
{
	printf("HAL_SCREEN_HX8394_GetBackLightState = %d\n", *penBackLightState);
    return 0;
}

static int32_t HAL_SCREEN_HX8394_Deinit(void)
{
    MIPIDSI_Deinit();
    return 0;
}

CVI_HAL_SCREEN_OBJ_S stHALSCREENObj =
{
    .pfnInit = HAL_SCREEN_HX8394_Init,
    .pfnGetAttr = HAL_SCREEN_HX8394_GetAttr,
    .pfnSetDisplayState = HAL_SCREEN_HX8394_SetDisplayState,
    .pfnGetDisplayState = HAL_SCREEN_HX8394_GetDisplayState,
    .pfnSetBackLightState = HAL_SCREEN_HX8394_SetBackLightState,
    .pfnGetBackLightState = HAL_SCREEN_HX8394_GetBackLightState,
    .pfnSetLuma = HAL_SCREEN_HX8394_SetLuma,
    .pfnGetLuma = HAL_SCREEN_HX8394_GetLuma,
    .pfnSetSaturature = HAL_SCREEN_HX8394_SetSatuature,
    .pfnGetSaturature = HAL_SCREEN_HX8394_GetSatuature,
    .pfnSetContrast = HAL_SCREEN_HX8394_SetContrast,
    .pfnGetContrast = HAL_SCREEN_HX8394_GetContrast,
    .pfnDeinit = HAL_SCREEN_HX8394_Deinit,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

