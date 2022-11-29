
#ifndef MMSE_H
#define MMSE_H

/* Include files */
#include <stddef.h>
#ifndef PLATFORM_RTOS
#include <stdlib.h>
#endif
#include "tmwtypes.h"

/* Function Declarations */
extern void NR(const float x[320], float len, float len1, float nFFT, float *frame, float y_old[160],
		float Xk_prev[321], float noise_mean[321], float noise_mu2[321], float *vad_slope_cnt, float *vad_slope, float amp_inst_band[15],
		float *vad_dr_cnt, float *vad_dr, float noisy_st_trough[5], float noisy_st_peak[5],
		float *vad_mmse, float *vad_mmse_cnt, float amp_inst_sum_hist[3], float Fs, float eta, float ksi_min, float vad_mmse_cnt_para,
		float initial_noise_pow_time, float y[160], boolean_T *speech_vad);

#endif


