#ifndef __LIB_CVI_AUDIO_H__
#define __LIB_CVI_AUDIO_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/pcm.h>

#define PCM_ALSA_OUT 0
#define PCM_ALSA_IN  1
#define FORMAT_PCM 1

struct pcm_alsa_config {
    unsigned int channels;
    unsigned int rate;
    snd_pcm_uframes_t period_size;
    unsigned int period_count;
    snd_pcm_format_t format;
    snd_pcm_uframes_t start_threshold;
    snd_pcm_uframes_t stop_threshold;
    snd_pcm_uframes_t silence_threshold;
};

extern void SAMPLE_AUDIO_Show_available_sample_formats(snd_pcm_t *handle,snd_pcm_hw_params_t* params);
extern unsigned int SAMPLE_AUDIO_pcm_format_to_bits(snd_pcm_format_t format);
extern unsigned int SAMPLE_AUDIO_pcm_frames_to_bytes(const struct pcm_alsa_config config, unsigned int frames);
extern snd_pcm_uframes_t SAMPLE_AUDIO_pcm_bytes_to_frames(const struct pcm_alsa_config config, unsigned int bytes);
extern int SAMPLE_AUDIO_set_pcm_snd_params(snd_pcm_t *handle,struct pcm_alsa_config *config);
extern snd_pcm_t *SAMPLE_AUDIO_pcm_open(unsigned int card, unsigned int device,
                     unsigned int flags, struct pcm_alsa_config *config);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif //__LIB_CVI_AUDIO_H__