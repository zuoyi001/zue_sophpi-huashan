/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   */
/*%                                                                 */
/*%       mmse.c                                                    */
/*%       Author: Sharon Lee                                        */
/*%       History:                                                  */
/*%       Created by Sharon Lee in August, 2019                     */
/*%                                                                 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   */

/* Include files */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "mmse_init.h"
#include "mmse.h"
#include "bessel.h"
#include "power.h"
#include "sum.h"
#include "log.h"
#include "dr_detection.h"
#include "slope_detection.h"
#include "abs.h"
#include "mmse_rtwutil.h"
#include "packfft.h"
#include "define.h"
#include "tmwtypes.h"


float b_noise_mean[321];
float gammak[321];
float b_win[640];
float ksi[321];
float C[321];
float j0_t[321];
float ji_t[321];
float vk[321];
float b_c[3];
float c_sig_tmp[321];
float b_vk[321];
float sig_re;
float nr_fft_out[640];	/* bin box order: [1].re, [2].re,[2].im, ... ,[320].re,[320].im,[321].re */
float nr_ifft_out[640];	/* bin box order: [1].re, [2].re,[2].im, ... ,[320].re,[320].im,[321].re */
float nr_sig[321];
float nr_sig2[321];
creal32_T c_sig[321];
creal32_T nr_fd_half[321];

#ifndef SSP_CALLOC
#define SSP_CALLOC(TYPE, COUNT) ((TYPE *)calloc(COUNT, sizeof(TYPE)))
#endif

#ifndef SSP_SAFE_FREE_BUF
#define SSP_SAFE_FREE_BUF(OBJ) {if (NULL != OBJ) {free(OBJ); OBJ = NULL; } }
#endif

#ifndef SSP_CHECK_NULL_PTR
#define SSP_CHECK_NULL_PTR(ptr)                                                                                            \
	do {                                                                                                           \
		if (!(ptr)) {                                                                                     \
			printf("func:%s,line:%d, NULL pointer\n", __func__, __LINE__);                                 \
			return;                                                                            \
		}                                                                                                      \
	} while (0)
#endif



#ifdef FAST_MATH_CALC
#if 0
//reference from:
https://math.stackexchange.com/questions/1098487/atan2-faster-approximation
https://gist.github.com/volkansalma/2972237
#endif
float fast_atan2(float y, float x) {
    float abs_x = abs(x);
    float abs_y = abs(y);
    float a = fmin (abs_x, abs_y) / fmax (abs_x, abs_y);
    float s = a * a;
    float r = ((-0.0464964749 * s + 0.15931422) * s - 0.327622764) * s * a + a;
    if (abs_y > abs_x) {
        r = 1.57079637 - r;
    }

    if (x < 0) {
        r = 3.14159274 - r;
    }

    if (y < 0) {
        r = -r;
    }
    return r;
}


float fast_atan3(float y, float x)
{
    //http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
    //Volkan SALMA

    const float ONEQTR_PI = M_PI / 4.0;
 const float THRQTR_PI = 3.0 * M_PI / 4.0;
 float r, angle;
 float abs_y = fabs(y) + 1e-10f;      // kludge to prevent 0/0 condition
 if ( x < 0.0f )
 {
  r = (x + abs_y) / (abs_y - x);
  angle = THRQTR_PI;
 }
 else
 {
  r = (x - abs_y) / (x + abs_y);
  angle = ONEQTR_PI;
 }
 angle += (0.1963f * r * r - 0.9817f) * r;
 if ( y < 0.0f )
  return( -angle );     // negate if in quad III or IV
 else
  return( angle );

}
#endif
//extern *fp_test;
//extern frame_cnt;

/* Function Definitions */
void NR(const float x[320], float len, float len1, float nFFT, float *frame, float y_old[160],
          float Xk_prev[321], float noise_mean[321], float noise_mu2[321], float *vad_slope_cnt, float *vad_slope, float amp_inst_band[15],
          float *vad_dr_cnt, float *vad_dr, float noisy_st_trough[5], float noisy_st_peak[5],
          float *vad_mmse, float *vad_mmse_cnt, float amp_inst_sum_hist[3], float Fs, float eta, float ksi_min, float vad_mmse_cnt_para,
          float initial_noise_pow_time, float y[160], boolean_T *speech_vad)
{
#ifdef SSP_STACK_TO_HEAP
  garbage task
  int i;
  float c  = 0.886226925F;
  float qkr = 2.33333333F;
  //float gammak[321];
  //float b_win[640];
  //float b_noise_mean[321];
  //float ksi[321];
  //float C[321];
  //float j0_t[321];
  //float ji_t[321];
  //float vk[321];
  //float c_sig_tmp[321];
  //float b_vk[321];
  //float nr_fft_out[640];	/* bin box order: [1].re, [2].re,[2].im, ... ,[320].re,[320].im,[321].re */
  //float nr_ifft_out[640];	/* bin box order: [1].re, [2].re,[2].im, ... ,[320].re,[320].im,[321].re */
  //float nr_sig[321];
  //float nr_sig2[321];
  float *gammak = SSP_CALLOC(float, 321);
  float *b_win = SSP_CALLOC(float, 640);
  float *b_noise_mean = SSP_CALLOC(float, 321);
  float *ksi = SSP_CALLOC(float, 321);
  float *C = SSP_CALLOC(float, 321);
  float *j0_t = SSP_CALLOC(float, 321);
  float *ji_t = SSP_CALLOC(float, 321);
  float *vk = SSP_CALLOC(float, 321);
  float *c_sig_tmp = SSP_CALLOC(float, 321);
  float *b_vk = SSP_CALLOC(float, 321);
  float *nr_fft_out = SSP_CALLOC(float, 640);
  float *nr_ifft_out = SSP_CALLOC(float, 640);
  float *nr_sig = SSP_CALLOC(float, 321);
  float *nr_sig2 = SSP_CALLOC(float, 321);
  float xtmp_re;
  float xtmp_im;
  float lambda;
  int k, j;
  int b_loop_ub;
  float absar;
  float Mb;
  boolean_T vad;
  float b_c[3];
  //creal32_T c_sig[321];
  creal32_T *c_sig = SSP_CALLOC(creal32_T, 321);
  float sig_re;
  //creal32_T nr_fd_half[321];
  creal32_T *nr_fd_half = SSP_CALLOC(creal32_T, 321);
  float tmp1;

  SSP_CHECK_NULL_PTR(gammak);
  SSP_CHECK_NULL_PTR(b_win);
  SSP_CHECK_NULL_PTR(b_noise_mean);
  SSP_CHECK_NULL_PTR(ksi);
  SSP_CHECK_NULL_PTR(C);
  SSP_CHECK_NULL_PTR(j0_t);
  SSP_CHECK_NULL_PTR(ji_t);
  SSP_CHECK_NULL_PTR(vk);
  SSP_CHECK_NULL_PTR(c_sig_tmp);
  SSP_CHECK_NULL_PTR(b_vk);
  SSP_CHECK_NULL_PTR(nr_fft_out);
  SSP_CHECK_NULL_PTR(nr_ifft_out);
  SSP_CHECK_NULL_PTR(nr_sig);
  SSP_CHECK_NULL_PTR(nr_sig2);
  SSP_CHECK_NULL_PTR(c_sig);
  SSP_CHECK_NULL_PTR(nr_fd_half);

#else
  //REMOVE ARRAY TO global variable incase the stack size explode!
  int i;
  float c  = 0.886226925F;
  float qkr = 2.33333333F;
  float xtmp_re;
  float xtmp_im;
  float lambda;
  int k, j;
  int b_loop_ub;
  float absar;
  float Mb;
  float tmp1;
  boolean_T vad;
#endif

  /*  Apply analysis window */
  for (i = 0; i < (int)(nFFT/2); i++) {
    b_win[i] = ana_win[i] * x[i];
    b_win[(int)(nFFT/2) + i] = 0.0F;
  }

  /*  Take fourier transform of one frame */
  speech_fft(nr_fft_table, b_win, nr_fft_out);    /* faster than matlab fft() */
  nr_fd_half[0].re = nr_fft_out[0] * nFFT;    /* multiply nFFT to get values as in matlab */
  nr_fd_half[0].im = (float)3.3495873e-008;    /* assign a small value to avoid dividing by zero */
  for (k = 1, j = 1; j < (int)(nFFT/2); k+=2, j++) {
    nr_fd_half[j].re = nr_fft_out[k] * nFFT;
    nr_fd_half[j].im = nr_fft_out[k+1] * nFFT;
  }
  nr_fd_half[(int)(nFFT/2)].re = nr_fft_out[(int)(nFFT-1)] * nFFT;
  nr_fd_half[(int)(nFFT/2)].im = (float)3.3495873e-008;    /* assign a small value to avoid dividing by zero */

  a_abs(nr_fd_half, nr_sig, (int)(nFFT/2+1));    /* amplitude */
  c_power(nr_sig, nr_sig2, (int)(nFFT/2+1));    /* power */

  /*  solve the issue of silence segment appeared in the beginning */
 //tmp1 = c_sum(nr_sig, (int)(nFFT/2+1));
 tmp1 = c_sum(nr_sig, (int)(nFFT/2));
  if (tmp1 / (nFFT / 2.0F) < 0.0003F) {    /*  small signal segment */
    *frame = 1.0F;    /*  reset initial noise power estimate */
    *vad_slope_cnt = 0.0F;
    *vad_slope = 0.0F;
    *vad_dr_cnt = 0.0F;
    *vad_dr = 0.0F;
    *vad_mmse = 0.0F;
    *vad_mmse_cnt = 0.0F;
    xtmp_im = nFFT / 2.0F;
    for (i = 0; i < (int)(xtmp_im + 1.0F); i++) {
      noise_mean[i] = 0.0F;
      noise_mu2[i] = 0.0F;
      Xk_prev[i] = 0.0F;
    }
    for (i = 0; i < 5; i++) {
      noisy_st_trough[i] = 0.0F;
      noisy_st_peak[i] = 0.0F;
      for (b_loop_ub = 0; b_loop_ub < 3; b_loop_ub++) {
        amp_inst_band[i + 5 * b_loop_ub] = 0.0F;
      }
    }
    for (i = 0; i < 3; i++) {
      amp_inst_sum_hist[i] = 0.0F;
    }
  }

  if (*frame <= initial_noise_pow_time) {    /*  amplitude of accumulating the first handful of frames */
    for (i = 0; i < 321; i++) {
      noise_mean[i] += nr_sig[i];
      b_noise_mean[i] = noise_mean[i] / (*frame);
    }
    c_power(b_noise_mean, noise_mu2, 321);	    /*  average power for one frame */
    /*for (i = 0; i < 321; i++)
	fprintf(fp_test, "%f \n", noise_mu2[i]); */
  }

  for (i = 0; i < (int)(nFFT/2+1); i++) {
    if (noise_mu2[i] == 0.0F) {    /*  zero denominator */
      if (nr_sig2[i] == 0.0F) {
        gammak[i] = 0.0F;
      } else {
        gammak[i] = 40.0F;
      }
    } else {
      absar = nr_sig2[i] / noise_mu2[i];
      if (!(absar < 40.0F)) {
        absar = 40.0F;
      }
      gammak[i] = absar;    /* posteriori SNR */
    }
  }

  if (*frame == 1.0F) {    /*  priori SNR for the first frame */
    for (k = 0; k < 321; k++) {
      xtmp_im = gammak[k] - 1.0F;
      xtmp_im = MAX(xtmp_im, 0.0F);
      ksi[k] = aa + (1.0F - aa) * xtmp_im;
    }
  } else {    /*  decision-directed estimate for priori SNR */
    for (k = 0; k < 321; k++) {
      xtmp_im = gammak[k] - 1.0F;
      xtmp_im = MAX(xtmp_im, 0.0F);
      if (0.0F == noise_mu2[k])
        noise_mu2[k] = (float)3.3495873e-008;    /* assign a small value to avoid dividing by zero */
      ksi[k] = aa * Xk_prev[k] / noise_mu2[k] + (1.0F - aa) * xtmp_im;
      ksi[k] = MAX(ksi[k], ksi_min);    /*  limit ksi greater than -25dB in power */
    }
  }

  /*  slope detection */
  for (i = 0; i < 5; i++) {
    amp_inst_band[10 + i] = amp_inst_band[5 + i];
    amp_inst_band[5 + i] = amp_inst_band[i];
  }

  amp_inst_band[0] = c_sum(*(float (*)[20])&nr_sig[4], 20);               /* 20 */
  amp_inst_band[1] = c_sum(*(float (*)[20])&nr_sig[24], 20);             /* 20 */
  amp_inst_band[2] = c_sum(*(float (*)[40])&nr_sig[44], 40);             /* 40 */
  amp_inst_band[3] = c_sum(*(float (*)[80])&nr_sig[84], 80);             /* 80 */
  amp_inst_band[4] = c_sum(*(float (*)[156])&nr_sig[164], 156);      /* 156 */
/* for (i = 0; i < 5; i++)
 	fprintf(fp_test, "%f \n", amp_inst_band[i]);	*/

  slope_detection(amp_inst_band, vad_slope_cnt, vad_slope, Fs);

  /*  dynamic range detection */
  dr_detection(amp_inst_band, noisy_st_trough, noisy_st_peak, vad_dr_cnt, vad_dr, Fs);

  if ((*vad_slope != 0.0F) || (*vad_dr != 0.0F)) {
    vad = true;
  } else {
    vad = false;
  }

  /* log_sigma_k calculation for mmse vad  */
  for (i = 0; i < 321; i++) {
    C[i] = 1.0F + ksi[i];
  }
  b_log(C);	/* C = log(1+ksi) */

  for (k = 0; k < 321; k++) {
    if (0.0F == (1.0F + ksi[k]))
      C[k] = gammak[k] * ksi[k] / ((float)3.3495873e-008) - C[k];    /* assign a small value to avoid dividing by zero */
    else
      C[k] = gammak[k] * ksi[k] / (1.0F + ksi[k]) - C[k];    /* log_sigma_k */
  }

  b_loop_ub = (int)(nFFT / 2.0F);
  xtmp_im = c_sum(C, b_loop_ub);    /* here only consider log_sigma_k(1:nFFT/2) */
  Mb = nFFT / 2.0F;
  xtmp_im = xtmp_im / Mb;

  if (xtmp_im <= eta) {
    *vad_mmse_cnt = MAX(0.0F, *vad_mmse_cnt - 1.0F);
    if (*vad_mmse_cnt == 0.0F) {
      *vad_mmse = 0.0F;
    }
  } else {
    *vad_mmse = 1.0F;
    *vad_mmse_cnt = vad_mmse_cnt_para;
  }

  /*  noise power estimate correction when it is too high */
  /*  small speech in quiet after big statioanry noise, correct wrong */
  amp_inst_sum_hist[2] = amp_inst_sum_hist[1];
  amp_inst_sum_hist[1] = amp_inst_sum_hist[0];
  amp_inst_sum_hist[0] = c_sum(*(float (*)[5])&amp_inst_band[0], 5);
  //fprintf(fp_test, "%f \n", amp_inst_sum_hist[0]);

  c_power(amp_inst_sum_hist, b_c, 3);    /*  power based */
  /*for (k = 0; k < 3; k++)
  	fprintf(fp_test, "%f \n", b_c[k]);	*/

  for (k = 0; k < 3; k++) {
    b_c[k] *= 0.3333F;    /* average power */
  }

  xtmp_im = b_c[0];
  for (k = 0; k < 2; k++) {
    xtmp_im += b_c[k + 1];
  }

  absar = noise_mu2[4];
  for (k = 0; k < 315; k++) {
    absar += noise_mu2[k + 5];
  }

  if (absar / 8.0F > xtmp_im) {	/* +/- 18dB for statioanry noise variance for the whole signals, assumption */
    for (k = 0; k < 321; k++) {
      noise_mu2[k] = 0.8F * noise_mu2[k] + (1.0F - 0.8F) * nr_sig2[k];    /*  here faster to update noise power */
    }
  }

  /*  %% keep better speech quality, not too easy to update noise power %% */
  *speech_vad = true;
  if ((*vad_mmse == 0.0F) || (!vad)) {
    for (k = 0; k < 321; k++) {
      noise_mu2[k] = mu * noise_mu2[k] + (1.0F - mu) * nr_sig2[k];    /* update noise power */
    }
    *speech_vad = false;
  }

  /*  === end of vad ===  */


  /* v(k) calculation */
  for (k = 0; k < 321; k++) {
    if (0.0F == (1.0F + ksi[k]))
      xtmp_im = ksi[k] * gammak[k] / ((float)3.3495873e-008);    /* assign a small value to avoid dividing by zero */
    else
      xtmp_im = ksi[k] * gammak[k] / (1.0F + ksi[k]);    /* v(k) */
    vk[k] = xtmp_im;
    b_vk[k] = xtmp_im / 2.0F;
    C[k] = (float)exp(-0.5F * xtmp_im);    /* C = exp(-0.5*vk) */
  }

  d_power(vk, c_sig_tmp);    /* vk.^0.5 */
  for (i = 0; i < 321; i++) {
      //if ((c_sig_tmp[i] == 0.0F) && (gammak[i] == 0.0F)) {     /* bug fixed to avoid zero denominator */
      if ((gammak[i] == 0.0F)) {
        c_sig_tmp[i] = 0.0F;
      } else {
        c_sig_tmp[i] = (c * c_sig_tmp[i] * C[i]) / gammak[i];
      }
  }

  bessel_function(0.0F, b_vk, j0_t);    /* zero order modified Bessel function */
  bessel_function(1.0F, b_vk, ji_t);    /* first order modified Bessel function */

  /* G(ksi, gammak) estimation */
  for (k = 0; k < 321; k++) {
    xtmp_re = j0_t[k] * (1.0F + vk[k]);
    sig_re = ji_t[k] * vk[k];
    if (0.0F == (1.0F + ksi[k]))
      lambda = qkr * (float)exp(vk[k]) / ((float)3.3495873e-008);    /* assign a small value to avoid dividing by zero */
    else
      lambda = qkr * (float)exp(vk[k]) / (1.0F + ksi[k]);
    xtmp_re = nr_sig[k] * (c_sig_tmp[k] * (xtmp_re + sig_re));
    if (0.0F == (1.0F + lambda))
      c_sig_tmp[k] = lambda / ((float)3.3495873e-008) * xtmp_re;    /* assign a small value to avoid dividing by zero */
    else
      c_sig_tmp[k] = lambda / (1.0F + lambda) * xtmp_re;    /* estimated amplitude of clean speech */

    //Xk_prev[k].re = c_sig[k].re * c_sig[k].re - c_sig[k].im * c_sig[k].im;    /*  use for updating ksi in next frame */
    //Xk_prev[k].im = c_sig[k].re * c_sig[k].im + c_sig[k].im * c_sig[k].re;    /*  use for updating ksi in next frame */
    Xk_prev[k] = c_sig_tmp[k] * c_sig_tmp[k];    /*  use for updating ksi in next frame */
  }

  for (k = 0; k < (int)(nFFT/2+1); k++) {
#ifdef FAST_MATH_CALC
    nr_sig[k] = (float)fast_atan3(nr_fd_half[k].im, nr_fd_half[k].re);    /* theta */
#else
    nr_sig[k] = (float)atan2(nr_fd_half[k].im, nr_fd_half[k].re);    /* theta */
#endif
  }

  for (k = 0; k < (int)(nFFT/2+1); k++) {
    if (nr_sig[k] == 0.0F) {
      xtmp_re = 1.0F;
      xtmp_im = 0.0F;
    } else {
      xtmp_re = (float)cos(nr_sig[k]);    /* Euler's formula */
      xtmp_im = (float)sin(nr_sig[k]);    /* Euler's formula */
    }
    nr_fd_half[k].re = xtmp_re;         /* phase part */
    nr_fd_half[k].im = xtmp_im;       /* phase part */
  }

  for (k = 0; k < 321; k++) {
    //sig_re = c_sig[k].re;
    //c_sig[k].re = c_sig[k].re * nr_fd_half[k].re - c_sig[k].im * nr_fd_half[k].im;    /* estimated clean speech signal */
    //c_sig[k].im = sig_re * nr_fd_half[k].im + c_sig[k].im * nr_fd_half[k].re;    /* estimated clean speech signal */
    c_sig[k].re = c_sig_tmp[k] * nr_fd_half[k].re;    /* estimated clean speech signal */
    c_sig[k].im = c_sig_tmp[k] * nr_fd_half[k].im;    /* estimated clean speech signal */
  }

  nr_fft_out[0] = c_sig[0].re;
  for (k = 1, j = 1; j < (int)(nFFT/2); k+=2, j++) {
    nr_fft_out[k] = c_sig[j].re;
    nr_fft_out[k+1] = c_sig[j].im;
  }
  nr_fft_out[(int)(nFFT-1)] = c_sig[(int)(nFFT/2)].re;

  /* restore nFFT points before doing inverse FFT, do conjugate symmetric for the lower half points inside speech_ifft() */
  speech_ifft(nr_fft_table, nr_fft_out, nr_ifft_out);    /* faster than matlab ifft(), in/out not use the same buffer */
  for (k = 0; k < (int)(len); k++) {    /* ignore output samples from (len+1) to (2*len) */
    nr_ifft_out[k] /= nFFT;    /* divide by nFFT to get values as in matlab */
  }

  for (k = 0; k < (int)(len1); k++) {
    y[k] = y_old[k] + (nr_ifft_out[k] * syn_win[k]);    /* apply synthesis window and 50% OLA for output */
    y_old[k] = nr_ifft_out[(int)len1 + k] * syn_win[(int)len1 + k];    /* apply synthesis window and update output OLA buffer */
  }

#ifdef SSP_STACK_TO_HEAP
  SSP_SAFE_FREE_BUF(gammak);
  SSP_SAFE_FREE_BUF(b_win);
  SSP_SAFE_FREE_BUF(b_noise_mean);
  SSP_SAFE_FREE_BUF(ksi);
  SSP_SAFE_FREE_BUF(C);
  SSP_SAFE_FREE_BUF(j0_t);
  SSP_SAFE_FREE_BUF(ji_t);
  SSP_SAFE_FREE_BUF(vk);
  SSP_SAFE_FREE_BUF(c_sig_tmp);
  SSP_SAFE_FREE_BUF(b_vk);
  SSP_SAFE_FREE_BUF(nr_fft_out);
  SSP_SAFE_FREE_BUF(nr_ifft_out);
  SSP_SAFE_FREE_BUF(nr_sig);
  SSP_SAFE_FREE_BUF(nr_sig2);
  SSP_SAFE_FREE_BUF(c_sig);
  SSP_SAFE_FREE_BUF(nr_fd_half);
#endif


}


