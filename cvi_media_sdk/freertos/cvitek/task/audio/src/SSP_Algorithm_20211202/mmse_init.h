
#ifndef MMSE_INIT_H
#define MMSE_INIT_H

/* Include files */
#include <stddef.h>
#ifndef PLATFORM_RTOS
#include <stdlib.h>
#endif
#include "tmwtypes.h"
#include "struct.h"

/* Function Declarations */
extern void NR_init(float *frame, short x_old[160], float y_old[160], float Xk_prev[321], float noise_mean[321], float noise_mu2[321],
               float *vad_slope_cnt, float *vad_slope, float amp_inst_band[15], float *vad_dr_cnt, float *vad_dr, float noisy_st_trough[5],
               float noisy_st_peak[5], float *vad_mmse, float *vad_mmse_cnt, float amp_inst_sum_hist[3], float *eta, float *ksi_min, float fs, float *vad_mmse_cnt_para,
               float *initial_noise_pow_time, int *nenr_silence_cnt);
extern void NR_para(ssp_para_struct *para, float fs, int *nenr_silence_time);

/* NR Parameters */


extern float aa;
extern float mu;
/* window coefficient */
extern float ana_win[320]; /* analysis window before performing fft */
extern float syn_win[320];/* synthesis window after performing ifft */
extern void* nr_fft_table;

#endif


