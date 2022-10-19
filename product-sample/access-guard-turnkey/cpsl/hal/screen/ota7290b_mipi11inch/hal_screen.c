#include <linux/types.h>
//#include <cvi_mipi.h>
//#include <stdio.h>

#include "cvi_hal_screen.h"
#include "screen_mipidsi.h"
#include "cvi_hal_gpio.h"
#include "cvi_hal_pwm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCREEN */
/** @{ */  /** <!-- [SCREEN] */

typedef struct tagHAL_SCREENCtx
{
	CVI_HAL_SCREEN_STATE_E screenDisplayState;
} HAL_SCREENCtx;
static HAL_SCREENCtx g_halScreenCtx = {CVI_HAL_SCREEN_STATE_BUIT};

CVI_HAL_SCREEN_GPIO_S screen_gpio[GPIO_TYPE_BUTT] = {
	[GPIO_TYPE_RESET] = {
		.gpio = CVI_GPIOC_08,
		.name = "reset"
	},
	[GPIO_TYPE_POWER] = {
		.gpio = CVI_GPIOC_09,
		.name = "power"
	},
	//there is no need to define bl gpio if use pwm
	#if 0
	[GPIO_TYPE_BACKLIGHT] = {
		.gpio = CVI_GPIOB_04,
		.name = "backlight"
	},
	#endif
};

CVI_HAL_SCREEN_PWM_S screen_bl = {
	.group  = 0,
	.channel = 1,
	.period = 5000,    //unit:ns
	.duty_cycle = 2500 //unit:ns
};

static const struct dsc_instr dsi_init_cmds[] = {
	{ .delay = 250, .data_type = 0x15, .size = 2, .data = {0x11, 0x00} },
	{ .delay = 50, .data_type = 0x15, .size = 2, .data = {0xB0, 0x5A} },

	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0xB0, 0x5A} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0xB1, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x89, 0x01} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x91, 0x17} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0xB1, 0x03} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x2C, 0x28} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x00, 0xB7} },

	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x01, 0x1B} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x02, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x03, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x04, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x05, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x06, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x07, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x08, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x09, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x0A, 0x01} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x0B, 0x3C} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x0C, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x0D, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x0E, 0x24} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x0F, 0x1C} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x10, 0xC9} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x11, 0x60} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x12, 0x70} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x13, 0x01} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x14, 0xE3} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x15, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x16, 0x3D} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x17, 0x0E} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x18, 0x01} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x19, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x1A, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x1B, 0xFC} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x1C, 0x0B} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x1D, 0xA0} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x1E, 0x03} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x1F, 0x04} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x20, 0x0C} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x21, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x22, 0x04} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x23, 0x81} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x24, 0x1F} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x25, 0x10} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x26, 0x9B} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x2D, 0x01} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x2E, 0x84} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x2F, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x30, 0x02} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x31, 0x08} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x32, 0x01} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x33, 0x1C} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x34, 0x70} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x35, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x36, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x37, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x38, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x39, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x3A, 0x05} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x3B, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x3C, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x3D, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x3E, 0x0F} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x3F, 0x8C} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x40, 0x2A} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x41, 0xFC} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x42, 0x01} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x43, 0x40} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x44, 0x05} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x45, 0xE8} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x46, 0x16} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x47, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x48, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x49, 0x88} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x4A, 0x08} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x4B, 0x05} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x4C, 0x03} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x4D, 0xD0} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x4E, 0x13} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x4F, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x50, 0x0A} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x51, 0x53} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x52, 0x26} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x53, 0x22} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x54, 0x09} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x55, 0x22} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x56, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x57, 0x1C} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x58, 0x03} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x59, 0x3F} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x5A, 0x28} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x5B, 0x01} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x5C, 0xCC} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x5D, 0x20} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x5E, 0xE8} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x5F, 0x1D} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x60, 0xE1} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x61, 0x73} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x62, 0x8D} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x63, 0x2D} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x64, 0x25} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x65, 0x82} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x66, 0x09} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x67, 0x21} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x68, 0x84} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x69, 0x10} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x6A, 0x42} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x6B, 0x08} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x6C, 0x21} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x6D, 0x84} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x6E, 0x10} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x6F, 0x42} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x70, 0x08} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x71, 0x21} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x72, 0x84} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x73, 0x10} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x74, 0x42} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x75, 0x08} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x76, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x77, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x78, 0x0F} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x79, 0xE0} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x7A, 0x01} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x7B, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x7C, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x7D, 0x0C} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x7E, 0x41} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x7F, 0xFE} },

	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0xB1, 0x02} },

	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x00, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x01, 0x05} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x02, 0xDC} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x03, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x04, 0x3E} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x05, 0x4E} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x06, 0x90} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x07, 0x10} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x08, 0xC0} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x09, 0x01} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x0A, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x0B, 0x14} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x0C, 0xE6} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x0D, 0x0D} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x0F, 0x08} },

	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x10, 0xF9} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x11, 0xF5} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x12, 0xA2} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x13, 0x03} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x14, 0x5E} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x15, 0xCF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x16, 0x63} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x17, 0x01} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x18, 0xE9} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x19, 0x5E} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x1A, 0x59} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x1B, 0x0E} },

	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x1C, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x1D, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x1E, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x1F, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x20, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x21, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x22, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x23, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x24, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x25, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x26, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x27, 0x1F} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x28, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x29, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x2A, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x2B, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x2C, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x2D, 0x07} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x33, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x35, 0x7E} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x36, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x38, 0x7E} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x3A, 0x80} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x3B, 0x01} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x3C, 0xC0} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x3D, 0x2D} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x3E, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x3F, 0xB8} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x40, 0x05} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x41, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x42, 0xB7} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x43, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x44, 0xE0} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x45, 0x06} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x46, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x47, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x48, 0x9B} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x49, 0xD2} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x4A, 0x71} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x4B, 0xE3} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x4C, 0x16} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x4D, 0xC0} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x4E, 0x0F} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x4F, 0x61} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x50, 0x78} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x51, 0x7A} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x52, 0x34} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x53, 0x99} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x54, 0xA2} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x55, 0x02} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x56, 0x24} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x57, 0xF8} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x58, 0xFC} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x59, 0xF4} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x5A, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x5B, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x5C, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x5D, 0xB2} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x5E, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x5F, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x60, 0x8F} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x61, 0x62} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x62, 0xB5} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x63, 0xB2} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x64, 0x5A} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x65, 0xAD} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x66, 0x56} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x67, 0x2B} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x68, 0x0C} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x69, 0x01} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x6A, 0x01} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x6B, 0xFC} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x6C, 0xFD} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x6D, 0xFD} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x6E, 0xFD} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x6F, 0xFD} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x70, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x71, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x72, 0x3F} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x73, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x74, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x75, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x76, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x77, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x78, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x79, 0x00} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x7A, 0xDC} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x7B, 0xDC} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x7C, 0xDC} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x7D, 0xDC} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x7E, 0xDC} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x7F, 0x6E} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x0B, 0x04} },

	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0xB1, 0x03} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x2C, 0x2C} },

	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0xB1, 0x00} },
	{ .delay = 200, .data_type = 0x15, .size = 2, .data = {0x89, 0x03} },

	{ .delay = 50, .data_type = 0x15, .size = 2, .data = {0x29, 0x0} },
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

static int32_t HAL_SCREEN_Init(void)
{
	int32_t ret = 0;
	int32_t i;

	for (i = GPIO_TYPE_RESET; i < GPIO_TYPE_BUTT; i++) {
		CVI_HAL_GpioExport(screen_gpio[i].gpio);
		CVI_HAL_GpioDirectionOutput(screen_gpio[i].gpio);
	}

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
			.vid_hsa_pixels = 30,
			.vid_hbp_pixels = 50,
			.vid_hfp_pixels = 150,
			.vid_hline_pixels = 440,
			.vid_vsa_lines = 20,
			.vid_vbp_lines = 30,
			.vid_vfp_lines = 150,
			.vid_active_lines = 1920,
			.vid_vsa_pos_polarity = false,
			.vid_hsa_pos_polarity = true,
		},
		.pixel_clk = 85224,
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

	//ctrl power
	CVI_HAL_GpioSetValue(screen_gpio[GPIO_TYPE_POWER].gpio, CVI_HAL_GPIO_VALUE_H);

	//ctrl reset
	CVI_HAL_GpioSetValue(screen_gpio[GPIO_TYPE_RESET].gpio, CVI_HAL_GPIO_VALUE_H);
	usleep(10*1000);
	CVI_HAL_GpioSetValue(screen_gpio[GPIO_TYPE_RESET].gpio, CVI_HAL_GPIO_VALUE_L);
	usleep(10*1000);
	CVI_HAL_GpioSetValue(screen_gpio[GPIO_TYPE_RESET].gpio, CVI_HAL_GPIO_VALUE_H);

	//init screen cmd
	HAL_SCREEN_CmdInit(CVI_HAL_SCREEN_IDX_0);
	//end

	ret = MIPIDSI_EnableVideoDataTransport();
	if (ret != 0) {
		printf("MIPIDSI_EnableVideoDataTransport failed.\n");
		return -1;
	}
	//ctrl backlight
	CVI_HAL_PwmExport(screen_bl.group, screen_bl.channel);
	CVI_HAL_PwmSet_Parm(screen_bl.group, screen_bl.channel, screen_bl.period, screen_bl.duty_cycle);
	CVI_HAL_PwmEnable(screen_bl.group, screen_bl.channel);
	//CVI_HAL_GpioSetValue(screen_gpio[GPIO_TYPE_BACKLIGHT].gpio, CVI_HAL_GPIO_VALUE_H);
	g_halScreenCtx.screenDisplayState = true;
	return 0;
}

static int32_t HAL_SCREEN_GetAttr(CVI_HAL_SCREEN_ATTR_S* pstAttr)
{
	pstAttr->enType = CVI_HAL_SCREEN_INTF_TYPE_MIPI;

    /* these magic value are given from screen attribute */
    pstAttr->stAttr.u32Width = 440;
    pstAttr->stAttr.u32Height = 1920;

	pstAttr->stAttr.u32Framerate = 60;

    pstAttr->stAttr.stSynAttr.u16Vact = 1920;
    pstAttr->stAttr.stSynAttr.u16Vbb = 30;
    pstAttr->stAttr.stSynAttr.u16Vfb = 150;
    pstAttr->stAttr.stSynAttr.u16Hact = 440;
    pstAttr->stAttr.stSynAttr.u16Hbb = 50;
    pstAttr->stAttr.stSynAttr.u16Hfb = 150;
    pstAttr->stAttr.stSynAttr.u16Hpw = 30;
    pstAttr->stAttr.stSynAttr.u16Vpw = 20;
    pstAttr->stAttr.stSynAttr.bIdv = 0;
    pstAttr->stAttr.stSynAttr.bIhs = 0;
    pstAttr->stAttr.stSynAttr.bIvs = 1;

	return 0;
}

static int32_t HAL_SCREEN_GetDisplayState(CVI_HAL_SCREEN_STATE_E* penDisplayState)
{
	*penDisplayState = g_halScreenCtx.screenDisplayState;
	return 0;
}

static int32_t HAL_SCREEN_SetDisplayState(CVI_HAL_SCREEN_STATE_E enDisplayState)
{
	g_halScreenCtx.screenDisplayState = enDisplayState;
	return 0;
}

static int32_t HAL_SCREEN_GetLuma(uint32_t* pu32Luma)
{
	printf("HAL_SCREEN_GetLuma = %d\n", *pu32Luma);
	return 0;
}

static int32_t HAL_SCREEN_SetLuma(uint32_t u32Luma)
{
	printf("HAL_SCREEN_SetLuma = %d\n", u32Luma);
	return 0;
}

static int32_t HAL_SCREEN_GetSatuature(uint32_t* pu32Satuature)
{
	printf("HAL_SCREEN_GetSatuature = %d\n", *pu32Satuature);
	return 0;
}

static int32_t HAL_SCREEN_SetSatuature(uint32_t u32Satuature)
{
	printf("HAL_SCREEN_SetSatuature = %d\n", u32Satuature);
	return 0;
}

static int32_t HAL_SCREEN_GetContrast(uint32_t* pu32Contrast)
{
	printf("HAL_SCREEN_GetContrast = %d\n", *pu32Contrast);
	return 0;
}

static int32_t HAL_SCREEN_SetContrast(uint32_t u32Contrast)
{
	printf("HAL_SCREEN_SetContrast = %d\n", u32Contrast);
	return 0;
}

static int32_t HAL_SCREEN_SetBackLightState(CVI_HAL_SCREEN_STATE_E enBackLightState)
{
	printf("HAL_SCREEN_SetBackLightState = %d\n", enBackLightState);
	return 0;
}

static int32_t HAL_SCREEN_GetBackLightState(CVI_HAL_SCREEN_STATE_E* penBackLightState)
{
	printf("HAL_SCREEN_GetBackLightState = %d\n", *penBackLightState);
	return 0;
}

static int32_t HAL_SCREEN_Deinit(void)
{
    MIPIDSI_Deinit();
    return 0;
}

CVI_HAL_SCREEN_OBJ_S stHALSCREENObj =
{
	.pfnInit = HAL_SCREEN_Init,
	.pfnGetAttr = HAL_SCREEN_GetAttr,
	.pfnSetDisplayState = HAL_SCREEN_SetDisplayState,
	.pfnGetDisplayState = HAL_SCREEN_GetDisplayState,
	.pfnSetBackLightState = HAL_SCREEN_SetBackLightState,
	.pfnGetBackLightState = HAL_SCREEN_GetBackLightState,
	.pfnSetLuma = HAL_SCREEN_SetLuma,
	.pfnGetLuma = HAL_SCREEN_GetLuma,
	.pfnSetSaturature = HAL_SCREEN_SetSatuature,
	.pfnGetSaturature = HAL_SCREEN_GetSatuature,
	.pfnSetContrast = HAL_SCREEN_SetContrast,
	.pfnGetContrast = HAL_SCREEN_GetContrast,
	.pfnDeinit = HAL_SCREEN_Deinit,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

