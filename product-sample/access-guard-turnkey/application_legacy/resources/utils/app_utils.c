#include "sys.h"
#include "audio.h"
#include "cvi_hal_gpio.h"
#include <pthread.h>

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <assert.h>
#include <sys/prctl.h>

#include "lv_i18n.h"

#include "app_utils.h"
#include "factory.h"
#include "cviconfig.h"
#include "cviconfigsystem.h"
#include "net.h"
#include "app_init.h"
#include "../audio/cvi_audio_main.h"
#include "public.h"

CVI_LOG_LEVEL_E dbg_level =  CVI_DEBUG;

#if 0 //def PERF_PROFILING
static struct timeval tv1,tv2;
static long elapsed = 0;
static long total_elapsed = 0;
#endif
#define RECORD_PATH "./record.db"

thread_pool_t pool;
hal_facelib_config_t facelib_config;

static pthread_mutex_t mutex_register = PTHREAD_MUTEX_INITIALIZER;

bool register_status = false;


CVI_STATE_E state_machine = CVI_INIT;

static pthread_mutex_t mutex_sm = PTHREAD_MUTEX_INITIALIZER;


int sys_init(void)
{

    #if 0 //test code
	printf ("/mnt/data  total=%dMB, free=%dMB\n", get_partition_total_size("/mnt/data"), get_partition_available_size("/mnt/data"));  
	// mac addr
	unsigned char macaddr[6];
	get_mac("eth0",macaddr);
	printf("mac addr:\n");
	for(int i=0;i<6;i++)
			printf("%x:",(unsigned char)macaddr[i]);

	printf("\n");
	//get current time
	get_time();
	//get bootup time
	print_system_boot_time();
    #endif

    //audio_init();

    if (0 != thread_pool_init(&pool, 2))
	{
        assert(0&&"create thead pool fail\n");
    }

// #ifndef CONFIG_CVI_SOC_CV182X
//     CVI_Compress_Log_File();
//     CVI_Copy_Log_To_Udisk();
// #endif

    return 0;
}


void sys_exit()
{
	//audio_exit();
}


static int access_guard_open(void)
{
    #ifdef ENABLE_PERI
    CVI_HAL_GpioSetValue(RELAY_GPIO,);//relay
    usleep(1000);
    CVI_HAL_GpioSetValue(RELAY_GPIO, CVI_HAL_GPIO_VALUE_L);//relay
    #endif
	return 0;
}


int verified_control(cvi_person_t person,  char *strName, char *face_img)
{
    int ret = 0;
    //static pthread_t tid = 0;

    // 0. play sound
    //msg_send(pst_audio_msgq,AUDIO_MSG_PLAY_TONE,NULL,0,NULL,0);
	//audio_play_single_tone();
    // 1. add to record
    cvi_record_ctx_t ctx = {0};
    if (0 > cvi_hal_record_open(&ctx, RECORD_PATH)) {
        printf("repo open fail\n");
        return -1;
    }

    cvi_record_t record = {0};
    snprintf(record.name, sizeof(record.name), "%s", strName);
    snprintf(record.serial, sizeof(record.serial), "%s", person.serial);
    snprintf(record.ic_card, sizeof(record.ic_card), "%s", person.ic_card);
	record.remote_id = person.remote_id;
	snprintf(record.type, sizeof(record.type), "%s", person.type);
    record.verification_type = 1;
    record.temperature = 36.5;
	record.timestamp = get_ms_timestamp();
    snprintf(record.image_path, sizeof(record.image_path), "%s", face_img);

    if (0 > cvi_hal_record_add(&ctx, &record)) {
        printf("cvi_record_add fail\n");
        return -1;
    }

    cvi_hal_record_close(&ctx);

    // 2. access guard open
    access_guard_open();

    return ret;
}

void set_facelib_attr(hal_facelib_config_t *facelib_config)
{
	memset(facelib_config,0,sizeof(hal_facelib_config_t));
	#if defined(CONFIG_ALGORITHM_VENDOR_AISDK)
    facelib_config->repo_path = "./TestRepo";
    facelib_config->attr.threshold_1v1 = 0.65;
    facelib_config->attr.threshold_1vN = 0.65;
    facelib_config->attr.threshold_facereg = 0.65;
    facelib_config->attr.pitch = 10;
    facelib_config->attr.yaw = 10;
    facelib_config->attr.roll = 10;
    facelib_config->attr.face_quality = 0;//?
    facelib_config->attr.face_pixel_min = 0;//?
    facelib_config->attr.light_sens = 0;//?
    facelib_config->attr.move_sens = 0;//?
    facelib_config->attr.threshold_liveness = 0.9;
    facelib_config->attr.wdr_en = 0;
	facelib_config->attr.min_face_h = 60;
	facelib_config->yolo_en = 0;
	facelib_config->fd_en = 1;
	facelib_config->facereg_en = 0;
	facelib_config->face_matching_en = 0;
	facelib_config->config_yolo = 0;
    facelib_config->is_face_quality = 1;
	facelib_config->config_liveness = 1;
	facelib_config->model_face_fd = "cvimodel/retinaface_mnet0.25_608_342.cvimodel";
    facelib_config->model_face_quality = "cvimodel/fqnet-v5_shufflenetv2-softmax.cvimodel";

	//facelib_config->model_face_extr = "cvimodel/cviface-v3-attribute.cvimodel";
	facelib_config->model_face_extr = "cvimodel/cviface-v5-s.cvimodel";
	facelib_config->model_face_liveness = "cvimodel/liveness-rgb-ir.cvimodel";

	facelib_config->model_yolo3 = "cvimodel/yolo_v3_320_int8_lw_memopt.cvimodel";
	facelib_config->db_repo_path = "feature.db";
	#endif

	#if defined(CONFIG_ALGORITHM_VENDOR_MEGVII)
    facelib_config->repo_path = "./TestRepo";
    facelib_config->attr.threshold_1v1 = 0.65;
    facelib_config->attr.threshold_1vN = 0.65;
    facelib_config->attr.threshold_facereg = 0.65;
    facelib_config->attr.threshold_det = 0.3;
    facelib_config->attr.pitch = 10;
    facelib_config->attr.yaw = 10;
    facelib_config->attr.roll = 10;
    facelib_config->attr.face_quality = 0;//?
    facelib_config->attr.face_pixel_min = 0;//?
    facelib_config->attr.light_sens = 0;//?
    facelib_config->attr.move_sens = 0;//?
    facelib_config->attr.threshold_liveness = 0.9;
    facelib_config->attr.wdr_en = 0;
    facelib_config->attr.min_face_h = 60;
	facelib_config->yolo_en = 0;
	facelib_config->fd_en = 1;
	facelib_config->facereg_en = 0;
	facelib_config->face_matching_en = 0;
	facelib_config->config_yolo = 0;
    facelib_config->is_face_quality = 0;
	facelib_config->config_liveness = 1;
	facelib_config->model_face_fd = "cvimodel/cv1835_detect_A.bin";
    facelib_config->model_face_quality = "";

	//facelib_config->model_face_extr = "cvimodel/cviface-v3-attribute.cvimodel";
	facelib_config->model_face_extr = "cvimodel/cv1835_feat_N.bin";
	facelib_config->model_face_stn = "cvimodel/cv1835_preFeat_N.bin";
	facelib_config->model_face_liveness = "cvimodel/cv1835_live_rgbir_A01.bin";
	facelib_config->model_face_liveness_bgr = "cvimodel/cv1835_live_rgb_A.bin";
	facelib_config->model_face_anchor = "cvimodel/cv1835_anchor_A.bin";
    facelib_config->model_face_attr         = "";
    facelib_config->model_face_landmark     = "cvimodel/cv1835_landmark_A.bin";
    facelib_config->model_face_poseblur     = "";
    facelib_config->model_face_postfilter   = "cvimodel/cv1835_postfilter_A.bin";
    facelib_config->model_face_search   = "cvimodel/cv1835_search_A.bin";

    facelib_config->model_yolo3 = "";
	facelib_config->db_repo_path = "feature.db";
	facelib_config->face_lib_path = "./facelib/libfacepass_wrapper.so";
	#endif
}

void *post_job_function(void *arg)
{
	// printf("test function %d \n", ((job_t *)arg)->data);
	CVI_S32 s32Ret = CVI_SUCCESS;

	DBG_PRINTF(CVI_INFO ,"test test_job_function\n");

	face_process_t *face_config = ((job_t *)arg)->data;

	cvi_person_t person;
	Cvi_person_Info(face_config->handle, face_config->face_id, &person);

    int width ;
    int height ;
	s32Ret = Cvi_Jpg2PNG(person.image_path,"/tmp/capture.png",&width,&height);
	if(CVI_SUCCESS == s32Ret)
	{
		gui_show_popmenu("/tmp/capture.png", person.name, _("Verify Pass"), true,width,height);
		verified_control(person, person.name, face_config->face_image);
	}

	

	#if 0 //def PERF_PROFILING
	gettimeofday(&tv1, NULL);
	elapsed = (tv1.tv_sec - tv2.tv_sec) * 1000000 + tv1.tv_usec - tv2.tv_usec;
	// total_elapsed += elapsed;
	DBG_PRINTF(CVI_INFO, "[Timestamp] 6.Periferal control cost: %ld us\n", elapsed);
	#endif

	return NULL;
}

CVI_STATE_E get_app_comp_sm()
{
    return state_machine;
}

void set_app_comp_sm(CVI_STATE_E state)
{
    pthread_mutex_lock(&mutex_sm);
    state_machine = state;
    pthread_mutex_unlock(&mutex_sm);
}


bool get_register_status()
{
	return register_status;
}

void set_register_status(bool state)
{
	pthread_mutex_lock(&mutex_register);
	register_status = state;
	pthread_mutex_unlock(&mutex_register);
}

int setFaceAeInfo(int frame_width,int frame_height,hal_face_t face_meta, ISP_SMART_INFO_S *pstSmartInfo,int face_num)
{
    hal_face_bbox_t rect;
	HAL_FACE_RectRescale(frame_width, frame_height, face_meta, 0, true, &rect);

    int x1,x2,y1,y2;
	x1 = rect.x1;
	x2 = rect.x2;
	y1 = rect.y1;
	y2 = rect.y2;
 
    // because access guard solution rotate vi 90 degree, so x =y ;y = x;height = width;width = height
    #if defined(CONFIG_VI_ROTATION_270)
	pstSmartInfo->stROI[0].u16FrameWidth = frame_height;
	pstSmartInfo->stROI[0].u16FrameHeight = frame_width;
	pstSmartInfo->stROI[0].u8Num = 1;
	pstSmartInfo->stROI[0].u16PosX[0] = pstSmartInfo->stROI[0].u16FrameWidth - y2;
	pstSmartInfo->stROI[0].u16PosY[0] = x1;
	pstSmartInfo->stROI[0].u16Width[0] = (y2-y1);
	pstSmartInfo->stROI[0].u16Height[0] = (x2-x1);
	pstSmartInfo->stROI[0].bEnable = 1;
    #elif defined(CONFIG_VI_ROTATION_90)
	pstSmartInfo->stROI[0].u16FrameWidth = frame_height;
	pstSmartInfo->stROI[0].u16FrameHeight = frame_width;
	pstSmartInfo->stROI[0].u8Num = 1;
	pstSmartInfo->stROI[0].u16PosX[0] = y1;
	pstSmartInfo->stROI[0].u16PosY[0] = pstSmartInfo->stROI[0].u16FrameHeight - x2;
	pstSmartInfo->stROI[0].u16Width[0] = (y2-y1);
	pstSmartInfo->stROI[0].u16Height[0] = (x2-x1);
	pstSmartInfo->stROI[0].bEnable = 1;
	#else
		error ...
	#endif
    return 0;
}

static void *net_init_thread(void *p)
{
	//char mac_address[32];
    char ip_address[32];
    char netmask[32];
    //char broadcast_address[32];
	char gateway[32];
	//char dns[32];
	int net_mode;
	char command[128] = {0};
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	struct wlan_cfg cfg;
	cvi_device_config_t device_cfg2;
	int web_onoff = 0;

	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	sleep(1);

	//cvi_get_device_config(&device_cfg2);
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	cvi_get_device_config_int(CONF_KEY_NET_MODE, &net_mode);
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	cvi_get_device_config_str(CONF_KEY_NET_IP, ip_address, sizeof(ip_address));
	cvi_get_device_config_str(CONF_KEY_NET_MASK, netmask, sizeof(netmask));
	cvi_get_device_config_str(CONF_KEY_GATEWAY, gateway, sizeof(gateway));

	cvi_get_device_config_int(CONF_KEY_WIFI_ONOFF, &device_cfg2.wifi_onoff);
	cvi_get_device_config_str(CONF_KEY_WIFI_SSID, device_cfg2.wifi_ssid, sizeof(device_cfg2.wifi_ssid));
	cvi_get_device_config_str(CONF_KEY_WIFI_PWD, device_cfg2.wifi_pwd, sizeof(device_cfg2.wifi_pwd));
	cvi_get_device_config_system_int(CONF_KEY_WEB_ONOFF, &web_onoff);

	if(0 == net_mode)
	{
		snprintf(command, sizeof(command), "killall udhcpc");
		system(command);
		printf("[command:%s %s,%d]\n",command,__FUNCTION__,__LINE__);

		sleep(1);
		memset(command, 0x00, sizeof(command));
	    snprintf(command, sizeof(command), "ifconfig eth0 %s netmask %s", ip_address, netmask);
	    system(command);
		printf("[command:%s %s,%d]\n",command,__FUNCTION__,__LINE__);
		sleep(1);
		memset(command, 0x00, sizeof(command));
	    snprintf(command, sizeof(command), "route add default gw %s", gateway);
		printf("[command:%s %s,%d]\n",command,__FUNCTION__,__LINE__);
	    system(command);
	}
	else
	{
		snprintf(command, sizeof(command), "udhcpc -b -i eth0 -R &");
	    system(command);
	}

	if(1 == device_cfg2.wifi_onoff)
	{
		system("killall wpa_supplicant &");
		sleep(3);
		CVI_NET_Wlan_Init(WLAN0_PORT);

		sleep(3);
		//sprintf(cfg.ssid,"%s",device_cfg2.wifi_ssid);
		//sprintf(cfg.passwd,"%s",device_cfg2.wifi_pwd);

		snprintf(cfg.ssid, sizeof(cfg.ssid), "%s", device_cfg2.wifi_ssid);
		snprintf(cfg.passwd, sizeof(cfg.passwd), "%s", device_cfg2.wifi_pwd);
		printf("[cfg.ssid:%s%s,%d]\n",cfg.ssid,__FUNCTION__,__LINE__);
		if(strlen(cfg.passwd) >= 8)
		{
			CVI_NET_Set_Wlan_Config(WLAN0_PORT, STATION, &cfg);
		}

		printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	}

	if(1 == web_onoff)
	{
		system("./webserver.sh &");
	}

	pthread_exit(NULL);

}

static pthread_t net_init_id;

int net_init(void)
{

    int res;

	/* Production test thread */
	res = pthread_create(&net_init_id, 0, net_init_thread, 0);
	if (res)
		CVI_SYSLOG(CVI_DBG_ERR, "%s", strerror(res));
	pthread_detach(net_init_id);

    return 0;
}


void setLogLevel(CVI_LOG_LEVEL_E level)
{
    dbg_level = level;
}