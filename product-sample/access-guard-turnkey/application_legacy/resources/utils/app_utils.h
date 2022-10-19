#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <syslog.h>
#include <sys/time.h>
#include "cvi_hal_db_repo.h"
#include "threadpool.h"
#include "cvi_hal_record.h"
#include "cvi_hal_face_api.h"
#include "app_comp.h"
#include "app_ai.h"
#include "app_video.h"
#include "draw_helper.h"
#include "app_peripheral.h"
// #include "tcli.h"
#include "msgq.h"
// #define PERF_PROFILING

#define ENABLE_WDR

#define DISP_WIDTH		(CONFIG_PANEL_DISPLAY_HOR)
#define DISP_HEIGHT		(CONFIG_PANEL_DISPLAY_VER)

#ifdef CV182X
#define RECORD_CNT 500
#else
#define RECORD_CNT 10000
#endif

#if defined(CONFIG_VPSS_VIDEO_FLIP_SUPPORT)
#define SUPPORT_FLIP CVI_TRUE
#else
#define SUPPORT_FLIP CVI_FALSE
#endif

#if defined(CONFIG_VPSS_VIDEO_MIRROR_SUPPORT)
#define SUPPORT_MIRROR CVI_TRUE
#else
#define SUPPORT_MIRROR CVI_FALSE
#endif

#define ENABLE_REFINE_VB
//#define CLI_CMD_DEBUG 1

#define FilePath "./face/"

/*
*/
typedef enum _CVI_LOG_LEVEL_E {
  CVI_ERROR,    //
	CVI_WARNING,
  CVI_DEBUG,
	CVI_INFO,
}CVI_LOG_LEVEL_E;
/**/
extern CVI_LOG_LEVEL_E dbg_level;
/**/
void setLogLevel(CVI_LOG_LEVEL_E level);
/**/
#define DBG_PRINTF(level,fmt,args...) \
  do { \
    if(dbg_level >= level) { \
        printf(fmt,##args); \
        syslog(level, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args); \
    } \
  } while (0)
/**/
#define CVI_SYSLOG(level, fmt,args...)                                                                                   \
	do {                                                                                                           \
		syslog(level, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args);                        \
	} while (0)
/*
*/
typedef enum _CVI_STATE_E {
    CVI_INIT,    //
	CVI_FD,
	CVI_FR,
	CVI_LIVENESS,
	CVI_VO,
	CVI_STOP,      //
	CVI_UNKNOW,
	CVI_GBR_INIT,
	CVI_IR_INIT
}CVI_STATE_E;
/*
*/
typedef struct {
	VIDEO_FRAME_INFO_S stfrFrame;
	hal_face_t face;
} verify_pass_config_t;


typedef struct {
	hal_facelib_handle_t handle;
	int face_id;
	char face_image[50];
} face_process_t;

extern hal_facelib_config_t facelib_config;

#ifdef CLI_CMD_DEBUG
extern TELNET_CLI_S_COMMAND utils_app_cli_cmd_list[];
#endif
extern thread_pool_t pool;

int sys_init(void);
int net_init(void);
void sys_exit();
// void verified_control(VIDEO_FRAME_INFO_S stfrFrame, cvai_face_t face);
int verified_control(cvi_person_t person,  char *strName, char *face_img);

void set_facelib_attr(hal_facelib_config_t *facelib_config);

void reset_video_status(void);
void *post_job_function(void *arg);
void sys_exit(void);
CVI_STATE_E get_app_comp_sm();

void set_app_comp_sm(CVI_STATE_E state);

bool get_register_status();
void set_register_status(bool state);

int setFaceAeInfo(int frame_width,int frame_height,hal_face_t face_meta, ISP_SMART_INFO_S *pstSmartInfo,int face_num);
unsigned long long get_ms_timestamp();


#endif
