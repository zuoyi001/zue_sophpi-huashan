
#ifndef PACKFFT_H
#define PACKFFT_H

 /* Include files */
#include "define.h"

/** Compute tables for an FFT */
extern void *speech_fft_init(int size);

/** Free tables for an FFT */
extern void speech_fft_free(void *table);

/** Forward (real to half-complex) transform */
extern void speech_fft(void *table, float *in, float *out);

/** Backward (half-complex to real) transform */
extern void speech_ifft(void *table, float *in, float *out);

#endif


