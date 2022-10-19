#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <sys/prctl.h>

#include "autoconf.h"
#include "cvi_sys.h"
#include "cvi_hal_face_api.h"
#include "app_init.h"
#include "app_utils.h"
#include "app_ai.h"
#include "app_peripheral.h"
#include "lv_i18n.h"
#include "gui_ext.h"
#include "app_video.h"


static msgq_t ai_thread_msgq;
static pthread_mutex_t mutex_sm = PTHREAD_MUTEX_INITIALIZER;
extern face_process_t face_config;
extern hal_facelib_handle_t facelib_handle;

struct timeval tv1,tv2;
//static long elapsed = 0;
//static long total_elapsed = 0;
int jump_counter = 0;

AI_STATE_E ai_state = AI_INIT;

int msg_type = 0;

AI_STATE_E APP_AI_GetState()
{
    return ai_state;
}

void APP_AI_SetState(CVI_STATE_E state)
{
    pthread_mutex_lock(&mutex_sm);
    ai_state = state;
    pthread_mutex_unlock(&mutex_sm);
}

void APP_AI_ThreadInit()
{
    msgq_init(&ai_thread_msgq, "ai_thread_msgq");
}

void APP_AI_ThreadSendCmd(int cmd, void *args, int args_len, void *ext, int ext_len)
{
    msg_node_t msg;

	if(cmd == CMD_AI_STOP)
	{
		DBG_PRINTF(CVI_DEBUG, "ai thread received stop\n");

		while(APP_AI_GetState() != AI_UNKNOWN)
		{

		}
		msgq_clear(&ai_thread_msgq);
	}


	if(APP_AI_GetState() != AI_UNKNOWN)
		return;

    memset(&msg, 0, sizeof(msg_node_t));
    msg.msg_type = cmd;
    if (args != NULL && args_len > 0)//&& args_len <= MSG_DATA_LEN)
    {
        memcpy(msg.data, args, args_len);
    }
    if (!msgq_send(&ai_thread_msgq, &msg))
    {
		DBG_PRINTF(CVI_ERROR, "error send cmd\n");
    }
}

void APP_AI_TaskHandler()
{
    do {
        msg_node_t msg;
        //memset(&msg, 0, sizeof(msg_node_t));
        if (msgq_receive_timeout(&ai_thread_msgq, &msg, 100))
        {
			msg_type = msg.msg_type;

			APP_AI_SetState(AI_INIT);
        }
    }while(0);
}

#if 0
static int _FrameMmap(VIDEO_FRAME_INFO_S *frame) {
	size_t length = 0;
    for (int i = 0; i < 3; ++i) {
        length += frame->stVFrame.u32Length[i];
    }
    frame->stVFrame.pu8VirAddr[0] = (CVI_U8 *)CVI_SYS_MmapCache(frame->stVFrame.u64PhyAddr[0],
                                                                     length);
    if (!frame->stVFrame.pu8VirAddr[0]) {
        printf("mmap frame virtual addr failed\n");
        return -1;
    }
    frame->stVFrame.pu8VirAddr[1] = frame->stVFrame.pu8VirAddr[0] + frame->stVFrame.u32Length[0];
    frame->stVFrame.pu8VirAddr[2] = frame->stVFrame.pu8VirAddr[1] + frame->stVFrame.u32Length[1];
	return 0;
}

static void _FrameUnmmap(VIDEO_FRAME_INFO_S *frame) {
	size_t length = 0;
    for (int i = 0; i < 3; ++i) {
        length += frame->stVFrame.u32Length[i];
    }
    CVI_SYS_Munmap((void *)frame->stVFrame.pu8VirAddr[0], length);
}
#endif



void* APP_AI_FaceThread(void *arg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stfdFrame, stIrFrame;
	#if defined(CONFIG_ALGORITHM_VENDOR_AISDK)
	VIDEO_FRAME_INFO_S stLivenessRGBFrame;
	#endif
	//msg_node_t msg;
	hal_face_t face;
	//int face_count;
	jump_counter = 0;

	prctl(PR_SET_NAME, "Face_AI_Thread", 0, 0, 0);
	APP_AI_SetState(AI_UNKNOWN);

	//init facelib
	set_facelib_attr(&facelib_config);

	printf("init facelib\n");
	s32Ret = APP_InitFacelib(&facelib_config);
	if (s32Ret != CVI_SUCCESS) {
		DBG_PRINTF(CVI_ERROR, "CVI_Init_Video_Output failed with %d.\n", s32Ret);
		//printf("raise SIGINT to exit!\n");
		//raise(SIGINT);
		//return s32Ret;
		pthread_exit((void *)(intptr_t)s32Ret);
	}
	printf("init facelib success\n");
	CviLoadIdentify(facelib_handle);
	HAL_FACE_Create(&face);

	while (true)
	{
		if(APP_AI_GetState() == AI_UNKNOWN)
		{
			APP_AI_TaskHandler();
		}
		switch (APP_AI_GetState())
		{
			case AI_INIT:

				gettimeofday(&tv1, NULL);

				while(true)
				{
					s32Ret = APP_VpssGetFrame(APP_VIDEO_FRAME_FD_RGB, &stfdFrame, 1000);
					if (s32Ret == CVI_SUCCESS) {
						break;
                    }
					else
						SAMPLE_PRT("APP_VpssGetFrame grp0 chn0 failed with %#x\n", s32Ret);
				}

				#if defined(CONFIG_ALGORITHM_VENDOR_AISDK)
				if(get_register_status() == false) {
					APP_VpssGetFrame(APP_VIDEO_FRAME_LIVENESS_RGB, &stLivenessRGBFrame, 1000);
					if (s32Ret != CVI_SUCCESS) {
						SAMPLE_PRT("APP_VpssGetFrame grp5 chn1 failed with %#x\n", s32Ret);
						assert(0);
					}
				}
				#endif

				#if defined(CONFIG_DUAL_SENSOR_SUPPORT)
				while(true)
				{
					s32Ret = APP_VpssGetFrame(APP_VIDEO_FRAME_FD_IR, &stIrFrame, 1000);
					if (s32Ret == CVI_SUCCESS) {
						break;
					}
					else
						SAMPLE_PRT("APP_VpssGetFrame grp1 chn0 failed with %#x\n", s32Ret);
				}
				#endif
				gettimeofday(&tv2, NULL);

				#ifdef PERF_PROFILING
				elapsed = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
				total_elapsed += elapsed;	
				DBG_PRINTF(CVI_DEBUG, "[Timestamp] AI thread. 1. Get Frame cost: %ld us, total cost: %ld us\n", elapsed,total_elapsed);
				#endif

				if(msg_type&CMD_AI_FD)
					APP_AI_SetState(AI_FD);
				else
					APP_AI_SetState(AI_STOP);
				break;

			case AI_FD:

				DBG_PRINTF(CVI_INFO, "case AI Thread CVI_FD start\n");
				CVI_SYS_TraceBegin("Retina RGB");
				if (HAL_FACE_Detect(&stfdFrame, face, facelib_config.attr.min_face_h)) {
					msg_type = 0;
					APP_AI_SetState(AI_STOP);
					break;
				}

				gettimeofday(&tv1, NULL);
				CVI_SYS_TraceEnd();
				#ifdef PERF_PROFILING
				elapsed = (tv1.tv_sec - tv2.tv_sec) * 1000000 + tv1.tv_usec - tv2.tv_usec;
				total_elapsed += elapsed;
				DBG_PRINTF(CVI_DEBUG, "[Timestamp] AI thread. 2. FD cost: %ld us, total cost: %ld us\n", elapsed,total_elapsed);
				#endif

				VIDEO_FRAME_INFO_S stdrawFrame;
				stdrawFrame.stVFrame.u32Width = DISP_WIDTH;
				stdrawFrame.stVFrame.u32Height = DISP_HEIGHT;
				draw_face_meta(&stdrawFrame, face,2);
				int face_size = HAL_FACE_GetNum(face);

				#if defined(CONFIG_ISP_FACE_AE_SUPPORT)
				if(face_size > 0) {
					ISP_SMART_INFO_S stSmartInfo;
					setFaceAeInfo(1080, 1920, face, &stSmartInfo, face_size);
					CVI_ISP_SetSmartInfo(0, &stSmartInfo, 0);
					printf("face ae set, face_size = %d.\n", face_size);
				}
				#endif

				if(face_size > 0 &&jump_counter-- == 0) 
				{
					if(Get_CurrentLv() <= TURNONLEG_THRESHOLD) 
					{
						LED_OnOFF(true);
					}

					msg_type=msg_type&(~CMD_AI_FD);
					
					if(msg_type&CMD_AI_LIVENESS)
					{
						APP_AI_SetState(AI_LIVENESS);
					}
					else if(msg_type&CMD_AI_FR)
					{
						APP_AI_SetState(AI_FR);
					}
					else 
					{
						msg_type = 0;
						APP_AI_SetState(AI_STOP);
					}
				} else {
					msg_type = 0;
					APP_AI_SetState(AI_STOP);
				}
				gettimeofday(&tv2, NULL);
				#ifdef PERF_PROFILING
				elapsed = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
				total_elapsed += elapsed;	
				DBG_PRINTF(CVI_DEBUG, "[Timestamp] AI thread. 3. FD Post cost: %ld us, total cost: %ld us\n", elapsed,total_elapsed);
				#endif
				DBG_PRINTF(CVI_INFO, "case AI Thread CVI_FD end\n");

				break;

			case AI_LIVENESS:
				DBG_PRINTF(CVI_INFO, "case AI Thread AI_LIVENESS start\n");

				gui_hide_popmenu();
				jump_counter = 0;
				CVI_SYS_TraceBegin("liveness process");

				#if defined(CONFIG_ALGORITHM_VENDOR_AISDK)
				#if defined(CONFIG_DUAL_SENSOR_SUPPORT)
				s32Ret = HAL_FACE_LivenessDetect(&stLivenessRGBFrame, &stIrFrame, face);
				#else
				s32Ret = HAL_FACE_LivenessDetect(&stLivenessRGBFrame, &stLivenessRGBFrame, face);
				#endif
				#endif
				#if defined(CONFIG_ALGORITHM_VENDOR_MEGVII)
				#if defined(CONFIG_DUAL_SENSOR_SUPPORT)
				s32Ret = HAL_FACE_LivenessDetect(&stfdFrame, &stIrFrame, face);
				#else
				s32Ret = CvFaceLivenessDetectBgr(&stfdFrame, face);
				#endif
				#endif
				CVI_SYS_TraceEnd();
				if (s32Ret) {
					msg_type = 0;
					APP_AI_SetState(AI_STOP);
					break;
				}
				gettimeofday(&tv1, NULL);
				#ifdef PERF_PROFILING
				elapsed = (tv1.tv_sec - tv2.tv_sec) * 1000000 + tv1.tv_usec - tv2.tv_usec;
				total_elapsed += elapsed;
				DBG_PRINTF(CVI_DEBUG, "[Timestamp] AI thread. 4. Liveness cost: %ld us, total cost: %ld us\n", elapsed,total_elapsed);
				#endif

				int pass_num = 0;
				for (int i = 0; i < HAL_FACE_GetNum(face); ) {
					float liveness_score;
					HAL_FACE_GetLivenessScore(face, i, &liveness_score);
					DBG_PRINTF(CVI_DEBUG, "idx:%d  liveness score:%f\n", i, liveness_score);
					if (liveness_score >= facelib_config.attr.threshold_liveness) { // first check if live person
						++pass_num;
						++i;
					} else {
						HAL_FACE_DelFace(face, i);
					}
				}
				msg_type=msg_type&(~CMD_AI_LIVENESS);
				if (pass_num > 0) {
					if(msg_type&CMD_AI_FR)
						APP_AI_SetState(AI_FR);
					else
						APP_AI_SetState(AI_STOP);
				} else {
					msg_type = 0;
					APP_AI_SetState(AI_STOP);
				}
				DBG_PRINTF(CVI_INFO, "case AI Thread AI_LIVENESS end\n");
				break;

			case AI_FR:

				DBG_PRINTF(CVI_INFO, "case AI Thread CVI_FR start\n");

				CVI_SYS_TraceBegin("FR process");
				//printf("===[%s][%d]===\n", __func__, __LINE__);
				if (HAL_FACE_Recognize(&stfdFrame, face)) {
					msg_type = 0;
					APP_AI_SetState(AI_STOP);
					break;
				}
				CVI_SYS_TraceEnd();

				gettimeofday(&tv2, NULL);

				#ifdef PERF_PROFILING
				elapsed = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
				total_elapsed += elapsed;
				DBG_PRINTF(CVI_DEBUG, "[Timestamp] AI thread. 5. FR cost: %ld us, total cost: %ld us\n", elapsed,total_elapsed);
				#endif


				CVI_SYS_TraceBegin("FaceIdentify1vN");
				int matched_id = HAL_FACE_Identify1vN(facelib_handle,face,0.5);
				CVI_SYS_TraceEnd();

				gettimeofday(&tv1, NULL);

				#ifdef PERF_PROFILING
				elapsed = (tv1.tv_sec - tv2.tv_sec) * 1000000 + tv1.tv_usec - tv2.tv_usec;
				total_elapsed += elapsed;
				DBG_PRINTF(CVI_DEBUG, "[Timestamp] AI thread. 6. FR 1:1 cost: %ld us, total cost: %ld us\n", elapsed,total_elapsed);
				#endif



				if(matched_id >= 0) // there is matched person
				{
					// strncpy(strName, face.info[0].name, sizeof(face.info[0].name));
					// draw_face_meta(&stVideoFrame, &face,1); //green rect

					srand(time(NULL));
					memset(face_config.face_image,0,sizeof(face_config.face_image));
					sprintf(face_config.face_image,"record/%d.jpg",rand()%RECORD_CNT);
					HAL_FACE_Encode(&stfdFrame,face,NULL,face_config.face_image);

					face_config.face_id = matched_id;
					face_config.handle = facelib_handle;
					job_t *job = (job_t *)malloc(sizeof(job_t));
					job->job_function = post_job_function;
					job->data = (void *)&face_config;
					thread_pool_add(&pool, job);
					jump_counter = JUMP_FD_NUM;

				}
				else // not matched
				{
					// draw_face_meta(&stVideoFrame, face,0); //red rect
					// if(verify_fail_counter++==fd_control_config.face_verify_fail_time)
					{
						gui_show_popmenu(NULL, NULL, _("Verify Fail"), false,0,0);
						jump_counter = JUMP_FD_NUM;
					}
				}

				gettimeofday(&tv2, NULL);
				#ifdef PERF_PROFILING
				elapsed = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
				total_elapsed += elapsed;
				DBG_PRINTF(CVI_DEBUG, "[Timestamp] AI thread. 7. FR Post cost: %ld us, total cost: %ld us\n", elapsed,total_elapsed);
				#endif

				msg_type=msg_type&(~CMD_AI_FR);
				APP_AI_SetState(AI_STOP);
				DBG_PRINTF(CVI_INFO, "case AI Thread CVI_FR end\n");

				break;

			case AI_STOP:
				if (APP_GetFreezeStatus()) {
					HAL_FACE_Encode(&stfdFrame,NULL,"/tmp/register.jpg",NULL);//encode image
					APP_SetFreezeStatus(false);
                }

				s32Ret = APP_VpssReleaseFrame(APP_VIDEO_FRAME_FD_RGB, &stfdFrame);
                if (s32Ret != CVI_SUCCESS) {
                    SAMPLE_PRT("APP_VpssReleaseFrame chn0 NG\n");
                    return NULL;
				}	
				#if defined(CONFIG_DUAL_SENSOR_SUPPORT)
				s32Ret = APP_VpssReleaseFrame(APP_VIDEO_FRAME_FD_IR, &stIrFrame);
				if (s32Ret != CVI_SUCCESS) {
					SAMPLE_PRT("APP_VpssReleaseFrame chn3 NG\n");
					assert(0);
				}
				#endif

				#if defined(CONFIG_ALGORITHM_VENDOR_AISDK)
				if(get_register_status() == false) {
					s32Ret = APP_VpssReleaseFrame(APP_VIDEO_FRAME_LIVENESS_RGB, &stLivenessRGBFrame) ;
					if (s32Ret != CVI_SUCCESS) {
						SAMPLE_PRT("APP_VpssReleaseFrame chn2 NG\n");
						return NULL;
					}
				}
				#endif

				gettimeofday(&tv1, NULL);
				#ifdef PERF_PROFILING
				elapsed = (tv1.tv_sec - tv2.tv_sec) * 1000000 + tv1.tv_usec - tv2.tv_usec;
				total_elapsed += elapsed;
				DBG_PRINTF(CVI_DEBUG, "[Timestamp] AI thread. 8. Relase frame cost: %ld us, total cost: %ld us\n", elapsed,total_elapsed);
				DBG_PRINTF(CVI_DEBUG, "[Timestamp] AI thread. AI fps = %ld\n", 1000000/total_elapsed);
				total_elapsed = 0;
				elapsed = 0;
				#endif

				HAL_FACE_Clear(face);
				APP_AI_SetState(AI_UNKNOWN);
				break;
			case AI_UNKNOWN:
				break;
			default:
				break;
		}

	}
	HAL_FACE_Free(face);
	HAL_FACE_LibClose();
}
