#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <assert.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <poll.h>

#include "acodec.h"
#include "cvi_audio_utils.h"
#include "cyclebuffer.h"
#include "cvi_audio_main.h"
// #include "tcli.h"
#include "msgq.h"
#include "alsacommon.h"
#include "app_utils.h"


//#define UNUSED(x) //x



#define SAMPLES_LEN (SAMPLES_FRAMES_LEN*2*2)   //2ch 16bit 160 samples 

#define AUDIO_FILE_TONE "/mnt/data/ding.wav"
#define AUDIO_FILE_LOOP_MUSIC "/mnt/data/16K_2ch.wav"
#define AUDIO_MIC_DATA_WAV "/mnt/data/mic_data.wav"




#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164


struct riff_wave_header {
    unsigned int riff_id;
    unsigned int riff_sz;
    unsigned int wave_id;
};

struct chunk_header {
    unsigned int id;
    unsigned int sz;
};

struct chunk_fmt {
    unsigned short audio_format;
    unsigned short num_channels;
    unsigned int sample_rate;
    unsigned int byte_rate;
    unsigned short block_align;
    unsigned short bits_per_sample;
};


static FILE *MicFile;

//static int i32Loopback = false;
static int bLoopBack = false;
static int bAudioPlayingFile = false;
static int bRecordMicData = false;
static int bPlayMicData = false;
//static int playcount = 0;
static int bCloseAudio = 0; 
int bAudioClosed = 0;
//static int i32AudioCount = 0;
unsigned int total_frames_read = 0;
pthread_mutex_t micwav_lock;


#ifdef CLI_CMD_DEBUG
extern void audio_cmd_register();
#endif
static msgq_t *pst_audio_msgq= NULL;
pthread_t msg_loopid = 0;

typedef struct THREAD_FILE_PLAY_INFO_T {
	int bNeedLoop;
	const char *filename;
}THREAD_FILE_PLAY_INFO;


int b_enable_dump = 0;

int b_enable_dumpcap = 0;


void dump_audiodata(char *filename,char *buf, unsigned int len)
{
	FILE *fp;
	if(filename == NULL)
	{
		return;
	}

	fp = fopen(filename,"ab+");
	fwrite(buf,1,len,fp);
	fclose(fp);
	
}


void audio_set_MicVolume(int left, int right)
{
	int fdAcodec_adc = -1;
	ACODEC_VOL_CTRL volL_ctrl;
	ACODEC_VOL_CTRL volR_ctrl;

	fdAcodec_adc = open("/dev/cv1835adc", O_RDWR);
	if(fdAcodec_adc < 0) {
		DBG_PRINTF(CVI_ERROR,"open /dev/cv1835adc failed.\n");
		return;
	}
	volL_ctrl.vol_ctrl_mute = 0;
	volL_ctrl.vol_ctrl = left> 7?7:(left<0)?0:left;
	if(volL_ctrl.vol_ctrl == 0) {
		volL_ctrl.vol_ctrl_mute = 0x1;
	}

	volR_ctrl.vol_ctrl_mute = 0;
	volR_ctrl.vol_ctrl = right> 7?7:(right<0)?0:right;
	if(volR_ctrl.vol_ctrl == 0) {
		volR_ctrl.vol_ctrl_mute = 0x1;
	}

	ioctl(fdAcodec_adc, ACODEC_SET_ADCL_VOL, &volL_ctrl);
	ioctl(fdAcodec_adc, ACODEC_SET_ADCR_VOL, &volR_ctrl);

	if (fdAcodec_adc > 0)
		close(fdAcodec_adc);
	return;
}



void * audio_thread_playback(void *arg)
{
	char *pBuffer;
	char *pTmp;
	char *pZero;
	int underrun_count = 0;
	int bStandby = 1;
	int ret = 0;
	char szThreadName[20]="playback_t";
	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);
	pBuffer = malloc(SAMPLES_LEN);//2ch 16bit 160 samples 
	pZero = malloc(SAMPLES_LEN);//2ch 16bit 160 samples 
	memset(pZero,0,SAMPLES_LEN);
	pTmp = pZero;
	audio_play_init();

	while(1)
	{
		if(bStandby)
		{
			ret = CycleBufferCBMaxLen();
			if(ret > SAMPLES_LEN*2) {
				bStandby = 0;
			}
		}

		if(!bStandby) {
			ret = CycleBufferReadCB(pBuffer,SAMPLES_LEN);
			if(ret == 0) {
				underrun_count++;
				if(underrun_count > 2) {
					//DBG_PRINTF(CVI_INFO, "underrun.\n",underrun_count);
					pTmp = pZero;
					if(underrun_count >=20) {
						bStandby = 1;
					}
				}else {
					usleep(1000*(SAMPLES_LEN/(4*8))/2);
					continue;
				}

			}
			else 
			{
				pTmp = pBuffer;
				underrun_count = 0;
			}
		}
		
		if(b_enable_dump) {
			dump_audiodata("/mnt/data/dumpplay.pcm",pTmp,SAMPLES_LEN);
		}
		audio_alsa_write(pTmp,SAMPLES_LEN);
	}
	audio_play_close();
	free(pBuffer);
	free(pZero);

	return NULL;
}


void * audio_thread_capture(void *arg)
{
	char *pBuffer;
	int ret = 0;
	//int timeout = 0;
	void *gpstCircleBuffer;
	char szThreadName[20]="capture_t";
	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);
	audio_capture_init();

	CycleBufferInit(&gpstCircleBuffer,SAMPLES_LEN*8,2);//2ch 16bit 160 samples 
	pBuffer = malloc(SAMPLES_LEN);//2ch 16bit 160 samples 

	while(1)
	{
		ret = audio_alsa_read(pBuffer,SAMPLES_LEN);
		if(ret != (SAMPLES_LEN/4))
		{
			DBG_PRINTF(CVI_INFO, "alsa read error,ret=%d.\n",ret);
			continue;
		}
		
		if(bLoopBack) {
			CycleBufferWrite(gpstCircleBuffer,pBuffer,SAMPLES_LEN);
		}

		
		if(b_enable_dumpcap) {
			dump_audiodata("/mnt/data/dumpcap.pcm",pBuffer,SAMPLES_LEN);
		}

		pthread_mutex_lock(&micwav_lock);
		if(bRecordMicData && MicFile) {
			fwrite(pBuffer,1,SAMPLES_LEN,MicFile);
		}
		pthread_mutex_unlock(&micwav_lock);
		
	}
	
	audio_capture_close();
	free(pBuffer);

	CycleBufferDestory(gpstCircleBuffer);
	return NULL;
}


int audio_msg_stop_audio(void)
{

	DBG_PRINTF(CVI_INFO, "audio_stop_working in.\n");
	
	if(bAudioPlayingFile) {
		bAudioPlayingFile = false;
	}
	
	if(bLoopBack) {
		bLoopBack = false;
	}

	bPlayMicData = false;

	DBG_PRINTF(CVI_INFO, "audio_stop_working out.\n");

	return 0;
}





void audio_loopback_start(void)
{
	bLoopBack = true;
	
	return;
}


static int audio_play_wav_file(const char * filename,int bNeedLoop)
{
    FILE *file;
    int ret = 0;
	void *pstCircleBuffer;
    struct riff_wave_header riff_wave_header;
    struct chunk_header chunk_header;
    struct chunk_fmt chunk_fmt;
    int more_chunks = 1;
    int size = 0;
    int num_framebytes = 0;
    char *buffer = NULL;
	long start_pos = 0;

	
	bAudioPlayingFile = true;
		
	
    file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        return -1;
    }

    fread(&riff_wave_header, sizeof(riff_wave_header), 1, file);
    if ((riff_wave_header.riff_id != ID_RIFF) ||
        (riff_wave_header.wave_id != ID_WAVE)) {
        fprintf(stderr, "Error: '%s' is not a riff/wave file\n", filename);
        fclose(file);
        return -2;
    }

	CycleBufferInit(&pstCircleBuffer,SAMPLES_LEN*3,2);

    do {
        fread(&chunk_header, sizeof(chunk_header), 1, file);

        switch (chunk_header.id) {
        case ID_FMT:
            fread(&chunk_fmt, sizeof(chunk_fmt), 1, file);
            /* If the format header is larger, skip the rest */
            if (chunk_header.sz > sizeof(chunk_fmt))
                fseek(file, chunk_header.sz - sizeof(chunk_fmt), SEEK_CUR);
            break;
        case ID_DATA:
            /* Stop looking for chunks */
            more_chunks = 0;
            chunk_header.sz = le32toh(chunk_header.sz);
            break;
        default:
            /* Unknown chunk, skip bytes */
            fseek(file, chunk_header.sz, SEEK_CUR);
        }
    } while (more_chunks);

    //size = get_audio_play_periodbyes();
    size = SAMPLES_LEN;
    #if 1
    buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "unable to allocate %d bytes\n", size);
        return -3;
    }
    #endif
	start_pos = ftell(file);

    while (bAudioPlayingFile) {
        num_framebytes = fread(buffer, 1, size, file);
        if (num_framebytes > 0) {
			//audio_alsa_write(buffer,num_read);
			do {
				num_framebytes = CycleBufferWrite(pstCircleBuffer,buffer,size);
				if(num_framebytes != size) {
					usleep(1000*num_framebytes/(4*16));//16k 2ch
					continue;
				}
			}while((num_framebytes != size) && bAudioPlayingFile);
        }else {
			if(bNeedLoop)
			{
				fseek(file,start_pos,SEEK_SET);
				continue;
			}else {
				break;
			}
		}

    }

    if(file) {
		fclose(file);
	}
	if(!strncmp(filename,AUDIO_MIC_DATA_WAV,strlen(AUDIO_MIC_DATA_WAV))) {
		bPlayMicData = false;
	}
    CycleBufferDestory(pstCircleBuffer);  
	//audio_play_close();
	free(buffer);
    return ret;
}


static void *audio_play_audiofile_thread(void *arg)
{
	THREAD_FILE_PLAY_INFO *pstThreadInfo = (THREAD_FILE_PLAY_INFO *)arg;
	if(pstThreadInfo && pstThreadInfo->filename) {		
		audio_play_wav_file(pstThreadInfo->filename,pstThreadInfo->bNeedLoop);
		free(pstThreadInfo);
	}

	return NULL;
}

void audio_play_filewav(const char * filename,int bNeedLoop)
{
	pthread_t audio_palyfiletid = 0;
	THREAD_FILE_PLAY_INFO *pstThreadInfo = (THREAD_FILE_PLAY_INFO *)malloc(sizeof(THREAD_FILE_PLAY_INFO));
	pstThreadInfo->filename = filename;
	pstThreadInfo->bNeedLoop = bNeedLoop;
	pthread_create(&audio_palyfiletid,NULL,audio_play_audiofile_thread,(void *)pstThreadInfo);
}
void audio_record_mic(char data0)
{
	struct wav_header header;
	pthread_mutex_lock(&micwav_lock);
	if(bPlayMicData) {
		printf("please stop the playing micdata first.\n");
		pthread_mutex_unlock(&micwav_lock);
		return;
	}
	
	if(data0) {
		if(bRecordMicData) {
			printf("already recording mic data...\n");
			pthread_mutex_unlock(&micwav_lock);
			return;
		}
		
		MicFile = fopen(AUDIO_MIC_DATA_WAV,"wb");
		if(!MicFile) {
			pthread_mutex_unlock(&micwav_lock);
			return;
		}
	    /* leave enough room for header */
	    fseek(MicFile, sizeof(struct wav_header), SEEK_SET);
		bRecordMicData = true;
	}else {
		if(!bRecordMicData) {
			pthread_mutex_unlock(&micwav_lock);
			return;
		}
		bRecordMicData = false;
		header.riff_id = ID_RIFF;
	    header.riff_sz = 0;
	    header.riff_fmt = ID_WAVE;
	    header.fmt_id = ID_FMT;
	    header.fmt_sz = 16;
	    header.audio_format = FORMAT_PCM;
	    header.num_channels = 2;
	    header.sample_rate = 16000;

		header.bits_per_sample = 16;
	    header.byte_rate = (header.bits_per_sample / 8) * header.num_channels * header.sample_rate;
	    header.block_align = header.num_channels * (header.bits_per_sample / 8);
	    header.data_id = ID_DATA;
		header.data_sz = total_frames_read * header.block_align;
	    header.riff_sz = header.data_sz + sizeof(header) - 8;
	    fseek(MicFile, 0, SEEK_SET);
	    fwrite(&header, sizeof(struct wav_header), 1, MicFile);
		total_frames_read = 0;
	    if (MicFile != NULL) {
	        fclose(MicFile);
			MicFile = NULL;
	    }
	}
	pthread_mutex_unlock(&micwav_lock);
}
void *audio_main_loop_thread(void *arg)
{
    char szThreadName[20]="main_loop";
	msg_node_t AudioMsg;
	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);
    //SAMPLE_AUDIO_Play(1,0,"/mnt/data/ding.wav");
    int len = 20*48*4;
	//int ret = 0;
    char *pZero= malloc(len);
    memset(pZero,0,sizeof(*pZero));
    bAudioClosed = 0;
	pthread_mutex_init(&micwav_lock,0);
	audio_common_init();

	pst_audio_msgq = (msgq_t *) malloc(sizeof(msgq_t));
	memset(pst_audio_msgq,0,sizeof(msgq_t));
	msgq_init(pst_audio_msgq,"pst_audio_msgq");
	
    while(!bCloseAudio)
    {
    	msgq_receive_timeout(pst_audio_msgq,&AudioMsg,0);
		switch(AudioMsg.msg_type)
		{
			case AUDIO_MSG_PLAY_TONE:
				DBG_PRINTF(CVI_INFO, "AUDIO_MSG_PLAY_TONE type(%d)\n",AudioMsg.msg_type);
				audio_play_wav_file(AUDIO_FILE_TONE,0);
			break;
			case AUDIO_MSG_LOOP_AUDIO:
				DBG_PRINTF(CVI_INFO, "AUDIO_MSG_LOOP_AUDIO type(%d)\n",AudioMsg.msg_type);
				audio_play_filewav(AUDIO_FILE_LOOP_MUSIC,1);
				//audio_play_filewav("/mnt/data/ding.wav",1);
			break;
			case AUDIO_MSG_PLAY_MICRECORD:
				DBG_PRINTF(CVI_INFO, "AUDIO_MSG_PLAY_MICRECORD type(%d)\n",AudioMsg.msg_type);
				if(bPlayMicData || bRecordMicData) {
					printf("error,mic recording or playing ...\n");
					break;
				}
				bPlayMicData = true;
				audio_play_filewav(AUDIO_MIC_DATA_WAV,0);
				
			break;
			case AUDIO_MSG_LOCAL_LOOPBACK:
				DBG_PRINTF(CVI_INFO, "AUDIO_MSG_LOCAL_LOOPBACK type(%d)\n",AudioMsg.msg_type);
				audio_set_MicVolume(3,3);
				audio_loopback_start();
			break;
			case AUDIO_MSG_STOP_AUDIO:
				DBG_PRINTF(CVI_INFO, "AUDIO_MSG_STOP_AUDIO type(%d)\n",AudioMsg.msg_type);
				audio_msg_stop_audio();
			break;
			case AUDIO_MSG_RECORD_MIC:
			{
				struct wav_header header;
				pthread_mutex_lock(&micwav_lock);
				if(bPlayMicData) {
					printf("please stop the playing micdata first.\n");
					pthread_mutex_unlock(&micwav_lock);
					break;
				}
				
				if(AudioMsg.data[0]) {
					if(bRecordMicData) {
						printf("already recording mic data...\n");
						pthread_mutex_unlock(&micwav_lock);
						break;
					}
					
					MicFile = fopen(AUDIO_MIC_DATA_WAV,"wb");
					if(!MicFile) {
						pthread_mutex_unlock(&micwav_lock);
						break;
					}
				    /* leave enough room for header */
				    fseek(MicFile, sizeof(struct wav_header), SEEK_SET);
					bRecordMicData = true;
				}else {
					if(!bRecordMicData) {
						pthread_mutex_unlock(&micwav_lock);
						break;
					}
					bRecordMicData = false;
					header.riff_id = ID_RIFF;
				    header.riff_sz = 0;
				    header.riff_fmt = ID_WAVE;
				    header.fmt_id = ID_FMT;
				    header.fmt_sz = 16;
				    header.audio_format = FORMAT_PCM;
				    header.num_channels = 2;
				    header.sample_rate = 16000;

					header.bits_per_sample = 16;
				    header.byte_rate = (header.bits_per_sample / 8) * header.num_channels * header.sample_rate;
				    header.block_align = header.num_channels * (header.bits_per_sample / 8);
				    header.data_id = ID_DATA;
					header.data_sz = total_frames_read * header.block_align;
				    header.riff_sz = header.data_sz + sizeof(header) - 8;
				    fseek(MicFile, 0, SEEK_SET);
				    fwrite(&header, sizeof(struct wav_header), 1, MicFile);
					total_frames_read = 0;
				    if (MicFile != NULL) {
				        fclose(MicFile);
						MicFile = NULL;
				    }
				}
			}
			pthread_mutex_unlock(&micwav_lock);
			DBG_PRINTF(CVI_INFO, "AUDIO_MSG_RECORD_MIC type(%d),data[0]:%d\n",AudioMsg.msg_type,AudioMsg.data[0]);
			break;
			audio_record_mic(AudioMsg.data[0]);
			DBG_PRINTF(CVI_INFO, "AUDIO_MSG_RECORD_MIC type(%d),data[0]:%d\n",AudioMsg.msg_type,AudioMsg.data[0]);
			break;
			default:
				DBG_PRINTF(CVI_INFO, "msg id(%d) type(%d)\n",AudioMsg.msg_id,AudioMsg.msg_type);
			break;
		}
    	
    }
    bAudioClosed = 1;

    return NULL;
}


int audio_init(void)
{
	int ret = 0;
	static pthread_t audio_playtid = 0;
	static pthread_t audio_captid = 0;

	#ifdef CLI_CMD_DEBUG
	audio_cmd_register();
	DBG_PRINTF(CVI_INFO, "register audio cmd.\n");
	#endif
	//CycleBufferCBIn(SAMPLES_LEN,2);
	pthread_create(&audio_captid,NULL,audio_thread_capture,NULL);
	pthread_create(&audio_playtid,NULL,audio_thread_playback,NULL);
	ret = pthread_create(&msg_loopid,NULL,audio_main_loop_thread,NULL);
    
    if(ret != 0)
    {
        DBG_PRINTF(CVI_ERROR, "pthread_creat fail\n");
    }
	DBG_PRINTF(CVI_INFO, "audio_init suc.\n");
	return ret;
}

void audio_exit(void)
{
	bCloseAudio = 1;
	audio_stop_working();
	while(!bAudioClosed)
	{
		usleep(10*1000);
	}
	//  audio_play_close();
	pthread_join(msg_loopid, 0);
	if(pst_audio_msgq) {
		msgq_clear(pst_audio_msgq);
		free(pst_audio_msgq);
	}
}

int audio_play_single_tone()
{
	if(!pst_audio_msgq)
	{
		DBG_PRINTF(CVI_ERROR, "audio mesgque is not init.\n");
		return -1;
	}
	return msg_send(pst_audio_msgq,AUDIO_MSG_PLAY_TONE,NULL,0,NULL,0);
}

int audio_local_loopback()
{
	if(!pst_audio_msgq)
	{
		DBG_PRINTF(CVI_ERROR, "audio mesgque is not init.\n");
		return -1;
	}
	return msg_send(pst_audio_msgq,AUDIO_MSG_LOCAL_LOOPBACK,NULL,0,NULL,0);
}

int audio_play_loop_audio()
{
	if(!pst_audio_msgq)
	{
		DBG_PRINTF(CVI_ERROR, "audio mesgque is not init.\n");
		return -1;
	}
	return msg_send(pst_audio_msgq,AUDIO_MSG_LOOP_AUDIO,NULL,0,NULL,0);
}

int audio_record_micdata_to_wav(int bSwitch)
{
	char data[4]; 
	data[0] = bSwitch?1:0;
	return msg_send(pst_audio_msgq,AUDIO_MSG_RECORD_MIC,data,4,NULL,0);
}
int audio_play_wav_micdata()
{
 
	return msg_send(pst_audio_msgq,AUDIO_MSG_PLAY_MICRECORD,NULL,0,NULL,0);
}
 
void audio_stop_working()
{
	msg_send(pst_audio_msgq,AUDIO_MSG_STOP_AUDIO,NULL,0,NULL,0);
}



