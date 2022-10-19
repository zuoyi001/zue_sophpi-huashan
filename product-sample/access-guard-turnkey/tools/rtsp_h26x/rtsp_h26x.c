#include <stdio.h>
#include <sys/prctl.h>
#include "cvi_venc.h"
#include "sample_comm.h"
#include "app_rtsp.h"


chnInputCfg pIc[2]; //for venc
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
	ipIc->bVariFpsEn = 0;
	ipIc->maxQp = -1;
	ipIc->minQp = -1;
	ipIc->maxIqp = -1;
	ipIc->minIqp = -1;
	ipIc->quality = -1;
	ipIc->maxbitrate = -1;
	ipIc->statTime = -1;
	ipIc->bind_mode = VENC_BIND_DISABLE;
	ipIc->pixel_format = 0;
	ipIc->bitstreamBufSize = 0;
	ipIc->single_LumaBuf = 0;
	ipIc->single_core = 0;
	ipIc->forceIdr = -1;
	ipIc->chgNum = -1;
	ipIc->tempLayer = 0;
	ipIc->testRoi = 0;
	ipIc->bgInterval = 0;
	ipIc->frameLost = -1;
	ipIc->frameLostBspThr = -1;
	ipIc->frameLostGap = -1;
	ipIc->MCUPerECS = 0;
	ipIc->sendframe_timeout = 20000;
	ipIc->getstream_timeout = -1;
	ipIc->s32IPQpDelta = -100;
}
static CVI_S32 checkInputCfg(chnInputCfg *pIc)
{
	if (!strcmp(pIc->codec, "264") || !strcmp(pIc->codec, "265")) {

		printf("framerate = %d\n", pIc->framerate);

		if (pIc->gop < 1) {
			if (!strcmp(pIc->codec, "264"))
				pIc->gop = DEF_264_GOP;
			else
				pIc->gop = DEF_GOP;
		}
		printf("gop = %d\n", pIc->gop);

		if (!strcmp(pIc->codec, "265")) {
			if (pIc->single_LumaBuf > 0) {
				printf("single_LumaBuf only supports H.264\n");
				pIc->single_LumaBuf = 0;
			}
		}
		pIc->iqp = (pIc->iqp >= 0) ? pIc->iqp : DEF_IQP;
		pIc->pqp = (pIc->pqp >= 0) ? pIc->pqp : DEF_PQP;

		if (pIc->rcMode == -1) {
			pIc->rcMode = SAMPLE_RC_FIXQP;
		}

		if (pIc->rcMode >= SAMPLE_RC_CBR || pIc->rcMode == SAMPLE_RC_AVBR) {

			if (pIc->rcMode == SAMPLE_RC_CBR) {
				if (pIc->bitrate <= 0) {
					printf("CBR bitrate must be not less than 0");
					return -1;
				}
				CVI_VENC_CFG("RC_CBR, bitrate = %d\n", pIc->bitrate);
			} else if (pIc->rcMode == SAMPLE_RC_VBR) {
				if (pIc->maxbitrate <= 0) {
					printf("VBR must be not less than 0");
					return -1;
				}
				printf("RC_VBR, maxbitrate = %d\n", pIc->maxbitrate);
			}

			pIc->firstFrmstartQp =
				(pIc->firstFrmstartQp < 0 ||
				 pIc->firstFrmstartQp > 51) ? 63 : pIc->firstFrmstartQp;
			printf("firstFrmstartQp = %d\n", pIc->firstFrmstartQp);

			pIc->maxIqp = (pIc->maxIqp >= 0) ? pIc->maxIqp : DEF_264_MAXIQP;
			pIc->minIqp = (pIc->minIqp >= 0) ? pIc->minIqp : DEF_264_MINIQP;
			pIc->maxQp = (pIc->maxQp >= 0) ? pIc->maxQp : DEF_264_MAXQP;
			pIc->minQp = (pIc->minQp >= 0) ? pIc->minQp : DEF_264_MINQP;
			printf("maxQp = %d, minQp = %d, maxIqp = %d, minIqp = %d\n",
					pIc->maxQp,
					pIc->minQp,
					pIc->maxIqp,
					pIc->minIqp);

			if (pIc->statTime == 0) {
				pIc->statTime = DEF_STAT_TIME;
			}
			printf("statTime = %d\n", pIc->statTime);
		} else if (pIc->rcMode == SAMPLE_RC_FIXQP) {
			if (pIc->firstFrmstartQp != -1) {
				printf("firstFrmstartQp is invalid in FixQP mode\n");
				pIc->firstFrmstartQp = -1;
			}

			pIc->bitrate = 0;
			printf("RC_FIXQP, iqp = %d, pqp = %d\n",
				pIc->iqp,
				pIc->pqp);
		} else {
			printf("codec = %s, rcMode = %d, not supported RC mode\n", pIc->codec, pIc->rcMode);
			return -1;
		}
	} else if (!strcmp(pIc->codec, "mjp") || !strcmp(pIc->codec, "jpg")) {
		if (pIc->rcMode == -1) {
			pIc->rcMode = SAMPLE_RC_FIXQP;
			pIc->quality = (pIc->quality != -1) ? pIc->quality : 0;
		} else if (pIc->rcMode == SAMPLE_RC_FIXQP) {
			if (pIc->quality < 0)
				pIc->quality = 0;
			else if (pIc->quality >= 100)
				pIc->quality = 99;
			else if (pIc->quality == 0)
				pIc->quality = 1;
		}
	} else {
		printf("codec = %s\n", pIc->codec);
		return -1;
	}

	return 0;
}

CVI_S32 APP_InitVenc(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VENC_CHN VencChn[2] = {0, 1};
	//VENC_CHN VencChn[1] = {0};
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
		pIc[i].rcMode = 4; // cbr
		pIc[i].iqp = 30;
		pIc[i].pqp = 30;
		pIc[i].gop = 60;
		checkInputCfg(&pIc[i]);
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

int main(int argc, char *argv[])
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    
	s32Ret = APP_InitVenc();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("InitVEnc  failed with %d\n", s32Ret);
		return s32Ret;
	}

	VENC_CHN VencChn[2] = {0, 1};
	//VENC_CHN VencChn[1] = {0};
	int VencNum = sizeof(VencChn)/sizeof(VENC_CHN);
	APP_StartRtspServer(VencChn, VencNum);


    // input c+enter to exit application
	while(1){
		if(getchar() == 99)
			break;
	}

	//do resource release here.
	APP_StopRtspServer(VencChn, VencNum);

	//unbind vpss grp0 ch1->venc ch0
	SAMPLE_COMM_VPSS_UnBind_VENC(0, 1, 0);
	SAMPLE_COMM_VENC_Stop(0);
	//unbind vpss grp1 ch1->venc ch1
	SAMPLE_COMM_VPSS_UnBind_VENC(1, 1, 1);
	SAMPLE_COMM_VENC_Stop(1);
}