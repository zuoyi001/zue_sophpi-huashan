#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "sample_comm.h"
#include "cvi_sys.h"
#include "cvi_type.h"
//#include "cvi_rtsp.h"
//#include "rtsp.h"
#include "defs.h"
#include "cvi_video.h"
#include "zscanner.h"

// static SAMPLE_VI_CONFIG_S g_stViConfig;
// static SAMPLE_INI_CFG_S g_stIniCfg;

pthread_t display_Thread;
pthread_t scanner_Thread;
void SAMPLE_VIO_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {
		//todo for release
		SAMPLE_PRT("Program termination abnormally\n");
	}
	exit(-1);
}
void * Scanner_Thread(void)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	size_t image_size;
	CVI_VOID *vir_addr;
	CVI_U32 plane_offset;

	while (1)
	{
		zscanner_init(640, 360);
		CVI_VPSS_GetChnFrame(0, 2, &stVideoFrame, 5000);
		image_size = stVideoFrame.stVFrame.u32Length[0] + stVideoFrame.stVFrame.u32Length[1] + stVideoFrame.stVFrame.u32Length[2];
		vir_addr = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], image_size);
		CVI_SYS_IonInvalidateCache(stVideoFrame.stVFrame.u64PhyAddr[0], vir_addr, image_size);
		plane_offset = 0;
		if (stVideoFrame.stVFrame.u32Length[0] != 0) {
			stVideoFrame.stVFrame.pu8VirAddr[0] = vir_addr + plane_offset;
			plane_offset += stVideoFrame.stVFrame.u32Length[0];
		}
		zscanner_scan((char*)stVideoFrame.stVFrame.pu8VirAddr[0]);
		CVI_SYS_Munmap(vir_addr, image_size);
		CVI_VPSS_ReleaseChnFrame(0, 2, &stVideoFrame);
	}

}

void * Display_Thread(void)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_S32 s32Ret = CVI_SUCCESS;

	while(1) {

		s32Ret = CVI_VPSS_GetChnFrame(0, 0, &stVideoFrame, 1000);/*&stVideoFrame*/
	
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_GetChnFrame grp0 chn2 videoframe failed with %#x\n", s32Ret);
		} else {
			s32Ret = CVI_VO_SendFrame(0, 0, &stVideoFrame, -1);
			s32Ret = CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VPSS_ReleaseChnFrame chn0 NG\n");
				assert(0 && "ReleaseChnFrame Faile");
			}
		}		
	}
}

int main(void)
{
	signal(SIGINT, SAMPLE_VIO_HandleSig);
	signal(SIGTERM, SAMPLE_VIO_HandleSig);

	CVI_Video_Init();
	pthread_create(&display_Thread, NULL, Display_Thread, NULL);
	pthread_create(&scanner_Thread, NULL, Scanner_Thread, NULL);
	pthread_join(display_Thread, NULL);
	pthread_join(scanner_Thread, NULL);
	//SAMPLE_COMM_VPSS_Bind_VO(0, 0, 0, 0); //grp0, chn0, voLayer0, vochn0.
	//CVI_VO_ShowChn(0,0);

	// VENC_CHN VencChn[2] = {0, 1};
	// int VencNum = sizeof(VencChn)/sizeof(VENC_CHN);
	// start_rtsp_server(VencChn, VencNum);

	while(1){
		if(getchar()== 99)
			break;
	}

	//stop_rtsp_server(VencChn, VencNum);
	CVI_Video_Release();
	zscanner_exit();

	return 0;
}

