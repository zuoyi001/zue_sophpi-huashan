#include <fcntl.h>
#include "app_utils.h"
#include "app_video.h"
#include "mipi_tx.h"


VO_PUB_ATTR_S stVoPubAttr;
static SAMPLE_VI_CONFIG_S stViConfig;

static pthread_mutex_t mutex_video = PTHREAD_MUTEX_INITIALIZER;
bool freeze_status = false;
bool flagSetVpssAttr = false;

static CVI_S32 _SetViConfigFromIni(SAMPLE_VI_CONFIG_S* pstViConfig)
{
	MMF_VERSION_S stVersion;
	SAMPLE_INI_CFG_S	   stIniCfg = {0};

	CVI_S32 s32Ret = CVI_SUCCESS;
	LOG_LEVEL_CONF_S log_conf;

	stIniCfg = (SAMPLE_INI_CFG_S) {
		.enSource  = VI_PIPE_FRAME_SOURCE_DEV,
		.devNum    = 2,
		.enSnsType[0] = SONY_IMX327_2L_MIPI_2M_30FPS_12BIT,
		.enWDRMode[0] = WDR_MODE_NONE,
		.s32BusId[0]  = 3,
		.MipiDev[0]   = 0xff,
		.enSnsType[1] = SONY_IMX327_SLAVE_MIPI_2M_30FPS_12BIT,
		.s32BusId[1] = 0,
		.MipiDev[1] = 0xff,
	};

	CVI_SYS_GetVersion(&stVersion);
	SAMPLE_PRT("MMF Version:%s\n", stVersion.version);

	log_conf.enModId = CVI_ID_LOG;
	log_conf.s32Level = CVI_DBG_INFO;
	CVI_LOG_SetLevelConf(&log_conf);

	// Get config from ini if found.
	if (SAMPLE_COMM_VI_ParseIni(&stIniCfg)) {
		SAMPLE_PRT("Parse complete\n");
	}

	//Set sensor number
	CVI_VI_SetDevNum(stIniCfg.devNum);

		/************************************************
	 * step1:  Config VI
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_IniToViCfg(&stIniCfg, pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		return s32Ret;
	}
	return s32Ret;
}

#if defined(CONFIG_VIDEO_RTSP_SUPPORT)
static void _VencInitInputCfg(chnInputCfg *ipIc)
{
    ipIc->rcMode = -1;
    ipIc->iqp = -1;
    ipIc->pqp = -1;
    ipIc->gop = -1;
    ipIc->bitrate = -1;
    ipIc->firstFrmstartQp = -1;
    ipIc->num_frames = -1;
    ipIc->framerate = 30;
    ipIc->maxQp = -1;
    ipIc->minQp = -1;
    ipIc->maxIqp = -1;
    ipIc->minIqp = -1;
}
chnInputCfg pIc[2]; //for venc

CVI_S32 APP_InitVenc(void)
{
        CVI_S32 s32Ret = CVI_SUCCESS;
        VENC_CHN VencChn[2] = {0, 1};
        PAYLOAD_TYPE_E enPayLoad = PT_H264;
        PIC_SIZE_E enSize = PIC_720P;
        VENC_GOP_MODE_E enGopMode = VENC_GOPMODE_NORMALP;
        VENC_GOP_ATTR_S stGopAttr;
        SAMPLE_RC_E enRcMode;
        CVI_U32 u32Profile = 0;
	CVI_U32 i;

	for(i = 0; i < sizeof(pIc)/sizeof(chnInputCfg); i++){
		memset(&pIc[i],0,sizeof(chnInputCfg));
		_VencInitInputCfg(&pIc[i]);
		strcpy(pIc[i].codec, "264");
		pIc[i].rcMode = 0; // cbr
		pIc[i].iqp = 38;
		pIc[i].pqp = 38;
		pIc[i].gop = 60;
		pIc[i].bitrate = 1024; // if fps = 20
		pIc[i].firstFrmstartQp = 50;
		pIc[i].num_frames = -1;
		pIc[i].framerate = 25;
		pIc[i].maxQp = 50;
		pIc[i].minQp = 20;
		pIc[i].maxIqp = 44;
		pIc[i].minIqp = 24;
		pIc[i].single_LumaBuf = 1;
		pIc[i].bitstreamBufSize = 2764800;
	}

    enRcMode = (SAMPLE_RC_E) pIc[0].rcMode;

    s32Ret = SAMPLE_COMM_VENC_GetGopAttr(enGopMode, &stGopAttr);
    if (s32Ret != CVI_SUCCESS) {
        printf("[Err]Venc Get GopAttr for %#x!\n", s32Ret);
        return CVI_FAILURE;
    }

	for(i = 0; i < sizeof(VencChn)/sizeof(VENC_CHN); i++) {
		pIc[i].bind_mode = VENC_BIND_VPSS,
		pIc[i].vpssGrp = i,
		pIc[i].vpssChn = 1,
		s32Ret = SAMPLE_COMM_VENC_Start(
				&pIc[i],
				VencChn[i],
				enPayLoad,
				enSize,
				enRcMode,
				u32Profile,
				CVI_FALSE,
				&stGopAttr);
		if (s32Ret != CVI_SUCCESS) {
			printf("[Err]Venc%d Start failed for %#x!\n", i, s32Ret);
			return CVI_FAILURE;
		}
	}

    return s32Ret;
}
#endif

int APP_PanelOff()
{
	int fd;
	int ret = 0;
	fd = open("/dev/cvi-mipi-tx", O_RDWR | O_NONBLOCK, 0);
	if (fd == -1) {
		SAMPLE_PRT("Cannot open mipi-tx dev error: %d, %s\n", errno, strerror(errno));
		assert(0);
		return -1;
	}
	mipi_tx_disable(fd);
	uint8_t cmd[3]={0x28,0x0};
	struct cmd_info_s cmd_info = { .devno = 0, .cmd_size = 2
		, .data_type = 0x15, .cmd = (void *)cmd};
	ret = mipi_tx_send_cmd(fd, &cmd_info);
	if (ret) {
		SAMPLE_PRT("panel_off fail.\n");
		assert(0);
		return ret;
	}
	return ret;
}

void APP_PanelOn()
{
	int fd;
	int ret;
	fd = open("/dev/cvi-mipi-tx", O_RDWR | O_NONBLOCK, 0);
	if (fd == -1) {
		SAMPLE_PRT("Cannot open mipi-tx dev error: %d, %s\n", errno, strerror(errno));
		return;
	}
	mipi_tx_disable(fd);
	uint8_t cmd[3]={0x29,0x0};
	struct cmd_info_s cmd_info = { .devno = 0, .cmd_size = 2
		, .data_type = 0x15, .cmd = (void *)cmd};
	ret = mipi_tx_send_cmd(fd, &cmd_info);
	if (ret) {
		SAMPLE_PRT("dsi panel_on fail.\n");
		return;
	}
	mipi_tx_enable(fd);
}

CVI_S32 APP_VideoRelease()
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	//CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	SAMPLE_COMM_All_ISP_Stop();
	SAMPLE_COMM_VI_DestroyVi(&stViConfig);
	s32Ret = SAMPLE_COMM_VI_CLOSE();
	if(s32Ret != CVI_SUCCESS) 
	{
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi close failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}
	//unbind vi ch0->vpss grp0
	SAMPLE_COMM_VI_UnBind_VPSS(0, 0, 0);
	//unbind vi ch1->vpss grp1
	SAMPLE_COMM_VI_UnBind_VPSS(0, 1, 1);
	#if defined(CONFIG_VIDEO_RTSP_SUPPORT)
	//unbind vpss grp0 ch1->venc ch0
	SAMPLE_COMM_VPSS_UnBind_VENC(0, 1, 0);
	//unbind vpss grp1 ch1->venc ch1
	SAMPLE_COMM_VPSS_UnBind_VENC(1, 1, 1);
	SAMPLE_COMM_VENC_Stop(0);
	SAMPLE_COMM_VENC_Stop(1);
	#endif
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}

#if 0
CVI_S32 APP_SamplePlatVoInit(void)
{
	RECT_S stDefDispRect  = {0, 0, 720, 1280};
	SIZE_S stDefImageSize = {720, 1280};
	CVI_S32 s32Ret = CVI_SUCCESS;

	stDefDispRect.u32Width = stVoLayerAttr.stDispRect.u32Width;
	stDefDispRect.u32Height = stVoLayerAttr.stDispRect.u32Height;
	stDefImageSize.u32Width = stVoLayerAttr.stDispRect.u32Width;
	stDefImageSize.u32Height = stVoLayerAttr.stDispRect.u32Height;

	s32Ret = SAMPLE_COMM_VO_GetDefConfig(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VO_GetDefConfig failed with %#x\n", s32Ret);
		return s32Ret;
	}
	
	stVoConfig.VoDev	 = 0;
	stVoConfig.stVoPubAttr.enIntfType  = VO_INTF_MIPI;
	stVoConfig.stVoPubAttr.enIntfSync  = VO_OUTPUT_720x1280_60;
	stVoConfig.stDispRect	 = stDefDispRect;
	stVoConfig.stImageSize	 = stDefImageSize;
	stVoConfig.enPixFormat	 = SAMPLE_PIXEL_FORMAT;
	stVoConfig.enVoMode	 = VO_MODE_1MUX;
	s32Ret = SAMPLE_COMM_VO_StartVO(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StartVO failed with %#x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}
#endif

CVI_S32 APP_VideoInit(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 VoLayer = 0;
	CVI_U32 VoChn = 0;
	// CVI_U32 panel_init = false;
	//start sys & isp(vi)
	#if defined(CONFIG_DUAL_SENSOR_SUPPORT)
	//CVI_Set_VI_Config_dual(&stViConfig);
	#else
	CVI_set_VI_config(&stViConfig);
	#endif
	_SetViConfigFromIni(&stViConfig);

	if(stViConfig.s32WorkingViNum == 2) {
		//set dual sensor.
		CVI_VI_SetDevNum(2);
	}

	stViConfig.bViRotation = true;
	s32Ret = APP_InitVideoInput(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Input failed with %d\n", s32Ret);
		return s32Ret;
	}

	if(stViConfig.s32WorkingViNum == 2) {
		//set IR sensor mono mode.
		ISP_MONO_ATTR_S monoAttr;
		monoAttr.Enable = true;
        //sensor 0 is IR sensor
		CVI_ISP_SetMonoAttr(0, &monoAttr);
	}


	s32Ret = APP_InitVpss();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_Init_Video_Process Grp1 failed with %d\n", s32Ret);
		return s32Ret;
	}

	//vo config & start vo
	//s32Ret = APP_SamplePlatVoInit();
	s32Ret = SAMPLE_PLAT_VO_INIT();
	#if defined(CONFIG_VO_ROTATION_270)
	ROTATION_E	enRotation = ROTATION_270;
	#elif defined(CONFIG_VO_ROTATION_180)
	ROTATION_E	enRotation = ROTATION_180;
	#elif defined(CONFIG_VO_ROTATION_90)
	ROTATION_E	enRotation = ROTATION_90;
	#elif defined(CONFIG_VO_ROTATION_NONE)
	ROTATION_E	enRotation = ROTATION_0;
	#endif
	CVI_VO_SetChnRotation(VoLayer, VoChn, enRotation);
	CVI_VO_HideChn(VoLayer,VoChn);

	return s32Ret;
}

void APP_SetIrVpssAspect(CVI_S32 vpsschn,CVI_S32 x,CVI_S32 y, CVI_S32 width, CVI_S32 height, PIXEL_FORMAT_E pixelFormat)
{
	VPSS_CHN_ATTR_S stChnAttr;
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_VPSS_DisableChn(2, vpsschn);

	CVI_VPSS_GetChnAttr(2, vpsschn, &stChnAttr);
	stChnAttr.stAspectRatio.stVideoRect.s32X = x;
	stChnAttr.stAspectRatio.stVideoRect.s32Y = y;
	stChnAttr.stAspectRatio.stVideoRect.u32Width = width;
	stChnAttr.stAspectRatio.stVideoRect.u32Height = height;

	stChnAttr.u32Width                    = width;
	stChnAttr.u32Height                   = height;
	stChnAttr.enVideoFormat               = VIDEO_FORMAT_LINEAR;
	stChnAttr.enPixelFormat               = pixelFormat;
	stChnAttr.stAspectRatio.enMode        = ASPECT_RATIO_MANUAL;


	CVI_VPSS_SetChnAttr(2, vpsschn, &stChnAttr);
	s32Ret = CVI_VPSS_EnableChn(2, vpsschn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_EnableChn is fail\n");
		//return s32Ret;
	}
}

bool APP_GetFreezeStatus()
{
	return freeze_status;
}

void APP_SetFreezeStatus(bool state)
{
	pthread_mutex_lock(&mutex_video);
	freeze_status = state;
	pthread_mutex_unlock(&mutex_video);
}

CVI_S32 CVI_APP_SetVpssAttr(void)
{
	VPSS_CHN_ATTR_S stChnAttr;
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_VPSS_DisableChn(1, 0);
	CVI_VPSS_GetChnAttr(1, 0, &stChnAttr);

	stChnAttr.u32Width = 1080;
	stChnAttr.u32Height = 1920;
	stChnAttr.enPixelFormat = VI_PIXEL_FORMAT;
	stChnAttr.bMirror = CVI_FALSE;
	stChnAttr.stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
	stChnAttr.stAspectRatio.stVideoRect.s32X = 160;
	stChnAttr.stAspectRatio.stVideoRect.s32Y = 310;
	stChnAttr.stAspectRatio.stVideoRect.u32Width = 755;
	stChnAttr.stAspectRatio.stVideoRect.u32Height = 1470;

	s32Ret = SAMPLE_COMM_VI_UnBind_VPSS(0, 0, 5);
	if(s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VI_UnBind_VPSS failed with %#x\n", s32Ret);
		CVI_VPSS_EnableChn(1, 0);
		return s32Ret;
	}
	flagSetVpssAttr = true;

	CVI_VPSS_SetChnAttr(1, 0, &stChnAttr);
	CVI_VPSS_EnableChn(1, 0);

	s32Ret = SAMPLE_COMM_VPSS_Bind_VPSS(1, 0, 5);
	if(s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VPSS_Bind_VPSS failed with %#x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

void CVI_APP_RecoverVpssAttr(void)
{
	VPSS_CHN_ATTR_S stChnAttr;

	SAMPLE_COMM_VPSS_UnBind_VPSS(1, 0, 5);
	if(flagSetVpssAttr == true) {
        SAMPLE_COMM_VI_Bind_VPSS(0, 0, 5);
        flagSetVpssAttr = false;
    }

	CVI_VPSS_DisableChn(1, 0);
	CVI_VPSS_GetChnAttr(1, 0, &stChnAttr);

	stChnAttr.u32Width = 270;
	stChnAttr.u32Height = 480;
	stChnAttr.enPixelFormat = PIXEL_FORMAT_BGR_888;
	stChnAttr.bMirror = CVI_TRUE;
	stChnAttr.stAspectRatio.enMode = ASPECT_RATIO_AUTO;

	CVI_VPSS_SetChnAttr(1, 0, &stChnAttr);
	CVI_VPSS_EnableChn(1, 0);
}

static void _VencinitInputCfg(APP_PARAM_VENC_CTX_S *Venc)
{
	int i = 0;
	int chn_num = 2;
	int buffSize = 0;

	Venc->s32VencChnCnt = chn_num;
	for (i = 0; i < chn_num; i++) 
    {
		Venc->astVencChnCfg[i].VencChn = i;
		Venc->astVencChnCfg[i].bEnable = 1;
		if (!Venc->astVencChnCfg[i].bEnable) 
        {
			printf("Venc_chn[%d] not enable!\n", i);
			continue;
		}

		Venc->astVencChnCfg[i].enType           = PT_H265;

        #if defined(CONFIG_RGB_VIDEO_RTSP_VENC_BIND_VI)
            printf("_VencinitInputCfg--CONFIG_RGB_VIDEO_RTSP_VENC_BIND_VI\r\n");
            Venc->astVencChnCfg[i].enBindMode = VENC_BIND_VI;
            Venc->astVencChnCfg[i].u32Width         = 1080;
		    Venc->astVencChnCfg[i].u32Height        = 1920;
        #elif defined(CONFIG_RGB_VIDEO_RTSP_VENC_BIND_VPSS)
            printf("_VencinitInputCfg--CONFIG_RGB_VIDEO_RTSP_VENC_BIND_VPSS\r\n");
            Venc->astVencChnCfg[i].enBindMode = VENC_BIND_VPSS;
            Venc->astVencChnCfg[i].u32Width         = 1280;
		    Venc->astVencChnCfg[i].u32Height        = 720;
            Venc->astVencChnCfg[i].VpssGrp          = i+1;
            Venc->astVencChnCfg[i].VpssChn          = 1;
        #elif defined(CONFIG_IR_VIDEO_RTSP_VENC_BIND_DISABLE)
            printf("_VencinitInputCfg--CONFIG_IR_VIDEO_RTSP_VENC_BIND_DISABLE\r\n");
            Venc->astVencChnCfg[i].enBindMode = VENC_BIND_DISABLE;
            Venc->astVencChnCfg[i].u32Width         = 720;
		    Venc->astVencChnCfg[i].u32Height        = 1280;
        #endif


		if (Venc->astVencChnCfg[i].enBindMode != VENC_BIND_DISABLE) 
        {
			Venc->astVencChnCfg[i].astChn[0].enModId    = 6;
			Venc->astVencChnCfg[i].astChn[0].s32DevId   = 1+i;
			Venc->astVencChnCfg[i].astChn[0].s32ChnId   = 1;
			Venc->astVencChnCfg[i].astChn[1].enModId    = 7;
			Venc->astVencChnCfg[i].astChn[1].s32DevId   = 0;
			Venc->astVencChnCfg[i].astChn[1].s32ChnId   = i;
		}

		printf("Venc_chn[%d] enType=%d VpssGrp=%d VpssChn=%d u32Width=%d u32Height=%d\n\n", 
			i, Venc->astVencChnCfg[i].enType, Venc->astVencChnCfg[i].VpssGrp, Venc->astVencChnCfg[i].VpssChn, 
			Venc->astVencChnCfg[i].u32Width, Venc->astVencChnCfg[i].u32Height);

		if (Venc->astVencChnCfg[i].enType != PT_JPEG) 
        {
			Venc->astVencChnCfg[i].u32Profile       = 0;
			Venc->astVencChnCfg[i].u32SrcFrameRate  = 25;
			Venc->astVencChnCfg[i].u32DstFrameRate = 25;
			Venc->astVencChnCfg[i].enGopMode        = 0;

			switch (Venc->astVencChnCfg[i].enGopMode) 
            {
				case VENC_GOPMODE_NORMALP:
					Venc->astVencChnCfg[i].unGopParam.stNormalP.s32IPQpDelta = 2;
				break;
				case VENC_GOPMODE_SMARTP:
					Venc->astVencChnCfg[i].unGopParam.stSmartP.s32BgQpDelta  = 0;
					Venc->astVencChnCfg[i].unGopParam.stSmartP.s32ViQpDelta  = 0;
					Venc->astVencChnCfg[i].unGopParam.stSmartP.u32BgInterval = 250;
				break;
				case VENC_GOPMODE_DUALP:
					Venc->astVencChnCfg[i].unGopParam.stDualP.s32IPQpDelta   = 2;
					Venc->astVencChnCfg[i].unGopParam.stDualP.s32SPQpDelta   = 2;
					Venc->astVencChnCfg[i].unGopParam.stDualP.u32SPInterval  = 3;
				break;
				case VENC_GOPMODE_BIPREDB:
					Venc->astVencChnCfg[i].unGopParam.stBipredB.s32BQpDelta  = -2;
					Venc->astVencChnCfg[i].unGopParam.stBipredB.s32IPQpDelta = 2;
					Venc->astVencChnCfg[i].unGopParam.stBipredB.u32BFrmNum   = 2;
				break;
				default:
					printf("venc_chn[%d] gop mode: %d invalid", i, Venc->astVencChnCfg[i].enGopMode);
				break;
			}

			Venc->astVencChnCfg[i].enRcMode         = 11;
			Venc->astVencChnCfg[i].u32BitRate       = 1800;
			Venc->astVencChnCfg[i].u32MaxBitRate    = 1800;
			buffSize = 1024;
			Venc->astVencChnCfg[i].u32StreamBufSize = (buffSize << 10); 
			Venc->astVencChnCfg[i].bSingleCore      = 0;
			Venc->astVencChnCfg[i].u32Gop           = 50;
			Venc->astVencChnCfg[i].u32IQp           = 38;
			Venc->astVencChnCfg[i].u32PQp           = 38;
			Venc->astVencChnCfg[i].statTime         = 2;
			Venc->astVencChnCfg[i].u32Duration      = 75;
			Venc->astVencChnCfg[i].stRcParam.u32ThrdLv            = 2;
			Venc->astVencChnCfg[i].stRcParam.s32FirstFrameStartQp = 35;
			Venc->astVencChnCfg[i].stRcParam.s32InitialDelay      = 1000;
			Venc->astVencChnCfg[i].stRcParam.u32MaxIprop          = 10;
			Venc->astVencChnCfg[i].stRcParam.u32MinIprop          = 1;
			Venc->astVencChnCfg[i].stRcParam.u32MaxQp             = 51;
			Venc->astVencChnCfg[i].stRcParam.u32MinQp             = 20;
			Venc->astVencChnCfg[i].stRcParam.u32MaxIQp            = 51;
			Venc->astVencChnCfg[i].stRcParam.u32MinIQp            = 20;
			Venc->astVencChnCfg[i].stRcParam.s32ChangePos         = 75;
			Venc->astVencChnCfg[i].stRcParam.s32MinStillPercent   = 10;
			Venc->astVencChnCfg[i].stRcParam.u32MaxStillQP        = 38;
			Venc->astVencChnCfg[i].stRcParam.u32MinStillPSNR      = 0;
			Venc->astVencChnCfg[i].stRcParam.u32MotionSensitivity = 24;
			Venc->astVencChnCfg[i].stRcParam.s32AvbrFrmLostOpen   = 0;
			Venc->astVencChnCfg[i].stRcParam.s32AvbrFrmGap        = 1;
			Venc->astVencChnCfg[i].stRcParam.s32AvbrPureStillThr  = 4;
			Venc->astVencChnCfg[i].stRcParam.s32MaxReEncodeTimes  = 0;

			printf("Venc_chn[%d] enRcMode=%d u32BitRate=%d u32MaxBitRate=%d enBindMode=%d bSingleCore=%d\n", 
				i, Venc->astVencChnCfg[i].enRcMode, Venc->astVencChnCfg[i].u32BitRate, Venc->astVencChnCfg[i].u32MaxBitRate, 
				Venc->astVencChnCfg[i].enBindMode, Venc->astVencChnCfg[i].bSingleCore);
			printf("u32Gop=%d statTime=%d u32ThrdLv=%d\n", 
				Venc->astVencChnCfg[i].u32Gop, Venc->astVencChnCfg[i].statTime, Venc->astVencChnCfg[i].stRcParam.u32ThrdLv);
			printf("u32MaxQp=%d u32MinQp=%d u32MaxIQp=%d u32MinIQp=%d s32ChangePos=%d s32InitialDelay=%d\n", 
				Venc->astVencChnCfg[i].stRcParam.u32MaxQp, Venc->astVencChnCfg[i].stRcParam.u32MinQp, Venc->astVencChnCfg[i].stRcParam.u32MaxIQp, 
				Venc->astVencChnCfg[i].stRcParam.u32MinIQp, Venc->astVencChnCfg[i].stRcParam.s32ChangePos, Venc->astVencChnCfg[i].stRcParam.s32InitialDelay);
		} 
        else 
        {
			Venc->astVencChnCfg[i].stJpegCodecParam.quality       = 20;
			Venc->astVencChnCfg[i].stJpegCodecParam.MCUPerECS     = 0;
			printf("Venc_chn[%d] quality=%d\n", i, Venc->astVencChnCfg[i].stJpegCodecParam.quality);
		}
	}
}

int APP_Venc_Chn_Attr_Set(VENC_ATTR_S *pstVencAttr, APP_VENC_CHN_CFG_S *pstVencChnCfg)
{
  	pstVencAttr->enType = pstVencChnCfg->enType;
    pstVencAttr->u32MaxPicWidth = pstVencChnCfg->u32Width;
    pstVencAttr->u32MaxPicHeight = pstVencChnCfg->u32Height;
    pstVencAttr->u32PicWidth = pstVencChnCfg->u32Width;
    pstVencAttr->u32PicHeight = pstVencChnCfg->u32Height;
    pstVencAttr->u32Profile = pstVencChnCfg->u32Profile;
    pstVencAttr->bSingleCore = pstVencChnCfg->bSingleCore;
    pstVencAttr->bByFrame = CVI_TRUE;
    pstVencAttr->bEsBufQueueEn = CVI_TRUE;
    pstVencAttr->bIsoSendFrmEn = true;
    pstVencAttr->u32BufSize = pstVencChnCfg->u32StreamBufSize;

   printf("enType=%d u32Profile=%d bSingleCore=%d\n",
        pstVencAttr->enType, pstVencAttr->u32Profile, pstVencAttr->bSingleCore);
   printf("u32MaxPicWidth=%d u32MaxPicHeight=%d u32PicWidth=%d u32PicHeight=%d\n",
        pstVencAttr->u32MaxPicWidth, pstVencAttr->u32MaxPicHeight, pstVencAttr->u32PicWidth, pstVencAttr->u32PicHeight);

    /* Venc encode type validity check */
    if ((pstVencAttr->enType != PT_H265) && (pstVencAttr->enType != PT_H264) 
        && (pstVencAttr->enType != PT_JPEG) && (pstVencAttr->enType != PT_MJPEG)) {
       printf("encode type = %d invalid\n", pstVencAttr->enType);
        return CVI_FAILURE;
    }

    if (pstVencAttr->enType == PT_H264) {
        // pstVencAttr->stAttrH264e.bSingleLumaBuf = 1;
    }

    if (PT_JPEG == pstVencChnCfg->enType || PT_MJPEG == pstVencChnCfg->enType) {
        VENC_ATTR_JPEG_S *pstJpegAttr = &pstVencAttr->stAttrJpege;

        pstJpegAttr->bSupportDCF = CVI_FALSE;
        pstJpegAttr->stMPFCfg.u8LargeThumbNailNum = 0;
        pstJpegAttr->enReceiveMode = VENC_PIC_RECEIVE_SINGLE;
    }

    return CVI_SUCCESS;
}

int APP_Venc_Gop_Attr_Set(VENC_GOP_ATTR_S *pstGopAttr, APP_VENC_CHN_CFG_S *pstVencChnCfg)
{
  VENC_GOP_MODE_E enGopMode = pstVencChnCfg->enGopMode;

    /* Venc gop mode validity check */
    if ((enGopMode != VENC_GOPMODE_NORMALP) && (enGopMode != VENC_GOPMODE_SMARTP) 
        && (enGopMode != VENC_GOPMODE_DUALP) && (enGopMode != VENC_GOPMODE_BIPREDB)) {
       printf("gop mode = %d invalid\n", enGopMode);
        return CVI_FAILURE;
    }

    switch (enGopMode) {
        case VENC_GOPMODE_NORMALP:
            pstGopAttr->stNormalP.s32IPQpDelta = pstVencChnCfg->unGopParam.stNormalP.s32IPQpDelta;

           printf("stNormalP -> s32IPQpDelta=%d\n",
                pstGopAttr->stNormalP.s32IPQpDelta);
            break;
        case VENC_GOPMODE_SMARTP:
            pstGopAttr->stSmartP.s32BgQpDelta = pstVencChnCfg->unGopParam.stSmartP.s32BgQpDelta;
            pstGopAttr->stSmartP.s32ViQpDelta = pstVencChnCfg->unGopParam.stSmartP.s32ViQpDelta;
            pstGopAttr->stSmartP.u32BgInterval = pstVencChnCfg->unGopParam.stSmartP.u32BgInterval;

           printf("stSmartP -> s32BgQpDelta=%d s32ViQpDelta=%d u32BgInterval=%d\n",
                pstGopAttr->stSmartP.s32BgQpDelta, pstGopAttr->stSmartP.s32ViQpDelta, pstGopAttr->stSmartP.u32BgInterval);
            break;

        case VENC_GOPMODE_DUALP:
            pstGopAttr->stDualP.s32IPQpDelta = pstVencChnCfg->unGopParam.stDualP.s32IPQpDelta;
            pstGopAttr->stDualP.s32SPQpDelta = pstVencChnCfg->unGopParam.stDualP.s32SPQpDelta;
            pstGopAttr->stDualP.u32SPInterval = pstVencChnCfg->unGopParam.stDualP.u32SPInterval;

           printf("stDualP -> s32IPQpDelta=%d s32SPQpDelta=%d u32SPInterval=%d\n",
                pstGopAttr->stDualP.s32IPQpDelta, pstGopAttr->stDualP.s32SPQpDelta, pstGopAttr->stDualP.u32SPInterval);
            break;

        case VENC_GOPMODE_BIPREDB:
            pstGopAttr->stBipredB.s32BQpDelta = pstVencChnCfg->unGopParam.stBipredB.s32BQpDelta;
            pstGopAttr->stBipredB.s32IPQpDelta = pstVencChnCfg->unGopParam.stBipredB.s32IPQpDelta;
            pstGopAttr->stBipredB.u32BFrmNum = pstVencChnCfg->unGopParam.stBipredB.u32BFrmNum;

           printf("stBipredB -> s32BQpDelta=%d s32IPQpDelta=%d u32BFrmNum=%d\n",
                pstGopAttr->stBipredB.s32BQpDelta, pstGopAttr->stBipredB.s32IPQpDelta, pstGopAttr->stBipredB.u32BFrmNum);
            break;

        default:
           printf("not support the gop mode !\n");
            return CVI_FAILURE;
    }

    pstGopAttr->enGopMode = enGopMode;
    if (PT_MJPEG == pstVencChnCfg->enType || PT_JPEG == pstVencChnCfg->enType) {
        pstGopAttr->enGopMode = VENC_GOPMODE_NORMALP;
        pstGopAttr->stNormalP.s32IPQpDelta = pstVencChnCfg->unGopParam.stNormalP.s32IPQpDelta;
    }

    return CVI_SUCCESS;
}

int APP_Venc_Rc_Attr_Set(VENC_RC_ATTR_S *pstRCAttr, APP_VENC_CHN_CFG_S *pstVencChnCfg)
{
    int SrcFrmRate = pstVencChnCfg->u32SrcFrameRate;
    int DstFrmRate = pstVencChnCfg->u32DstFrameRate;
    int BitRate    = pstVencChnCfg->u32BitRate;
    int MaxBitrate = pstVencChnCfg->u32MaxBitRate;
    int StatTime   = pstVencChnCfg->statTime;
    int Gop        = pstVencChnCfg->u32Gop;
    int IQP        = pstVencChnCfg->u32IQp;
    int PQP        = pstVencChnCfg->u32PQp;

    printf("RcMode=%d EncType=%d SrcFR=%d DstFR=%d\n", 
        pstVencChnCfg->enRcMode, pstVencChnCfg->enType, 
        pstVencChnCfg->u32SrcFrameRate, pstVencChnCfg->u32DstFrameRate);
    printf("BR=%d MaxBR=%d statTime=%d gop=%d IQP=%d PQP=%d\n", 
        pstVencChnCfg->u32BitRate, pstVencChnCfg->u32MaxBitRate, 
        pstVencChnCfg->statTime, pstVencChnCfg->u32Gop, pstVencChnCfg->u32IQp, pstVencChnCfg->u32PQp);

    pstRCAttr->enRcMode = pstVencChnCfg->enRcMode;
    switch (pstVencChnCfg->enType) {
    case PT_H265: {
        if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H265CBR) {
            VENC_H265_CBR_S *pstH265Cbr = &pstRCAttr->stH265Cbr;

            pstH265Cbr->u32Gop = Gop;
            pstH265Cbr->u32StatTime = StatTime;
            pstH265Cbr->u32SrcFrameRate = SrcFrmRate;
            pstH265Cbr->fr32DstFrameRate = DstFrmRate;
            pstH265Cbr->u32BitRate = BitRate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H265FIXQP) {
            VENC_H265_FIXQP_S *pstH265FixQp = &pstRCAttr->stH265FixQp;

            pstH265FixQp->u32Gop = Gop;
            pstH265FixQp->u32SrcFrameRate = SrcFrmRate;
            pstH265FixQp->fr32DstFrameRate = DstFrmRate;
            pstH265FixQp->u32IQp = IQP;
            pstH265FixQp->u32PQp = PQP;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H265VBR) {
            VENC_H265_VBR_S *pstH265Vbr = &pstRCAttr->stH265Vbr;

            pstH265Vbr->u32Gop = Gop;
            pstH265Vbr->u32StatTime = StatTime;
            pstH265Vbr->u32SrcFrameRate = SrcFrmRate;
            pstH265Vbr->fr32DstFrameRate = DstFrmRate;
            pstH265Vbr->u32MaxBitRate = MaxBitrate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H265AVBR) {
            VENC_H265_AVBR_S *pstH265AVbr = &pstRCAttr->stH265AVbr;

            pstH265AVbr->u32Gop = Gop;
            pstH265AVbr->u32StatTime = StatTime;
            pstH265AVbr->u32SrcFrameRate = SrcFrmRate;
            pstH265AVbr->fr32DstFrameRate = DstFrmRate;
            pstH265AVbr->u32MaxBitRate = MaxBitrate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H265QVBR) {
            VENC_H265_QVBR_S *pstH265QVbr = &pstRCAttr->stH265QVbr;

            pstH265QVbr->u32Gop = Gop;
            pstH265QVbr->u32StatTime = StatTime;
            pstH265QVbr->u32SrcFrameRate = SrcFrmRate;
            pstH265QVbr->fr32DstFrameRate = DstFrmRate;
            pstH265QVbr->u32TargetBitRate = BitRate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H265QPMAP) {
            VENC_H265_QPMAP_S *pstH265QpMap = &pstRCAttr->stH265QpMap;

            pstH265QpMap->u32Gop = Gop;
            pstH265QpMap->u32StatTime = StatTime;
            pstH265QpMap->u32SrcFrameRate = SrcFrmRate;
            pstH265QpMap->fr32DstFrameRate = DstFrmRate;
            pstH265QpMap->enQpMapMode = VENC_RC_QPMAP_MODE_MEANQP;
        } else {
            printf("VencChn(%d) enRcMode(%d) not support\n", pstVencChnCfg->VencChn, pstVencChnCfg->enRcMode);
            return CVI_FAILURE;
        }
    }
    break;
    case PT_H264: {
        if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H264CBR) {
            VENC_H264_CBR_S *pstH264Cbr = &pstRCAttr->stH264Cbr;

            pstH264Cbr->u32Gop = Gop;
            pstH264Cbr->u32StatTime = StatTime;
            pstH264Cbr->u32SrcFrameRate = SrcFrmRate;
            pstH264Cbr->fr32DstFrameRate = DstFrmRate;
            pstH264Cbr->u32BitRate = BitRate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H264FIXQP) {
            VENC_H264_FIXQP_S *pstH264FixQp = &pstRCAttr->stH264FixQp;

            pstH264FixQp->u32Gop = Gop;
            pstH264FixQp->u32SrcFrameRate = SrcFrmRate;
            pstH264FixQp->fr32DstFrameRate = DstFrmRate;
            pstH264FixQp->u32IQp = IQP;
            pstH264FixQp->u32PQp = PQP;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H264VBR) {
            VENC_H264_VBR_S *pstH264Vbr = &pstRCAttr->stH264Vbr;

            pstH264Vbr->u32Gop = Gop;
            pstH264Vbr->u32StatTime = StatTime;
            pstH264Vbr->u32SrcFrameRate = SrcFrmRate;
            pstH264Vbr->fr32DstFrameRate = DstFrmRate;
            pstH264Vbr->u32MaxBitRate = MaxBitrate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H264AVBR) {
            VENC_H264_AVBR_S *pstH264AVbr = &pstRCAttr->stH264AVbr;

            pstH264AVbr->u32Gop = Gop;
            pstH264AVbr->u32StatTime = StatTime;
            pstH264AVbr->u32SrcFrameRate = SrcFrmRate;
            pstH264AVbr->fr32DstFrameRate = DstFrmRate;
            pstH264AVbr->u32MaxBitRate = MaxBitrate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H264QVBR) {
            VENC_H264_QVBR_S *pstH264QVbr = &pstRCAttr->stH264QVbr;

            pstH264QVbr->u32Gop = Gop;
            pstH264QVbr->u32StatTime = StatTime;
            pstH264QVbr->u32SrcFrameRate = SrcFrmRate;
            pstH264QVbr->fr32DstFrameRate = DstFrmRate;
            pstH264QVbr->u32TargetBitRate = BitRate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_H264QPMAP) {
            VENC_H264_QPMAP_S *pstH264QpMap = &pstRCAttr->stH264QpMap;

            pstH264QpMap->u32Gop = Gop;
            pstH264QpMap->u32StatTime = StatTime;
            pstH264QpMap->u32SrcFrameRate = SrcFrmRate;
            pstH264QpMap->fr32DstFrameRate = DstFrmRate;
        } else {
            printf("enRcMode(%d) not support\n", pstVencChnCfg->enRcMode);
            return CVI_FAILURE;
        }
    }
    break;
    case PT_MJPEG: {
        if (pstVencChnCfg->enRcMode == VENC_RC_MODE_MJPEGFIXQP) {
            VENC_MJPEG_FIXQP_S *pstMjpegeFixQp = &pstRCAttr->stMjpegFixQp;

            // 0 use old q-table for forward compatible.
            pstMjpegeFixQp->u32Qfactor = 0;
            pstMjpegeFixQp->u32SrcFrameRate = SrcFrmRate;
            pstMjpegeFixQp->fr32DstFrameRate = DstFrmRate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_MJPEGCBR) {
            VENC_MJPEG_CBR_S *pstMjpegeCbr = &pstRCAttr->stMjpegCbr;

            pstMjpegeCbr->u32StatTime = StatTime;
            pstMjpegeCbr->u32SrcFrameRate = SrcFrmRate;
            pstMjpegeCbr->fr32DstFrameRate = DstFrmRate;
            pstMjpegeCbr->u32BitRate = BitRate;
        } else if (pstVencChnCfg->enRcMode == VENC_RC_MODE_MJPEGVBR) {
            VENC_MJPEG_VBR_S *pstMjpegeVbr = &pstRCAttr->stMjpegVbr;

            pstMjpegeVbr->u32StatTime = StatTime;
            pstMjpegeVbr->u32SrcFrameRate = SrcFrmRate;
            pstMjpegeVbr->fr32DstFrameRate = DstFrmRate;
            pstMjpegeVbr->u32MaxBitRate = MaxBitrate;
        } else {
            printf("cann't support other mode(%d) in this version!\n", pstVencChnCfg->enRcMode);
            return CVI_FAILURE;
        }
    }
    break;

    case PT_JPEG:
        break;

    default:
        printf("cann't support this enType (%d) in this version!\n", pstVencChnCfg->enType);
        return CVI_ERR_VENC_NOT_SUPPORT;
    }

    return CVI_SUCCESS;
}

void APP_Venc_Attr_Check(VENC_CHN_ATTR_S *pstVencChnAttr)
{
    if (pstVencChnAttr->stVencAttr.enType == PT_H264) {
        // pstVencChnAttr->stVencAttr.stAttrH264e.bSingleLumaBuf = 1;
    }

    if ((pstVencChnAttr->stGopAttr.enGopMode == VENC_GOPMODE_BIPREDB) &&
        (pstVencChnAttr->stVencAttr.enType == PT_H264)) {
        if (pstVencChnAttr->stVencAttr.u32Profile == 0) {
            pstVencChnAttr->stVencAttr.u32Profile = 1;
            printf("H.264 base not support BIPREDB, change to main\n");
        }
    }

    if ((pstVencChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264QPMAP) ||
        (pstVencChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265QPMAP)) {
        if (pstVencChnAttr->stGopAttr.enGopMode == VENC_GOPMODE_ADVSMARTP) {
            pstVencChnAttr->stGopAttr.enGopMode = VENC_GOPMODE_SMARTP;
            printf("advsmartp not support QPMAP, so change gopmode to smartp!\n");
        }
    }

    if ((pstVencChnAttr->stGopAttr.enGopMode == VENC_GOPMODE_BIPREDB) &&
        (pstVencChnAttr->stVencAttr.enType == PT_H264)) {
        if (pstVencChnAttr->stVencAttr.u32Profile == 0) {
            pstVencChnAttr->stVencAttr.u32Profile = 1;
            printf("H.264 base not support BIPREDB, change to main\n");
        }
    }
    if ((pstVencChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H264QPMAP) ||
        (pstVencChnAttr->stRcAttr.enRcMode == VENC_RC_MODE_H265QPMAP)) {
        if (pstVencChnAttr->stGopAttr.enGopMode == VENC_GOPMODE_ADVSMARTP) {
            pstVencChnAttr->stGopAttr.enGopMode = VENC_GOPMODE_SMARTP;
            printf("advsmartp not support QPMAP, so change gopmode to smartp!\n");
        }
    }
}

static CVI_S32 APP_Venc_Roi_Set(VENC_CHN vencChn, APP_VENC_ROI_CFG_S *pstRoiAttr)
{
    if (NULL == pstRoiAttr)
    {
        printf( "pstRoiAttr is NULL!\n");
        return CVI_FAILURE;
    }

    // CVI_S32 ret;
    // CVI_S32 i = 0;
    // VENC_ROI_ATTR_S roiAttr;
    // memset(&roiAttr, 0, sizeof(roiAttr));

    // for (i = 0; i < MAX_NUM_ROI; i++)
    // {
    //     if (vencChn == pstRoiAttr[i].VencChn)
    //     {
    //         ret = CVI_VENC_GetRoiAttr(vencChn, i, &roiAttr);
    //         if (ret != CVI_SUCCESS)
    //         {
    //             printf( "GetRoiAttr failed!\n");
    //             return CVI_FAILURE;
    //         }
    //         roiAttr.bEnable = pstRoiAttr[i].bEnable;
    //         roiAttr.bAbsQp = pstRoiAttr[i].bAbsQp;
    //         roiAttr.s32Qp = pstRoiAttr[i].u32Qp;
    //         roiAttr.stRect.s32X = pstRoiAttr[i].u32X;
    //         roiAttr.stRect.s32Y = pstRoiAttr[i].u32Y;
    //         roiAttr.stRect.u32Width = pstRoiAttr[i].u32Width;
    //         roiAttr.stRect.u32Height = pstRoiAttr[i].u32Height;
    //         ret = CVI_VENC_SetRoiAttr(vencChn, &roiAttr);
    //         if (ret != CVI_SUCCESS)
    //         {
    //             printf( "SetRoiAttr failed!\n");
    //             return CVI_FAILURE;
    //         }
    //     }
    // }

    return CVI_SUCCESS;
}

int APP_Venc_Rc_Param_Set(
    VENC_CHN VencChn,
    PAYLOAD_TYPE_E enCodecType,
    VENC_RC_MODE_E enRcMode,
    APP_RC_PARAM_S *pstRcParamCfg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VENC_RC_PARAM_S stRcParam, *pstRcParam = &stRcParam;

    s32Ret = CVI_VENC_GetRcParam(VencChn, pstRcParam);
    if (s32Ret != CVI_SUCCESS) {
        CVI_VENC_ERR("GetRcParam, 0x%X\n", s32Ret);
        return s32Ret;
    }

    CVI_U32 u32MaxIprop          = pstRcParamCfg->u32MaxIprop;
    CVI_U32 u32MinIprop          = pstRcParamCfg->u32MinIprop;
    CVI_U32 u32MaxQp             = pstRcParamCfg->u32MaxQp;
    CVI_U32 u32MinQp             = pstRcParamCfg->u32MinQp;
    CVI_U32 u32MaxIQp            = pstRcParamCfg->u32MaxIQp;
    CVI_U32 u32MinIQp            = pstRcParamCfg->u32MinIQp;
    CVI_S32 s32ChangePos         = pstRcParamCfg->s32ChangePos;
    CVI_S32 s32MinStillPercent   = pstRcParamCfg->s32MinStillPercent;
    CVI_U32 u32MaxStillQP        = pstRcParamCfg->u32MaxStillQP;
    CVI_U32 u32MotionSensitivity = pstRcParamCfg->u32MotionSensitivity;
    CVI_S32 s32AvbrFrmLostOpen   = pstRcParamCfg->s32AvbrFrmLostOpen;
    CVI_S32 s32AvbrFrmGap        = pstRcParamCfg->s32AvbrFrmGap;
    CVI_S32 s32AvbrPureStillThr  = pstRcParamCfg->s32AvbrPureStillThr;

    CVI_U32 u32ThrdLv            = pstRcParamCfg->u32ThrdLv;
    CVI_S32 s32FirstFrameStartQp = pstRcParamCfg->s32FirstFrameStartQp;
    CVI_S32 s32InitialDelay      = pstRcParamCfg->s32InitialDelay;

   printf("MaxIprop=%d MinIprop=%d MaxQp=%d MinQp=%d MaxIQp=%d MinIQp=%d\n",
        u32MaxIprop, u32MinIprop, u32MaxQp, u32MinQp, u32MaxIQp, u32MinIQp);
   printf("ChangePos=%d MinStillPercent=%d MaxStillQP=%d MotionSensitivity=%d AvbrFrmLostOpen=%d\n",
        s32ChangePos, s32MinStillPercent, u32MaxStillQP, u32MotionSensitivity, s32AvbrFrmLostOpen);
   printf("AvbrFrmGap=%d AvbrPureStillThr=%d ThrdLv=%d FirstFrameStartQp=%d InitialDelay=%d\n",
        s32AvbrFrmGap, s32AvbrPureStillThr, u32ThrdLv, s32FirstFrameStartQp, s32InitialDelay);

    pstRcParam->u32ThrdLv = u32ThrdLv;
    pstRcParam->s32FirstFrameStartQp = s32FirstFrameStartQp;
    pstRcParam->s32InitialDelay = s32InitialDelay;

    switch (enCodecType) {
    case PT_H265: {
        if (enRcMode == VENC_RC_MODE_H265CBR) {
            pstRcParam->stParamH265Cbr.u32MaxIprop = u32MaxIprop;
            pstRcParam->stParamH265Cbr.u32MinIprop = u32MinIprop;
            pstRcParam->stParamH265Cbr.u32MaxIQp = u32MaxIQp;
            pstRcParam->stParamH265Cbr.u32MinIQp = u32MinIQp;
            pstRcParam->stParamH265Cbr.u32MaxQp = u32MaxQp;
            pstRcParam->stParamH265Cbr.u32MinQp = u32MinQp;
        } else if (enRcMode == VENC_RC_MODE_H265VBR) {
            pstRcParam->stParamH265Vbr.u32MaxIprop = u32MaxIprop;
            pstRcParam->stParamH265Vbr.u32MinIprop = u32MinIprop;
            pstRcParam->stParamH265Vbr.u32MaxIQp = u32MaxIQp;
            pstRcParam->stParamH265Vbr.u32MinIQp = u32MinIQp;
            pstRcParam->stParamH265Vbr.u32MaxQp = u32MaxQp;
            pstRcParam->stParamH265Vbr.u32MinQp = u32MinQp;
            pstRcParam->stParamH265Vbr.s32ChangePos = s32ChangePos;
        } else if (enRcMode == VENC_RC_MODE_H265AVBR) {
            pstRcParam->stParamH265AVbr.u32MaxIprop = u32MaxIprop;
            pstRcParam->stParamH265AVbr.u32MinIprop = u32MinIprop;
            pstRcParam->stParamH265AVbr.u32MaxIQp = u32MaxIQp;
            pstRcParam->stParamH265AVbr.u32MinIQp = u32MinIQp;
            pstRcParam->stParamH265AVbr.u32MaxQp = u32MaxQp;
            pstRcParam->stParamH265AVbr.u32MinQp = u32MinQp;
            pstRcParam->stParamH265AVbr.s32ChangePos = s32ChangePos;
            pstRcParam->stParamH265AVbr.s32MinStillPercent = s32MinStillPercent;
            pstRcParam->stParamH265AVbr.u32MaxStillQP = u32MaxStillQP;
            pstRcParam->stParamH265AVbr.u32MotionSensitivity = u32MotionSensitivity;
            pstRcParam->stParamH265AVbr.s32AvbrFrmLostOpen = s32AvbrFrmLostOpen;
            pstRcParam->stParamH265AVbr.s32AvbrFrmGap = s32AvbrFrmGap;
            pstRcParam->stParamH265AVbr.s32AvbrPureStillThr = s32AvbrPureStillThr;
        } else {
            printf("enRcMode(%d) not support\n", enRcMode);
            return CVI_FAILURE;
        }
    }
    break;
    case PT_H264: {
        if (enRcMode == VENC_RC_MODE_H264CBR) {
            pstRcParam->stParamH264Cbr.u32MaxIprop = u32MaxIprop;
            pstRcParam->stParamH264Cbr.u32MinIprop = u32MinIprop;
            pstRcParam->stParamH264Cbr.u32MaxIQp = u32MaxIQp;
            pstRcParam->stParamH264Cbr.u32MinIQp = u32MinIQp;
            pstRcParam->stParamH264Cbr.u32MaxQp = u32MaxQp;
            pstRcParam->stParamH264Cbr.u32MinQp = u32MinQp;
        } else if (enRcMode == VENC_RC_MODE_H264VBR) {
            pstRcParam->stParamH264Vbr.u32MaxIprop = u32MaxIprop;
            pstRcParam->stParamH264Vbr.u32MinIprop = u32MinIprop;
            pstRcParam->stParamH264Vbr.u32MaxIQp = u32MaxIQp;
            pstRcParam->stParamH264Vbr.u32MinIQp = u32MinIQp;
            pstRcParam->stParamH264Vbr.u32MaxQp = u32MaxQp;
            pstRcParam->stParamH264Vbr.u32MinQp = u32MinQp;
            pstRcParam->stParamH264Vbr.s32ChangePos = s32ChangePos;
        } else if (enRcMode == VENC_RC_MODE_H264AVBR) {
            pstRcParam->stParamH264AVbr.u32MaxIprop = u32MaxIprop;
            pstRcParam->stParamH264AVbr.u32MinIprop = u32MinIprop;
            pstRcParam->stParamH264AVbr.u32MaxIQp = u32MaxIQp;
            pstRcParam->stParamH264AVbr.u32MinIQp = u32MinIQp;
            pstRcParam->stParamH264AVbr.u32MaxQp = u32MaxQp;
            pstRcParam->stParamH264AVbr.u32MinQp = u32MinQp;
            pstRcParam->stParamH264AVbr.s32ChangePos = s32ChangePos;
            pstRcParam->stParamH264AVbr.s32MinStillPercent = s32MinStillPercent;
            pstRcParam->stParamH264AVbr.u32MaxStillQP = u32MaxStillQP;
            pstRcParam->stParamH264AVbr.u32MotionSensitivity = u32MotionSensitivity;
            pstRcParam->stParamH264AVbr.s32AvbrFrmLostOpen = s32AvbrFrmLostOpen;
            pstRcParam->stParamH264AVbr.s32AvbrFrmGap = s32AvbrFrmGap;
            pstRcParam->stParamH264AVbr.s32AvbrPureStillThr = s32AvbrPureStillThr;
        } else {
            printf("enRcMode(%d) not support\n", enRcMode);
            return CVI_FAILURE;
        }
    }
    break;
    default:
        printf("cann't support this enType (%d) in this version!\n", enCodecType);
        return CVI_ERR_VENC_NOT_SUPPORT;
    }

    s32Ret = CVI_VENC_SetRcParam(VencChn, pstRcParam);
    if (s32Ret != CVI_SUCCESS) {
        CVI_VENC_ERR("SetRcParam, 0x%X\n", s32Ret);
        return s32Ret;
    }

    return CVI_SUCCESS;
}

CVI_S32 APP_Venc_FrameLost_Set(VENC_CHN VencChn, APP_FRAMELOST_PARAM_S *pFrameLostCfg)
{
    VENC_FRAMELOST_S stFL, *pstFL = &stFL;

    CVI_VENC_GetFrameLostStrategy(VencChn, pstFL);

    pstFL->bFrmLostOpen = (pFrameLostCfg->frameLost) == 1 ? CVI_TRUE : CVI_FALSE;
    pstFL->enFrmLostMode = FRMLOST_PSKIP;
    pstFL->u32EncFrmGaps = pFrameLostCfg->frameLostGap;
    pstFL->u32FrmLostBpsThr = pFrameLostCfg->frameLostBspThr;

    CVI_VENC_SetFrameLostStrategy(VencChn, pstFL);

    return CVI_SUCCESS;
}

CVI_S32 APP_Venc_Jpeg_Param_Set(VENC_CHN VencChn, APP_JPEG_CODEC_PARAM_S *pstJpegCodecCfg)
{
    VENC_JPEG_PARAM_S stJpegParam, *pstJpegParam = &stJpegParam;

    CVI_VENC_GetJpegParam(VencChn, pstJpegParam);

    if (pstJpegCodecCfg->quality <= 0)
        pstJpegCodecCfg->quality = 1;
    else if (pstJpegCodecCfg->quality >= 100)
        pstJpegCodecCfg->quality = 99;

    pstJpegParam->u32Qfactor = pstJpegCodecCfg->quality;
    pstJpegParam->u32MCUPerECS = pstJpegCodecCfg->MCUPerECS;

    CVI_VENC_SetJpegParam(VencChn, pstJpegParam);

    return CVI_SUCCESS;
}

APP_PARAM_VENC_CTX_S g_stVencCtx, *g_pstVencCtx = &g_stVencCtx;

CVI_S32 APP_Venc_Init(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	_VencinitInputCfg(g_pstVencCtx);

	for (VENC_CHN s32ChnIdx = 0; s32ChnIdx < g_pstVencCtx->s32VencChnCnt; s32ChnIdx++) 
    {
        APP_VENC_CHN_CFG_S *pstVencChnCfg = &g_pstVencCtx->astVencChnCfg[s32ChnIdx];
        APP_VENC_ROI_CFG_S *pstVencRoiCfg = g_pstVencCtx->astRoiCfg;
        VENC_CHN VencChn = pstVencChnCfg->VencChn;
        if ((!pstVencChnCfg->bEnable) || (pstVencChnCfg->bStart))
            continue;

		printf("Ven_%d init info\n", VencChn);
        printf("VpssGrp=%d VpssChn=%d size_W=%d size_H=%d CodecType=%d\n", 
            pstVencChnCfg->VpssGrp, pstVencChnCfg->VpssChn, pstVencChnCfg->u32Width, pstVencChnCfg->u32Height,
            pstVencChnCfg->enType);

        PAYLOAD_TYPE_E enCodecType = pstVencChnCfg->enType;
        VENC_CHN_ATTR_S stVencChnAttr, *pstVencChnAttr = &stVencChnAttr;
        memset(&stVencChnAttr, 0, sizeof(stVencChnAttr));

		/* Venc channel validity check */
        if (VencChn != pstVencChnCfg->VencChn) 
        {
           printf("VencChn error %d\n", VencChn);
            goto VENC_EXIT0;
        }

		s32Ret = APP_Venc_Chn_Attr_Set(&pstVencChnAttr->stVencAttr, pstVencChnCfg);
		if (s32Ret != CVI_SUCCESS) 
        {
			printf("media_venc_set_attr [%d] failed with 0x%x\n", VencChn, s32Ret);
			goto VENC_EXIT0;
		}

		s32Ret = APP_Venc_Gop_Attr_Set(&pstVencChnAttr->stGopAttr, pstVencChnCfg);
		if (s32Ret != CVI_SUCCESS) 
        {
			printf("media_venc_set_gop [%d] failed with 0x%x\n", VencChn, s32Ret);
			goto VENC_EXIT0;
		}

		s32Ret = APP_Venc_Rc_Attr_Set(&pstVencChnAttr->stRcAttr, pstVencChnCfg);
		if (s32Ret != CVI_SUCCESS) 
        {
			printf("media_venc_set_rc_attr [%d] failed with 0x%x\n", VencChn, s32Ret);
			goto VENC_EXIT0;
		}

		APP_Venc_Attr_Check(pstVencChnAttr);

		s32Ret = CVI_VENC_CreateChn(VencChn, pstVencChnAttr);
		if (s32Ret != CVI_SUCCESS) 
        {
			printf("CVI_VENC_CreateChn [%d] failed with 0x%x\n", VencChn, s32Ret);
			goto VENC_EXIT1;
		}

		if ((enCodecType == PT_H265) || (enCodecType == PT_H264)) 
        {
            if (enCodecType == PT_H264)
            {
                s32Ret = APP_Venc_Roi_Set(VencChn, pstVencRoiCfg);
                if (s32Ret != CVI_SUCCESS) 
                {
                    printf("APP_Venc_Roi_Set [%d] failed with 0x%x\n", VencChn, s32Ret);
                    goto VENC_EXIT1;
                }
            }

            s32Ret = APP_Venc_Rc_Param_Set(VencChn, enCodecType, pstVencChnCfg->enRcMode, &pstVencChnCfg->stRcParam);
            if (s32Ret != CVI_SUCCESS) 
            {
                printf("Venc_%d RC Param Set failed with 0x%x\n", VencChn, s32Ret);
                goto VENC_EXIT1;
            }

            s32Ret = APP_Venc_FrameLost_Set(VencChn, &pstVencChnCfg->stFrameLostCtrl);
            if (s32Ret != CVI_SUCCESS) 
            {
                printf("Venc_%d RC frame lost control failed with 0x%x\n", VencChn, s32Ret);
                goto VENC_EXIT1;
            }
        } 
        else if (enCodecType == PT_JPEG) 
        {
            s32Ret = APP_Venc_Jpeg_Param_Set(VencChn, &pstVencChnCfg->stJpegCodecParam);
            if (s32Ret != CVI_SUCCESS) 
            {
                printf("Venc_%d JPG Param Set failed with 0x%x\n", VencChn, s32Ret);
                goto VENC_EXIT1;
            }
        }

		if (pstVencChnCfg->enBindMode != VENC_BIND_DISABLE) 
        {
            s32Ret = CVI_SYS_Bind(&pstVencChnCfg->astChn[0], &pstVencChnCfg->astChn[1]);
            if (s32Ret != CVI_SUCCESS) 
            {
               printf("CVI_SYS_Bind failed with %#x\n", s32Ret);
                goto VENC_EXIT1;
            }
        }
	}
    return CVI_SUCCESS;

VENC_EXIT1:
    for (VENC_CHN s32ChnIdx = 0; s32ChnIdx < g_pstVencCtx->s32VencChnCnt; s32ChnIdx++) 
    {
        CVI_VENC_ResetChn(g_pstVencCtx->astVencChnCfg[s32ChnIdx].VencChn);
        CVI_VENC_DestroyChn(g_pstVencCtx->astVencChnCfg[s32ChnIdx].VencChn);
    }

VENC_EXIT0:
   printf("VENC_EXIT0!!!!!!!!!!!!!!!!!!\n");

    return s32Ret;
}