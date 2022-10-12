
/* Include files */
#include "mmse.h"
#include "dr_detection.h"

/* Function Definitions */
void dr_detection(const float amp_inst_band[15], float noisy_st_trough[5], float
                  noisy_st_peak[5], float *vad_dr_cnt, float *vad_dr, float Fs)
{
  float DR_bin_cnt;
  int k;
  float alpha;

  /*  DR based VAD is good at low and high SNRs, but not good for tone */
  /*  alike signals and also sensitive to DC setup */
  DR_bin_cnt = 0.0F;
  for (k = 0; k < 5; k++) {
    if (amp_inst_band[k] - noisy_st_trough[k] > 0.0F) {
      alpha = 0.98F;
    } else {
      alpha = 0.5F;
    }

    noisy_st_trough[k] = alpha * noisy_st_trough[k] + (1.0F - alpha) * amp_inst_band[k];
    if (amp_inst_band[k] - noisy_st_peak[k] > 0.0F) {
      alpha = 0.9F;
    } else {
      alpha = 0.93F;
    }

    noisy_st_peak[k] = alpha * noisy_st_peak[k] + (1.0F - alpha) * amp_inst_band[k];

    /*  to make sure noisy_st_peak cannot go up too much, now maximal 24 dB range */
    if (noisy_st_peak[k] / 16.0F - noisy_st_trough[k] > 0.0F) {
      noisy_st_peak[k] = noisy_st_trough[k] * 16.0F;
    }

    /*  the smaller the threshold, the faster to suppress noise but easily introduce more speech distortion as trade-off */
    if (noisy_st_peak[k] * 0.5012F - noisy_st_trough[k] > 0.0F) {
      DR_bin_cnt++;
    }
  }

  if (DR_bin_cnt == 0.0F) {
    DR_bin_cnt = *vad_dr_cnt - 1.0F;
    if (0.0F > DR_bin_cnt) {
      DR_bin_cnt = 0.0F;
    }

    *vad_dr_cnt = DR_bin_cnt;
    if (DR_bin_cnt == 0.0F) {
      *vad_dr = 0.0F;
    }
  } else {
    *vad_dr = 1.0F;
    if (Fs == 16000.0F) {
      *vad_dr_cnt = 7.0F;
    } else {
      if (Fs == 8000.0F) {
        *vad_dr_cnt = 7.0F;
      }
    }
  }

}


