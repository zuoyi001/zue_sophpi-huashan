
#ifndef SLOPE_DETECTION_H
#define SLOPE_DETECTION_H

/* Include files */
#include <stddef.h>
#ifndef PLATFORM_RTOS
#include <stdlib.h>
#endif
#include "tmwtypes.h"

/* Function Declarations */
extern void slope_detection(const float amp_inst_band[15], float *vad_slope_cnt, float *vad_slope, float Fs);

#endif


