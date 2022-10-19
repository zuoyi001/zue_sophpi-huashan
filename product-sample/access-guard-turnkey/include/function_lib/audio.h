#ifndef __LIBSAMPLE_AUDIO_H__
#define __LIBSAMPLE_AUDIO_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

extern int SAMPLE_AUDIO_capture(const char * filename, unsigned int capture_time /*seconds*/);
extern int SAMPLE_AUDIO_Play(unsigned int card,unsigned int device,const char * filename);
extern int SAMPLE_AUDIO_AO_SetVolume(unsigned int AoDevId, int s32VolumeDb);
extern int SAMPLE_AUDIO_AO_GetVolume(unsigned int AoDevId, int *ps32VolumeDb);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif