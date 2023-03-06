
#ifndef CVI_MIPI_TX_H_
#define CVI_MIPI_TX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum
{
    E_CVI_MIPI_TX_VIDEO_1024x768_60HZ,
    E_CVI_MIPI_TX_VIDEO_1280x720_60HZ,
    E_CVI_MIPI_TX_VIDEO_1280x1024_60HZ,
    E_CVI_MIPI_TX_VIDEO_1600x1200_60HZ,
    E_CVI_MIPI_TX_VIDEO_1920x1080_30HZ,
    E_CVI_MIPI_TX_VIDEO_1920x1080_60HZ,
    E_CVI_MIPI_TX_VIDEO_MAX
} CVI_MIPI_TX_VIDEO_FORMAT_E;

void cvi_mipi_tx_set(CVI_MIPI_TX_VIDEO_FORMAT_E enVideoFormat);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
