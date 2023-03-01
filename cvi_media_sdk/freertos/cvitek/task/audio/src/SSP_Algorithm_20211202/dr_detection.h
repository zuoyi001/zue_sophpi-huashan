
#ifndef DR_DETECTION_H
#define DR_DETECTION_H

/* Include files */
#include <stddef.h>
#include <stdlib.h>
#include "tmwtypes.h"

/* Function Declarations */
extern void dr_detection(const float amp_inst_band[15], float noisy_st_trough[5],
  float noisy_st_peak[5], float *vad_dr_cnt, float *vad_dr, float Fs);

#endif


