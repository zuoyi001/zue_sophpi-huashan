
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "linux/cvi_comm_video.h"
#include "cvi_mipi_tx.h"
#include "mipi_tx.h"
#include "sample_comm.h"

//#define MIPI_LOGD(...)   CVI_NVRLOGD(__VA_ARGS__)
//#define MIPI_LOGI(...)   CVI_NVRLOGI(__VA_ARGS__)
//#define MIPI_LOGW(...)   CVI_NVRLOGW(__VA_ARGS__)
//#define MIPI_LOGE(...)   CVI_NVRLOGE(__VA_ARGS__)

#define YINGMA    0x01
#define YUANWEI   0x02

#define MIPI_CONFIG  YUANWEI
#define MIPI_TX_NAME "/dev/cvi-mipi-tx"
//DTV 									      // hfp, hs, hbp,hact,htotal,vfp, vs, vbp,vact,vtotal, hfp, hs, hbp,hact,htotal,vfp, vs, vbp,vact,vtotal, h_polary, v_polary, vic, pclk_khz

//static struct video_timing video_1920x1080_60Hz   ={88, 44, 148,1920,  2200,  4,  5,  36, 1080,  1125, 1,1,16,AR_16_9,148500};
static struct combo_dev_cfg_s dev_cfg_video_1920x1080_60Hz = {
	.devno = 0,
	.lane_id = {MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_2, MIPI_TX_LANE_CLK, MIPI_TX_LANE_3},  // yuan wei
	.lane_pn_swap = {false, false, false, false, false},
	.output_mode = OUTPUT_MODE_DSI_VIDEO,
	.video_mode = BURST_MODE,
	.output_format = OUT_FORMAT_RGB_24_BIT,
	.sync_info = {
		.vid_hsa_pixels = 44,
		.vid_hbp_pixels = 148,
		.vid_hfp_pixels = 88,
		.vid_hline_pixels = 1920,
		.vid_vsa_lines = 5,
		.vid_vbp_lines = 36,
		.vid_vfp_lines = 4,
		.vid_active_lines = 1080,
		.vid_vsa_pos_polarity = true,
		.vid_hsa_pos_polarity = true,
	},
	.pixel_clk = 148500,
};


//static struct video_timing video_1920x1080_30Hz   ={88, 44, 148,1920,  2200,  4,  5,  36, 1080,  1125, 1,1,34,AR_16_9,74250};
static struct combo_dev_cfg_s dev_cfg_video_1920x1080_30Hz = {
	.devno = 0,
	.lane_id = {MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_2, MIPI_TX_LANE_CLK, MIPI_TX_LANE_3},  // yuan wei
	.lane_pn_swap = {false, false, false, false, false},
	.output_mode = OUTPUT_MODE_DSI_VIDEO,
	.video_mode = BURST_MODE,
	.output_format = OUT_FORMAT_RGB_24_BIT,
	.sync_info = {
		.vid_hsa_pixels = 44,
		.vid_hbp_pixels = 148,
		.vid_hfp_pixels = 88,
		.vid_hline_pixels = 1920,
		.vid_vsa_lines = 5,
		.vid_vbp_lines = 36,
		.vid_vfp_lines = 4,
		.vid_active_lines = 1080,
		.vid_vsa_pos_polarity = true,
		.vid_hsa_pos_polarity = true,
	},
	.pixel_clk = 74250,
};

//static struct video_timing video_1280x720_60Hz    ={110,40, 220,1280,  1650,  5,  5,  20, 720,   750, 1, 1, 4, AR_16_9,74250};
static struct combo_dev_cfg_s dev_cfg_video_1280x720_60Hz = {
	.devno = 0,
	.lane_id = {MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_2, MIPI_TX_LANE_CLK, MIPI_TX_LANE_3},  // yuan wei
	.lane_pn_swap = {false, false, false, false, false},
	.output_mode = OUTPUT_MODE_DSI_VIDEO,
	.video_mode = BURST_MODE,
	.output_format = OUT_FORMAT_RGB_24_BIT,
	.sync_info = {
		.vid_hsa_pixels = 40,
		.vid_hbp_pixels = 220,
		.vid_hfp_pixels = 110,
		.vid_hline_pixels = 1280,
		.vid_vsa_lines = 5,
		.vid_vbp_lines = 20,
		.vid_vfp_lines = 5,
		.vid_active_lines = 720,
		.vid_vsa_pos_polarity = true,
		.vid_hsa_pos_polarity = true,
	},
	.pixel_clk = 74250,
};

//static struct video_timing video_1024x768_60Hz    ={24, 136, 160, 1024, 1344,  3,  6,  29, 768, 806,   0,0,0,AR_16_9,65000};
static struct combo_dev_cfg_s dev_cfg_video_1024x768_60Hz = {
	.devno = 0,
	.lane_id = {MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_2, MIPI_TX_LANE_CLK, MIPI_TX_LANE_3},  // yuan wei
	.lane_pn_swap = {false, false, false, false, false},
	.output_mode = OUTPUT_MODE_DSI_VIDEO,
	.video_mode = BURST_MODE,
	.output_format = OUT_FORMAT_RGB_24_BIT,
	.sync_info = {
		.vid_hsa_pixels = 136,
		.vid_hbp_pixels = 160,
		.vid_hfp_pixels = 24,
		.vid_hline_pixels = 1024,
		.vid_vsa_lines = 6,
		.vid_vbp_lines = 29,
		.vid_vfp_lines = 3,
		.vid_active_lines = 768,
		.vid_vsa_pos_polarity = false,
		.vid_hsa_pos_polarity = false,
	},
	.pixel_clk = 65000,
};

//DTV 									      // hfp, hs, hbp,hact,htotal,vfp, vs, vbp,vact,vtotal, hfp, hs, hbp,hact,htotal,vfp, vs, vbp,vact,vtotal, h_polary, v_polary, vic, pclk_khz

//static struct video_timing video_1280x1024_60Hz   ={48, 112, 248, 1280, 1688,  1,  3,  38, 1024, 1066, 1,1,0,AR_16_9,108000};
static struct combo_dev_cfg_s dev_cfg_video_1280x1024_60Hz = {
	.devno = 0,
	.lane_id = {MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_2, MIPI_TX_LANE_CLK, MIPI_TX_LANE_3},  // yuan wei
	.lane_pn_swap = {false, false, false, false, false},
	.output_mode = OUTPUT_MODE_DSI_VIDEO,
	.video_mode = BURST_MODE,
	.output_format = OUT_FORMAT_RGB_24_BIT,
	.sync_info = {
		.vid_hsa_pixels = 112,
		.vid_hbp_pixels = 248,
		.vid_hfp_pixels = 48,
		.vid_hline_pixels = 1280,
		.vid_vsa_lines = 3,
		.vid_vbp_lines = 38,
		.vid_vfp_lines = 1,
		.vid_active_lines = 1024,
		.vid_vsa_pos_polarity = true,
		.vid_hsa_pos_polarity = true,
	},
	.pixel_clk = 108000,
};

//static struct video_timing video_1600x1200_60Hz   ={64, 192, 304, 1600, 2160,  1,  3,  46, 1200, 1250, 1,1,0,AR_16_9,162000};
static struct combo_dev_cfg_s dev_cfg_video_1600x1200_60Hz = {
	.devno = 0,
	.lane_id = {MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_2, MIPI_TX_LANE_CLK, MIPI_TX_LANE_3},  // yuan wei
	.lane_pn_swap = {false, false, false, false, false},
	.output_mode = OUTPUT_MODE_DSI_VIDEO,
	.video_mode = BURST_MODE,
	.output_format = OUT_FORMAT_RGB_24_BIT,
	.sync_info = {
		.vid_hsa_pixels = 192,
		.vid_hbp_pixels = 304,
		.vid_hfp_pixels = 64,
		.vid_hline_pixels = 1600,
		.vid_vsa_lines = 3,
		.vid_vbp_lines = 46,
		.vid_vfp_lines = 1,
		.vid_active_lines = 1200,
		.vid_vsa_pos_polarity = true,
		.vid_hsa_pos_polarity = true,
	},
	.pixel_clk = 162000,
};

static struct hs_settle_s hs_timing_cfg = { .prepare = 6, .zero = 32, .trail = 5 };

void cvi_mipi_tx_set(CVI_MIPI_TX_VIDEO_FORMAT_E enVideoFormat)
{
    int fd = open(MIPI_TX_NAME, O_RDWR | O_NONBLOCK, 0);
    
    if (fd == -1) {
        printf("Cannot open %s", MIPI_TX_NAME);
        return;
    }
    
    struct combo_dev_cfg_s *pcfg = NULL;

    switch(enVideoFormat)
    {
    case E_CVI_MIPI_TX_VIDEO_1024x768_60HZ:
        pcfg = &dev_cfg_video_1024x768_60Hz;
        printf("cvi_mipi_tx_set to 1024x768@60fps...");
        break;
    case E_CVI_MIPI_TX_VIDEO_1280x720_60HZ:
        pcfg = &dev_cfg_video_1280x720_60Hz;
        printf("cvi_mipi_tx_set to 1280x720@60fps...");
        break;
    case E_CVI_MIPI_TX_VIDEO_1280x1024_60HZ:
        pcfg = &dev_cfg_video_1280x1024_60Hz;
        printf("cvi_mipi_tx_set to 1280x1024@60fps...");
        break;
    case E_CVI_MIPI_TX_VIDEO_1600x1200_60HZ:
        pcfg = &dev_cfg_video_1600x1200_60Hz;
        printf("cvi_mipi_tx_set to 1600x1200@60fps...");
        break;
    case E_CVI_MIPI_TX_VIDEO_1920x1080_30HZ:
        pcfg = &dev_cfg_video_1920x1080_30Hz;
        printf("cvi_mipi_tx_set to 1920x1080@30fps...");
        break;
    case E_CVI_MIPI_TX_VIDEO_1920x1080_60HZ:
        pcfg = &dev_cfg_video_1920x1080_60Hz;
        printf("cvi_mipi_tx_set to 1920x1080@60fps...");
        break;
    default:
        printf("ERROR: unknown video format...");
        break;
    }

    mipi_tx_disable(fd);
    
    mipi_tx_cfg(fd, pcfg);
    
    mipi_tx_set_hs_settle(fd, &hs_timing_cfg);
    
    mipi_tx_enable(fd);

    close(fd);
}

