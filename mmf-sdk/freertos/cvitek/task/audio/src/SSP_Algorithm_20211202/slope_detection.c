
/* Include files */
#include "mmse.h"
#include "slope_detection.h"

/* Function Definitions */
void slope_detection(const float amp_inst_band[15], float *vad_slope_cnt, float *vad_slope, float Fs)
{
  int instant_slope;
  int k;
  float temp;

  /*  signal power change based VAD */
  /*  consider small speech at the biginning, ensure it is not suppressed */
  instant_slope = 0;
  for (k = 0; k < 5; k++) {    /* subband based */
    temp = amp_inst_band[k] / 4.0F;
    if ((temp - amp_inst_band[5 + k] > 0.0F) || (temp - amp_inst_band[10 + k] > 0.0F)) {
      instant_slope = 1;
    }
  }

  if (instant_slope == 0) {
    temp = *vad_slope_cnt - 1.0F;
    if (0.0F > temp) {
      temp = 0.0F;
    }

    *vad_slope_cnt = temp;
    if (temp == 0.0F) {
      *vad_slope = 0.0F;
    }
  } else {
    *vad_slope = 1.0F;
    if (Fs == 16000.0F) {
      *vad_slope_cnt = 8.0F;
    } else {
      if (Fs == 8000.0F) {
        *vad_slope_cnt = 4.0F;
      }
    }
  }

}


