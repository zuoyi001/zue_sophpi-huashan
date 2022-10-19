#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <locale.h>
#include <assert.h>
#include <termios.h>
#include <signal.h>
#include <poll.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <endian.h>
#include "alsacommon.h"
#include "app_utils.h"


void SAMPLE_AUDIO_Show_available_sample_formats(snd_pcm_t *handle,snd_pcm_hw_params_t* params)
{
	snd_pcm_format_t format;

	fprintf(stderr, "Available formats:\n");
	for (format = 0; format <= SND_PCM_FORMAT_LAST; format++) {
		if (snd_pcm_hw_params_test_format(handle, params, format) == 0)
			fprintf(stderr, "- %s\n", snd_pcm_format_name(format));
	}
}



unsigned int SAMPLE_AUDIO_pcm_format_to_bits(snd_pcm_format_t format)
{
    switch (format) {
    case SND_PCM_FORMAT_S32_LE:
    case SND_PCM_FORMAT_S32_BE:
    case SND_PCM_FORMAT_S24_LE:
    case SND_PCM_FORMAT_S24_BE:
        return 32;
    case SND_PCM_FORMAT_S24_3LE:
    case SND_PCM_FORMAT_S24_3BE:
        return 24;
    default:
    case SND_PCM_FORMAT_S16_LE:
    case SND_PCM_FORMAT_S16_BE:
        return 16;
    case SND_PCM_FORMAT_S8:
        return 8;
    };
}



unsigned int SAMPLE_AUDIO_pcm_frames_to_bytes(const struct pcm_alsa_config config, unsigned int frames)
{
    return frames * config.channels *
        (SAMPLE_AUDIO_pcm_format_to_bits(config.format) >> 3);
}

snd_pcm_uframes_t SAMPLE_AUDIO_pcm_bytes_to_frames(const struct pcm_alsa_config config, unsigned int bytes)
{
    return bytes / (config.channels *
        (SAMPLE_AUDIO_pcm_format_to_bits(config.format) >> 3));
}


int SAMPLE_AUDIO_set_pcm_snd_params(snd_pcm_t *handle,struct pcm_alsa_config *config)
{
    snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *swparams;
	snd_pcm_uframes_t buffer_size = config->period_size * config->period_count;
	int err = 0;
	unsigned int rate = config->rate;
	snd_pcm_uframes_t start_threshold = config->start_threshold;
    snd_pcm_uframes_t stop_threshold = config->stop_threshold;
    snd_pcm_uframes_t period_size = config->period_size;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&swparams);
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0) {
		DBG_PRINTF(CVI_INFO, "Broken configuration for this PCM: no configurations available");
		return -1;
	}

    err = snd_pcm_hw_params_set_access(handle, params,
						   SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
		DBG_PRINTF(CVI_INFO, "snd_pcm_hw_params_set_access error.\n");
		return err;
	}

    err = snd_pcm_hw_params_set_format(handle, params, config->format);
	if (err < 0) {
		DBG_PRINTF(CVI_INFO, "snd_pcm_hw_params_set_format %d:%s error.\n",config->format,snd_pcm_format_name(config->format));
        SAMPLE_AUDIO_Show_available_sample_formats(handle,params);
		return err;
	}
	err = snd_pcm_hw_params_set_channels(handle, params, config->channels);
	if (err < 0) {
		DBG_PRINTF(CVI_INFO, "snd_pcm_hw_params_set_channels error.\n");
		return err;
	}


    err = snd_pcm_hw_params_set_rate_near(handle,
			params, &config->rate,
			0);
	if (err < 0) {
		DBG_PRINTF(CVI_INFO,  "Set parameter to device error: sample rate: %d (%d)\n"
				,rate, err);
		return err;
	}

    err = snd_pcm_hw_params_set_buffer_size_near(handle,
				params, &buffer_size);
	if (err < 0) {
	    DBG_PRINTF(CVI_INFO,  "Set parameter to device error: buffer_size: %ld (%d)\n"
				,buffer_size,err);
		return err;
	}

	err = snd_pcm_hw_params_set_period_size_near(handle,
			params, &period_size, 0);
	if (err < 0) {
		DBG_PRINTF(CVI_INFO,  "Set parameter to device error: period_size: %ld (%d)\n"
				,period_size, err);
		return err;
	}

    /* Write the parameters to the driver */
	if (snd_pcm_hw_params(handle, params) < 0) {
		DBG_PRINTF(CVI_INFO,  "snd_pcm_hw_params error \n");
		return -EINVAL;
	}

    err = snd_pcm_sw_params_current(handle, swparams);
	if (err < 0) {
		DBG_PRINTF(CVI_INFO, "Unable to get current sw params.");
		return err;
	}

    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, start_threshold);
    if (err < 0) {
		DBG_PRINTF(CVI_INFO,  "snd_pcm_sw_params_set_start_threshold error: start_threshold: %ld  \n",start_threshold);
		return err;
	}
    err = snd_pcm_sw_params_set_stop_threshold(handle, swparams, stop_threshold);
    if (err < 0) {
		DBG_PRINTF(CVI_INFO,  "snd_pcm_sw_params_set_start_threshold error: stop_threshold: %ld  \n",stop_threshold);
		return err;
	}
    err = snd_pcm_sw_params(handle, swparams);
    if (err < 0)  {
		DBG_PRINTF(CVI_INFO,  "snd_pcm_sw_params error.\n");
		return err;
	}

	return 0;
}



snd_pcm_t *SAMPLE_AUDIO_pcm_open(unsigned int card, unsigned int device,
                     unsigned int flags, struct pcm_alsa_config *config)
{
    char pcm_name[20]={0};
    int err;
    snd_pcm_t *pcm_handle = NULL;
    
    snprintf(pcm_name,sizeof(pcm_name),"hw:%d,%d",card,device);
    //DBG_PRINTF(CVI_INFO, "[%s,%d] pcm_name:%s\n",__FUNCTION__,__LINE__,pcm_name);
    
    err = snd_pcm_open(&pcm_handle, pcm_name, (snd_pcm_stream_t)flags, 0);
	if (err < 0) {
		DBG_PRINTF(CVI_INFO, "audio open error\n");
		return NULL;
	}

    err = SAMPLE_AUDIO_set_pcm_snd_params(pcm_handle,config);
    if (err < 0) {
        snd_pcm_close(pcm_handle);
		DBG_PRINTF(CVI_INFO, "set_pcm_snd_params error,err:%d.\n",err);
		return NULL;
	}

    return pcm_handle;

}






