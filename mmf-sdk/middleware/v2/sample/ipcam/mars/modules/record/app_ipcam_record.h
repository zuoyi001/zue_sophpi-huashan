#include "cvi_comm_aio.h"
#include "cvi_comm_vb.h"
#include "linux/cvi_common.h"
#include "linux/cvi_comm_venc.h"
#include "app_ipcam_comm.h"
#include "cvi_record.h"
#include "app_ipcam_mq.h"

int app_ipcam_Record_Init();

int app_ipcam_Record_UnInit();

CVI_S32 app_ipcam_Record_AudioInput(PAYLOAD_TYPE_E enType, AUDIO_STREAM_S *pstStream, CVI_S32 stFrameNum);

CVI_S32 app_ipcam_Record_VideoInput(PAYLOAD_TYPE_E enType, VENC_STREAM_S *pstStream, CVI_S32 stFrameNum);

CVI_S32 app_ipcam_Record_StartReplay(char *pReplayTime);

CVI_VOID app_ipcam_Record_StopReplay();

int app_ipcam_CmdTask_Record_Task(CVI_MQ_MSG_t *msg, CVI_VOID *userdate);

int app_ipcam_Set_Record_Status(int iStatus);

int app_ipcam_Get_Record_Status();

CVI_S32 app_ipcam_Set_Record_Check(char *pReplayTime);

CVI_S32 app_ipcam_Get_Record_Check(char **pCheckSeg);

CVI_VOID app_ipcam_Clean_Record_Seg();

