#ifndef __CVI_AUDIO_MAIN_H__
#define __CVI_AUDIO_MAIN_H__

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif


#define AUDIO_MSG_PLAY_TONE (100)
#define AUDIO_MSG_STOP_AUDIO (101)
#define AUDIO_MSG_LOOP_AUDIO (102)
#define AUDIO_MSG_LOCAL_LOOPBACK (103)
#define AUDIO_MSG_RECORD_MIC (104)
#define AUDIO_MSG_PLAY_MICRECORD (105)




extern int is_audio_working();
extern void audio_stop_working();
extern int audio_local_loopback();
extern int audio_play_loop_audio();
extern int audio_play_single_tone();
extern int audio_init(void);
extern void audio_exit(void);
extern int audio_record_micdata_to_wav(int bSwitch);
extern int audio_play_wav_micdata();
extern void audio_play_filewav(const char * filename,int bNeedLoop);


#endif
