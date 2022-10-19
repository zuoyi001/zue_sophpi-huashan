#include <stdio.h>
#include <stdlib.h>
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
#include <pthread.h>

#include "alsacommon.h"
#include "cvi_audio_utils.h"
#include "tinyalsa.h"
#include "app_utils.h"

//static snd_pcm_t * pcm_playhandle = NULL; 
//static snd_pcm_t * pcm_caphandle = NULL;
pthread_mutex_t play_lock;
pthread_mutex_t cap_lock;

struct pcm *pcm_playback;
struct pcm *pcm_capture;

#define USE_TINYALSA 1
//play and read


#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

struct command {
    const char *filename;
    const char *filetype;
    unsigned int card;
    unsigned int device;
    int flags;
    struct pcm_alsa_config config;
    unsigned int bits;
};


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




#if USE_TINYALSA 

struct pcm_config stPcmConfig = {
	.channels = 2,
	.rate = 16000,
	.period_size = SAMPLES_FRAMES_LEN,
	.period_count = 8,
	.format = PCM_FORMAT_S16_LE,
	.start_threshold = 0,
	.stop_threshold = INT_MAX,
};

struct pcm_config stPcmPlaybackConfig = {
	.channels = 2,
	.rate = 16000,
	.period_size = SAMPLES_FRAMES_LEN,
	.period_count = 3,
	.format = PCM_FORMAT_S16_LE,
	.start_threshold = 0,
	.stop_threshold = INT_MAX,
};

#else

	static struct pcm_alsa_config stPcmConfig = {
    .channels = 2,
    .rate = 16000,
    .period_size = SAMPLES_FRAMES_LEN,
    .period_count = 8,
    .format = SND_PCM_FORMAT_S16_LE,
    .start_threshold = 0,
    .stop_threshold = INT_MAX,
};
#endif


#if USE_TINYALSA   //tinyalsa
int audio_play_init()
{
	pthread_mutex_lock(&play_lock);
	if(pcm_playback)
	{
		DBG_PRINTF(CVI_INFO, "[warn]audio_play_init already init.\n");
		pthread_mutex_unlock(&play_lock);
		return -1;
	}
	pcm_playback = pcm_open(1, 0, PCM_OUT, &stPcmPlaybackConfig);
	DBG_PRINTF(CVI_INFO, "tinyalsa audio_play_init ok.\n");
	pthread_mutex_unlock(&play_lock);
    return 0;
}


void audio_play_close()
{
	pthread_mutex_lock(&play_lock);
    if(!pcm_playback) {
		pthread_mutex_unlock(&play_lock);
		return;
	}
	pcm_close(pcm_playback);
    pcm_playback = NULL;
	DBG_PRINTF(CVI_INFO, "tinyalsa audio_play_close ok.\n");
	pthread_mutex_unlock(&play_lock);
    return ;
}


unsigned int get_audio_play_periodbyes()
{

	return pcm_frames_to_bytes(pcm_playback, stPcmPlaybackConfig.period_size);
}


//extern int g_queid;


int audio_alsa_write(char *buffer,unsigned int len)
{
    int r = 0;
	pthread_mutex_lock(&play_lock);
    if(pcm_playback)
    {
        r = pcm_write(pcm_playback,buffer,len);
		if(r < 0 )
		{
			DBG_PRINTF(CVI_INFO, "write error: r=%d.\n",r);
		}
    }
	pthread_mutex_unlock(&play_lock);
    return (int)r;
}


int audio_capture_init()
{
	pthread_mutex_lock(&cap_lock);
	if(pcm_capture) {
		DBG_PRINTF(CVI_INFO, "error capture init already.\n");
		return -1;
	}
	pcm_capture = pcm_open(0, 0, PCM_IN, &stPcmConfig);

	DBG_PRINTF(CVI_INFO, "tinyalsa capture init ok.\n");
	pthread_mutex_unlock(&cap_lock);
	return 0;
}

int audio_alsa_read(char *buffer,unsigned int len) // len is bytes
{
    int r = 0;
	pthread_mutex_lock(&cap_lock);
    if(!pcm_capture)
    {
    	DBG_PRINTF(CVI_INFO, "capture is not init.\n");
		pthread_mutex_unlock(&cap_lock);
		return 0;
    }
	
    {
        r = pcm_read(pcm_capture, buffer, len);
		if(r < 0 )
		{
			DBG_PRINTF(CVI_INFO, "read error: r=%d.\n",r);
		}
    }
	pthread_mutex_unlock(&cap_lock);
    return len/(stPcmConfig.channels*2); //r is samples
}

void audio_capture_close()
{
	pthread_mutex_lock(&cap_lock);
	if(pcm_capture) {
		pcm_close(pcm_capture);
	}
    pcm_capture = NULL;
	pthread_mutex_unlock(&cap_lock);
	DBG_PRINTF(CVI_INFO, "tinyalsa capture close ok.\n");
	return;
}

#else  //use alsa
int audio_play_init()
{
	pthread_mutex_lock(&play_lock);
	if(pcm_playhandle)
	{
		DBG_PRINTF(CVI_INFO, "[warn]audio_play_init already init.\n");
		pthread_mutex_unlock(&play_lock);
		return;
	}
    pcm_playhandle = SAMPLE_AUDIO_pcm_open(1, 0,PCM_ALSA_OUT,&stPcmPlaybackConfig);
	DBG_PRINTF(CVI_INFO, "audio_play_init ok.\n");
	pthread_mutex_unlock(&play_lock);
    return 0;
}


void audio_play_close()
{
	pthread_mutex_lock(&play_lock);
    if(!pcm_playhandle) {
		pthread_mutex_unlock(&play_lock);
		return;
	}
  //  snd_pcm_nonblock(pcm_playhandle, 0);
	//snd_pcm_drain(pcm_playhandle);
//	snd_pcm_nonblock(pcm_playhandle, 0);
	snd_pcm_abort(pcm_playhandle);

    snd_pcm_close(pcm_playhandle);
    pcm_playhandle = NULL;
	DBG_PRINTF(CVI_INFO, "audio_play_close ok.\n");
	pthread_mutex_unlock(&play_lock);
    return ;
}


unsigned int get_audio_play_periodbyes()
{
	return SAMPLE_AUDIO_pcm_frames_to_bytes(stPcmConfig, stPcmPlaybackConfig.period_size);
}


//extern int g_queid;


int audio_alsa_write(char *buffer,unsigned int len)
{
    snd_pcm_sframes_t r = 0;
	pthread_mutex_lock(&play_lock);
    if(pcm_playhandle)
    {
        r = snd_pcm_writei(pcm_playhandle, buffer, SAMPLE_AUDIO_pcm_bytes_to_frames(stPcmPlaybackConfig, len));
		if (r == -EPIPE) {
		    DBG_PRINTF(CVI_INFO, "write error xrun .\n");
		} else if (r < 0) {
			DBG_PRINTF(CVI_INFO, "write error: %ld\n", r);
		}
    }
	pthread_mutex_unlock(&play_lock);
    return (int)r;
}


int audio_capture_init()
{
	pthread_mutex_lock(&cap_lock);
	if(pcm_caphandle) {
		DBG_PRINTF(CVI_INFO, "error capture init already.\n");
		return -1;
	}
	pcm_caphandle = SAMPLE_AUDIO_pcm_open(0, 0,PCM_ALSA_IN,&stPcmConfig);
	pthread_mutex_unlock(&cap_lock);
	return 0;
}

int audio_alsa_read(char *buffer,unsigned int len) // len is bytes
{
    snd_pcm_sframes_t r = 0;
	pthread_mutex_lock(&cap_lock);
    if(!pcm_caphandle)
    {
    	DBG_PRINTF(CVI_INFO, "capture is not init.\n");
		pthread_mutex_unlock(&cap_lock);
		return 0;
    }
	
    {
        r = snd_pcm_readi(pcm_caphandle, buffer, SAMPLE_AUDIO_pcm_bytes_to_frames(stPcmConfig, len));
		if (r == -EPIPE) {
		    DBG_PRINTF(CVI_INFO, "read error xrun .\n");
		} else if (r < 0) {
			DBG_PRINTF(CVI_INFO, "read error: %ld\n", r);
		}
    }
	pthread_mutex_unlock(&cap_lock);
    return (int)r; //r is samples
}

void audio_capture_close()
{
	pthread_mutex_lock(&cap_lock);
	if(pcm_caphandle) {
		snd_pcm_close(pcm_caphandle);
	}
    pcm_caphandle = NULL;
	pthread_mutex_unlock(&cap_lock);
	
	return;
}


#endif



int audio_record_wav_file(const char * recordfile,unsigned int capture_time)
{
	char *buffer;
	int size = 0;
	unsigned int total_frames_read = 0;
	unsigned int frames_read;
	unsigned int bytes_per_frame;
	int capturing;
	FILE *pFile;
	struct wav_header header;
	
    if(recordfile == NULL) {
		DBG_PRINTF(CVI_INFO, "recordfile is NULL.\n");
		return -1;
	}

	pFile = fopen(recordfile,"wb");
	header.riff_id = ID_RIFF;
    header.riff_sz = 0;
    header.riff_fmt = ID_WAVE;
    header.fmt_id = ID_FMT;
    header.fmt_sz = 16;
    header.audio_format = FORMAT_PCM;
    header.num_channels = stPcmConfig.channels;
    header.sample_rate = stPcmConfig.rate;

	header.bits_per_sample = 16;
    header.byte_rate = (header.bits_per_sample / 8) * stPcmConfig.channels * stPcmConfig.rate;
    header.block_align = stPcmConfig.channels * (header.bits_per_sample / 8);
    header.data_id = ID_DATA;

    /* leave enough room for header */
    fseek(pFile, sizeof(struct wav_header), SEEK_SET);
	
	if(audio_capture_init()<0) {
		DBG_PRINTF(CVI_INFO, "record is unavailable.\n");
		fclose(pFile);
		return -2;
	}

	//size = SAMPLE_AUDIO_pcm_frames_to_bytes(stPcmConfig,stPcmConfig.period_size);
	size = stPcmConfig.channels*stPcmConfig.period_size*(16/8);
    buffer = malloc(size);
    if (!buffer) {
        audio_capture_close();
		fclose(pFile);
        return -3;
    }

	if(capture_time > 0 ) {
		capturing = 1;
	}

	bytes_per_frame = stPcmConfig.channels*(16/8);
	
	while (capturing) {
		frames_read = audio_alsa_read(buffer,size);
		if(frames_read) {
			total_frames_read += frames_read;
	        if ((total_frames_read / stPcmConfig.rate) >= capture_time) {
	            capturing = 0;
	        }
			if (fwrite(buffer, bytes_per_frame, frames_read, pFile) != frames_read) {
            	fprintf(stderr,"Error capturing sample\n");
            	break;
        	}
		}
	}
	
	audio_capture_close();

	header.data_sz = total_frames_read * header.block_align;
    header.riff_sz = header.data_sz + sizeof(header) - 8;
    fseek(pFile, 0, SEEK_SET);
    fwrite(&header, sizeof(struct wav_header), 1, pFile);

    if (pFile != NULL) {
        fclose(pFile);
    }

	free(buffer);

    return 0;
}


void audio_common_init()
{
	//int ret = 0;
	pthread_mutex_init(&play_lock, NULL);
	pthread_mutex_init(&cap_lock, NULL);
	
}

void audio_common_deinit()
{
	//pthread_mutex_destory(&play_lock);
	//pthread_mutex_destory(&cap_lock);
}




