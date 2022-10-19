#ifndef __CVI_AUDIO_UTILS_H__
#define __CVI_AUDIO_UTILS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#include <stdint.h>
#include <autoconf.h>

#ifdef CONFIG_CVI_SOC_CV182X
#define SAMPLES_FRAMES_LEN (160)
#else
#define SAMPLES_FRAMES_LEN (320)
#endif

struct wav_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t riff_fmt;
    uint32_t fmt_id;
    uint32_t fmt_sz;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_id;
    uint32_t data_sz;
};



//extern int SAMPLE_AUDIO_capture(const char * filename, unsigned int capture_time /*seconds*/);
extern int sample_audio_play_wavefile(unsigned int card,unsigned int device,const char * filename); //interface;
//extern int SAMPLE_AUDIO_APlay_init(const FILE *file,unsigned int bits,struct pcm_config *pPcmConfig);
extern int audio_play_init();
extern void audio_play_close();
extern int audio_alsa_write(char *buffer,unsigned int len);
extern int audio_record_wav_file(const char * recordfile,unsigned int capture_time);

extern int audio_capture_init();
extern int audio_alsa_read(char *buffer,unsigned int len) ;
extern void audio_capture_close();
extern unsigned int get_audio_play_periodbyes();
extern void audio_common_init();
extern void audio_common_deinit();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif //__CVI_AUDIO_UTILS_H__


