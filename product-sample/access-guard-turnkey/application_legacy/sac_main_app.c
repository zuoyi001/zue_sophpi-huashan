#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <assert.h>
#include <dlfcn.h>
#include <sys/prctl.h>

// #include "cvi_venc.h"
#include "cvi_sys.h"
#include "cvi_hal_face_api.h"
#include "app_init.h"
#include "app_utils.h"
#include "config_data.h"
#include "factory.h"
#include "app_rtsp.h"
#include "app_menu.h"

static struct timeval tv1,tv2;
//static long elapsed = 0;
//static long total_elapsed = 0;
static bool flagVpssBindVo = false;

face_process_t face_config;
hal_facelib_handle_t facelib_handle = NULL;

#if defined(CONFIG_VIDEO_RTSP_SUPPORT)
#if defined(CONFIG_DUAL_SENSOR_SUPPORT)
VENC_CHN VencChn[2] = {0, 1};
#else
VENC_CHN VencChn[1] = {0};
#endif
int VencNum = 0;
#endif

#define PIDFILE_PATH "/var/run/app.pid"

void Sample_HandleSig(CVI_S32 signo)
{
	// if (SIGINT == signo || SIGTERM == signo)
	{
#if defined(CONFIG_VIDEO_RTSP_SUPPORT)		
		APP_StopRtspServer(VencChn, VencNum);
#endif
		APP_VideoRelease();
		unlink(PIDFILE_PATH);
	}
	DBG_PRINTF(CVI_INFO, "Sample_HandleSig signal number %d\n",signo);
	exit(-1);
}

void Reload_Feature(CVI_S32 signo)
{
	if (facelib_handle != NULL) {
		CviLoadIdentify(facelib_handle);
	}

	APP_CompSendCmd(CMD_APP_COMP_RESTART, NULL, 0, NULL, 0);

}

void Stop_AI(CVI_S32 signo)
{
	DBG_PRINTF(CVI_WARNING,"stop AI from other process");
    APP_CompSendCmd(CMD_APP_COMP_VIDEOSTOP, NULL, 0, NULL, 0);
}


void Create_PID_File()
{
	FILE *fp = NULL;
	if (NULL == (fp = fopen(PIDFILE_PATH, "w"))) {
		return;
	}

	fprintf(fp, "%d", getpid());

	fclose(fp);
}

void signal_process()
{
	signal(SIGINT, Sample_HandleSig);
	signal(SIGTERM, Sample_HandleSig);
	signal(SIGUSR1, Reload_Feature);
	signal(SIGUSR2, Stop_AI);
	// signal(SIGSEGV, Sample_HandleSig);
	// signal(SIGABRT, Sample_HandleSig);
}

void* SAC_MAINAPP_DisplayThread(void *arg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	//VIDEO_FRAME_INFO_S stVideoFrame;
	//VIDEO_FRAME_INFO_S stVideoGBRFrame;
	VIDEO_FRAME_INFO_S stVideoIRFrame;

	//int face_count;
	prctl(PR_SET_NAME, "Display_Thread", 0, 0, 0);

	while (true)
	{
		APP_CompTaskHandler();

		switch (get_app_comp_sm())
		{
			case CVI_INIT:

				DBG_PRINTF(CVI_INFO, "case Main Thread CVI_INIT start\n");
				gettimeofday(&tv1, NULL);

				if(flagVpssBindVo == false) {
					s32Ret = APP_VpssBindVo();
					CVI_VO_ShowChn(0,0);
					if (s32Ret == CVI_SUCCESS) {
						flagVpssBindVo = true;
					}
					else {
						SAMPLE_PRT("SAMPLE_COMM_VPSS_Bind_VO failed with %#x\n", s32Ret);
					}
				}
				else{
					usleep(30*1000);
				}

				gettimeofday(&tv2, NULL);

				#ifdef PERF_PROFILING
				elapsed = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
				total_elapsed += elapsed;
				DBG_PRINTF(CVI_DEBUG, "[Timestamp] Video Thread. 1. Vpss bind vo cost: %ld us, total cost: %ld us\n", elapsed,total_elapsed);
				#endif

				if(get_register_status())
				{
					DBG_PRINTF(CVI_INFO, "send CMD_AI_NONE\n");

					APP_AI_ThreadSendCmd((CMD_AI_NONE),NULL, 0, NULL, 0);
				}
				else
				{
					APP_AI_ThreadSendCmd((CMD_AI_FD|CMD_AI_LIVENESS|CMD_AI_FR),NULL, 0, NULL, 0);
				}
				set_app_comp_sm(CVI_INIT);

				DBG_PRINTF(CVI_INFO, "case Main Thread CVI_INIT end\n");

				break;
			case CVI_GBR_INIT:

				DBG_PRINTF(CVI_INFO, "case CVI_GBR_INIT start\n");

				if(flagVpssBindVo == false) {
					s32Ret = APP_VpssBindVo();
					CVI_VO_ShowChn(0,0);
					if (s32Ret == CVI_SUCCESS) {
						flagVpssBindVo = true;
					}
					else {
						DBG_PRINTF(CVI_ERROR, "SAMPLE_COMM_VPSS_Bind_VO failed with %#x\n", s32Ret);
					}
				}
				else{
					usleep(30*1000);
				}
				set_app_comp_sm(CVI_GBR_INIT);

				DBG_PRINTF(CVI_INFO, "case CVI_GBR_INIT end\n");

				break;
			case CVI_IR_INIT:

				DBG_PRINTF(CVI_INFO, "case CVI_IR_INIT start\n");

				if(flagVpssBindVo == true) {
					APP_VpssUnBindVo();
					printf("unbind vpss and vo!\n");
					flagVpssBindVo =false;
				}
				while(true)
				{
					s32Ret = APP_VpssGetFrame(APP_VIDEO_FRAME_FD_IR, &stVideoIRFrame, 1000);/*&stVideoFrame*/
					if (s32Ret == CVI_SUCCESS) {
						break;
					}
					else
						SAMPLE_PRT("CVI_VPSS_GetChnFrame grp1 chn0 videoframe failed with %#x\n", s32Ret);
				}
				s32Ret = CVI_VO_SendFrame(0, 0, &stVideoIRFrame, -1);
				if (s32Ret != CVI_SUCCESS) {
					assert(0);
					SAMPLE_PRT("CVI_VO_SendFrame failed with %#x\n", s32Ret);
				}
				CVI_VO_ShowChn(0,0);
				s32Ret = APP_VpssReleaseFrame(APP_VIDEO_FRAME_FD_IR, &stVideoIRFrame);
				if (s32Ret != CVI_SUCCESS) {
					SAMPLE_PRT("CVI_VPSS_ReleaseChnFrame chn2 NG\n");
					assert(0&&"ReleaseChnFrame Faile");
				}
				set_app_comp_sm(CVI_IR_INIT);
				DBG_PRINTF(CVI_INFO, "case CVI_IR_INIT end\n");

				break;

			case CVI_STOP:
				DBG_PRINTF(CVI_INFO, "case Main Thread CVI_STOP start\n");

				#ifndef CONFIG_CVI_SOC_CV182X
				if(flagVpssBindVo == true) {
					APP_VpssUnBindVo();
					printf("unbind vpss and vo!\n");
					flagVpssBindVo =false;
				}
				#endif
				APP_AI_ThreadSendCmd((CMD_AI_STOP),NULL, 0, NULL, 0);

				DBG_PRINTF(CVI_INFO, "case Main Thread CVI_STOP end\n");

				set_app_comp_sm(CVI_UNKNOW);
				break;

			default:
				usleep(30*1000);
				break;

		}

	}
}


#if defined(CONFIG_ENABLE_ISPD)
void APP_LoadIspd(void)
{
#define ISPD_LIBNAME "libcvi_ispd.so"
#define ISPD_CONNECT_PORT 5566

	void *handle = NULL;

	handle = dlopen(ISPD_LIBNAME, RTLD_NOW);

	if (handle) {
		char *error = NULL;
		void (*daemon_init)(unsigned int port);

		printf("Load dynamic library %s success\n", ISPD_LIBNAME);

		dlerror();
		daemon_init = dlsym(handle, "isp_daemon_init");
		error = dlerror();
		if (error == NULL) {
			(*daemon_init)(ISPD_CONNECT_PORT);
		} else {
			printf("Run daemon initial fail\n");
			dlclose(handle);
		}
	}
}
#endif

pthread_t display_thread, face_ai_thread;
VENC_CHN VencChn[2] = {0, 1};

int SAC_MAINAPP_StartUp(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	signal_process();
	Create_PID_File();
	sys_init();

	#ifdef ENABLE_PERI
	peri_init();
	#endif

	s32Ret = APP_VideoInit();
	if (s32Ret != CVI_SUCCESS) {
		DBG_PRINTF(CVI_ERROR, "APP_VideoInit failed with %d\n", s32Ret);
		assert(0&&"SAC_APP_VideoInit fail!!");
		return s32Ret;
	}

	#ifdef USE_CONFIG_DATA
    config_data_init();
    #endif

	#if defined(CONFIG_ENABLE_ISPD)
	APP_LoadIspd();
	#endif

	APP_InitMenu();
	APP_CompInit();
	APP_AI_ThreadInit();

	pthread_create(&display_thread, NULL, SAC_MAINAPP_DisplayThread, NULL);
	pthread_create(&face_ai_thread, NULL, APP_AI_FaceThread, NULL);
#if (defined(CONFIG_RGB_VIDEO_RTSP_SUPPORT) && defined(CONFIG_IR_VIDEO_RTSP_SUPPORT))
	APP_Venc_Init();
	#if defined(CONFIG_RGB_VIDEO_RTSP_SUPPORT)
	RTSP_CreateServer(VencChn, 0);
	#endif
	#if defined(CONFIG_IR_VIDEO_RTSP_SUPPORT)
	RTSP_CreateServer(VencChn, 1);
	#endif
#endif

	// input c+enter to exit application
	while(1){
		usleep(30*1000);
	}

	return s32Ret;

}

static int _testcase_1(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i = 0;

	VIDEO_FRAME_INFO_S stVideoFrame;
	while(1) 
	{
		s32Ret = APP_VideoInit();
		if (s32Ret != CVI_SUCCESS) {
			DBG_PRINTF(CVI_ERROR, "CVI_Video_Init failed with %d\n", s32Ret);
			assert(0&&"CVI_Video_Init fail!!");
			return s32Ret;
		}
		CVI_VO_ShowChn(0, 0);
		for (i=0; i<500; i++)
		{
			s32Ret = CVI_VPSS_GetChnFrame(0, 2, &stVideoFrame, 3000);/*&stVideoFrame*/
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VPSS_GetChnFrame grp0 chn1 videoframe failed with %#x\n", s32Ret);
			} else {
				s32Ret = CVI_VO_SendFrame(0, 0, &stVideoFrame, -1);
				s32Ret = CVI_VPSS_ReleaseChnFrame(0, 2, &stVideoFrame);
				if (s32Ret != CVI_SUCCESS) {
					SAMPLE_PRT("CVI_VPSS_ReleaseChnFrame chn0 NG\n");
					assert(0&&"ReleaseChnFrame Faile");
				}
			}
		}
		for (i=0; i<500; i++)
		{
			s32Ret = CVI_VPSS_GetChnFrame(1, 2, &stVideoFrame, 3000);/*&stVideoFrame*/
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VPSS_GetChnFrame grp0 chn1 videoframe failed with %#x\n", s32Ret);
			} else {
				s32Ret = CVI_VO_SendFrame(0, 0, &stVideoFrame, -1);
				s32Ret = CVI_VPSS_ReleaseChnFrame(1, 2, &stVideoFrame);
				if (s32Ret != CVI_SUCCESS) {
					SAMPLE_PRT("CVI_VPSS_ReleaseChnFrame chn0 NG\n");
					assert(0&&"ReleaseChnFrame Faile");
				}
			}
		}
		CVI_VO_HideChn(0, 0);
		sleep(1);
		CVI_VO_ShowChn(0, 0);
		SAMPLE_COMM_VPSS_Bind_VO(0, 2, 0, 0);
		sleep(10);
		SAMPLE_COMM_VPSS_UnBind_VO(0, 2, 0, 0);
		SAMPLE_COMM_VPSS_Bind_VO(1, 2, 0, 0);
		sleep(10);
		SAMPLE_COMM_VPSS_UnBind_VO(1, 2, 0, 0);
		//
		APP_VideoRelease();
		sleep(1);
	}
}

int sample_testcase(int num)
{
	printf("wl dbg: run testcase %d\n", num);

	switch(num)
	{
		case 1:
			_testcase_1();
			break;
		default:
			printf("sorry, case %d not support.", num);
			break;
	}
	return 0;
}

enum sample_app_opts
{
	opt_null, //0
	opt_help, //1
	opt_testcase,
	//opt_,
	//opt_,
};

static struct option long_options[] = \
{
	{"help",		no_argument,		NULL,	opt_help},
	{"testcase", 	no_argument,		NULL,	opt_testcase},
	{NULL,			 0,					NULL,	opt_null}
};

void print_help(void)
{
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	printf("Usage        :\n");
	printf("help         : ./sample --help\n");
	printf("testcase   : ./sample --testcase [1/2/3/...]\n");
	printf("If no options will run SAC_APP_MainSample\n");
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}

int main(int argc, char *argv[])
{
    int input_opt;
    int option_index = 0;
    //char *string = "h:t";

	if (argc == 1)
	{
		SAC_MAINAPP_StartUp();
		return 0;
	}

	while((input_opt = getopt_long(argc, argv, "h:t", long_options, &option_index))!= -1)
	{
		printf("opt = %d, optarg = %s, optind = %d, argv[optind] = %s, option_index = %d.\n",input_opt,\
			(optarg == NULL)?"Null":optarg, optind, (argv[optind] == NULL)?"Null":argv[optind], option_index);

		switch (input_opt)
		{
			case opt_help:
				print_help();
				return 0;
			case opt_testcase:
				if(argv[optind] == NULL)
					print_help();
				else
					sample_testcase(atoi(argv[optind]));
				break;
			default:
				break;
		}
	}
	return 0;
}
