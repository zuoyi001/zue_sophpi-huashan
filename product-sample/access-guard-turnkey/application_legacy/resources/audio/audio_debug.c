#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#include "cvi_audio_main.h"
// #include "tcli.h"
#include "cvi_audio.h"
#include "acodec.h"
#ifndef UNUSED
#define UNUSED(x) //x
#endif

#ifdef CLI_CMD_DEBUG
int audio_debug_test_playtone(int argc,char *argv[]);
int audio_debug_test_local_loopback(int argc,char *argv[]);
int audio_debug_test_loopaudio(int argc,char *argv[]);
int audio_debug_test_stop(int argc,char *argv[]);
int audio_debug_set_AoVolume(int argc,char *argv[]);
int audio_debug_dump_enable(int argc,char *argv[]);
int audio_debug_show_cbinfo(int argc,char *argv[]);
int audio_debug_dumpcap_enable(int argc,char *argv[]);
int audio_debug_set_MicVolume(int32_t argc,char *argv[]);
int audio_debug_test_playMicData(int argc,char *argv[]);
int audio_debug_test_recordMic(int argc,char *argv[]);






TELNET_CLI_S_COMMAND audio_cli_cmd_list[]={
    DECLARE_CLI_CMD_MACRO(tone,0,audio_debug_test_playtone,"audio tone command",0),
	DECLARE_CLI_CMD_MACRO(volume,NULL,audio_debug_set_AoVolume,"set/get ao volume",0),
	DECLARE_CLI_CMD_MACRO(loopback,0,audio_debug_test_local_loopback,"local loopback",0),
	DECLARE_CLI_CMD_MACRO(loopaudio,0,audio_debug_test_loopaudio,"loop audio",0),
	DECLARE_CLI_CMD_MACRO(stopaudio,0,audio_debug_test_stop,"stop audio",0),
	DECLARE_CLI_CMD_MACRO(playdump,0,audio_debug_dump_enable,"play audio capture",0),
	DECLARE_CLI_CMD_MACRO(capdump,0,audio_debug_dumpcap_enable,"capture audio dump",0),
	DECLARE_CLI_CMD_MACRO(cbinfo,0,audio_debug_show_cbinfo,"show cb info",0),
	DECLARE_CLI_CMD_MACRO(micvol,0,audio_debug_set_MicVolume,"set mic volume",0),
	DECLARE_CLI_CMD_MACRO(record,0,audio_debug_test_recordMic,"record mic data",0),
	DECLARE_CLI_CMD_MACRO(playrecord,0,audio_debug_test_playMicData,"play mic records",0),
    DECLARE_CLI_CMD_MACRO_END()
};

int audio_debug_test_playtone(int argc,char *argv[])
{
    int iCount = 1;
	int ret = 0;
    if(argc >= 2)
    {
        iCount = atoi(argv[1]);
    }

    if(iCount)
    {
	    ret = audio_play_single_tone();
    }

   return ret;
}

int audio_debug_test_recordMic(int argc,char *argv[])
{
	int ret = 0;
	int bCapEnable = 0;
    if(argc >= 2)
    {
    	bCapEnable = atoi(argv[1])?1:0;
    }
	else {
		tcli_print("cmd failed,b_enable_dump = %d.\n",bCapEnable);
		return -1;
	}

    ret = audio_record_micdata_to_wav(bCapEnable);
    if (ret)
	{
	 tcli_print("error send cmd ret = %d.\n",ret);
	 return -1;
	}

	return 0;
}

int audio_debug_test_playMicData(int argc,char *argv[])
{
	int ret = 0;
	UNUSED(argc);
	UNUSED(argv);
	
	ret = audio_play_wav_micdata();
	if (ret)
	{
		tcli_print("error send cmd ret = %d.\n",ret);
		return -1;
	}

	return 0;
}


int audio_debug_test_local_loopback(int argc,char *argv[])
{
	int ret = 0;
	UNUSED(argc);
	UNUSED(argv);

	ret = audio_local_loopback();
    if (ret)
    {
        tcli_print("error send cmd ret = %d.\n",ret);
		return -1;
    }

   return 0;
}

int audio_debug_test_loopaudio(int argc,char *argv[])
{
	int ret = 0;
    UNUSED(argc);
	UNUSED(argv);

    ret = audio_play_loop_audio();
    if (ret)
    {
        tcli_print("error send cmd ret = %d.\n",ret);
		return -1;
    }

   return 0;
}

int audio_debug_test_stop(int argc,char *argv[])
{
	int ret = 0;
    UNUSED(argc);
	UNUSED(argv);

	audio_stop_working();

   return ret;
}

extern int b_enable_dump;
int audio_debug_dump_enable(int argc,char *argv[])
{
    int bDumpEnable = 0;
	int ret = 0;
    if(argc >= 2)
    {
        bDumpEnable = atoi(argv[1])?1:0;
    }
	else {
		tcli_print("cmd failed,b_enable_dump = %d.\n",b_enable_dump);
		return -1;
	}

	b_enable_dump = bDumpEnable;
    tcli_print("set suc,b_enable_dump = %d.\n",b_enable_dump);

   return ret;
}


extern int b_enable_dumpcap;
int audio_debug_dumpcap_enable(int argc,char *argv[])
{
    int bDumpEnable = 0;
	int ret = 0;
    if(argc >= 2)
    {
        bDumpEnable = atoi(argv[1])?1:0;
    }
	else {
		tcli_print("cmd failed,b_enable_dumpcap = %d.\n",b_enable_dumpcap);
		return -1;
	}

	b_enable_dumpcap = bDumpEnable;
    tcli_print("set suc,b_enable_dumpcap = %d.\n",b_enable_dumpcap);

   return ret;
}


int audio_debug_set_AoVolume(int32_t argc, char *argv[])
{
   int i32volume = 0;
   if(argc >= 2)
   {
       i32volume = atoi(argv[1]);
       i32volume = i32volume> 15?15:(i32volume<0)?0:i32volume;
       CVI_AO_SetVolume(1,i32volume);
       tcli_print("i32volume set to %d suc.\n",i32volume);
	}else {
       CVI_AO_GetVolume(1, &i32volume);
       tcli_print("current i32volume is %d .\n",i32volume);
	}

	return 0;
}

extern void audio_set_MicVolume(int left,int right);
int audio_debug_set_MicVolume(int32_t argc,char *argv[])
{
	int i32volume = 0; 
	CVI_S32 fdAcodec_adc = -1;
	ACODEC_VOL_CTRL volL_ctrl;

	
   if(argc >= 2)
   {
       i32volume = atoi(argv[1]);
       i32volume = i32volume> 7?7:(i32volume<0)?0:i32volume;
	   audio_set_MicVolume(i32volume,i32volume);
       tcli_print("i32volume set mic to %d suc.\n",i32volume);
	} else {
		fdAcodec_adc = open("/dev/cv1835adc", O_RDWR);
		if(fdAcodec_adc) {
			ioctl(fdAcodec_adc, ACODEC_GET_ADCL_VOL, &volL_ctrl);
			close(fdAcodec_adc);
		}
	   tcli_print("i32volume set mic to %d suc.\n",volL_ctrl.vol_ctrl);
	}

	return 0;
}



extern void CycleBufferInfo(void);

int audio_debug_show_cbinfo(int argc,char *argv[])
{
    UNUSED(argc);
	UNUSED(argv);

	CycleBufferInfo();

   return 0;
}


void audio_cmd_register()
{
	RegisterCliCommand(audio_cli_cmd_list);
}

#endif




