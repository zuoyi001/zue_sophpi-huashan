#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "tmwtypes.h"
#include "mmse_init.h"
#include "mmse.h"
#include "define.h"
#include "struct.h"
#include "agc_init.h"
#include "agc.h"
#include "lpaec.h"
#include "packfft.h"
#include "nlpaes.h"
#include "notch.h"
#include "dc.h"
#include "functrl.h"
#include "struct.h"
#include "dg.h"
#include "delay.h"
#include "cvi_ssp_interface.h"
#include "memalloc.h"
#ifndef PLATFORM_RTOS
not support to enter here
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "cvi_defines.h"
#include "cvi_type.h"
//#include "cvi_audio.h"
#else
//#include "malloc.h"
#endif



#ifdef PLATFORM_RTOS
int cviaud_dbg = 0;
#endif
#ifdef TYPE_ARM
int cviaud_dbg = 0;
#endif

#define UNUSED(x) ((x) = (x))

//int dbg_level = 2;
#define LOG_ERROR(fmt, args...) \
			do{\
				if (cviaud_dbg > 0) \
				printf("[error][%s][%d] "fmt, __func__, __LINE__, ##args); \
			}while(0);

#define LOG_PRINTF(fmt, args...) \
			do{\
				if (cviaud_dbg > 1) \
				printf("[%s][%d] "fmt, __func__, __LINE__, ##args); \
			}while(0);

#ifndef AEC_FRAME_LENGTH
#define AEC_FRAME_LENGTH (160)
#endif

typedef struct stSpkInfo {
	float fs; //sample rate
	agc_struct spk_agc_obj;
	ssp_para_struct spk_ssp_para_obj;
} ST_SPK_INFO;

struct st3AInfo {
	/* Delay History Buffer */
	DelayState *delay_state;
	/* DG History Buffer */
	dg_struct dg_obj;
	/* DC Filter History Buffer */
	dcfilter_struct dc_obj;
	/* Notch Filter History Buffer */
	qfilter_struct notch_obj;
	int notch_state[4];
	/* LP AEC History Buffer */
	/* This holds the state of the linear echo canceller, need one per channel */
	LinearEchoState* aec_state;
	/* NLP AES History Buffer */
	NonLinearEchoState *aes_state;
	short filter_length;
	short init_filter_length;
	//float fe_smooth_coef;
	boolean_T aec_supp16k_ena;

	float fs; //sample rate
	ssp_para_struct para_obj;
	int nenr_silence_time;
	int nenr_silence_cnt;
	/* NLP AES History Buffer */
	boolean_T st_vad;
	short aes_std_in[160];
	float aes_supp_coef1;
	float aes_supp_coef2;
	float std_thrd;

	//struct stAnrInfor * pstAnrInfor;
	/* NR History Buffer */
	short x_old[160];                                               /* input overlap buffer */
	float y_old[160];                                                /* output OLA buffer */
	float Xk_prev[321];                                           /* estimated clean speech power */
	float noise_mean[321];                                  /* noise mean */
	float noise_mu2[321];                                     /* estimated noise power */
	float amp_inst_band[5*3];
	float noisy_st_trough[5];
	float noisy_st_peak[5];
	float amp_inst_sum_hist[3];
	float vad_slope_cnt;
	float vad_slope;
	float vad_dr_cnt;
	float vad_dr;
	float vad_mmse;
	float vad_mmse_cnt;
	boolean_T speech_vad;
	float frame;
	float eta;
	float ksi_min;
	float vad_mmse_cnt_para;
	float initial_noise_pow_time;
	float nr_frame1_last_sample;
	// AGC info
	//struct stAgcInfor *pstAgcInfor;
	/* Input/Output Buffer */
	short ne_fixed_in[320];         /* near-end and fixed-point input */
	float ne_float_in[320];            /* near-end and floating-point input */
	short fe_fixed_in[320];          /* far-end and fixed-point input */
	//float fe_float_in[320];         /* far-end and floating-point input */
	float ne_float_out[160];         /* near-end and floating-point output */
	short ne_fixed_out[160];       /* near-end and fixed-point output */

	agc_struct agc_obj;
	int bInitOk;
};
#if 0//ndef PLATFORM_RTOS
static int _cviaudio_get_GetChipId(unsigned int *pu32ChipId)
{
	//mimic CVI_S32 CVI_SYS_GetChipId(unsigned int *pu32ChipId)
	static unsigned int id = 0xffffffff;
	int fd;

	if (id == 0xffffffff) {
		unsigned int tmp = 0;

		fd = open("/dev/cvi-base", O_RDWR | O_SYNC);
		if (fd == -1) {
			LOG_ERROR("Can't open device, cvi-base.\n");
			LOG_ERROR("system not ready\n");
			//abort();//no need abort
			return (-1);
		}

		if (ioctl(fd, IOCTL_READ_CHIP_ID, &tmp) < 0) {
			LOG_ERROR("ioctl IOCTL_READ_CHIP_ID failed\n");
			//abort();
			return (-1);
		}

		switch (tmp) {
		case E_CHIPID_CV1822:
			id = CVI1822;
		break;
		case E_CHIPID_CV1832:
			id = CVI1832;
		break;
		case E_CHIPID_CV1835:
			id = CVI1835;
		break;
		case E_CHIPID_CV1838:
			id = CVI1838;
		break;
		case E_CHIPID_CV1829:
			id = CVI1829;
		break;
		case E_CHIPID_CV1826:
			id = CVI1826;
		break;
		case E_CHIPID_CV1821:
			id = CVI1821;
		break;
		case E_CHIPID_CV1820:
			id = CVI1820;
		break;
		case E_CHIPID_CV1823:
			id = CVI1823;
		break;
		case E_CHIPID_CV1825:
			id = CVI1825;
		break;
		default:
			LOG_ERROR("unknown id(%#x)\n", tmp);
			return (-1);
		break;
		}

		close(fd);
	}

	*pu32ChipId = id;
	return 0;
}
#endif

void *audio_ssp_init(void *para_obj, unsigned int sample_rate)
{

	struct st3AInfo *pst3Ainfo = NULL;
	short cond1, cond2;
	unsigned int u32ChipId = 0xFF;
	ssp_para_struct *para_info = (ssp_para_struct *)para_obj;
	int turn_on_notch_filter = 0;
#if 0//ndef PLATFORM_RTOS
	cviAudioGetDbgMask(&cviaud_dbg);

	_cviaudio_get_GetChipId(&u32ChipId);

	if (u32ChipId == CVI1835 ||
		u32ChipId == CVI1832 ||
		u32ChipId == CVI1838 ||
		u32ChipId == CVI1822 ||
		u32ChipId == CVI1821 ||
		u32ChipId == CVI1829 ||
		u32ChipId == CVI1820 ||
		u32ChipId == CVI1823 ||
		u32ChipId == CVI1825 ||
		u32ChipId == CVI1826) {

		LOG_PRINTF("cvitek support chip on cviaudio_algo\n");
		}
	else {
		LOG_ERROR("[Error]Cvitek verification chip id failure...[%d]\n", u32ChipId);
		//return NULL;
	}
#endif

#ifndef PLATFORM_RTOS
	pst3Ainfo = (struct st3AInfo *)malloc(sizeof(struct st3AInfo));
#else
	//pst3Ainfo = (struct st3AInfo *)pvPortMalloc(sizeof(struct st3AInfo));
	pst3Ainfo = (struct st3AInfo *)mem_alloc(sizeof(struct st3AInfo));
#endif
	if(NULL == pst3Ainfo)
	{
	    LOG_ERROR("malloc failed.aec init failed.\n");
	    return NULL;
	}

	memset(pst3Ainfo,0,sizeof(struct st3AInfo));
	memcpy(&pst3Ainfo->para_obj,para_info,sizeof(ssp_para_struct));
	pst3Ainfo->fs = sample_rate;// pst3Ainfo->para_obj.para_sample_rate;

	printf("[SSP_VER20211202]user setting config param------dump---start\n");
	printf("[SSP_VER20211202]==========ssp interface param\n");
	printf("[SSP_VER20211202]sample_rate:%d.\n", (int)pst3Ainfo->fs);
	printf("[SSP_VER20211202]para_client_config:%d.\n",  pst3Ainfo->para_obj.para_client_config);
	printf("[SSP_VER20211202]para_fun_config:%d.\n",  pst3Ainfo->para_obj.para_fun_config);
	printf("[SSP_VER20211202]para_delay_sample:%d.\n",  pst3Ainfo->para_obj.para_delay_sample);
	printf("[SSP_VER20211202]para_dg_target:%d.\n",  pst3Ainfo->para_obj.para_dg_target);
	printf("[SSP_VER20211202]para_aec_init_filter_len:%d.\n",  pst3Ainfo->para_obj.para_aec_init_filter_len);
	printf("[SSP_VER20211202]para_aec_filter_len:%d.\n",  pst3Ainfo->para_obj.para_aec_filter_len);
	printf("[SSP_VER20211202]para_aes_std_thrd:%d.\n",  pst3Ainfo->para_obj.para_aes_std_thrd);
	printf("[SSP_VER20211202]para_aes_supp_coeff:%d.\n",  pst3Ainfo->para_obj.para_aes_supp_coeff);
	printf("[SSP_VER20211202]para_nr_init_sile_time:%d.\n",  pst3Ainfo->para_obj.para_nr_init_sile_time);
	printf("[SSP_VER20211202]para_nr_snr_coeff:%d.\n",  pst3Ainfo->para_obj.para_nr_snr_coeff);
	printf("[SSP_VER20211202]para_agc_max_gain:%d.\n",  pst3Ainfo->para_obj.para_agc_max_gain);
	printf("[SSP_VER20211202]para_agc_target_high:%d.\n",  pst3Ainfo->para_obj.para_agc_target_high);
	printf("[SSP_VER20211202]para_agc_target_low:%d.\n",  pst3Ainfo->para_obj.para_agc_target_low);
	printf("[SSP_VER20211202]para_agc_vad_ena:%d.\n",  pst3Ainfo->para_obj.para_agc_vad_ena);
	printf("[SSP_VER20211202]para_notch_freq:%d.\n",  pst3Ainfo->para_obj.para_notch_freq);
	printf("[SSP_VER20211202]para_nr_noise_coeff:%d.\n",  pst3Ainfo->para_obj.para_nr_noise_coeff);
	printf("[SSP_VER20211202]para_agc_vad_cnt:%d.\n",  pst3Ainfo->para_obj.para_agc_vad_cnt);
	printf("[SSP_VER20211202]para_agc_cut6_ena:%d.\n",  pst3Ainfo->para_obj.para_agc_cut6_ena);
	printf("[SSP_VER20211202]para_spk_fun_config:%d.\n",  pst3Ainfo->para_obj.para_spk_fun_config);
	printf("[SSP_VER20211202]para_spk_agc_max_gain:%d.\n",  pst3Ainfo->para_obj.para_spk_agc_max_gain);
	printf("[SSP_VER20211202]para_spk_agc_target_high:%d.\n",  pst3Ainfo->para_obj.para_spk_agc_target_high);
	printf("[SSP_VER20211202]para_spk_agc_target_low:%d.\n",  pst3Ainfo->para_obj.para_spk_agc_target_low);
	printf("[SSP_VER20211202]para_spk_agc_vad_ena:%d.\n",  pst3Ainfo->para_obj.para_spk_agc_vad_ena);
	printf("[SSP_VER20211202]para_spk_agc_vad_cnt:%d.\n",  pst3Ainfo->para_obj.para_spk_agc_vad_cnt);
	printf("[SSP_VER20211202]para_spk_agc_cut6_ena:%d.\n",  pst3Ainfo->para_obj.para_spk_agc_cut6_ena);
	printf("[SSP_VER20211202]user setting config param------dump---end\n");

	//TODO: check ssp init with ssp_main.c
	//reference from ssp_main.c
	if (pst3Ainfo->para_obj.para_notch_freq != 0) {
		printf("[SSP_VER20211202]turn on notch through para_notch_freq\n");
		turn_on_notch_filter = 1;
	} else if (pst3Ainfo->para_obj.para_fun_config & DCREMOVER_ENABLE) {
		printf("[SSP_VER20211202]turn on notch DC_REMOVE through fun_config\n");
	} else if (pst3Ainfo->para_obj.para_fun_config & NOTCH_ENABLE) {
		printf("[SSP_VER20211202]turn on notch  through fun_config\n");
	}

	cond1 = pst3Ainfo->para_obj.para_fun_config & DELAY_ENABLE;
	if (cond1) {
		delay_para(&pst3Ainfo->para_obj);
		pst3Ainfo->delay_state = delay_init(&pst3Ainfo->para_obj);
	}

	cond1 = pst3Ainfo->para_obj.para_fun_config & DG_ENABLE;
	if (cond1) {
		DG_para(&pst3Ainfo->dg_obj, &pst3Ainfo->para_obj);
	}

	cond1 = pst3Ainfo->para_obj.para_fun_config & DCREMOVER_ENABLE;
	if (cond1 || turn_on_notch_filter) {
		DC_remover_init(&pst3Ainfo->dc_obj, pst3Ainfo->fs);
	}

	cond1 = pst3Ainfo->para_obj.para_fun_config & NOTCH_ENABLE;
	if (cond1 || turn_on_notch_filter) {
		notch_para(&pst3Ainfo->notch_obj, &pst3Ainfo->para_obj, pst3Ainfo->fs);
		notch_init(&pst3Ainfo->notch_obj, pst3Ainfo->notch_state);
	}

	cond1 = pst3Ainfo->para_obj.para_fun_config & LP_AEC_ENABLE;
	cond2 = pst3Ainfo->para_obj.para_fun_config & NLP_AES_ENABLE;
	if (cond1 | cond2) {
		LP_AEC_para(&pst3Ainfo->para_obj, &pst3Ainfo->filter_length, &pst3Ainfo->init_filter_length);    /* creates parameters for linear acoustic echo canceller */
		pst3Ainfo->aec_state = LP_AEC_init(AEC_FRAME_LENGTH, pst3Ainfo->filter_length, pst3Ainfo->init_filter_length,
						pst3Ainfo->x_old, pst3Ainfo->fs);    /* creates AEC state */
	}
	if (cond2) {
		NLP_AES_para(&pst3Ainfo->para_obj, &pst3Ainfo->std_thrd, &pst3Ainfo->aes_supp_coef1, &pst3Ainfo->aes_supp_coef2);     /* creates parameters for nonlinear acoustic echo suppression */
		pst3Ainfo->aes_state = NLP_AES_init(AEC_FRAME_LENGTH, pst3Ainfo->fs, pst3Ainfo->std_thrd, pst3Ainfo->aes_supp_coef1, pst3Ainfo->aes_supp_coef2);    /* creates AES  state */
		pst3Ainfo->aes_state->echo_state = pst3Ainfo->aec_state;
	}

	cond1 = pst3Ainfo->para_obj.para_fun_config & NR_ENABLE;
	cond2 = pst3Ainfo->para_obj.para_fun_config & AGC_ENABLE;
	if (cond1 | cond2 | pst3Ainfo->para_obj.para_agc_vad_ena) {
		NR_para(&pst3Ainfo->para_obj, pst3Ainfo->fs, &pst3Ainfo->nenr_silence_time);      /* assign value to NR parameters */
  		NR_init(&pst3Ainfo->frame, pst3Ainfo->x_old, pst3Ainfo->y_old, pst3Ainfo->Xk_prev, pst3Ainfo->noise_mean, pst3Ainfo->noise_mu2,
			&pst3Ainfo->vad_slope_cnt, &pst3Ainfo->vad_slope, pst3Ainfo->amp_inst_band, &pst3Ainfo->vad_dr_cnt, &pst3Ainfo->vad_dr, pst3Ainfo->noisy_st_trough, pst3Ainfo->noisy_st_peak,
			&pst3Ainfo->vad_mmse, &pst3Ainfo->vad_mmse_cnt, pst3Ainfo->amp_inst_sum_hist, &pst3Ainfo->eta, &pst3Ainfo->ksi_min, pst3Ainfo->fs, &pst3Ainfo->vad_mmse_cnt_para,
			&pst3Ainfo->initial_noise_pow_time, &pst3Ainfo->nenr_silence_cnt);      /* initialize NR input arguments */
	}
	if (cond2) {
		AGC_para(&pst3Ainfo->agc_obj, &pst3Ainfo->para_obj, 1);    /* assign value to AGC parameters */
		AGC_init(&pst3Ainfo->agc_obj, &pst3Ainfo->para_obj, pst3Ainfo->fs, AEC_FRAME_LENGTH, 1);  /* initialize AGC input arguments */
	}

	pst3Ainfo->speech_vad = true;
	pst3Ainfo->st_vad = false;    /* default */
	pst3Ainfo->frame = 1.0F;
	pst3Ainfo->nenr_silence_cnt = 0;

	pst3Ainfo->para_obj.para_nr_init_sile_time = MIN(MAX(pst3Ainfo->para_obj.para_nr_init_sile_time, 0), 250);
	if (pst3Ainfo->fs == 16000.0F) {
		pst3Ainfo->initial_noise_pow_time = 14;
		pst3Ainfo->nenr_silence_time = pst3Ainfo->para_obj.para_nr_init_sile_time*2;
	} else {
		pst3Ainfo->initial_noise_pow_time = 7;
		pst3Ainfo->nenr_silence_time = pst3Ainfo->para_obj.para_nr_init_sile_time;
	}

	pst3Ainfo->bInitOk = true;

	return (void *)pst3Ainfo;
}



int audio_ssp_process(void *handle, short *mic_in, short *ref_in, short *dataout, int iLength)
{
	int hopsize = iLength;                                            /* input hop size */
	int i;
	int len = 2* iLength;
	short cond1, cond2, cond3, cond4, cond5, cond6;

	int len1 = len/2;
	int nFFT = 2*len;
	float alpha, alpha2;
	float tmp1 = 0.0;
	struct st3AInfo *pst3Ainfo = (struct st3AInfo *)handle;

	if (!pst3Ainfo) {
		LOG_PRINTF("audio_ssp_process error.\n");
		return -1;
	}

	if(!(pst3Ainfo && pst3Ainfo->bInitOk))
	{
		LOG_ERROR("audio_ssp_process error params.\n");
		return -1;
	}

	if(iLength != 160)
	{
		LOG_ERROR("audio_ssp_process parameter len must be 160 frames.\n");
		return -1;
	}
	if(!(mic_in || dataout))
	{
		LOG_ERROR("audio_ssp_process NULL parameers.\n");
		return -2;
	}
	if (ref_in == NULL) {
		pst3Ainfo->para_obj.para_fun_config &= (~(LP_AEC_ENABLE | NLP_AES_ENABLE));
	}
	//port from ssp_main.c
	if (pst3Ainfo->para_obj.para_client_config == 2) {
		float tmp = 0;

		for (i = 0; i < hopsize; i++) {
			tmp = (float)(ref_in[i])/(float)(32768.0F) * 15.84F;
  			tmp = MIN(MAX(tmp, -1.0F), 1.0F);
  			ref_in[i] = ROUND_POS(tmp*(short)(32767));
			mic_in[i] = ROUND_POS(((float)(mic_in[i])/(float)(32768.0F) * 0.5F)*(short)(32767));
		}
	}


	if(!(pst3Ainfo->para_obj.para_fun_config)) {
		for(i=0;i< hopsize;i++)
			dataout[i]=mic_in[i];
	} else {
		/* Delay ref signal frame-by-frame processing */
		cond1 = pst3Ainfo->para_obj.para_fun_config & DELAY_ENABLE;
		if (cond1) {
			delay_ref(ref_in, pst3Ainfo->delay_state, hopsize);
		}

		/* Apply DG frame-by-frame processing */
		cond1 = pst3Ainfo->para_obj.para_fun_config & DG_ENABLE;
		if (cond1) {
			apply_DG(mic_in, mic_in, &pst3Ainfo->dg_obj, hopsize);
		}

		/* DC filter frame-by-frame processing */
		cond1 = pst3Ainfo->para_obj.para_fun_config & DCREMOVER_ENABLE;
		if (cond1) {
			DC_remover(mic_in, mic_in, &pst3Ainfo->dc_obj, hopsize);
		}

		/* Notch filter frame-by-frame processing */
		cond1 = pst3Ainfo->para_obj.para_fun_config & NOTCH_ENABLE;
		if (cond1) {
			notch_filter(mic_in, mic_in, &pst3Ainfo->notch_obj, hopsize);
		}


		/* Linear Processing AEC frame-by-frame processing */
		cond1 = pst3Ainfo->para_obj.para_fun_config & LP_AEC_ENABLE;
		cond2 = pst3Ainfo->para_obj.para_fun_config & NLP_AES_ENABLE;
		if (cond1 | cond2) {
			LP_AEC(pst3Ainfo->aec_state, mic_in, ref_in, dataout, NULL);
			//memcpy(&pst3Ainfo->aes_std_in[0], &mic_in[0], hopsize*2);
			if (cond2) {
				for (i = 0; i < hopsize; i++)
					pst3Ainfo->aes_state->aes_std_in[i] = mic_in[i];
			}
		}

		if (cond1 | cond2) {
			memcpy(&mic_in[0], &pst3Ainfo->x_old[0], hopsize*2);    /* the copy unit in memcpy is byte */
			memcpy(&mic_in[hopsize], &dataout[0], hopsize*2);
		} else {
			memcpy(&mic_in[hopsize], &mic_in[0], hopsize*2);    /* go first to avoid overwriting by x_old */
			memcpy(&mic_in[0], &pst3Ainfo->x_old[0], hopsize*2);
		}

		memcpy(&pst3Ainfo->x_old[0], &mic_in[hopsize], hopsize*2);    /* update input overlap data */

		cond3 = pst3Ainfo->para_obj.para_fun_config & NR_ENABLE;
		cond4 = pst3Ainfo->para_obj.para_fun_config & AGC_ENABLE;
		if (!(cond2 | cond3 | cond4)) {
			/* Compensate DG frame-by-frame processing */
			cond1 = pst3Ainfo->para_obj.para_fun_config & DG_ENABLE;
			if (cond1) {
				compen_DG(&mic_in[hopsize], &mic_in[hopsize], &pst3Ainfo->dg_obj, hopsize);
			}
			for (i = 0; i < hopsize; i++)
				dataout[i] = mic_in[hopsize + i];    /* save MCPS if NR+AGC off */
		} else {
			/*  Transform to floating-point with float precision for input signals, input sample have to be 16-bit length */
			for (i = 0; i < len; i++) {
				pst3Ainfo->ne_float_in[i] = (float)(mic_in[i])/(float)(32768.0F);
			}

			if (cond3 | pst3Ainfo->para_obj.para_agc_vad_ena) {
				/* Stationary NR frame-by-frame processing */
				if (pst3Ainfo->nenr_silence_cnt >= pst3Ainfo->nenr_silence_time) {
				//printf("go to NR\n");
				NR(pst3Ainfo->ne_float_in,
					(float)len,
					(float)len1,
					(float)nFFT,
					&pst3Ainfo->frame,
					pst3Ainfo->y_old,
					pst3Ainfo->Xk_prev,
					pst3Ainfo->noise_mean,
					pst3Ainfo->noise_mu2,
					&pst3Ainfo->vad_slope_cnt,
					&pst3Ainfo->vad_slope,
					pst3Ainfo->amp_inst_band,
					&pst3Ainfo->vad_dr_cnt,
					&pst3Ainfo->vad_dr,
					pst3Ainfo->noisy_st_trough,
					pst3Ainfo->noisy_st_peak,
					&pst3Ainfo->vad_mmse,
					&pst3Ainfo->vad_mmse_cnt,
					pst3Ainfo->amp_inst_sum_hist,
					pst3Ainfo->fs,
					pst3Ainfo->eta,
					pst3Ainfo->ksi_min,
					pst3Ainfo->vad_mmse_cnt_para,
					pst3Ainfo->initial_noise_pow_time,
					pst3Ainfo->ne_float_out,
					&pst3Ainfo->speech_vad);

				if (1 == (unsigned short)pst3Ainfo->frame)
					pst3Ainfo->nr_frame1_last_sample = pst3Ainfo->ne_float_out[hopsize-1];
				if (2 == (unsigned short)pst3Ainfo->frame) {
					alpha2 = 1.0F/160.0F;
					alpha = 1 - alpha2;
					pst3Ainfo->ne_float_out[0] = alpha * pst3Ainfo->nr_frame1_last_sample + alpha2 * pst3Ainfo->ne_float_out[0];
					for (i = 0; i < hopsize-1; i++) {
						alpha2 = (float)(1.0F + i)/160.0F;
						alpha = 1 - alpha2;
						pst3Ainfo->ne_float_out[i+1] =  alpha * pst3Ainfo->ne_float_out[i] + alpha2 * pst3Ainfo->ne_float_out[i+1];
					}
				}
				} else {
					for (i = 0; i < hopsize; i++) {
						pst3Ainfo->ne_float_out[i] = 0.0F;
					}
				}


			}

			if (cond3) {
				/*  Transform to fixed-point with 16-bit length for output */
				for (i = 0; i < hopsize; i++) {
					pst3Ainfo->ne_float_out[i] = MIN(MAX(pst3Ainfo->ne_float_out[i], -1.0F), 1.0F);    /* saturation protection to avoid overflow */
					if (pst3Ainfo->ne_float_out[i] < 0.0F)
						dataout[i] = ROUND_NEG((pst3Ainfo->ne_float_out[i]*(int)(32768)));
					else
						dataout[i] = ROUND_POS((pst3Ainfo->ne_float_out[i]*(short)(32767)));
				}
			} else {
				for (i = 0; i < hopsize; i++)
					dataout[i] = mic_in[hopsize + i];
			}

			/* Nonlinear Processing AES frame-by-frame processing */
			if (cond2) {
				NLP_AES(pst3Ainfo->aes_state, dataout, pst3Ainfo->frame, pst3Ainfo->speech_vad);
				pst3Ainfo->st_vad = (boolean_T)pst3Ainfo->aes_state->st_vad;
			}

			if (2 == pst3Ainfo->para_obj.para_client_config) {
				float tmp = 0;

				for (i = 0; i < hopsize; i++) {
					tmp = (float)(dataout[i])/(float)(32768.0F) * 1.9952F;
					tmp = MIN(MAX(tmp, -1.0F), 1.0F);
					dataout[i] = ROUND_POS(tmp*(short)(32767));
				}
			}

			/* Compensate DG frame-by-frame processing */
			cond1 = pst3Ainfo->para_obj.para_fun_config & DG_ENABLE;
			if (cond1) {
				compen_DG(dataout, dataout, &pst3Ainfo->dg_obj, hopsize);
			}

			/* AGC frame-by-frame processing */
			if (cond4) {
				//printf("got to AGC\n");
				AGC(dataout,
					dataout,
					&pst3Ainfo->agc_obj,
					&pst3Ainfo->para_obj,
					pst3Ainfo->speech_vad,
					pst3Ainfo->st_vad,
					pst3Ainfo->nenr_silence_time,
					pst3Ainfo->nenr_silence_cnt,
					1);
			}
		}
    	}
	//TODO: start from here!!!
	if (pst3Ainfo->nenr_silence_cnt < pst3Ainfo->nenr_silence_time)
		pst3Ainfo->nenr_silence_cnt += 1;
	else
		pst3Ainfo->nenr_silence_cnt = pst3Ainfo->nenr_silence_time + 1;    /* avoid overflow */

	if (pst3Ainfo->nenr_silence_cnt > pst3Ainfo->nenr_silence_time) {
		if (pst3Ainfo->frame < pst3Ainfo->initial_noise_pow_time)    /* avoid overflow, 140ms */
			pst3Ainfo->frame += 1;
		else
			pst3Ainfo->frame = pst3Ainfo->initial_noise_pow_time + 1;	    /* stop initial noise power estimate */
	}

	return 0;
}


int audio_ssp_deinit(void *handle)
{
	struct st3AInfo *pst3Ainfo = (struct st3AInfo *)handle;

	if (!pst3Ainfo) {
		LOG_ERROR("audio_ssp_deinit error.\n");
		return -1;
	}
	short cond1, cond2, cond3, cond4, cond6;

	if (!pst3Ainfo->bInitOk) {
		LOG_ERROR("audio_ssp_deinit error params.\n");
		return -1;
	}

	cond1 = pst3Ainfo->para_obj.para_fun_config & LP_AEC_ENABLE;
	cond2 = pst3Ainfo->para_obj.para_fun_config & NLP_AES_ENABLE;
	cond3 = pst3Ainfo->para_obj.para_fun_config & NR_ENABLE;
	cond4 = pst3Ainfo->para_obj.para_fun_config & AGC_ENABLE;
	cond6 = pst3Ainfo->para_obj.para_fun_config & DELAY_ENABLE;
	if (cond1 | cond2) {
		LP_AEC_free(pst3Ainfo->aec_state);
	}
	if (cond2) {
		NLP_AES_free(pst3Ainfo->aes_state);
	}
	if (cond3 | cond4) {
		speech_fft_free(nr_fft_table);
	}
	if (cond6) {
		delay_free(pst3Ainfo->delay_state);
	}
#ifndef PLATFORM_RTOS
	free(pst3Ainfo);
#else
	vPortFree(pst3Ainfo);
#endif
	//pst3Ainfo = NULL;
	pst3Ainfo->bInitOk=0;

	return 0;
}


void *audio_ssp_spk_init(void *para_obj, unsigned int sample_rate)
{

	ST_SPK_INFO *pstSpkInfo = NULL;
	short cond1, cond2;
	unsigned int u32ChipId = 0xFF;
	ssp_para_struct * para_info = (ssp_para_struct *)para_obj;
	int turn_on_notch_filter = 0;
	int hopsize = 160;    /* input hop size */
#ifndef PLATFORM_RTOS
	//cviAudioGetDbgMask(&cviaud_dbg);
	pstSpkInfo = ((ST_SPK_INFO *)calloc(1, sizeof(ST_SPK_INFO)));
#else
	//pstSpkInfo = (ST_SPK_INFO *)pvPortMalloc(sizeof(ST_SPK_INFO));
	pstSpkInfo = (ST_SPK_INFO *)mem_alloc(sizeof(ST_SPK_INFO));
#endif


	if(NULL == pstSpkInfo)
	{
	    LOG_ERROR("calloc failed audio_ssp_spk_init failed.\n");
	    return NULL;
	}

	memcpy(&pstSpkInfo->spk_ssp_para_obj, para_info,sizeof(ssp_para_struct));
	pstSpkInfo->fs = sample_rate;//pstSpkInfo->spk_ssp_para_obj.para_sample_rate;


	LOG_PRINTF("[SSP_VER20211202]user setting config param------dump---start\n");
	LOG_PRINTF("[SSP_VER20211202]==========ssp interface param\n");
	LOG_PRINTF("[SSP_VER20211202]sample_rate:%d\n", (int)pstSpkInfo->fs);
	LOG_PRINTF("[SSP_VER20211202]para_client_config:%d.\n",  pstSpkInfo->spk_ssp_para_obj.para_client_config);
	LOG_PRINTF("[SSP_VER20211202]para_fun_config:%d.\n",  pstSpkInfo->spk_ssp_para_obj.para_fun_config);
	LOG_PRINTF("[SSP_VER20211202]para_spk_fun_config:%d.\n",  pstSpkInfo->spk_ssp_para_obj.para_spk_fun_config);
	LOG_PRINTF("[SSP_VER20211202]para_spk_agc_max_gain:%d.\n",  pstSpkInfo->spk_ssp_para_obj.para_spk_agc_max_gain);
	LOG_PRINTF("[SSP_VER20211202]para_spk_agc_target_high:%d.\n",  pstSpkInfo->spk_ssp_para_obj.para_spk_agc_target_high);
	LOG_PRINTF("[SSP_VER20211202]para_spk_agc_target_low:%d.\n",  pstSpkInfo->spk_ssp_para_obj.para_spk_agc_target_low);
	LOG_PRINTF("[SSP_VER20211202]para_spk_agc_vad_ena:%d.\n",  pstSpkInfo->spk_ssp_para_obj.para_spk_agc_vad_ena);
	LOG_PRINTF("[SSP_VER20211202]para_spk_agc_vad_cnt:%d.\n",  pstSpkInfo->spk_ssp_para_obj.para_spk_agc_vad_cnt);
	LOG_PRINTF("[SSP_VER20211202]para_spk_agc_cut6_ena:%d.\n",  pstSpkInfo->spk_ssp_para_obj.para_spk_agc_cut6_ena);
	LOG_PRINTF("[SSP_VER20211202]user setting config param------dump---end\n");



	//reference from ssp_main.c of cond5 spk condition
	if ((pstSpkInfo->spk_ssp_para_obj.para_spk_fun_config) & SPK_AGC_ENABLE) {
		AGC_para(&pstSpkInfo->spk_agc_obj, &pstSpkInfo->spk_ssp_para_obj, 2);
		AGC_init(&pstSpkInfo->spk_agc_obj, &pstSpkInfo->spk_ssp_para_obj, pstSpkInfo->fs, hopsize, 2);
	} else {
		printf("[Error]Not a valid mask for AO VQE[0x%x]]\n", pstSpkInfo->spk_ssp_para_obj.para_spk_fun_config);
		return NULL;
	}

	return (void *)pstSpkInfo;
}


int audio_ssp_spk_process(void *handle, short *spk_in, short *dataout, int iLength)
{
	ST_SPK_INFO *pstSpkInfo = (ST_SPK_INFO *)handle;

	UNUSED(iLength);
	if (!pstSpkInfo) {
		LOG_ERROR("audio_ssp_spk_process error.\n");
		return -1;
	}

	//everyime entering hopsize = 160 samples
	//AGC(fe_fixed_in, fe_fixed_out, &spk_agc_obj, &ssp_para_obj, hopsize, true, false, 0, 0, fs, 2);
	AGC(spk_in,
		dataout,
		&pstSpkInfo->spk_agc_obj,
		&pstSpkInfo->spk_ssp_para_obj,
		true,
		false,
		0,
		0,
		2);

	return 0;
}

int audio_ssp_spk_deinit(void *handle)
{
	ST_SPK_INFO *pstSpkInfo = (ST_SPK_INFO *)handle;

	if (!pstSpkInfo) {
		LOG_ERROR("audio_ssp_spk_deinit handle not exist!!\n");
		return -1;
	} else {
#ifndef PLATFORM_RTOS
		free(pstSpkInfo);
#else
		vPortFree(pstSpkInfo);
#endif
		pstSpkInfo = NULL;

	}

	return 0;
}



