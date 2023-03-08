#include <stdio.h>
#include <stdlib.h>

#include "app_ipcam_os.h"
#include "app_ipcam_selftest.h"
#include "app_ipcam_mq.h"

#include "app_ipcam_paramparse.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define CVI_CMD_CLIENT_ID_MT_TOOL (CVI_MQ_CLIENT_ID_USER_0)
#define CVI_CMD_CHANNEL_ID_MT(mt_id) (0x00 + (mt_id))


/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

typedef void * CVI_CMD_HANDLE_T;

typedef struct APP_PARAM_CMD_CTX_S {
    volatile uint32_t           shutdown;
    pthread_mutex_t             mt_mutex;
    // APP_PARAM_TASK_HANDLE_T      mt_task;

    // event task
    APP_PARAM_TASK_HANDLE_T      event_task;

    // MQ task
    CVI_MQ_ENDPOINT_HANDLE_t    mq_ep;
} APP_PARAM_CMD_CTX_T, *APP_PARAM_CMD_CTX_HANDLE_T;

typedef CVI_S32 (*pfpCmdCallBack)(CVI_MQ_MSG_t *msg, CVI_VOID *userdate);
int app_ipcam_CmdTask_None(CVI_MQ_MSG_t *msg, CVI_VOID *userdate);

typedef struct APP_PARAM_CMD_CB_S {
    pfpCmdCallBack CmdCallBack;
    uint32_t flags;
} APP_PARAM_CMD_CB_T;

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static CVI_CMD_HANDLE_T pCmdHdl;

static APP_PARAM_CMD_CB_T CmdCbTbl[] = {
    {app_ipcam_CmdTask_VideoAttr_Set,        0},
    {app_ipcam_CmdTask_Osd_Switch,           0},
    {app_ipcam_CmdTask_Cover_Switch,         0},
    {app_ipcam_CmdTask_Rect_Switch,          0},
    #ifdef AUDIO_SUPPORT
    {app_ipcam_CmdTask_AudioAttr_Set,        0},
    #else
    {app_ipcam_CmdTask_None,                 0},
    #endif
    #ifdef AI_SUPPORT
    {app_ipcam_CmdTask_Ai_PD_Switch,         0},
    {app_ipcam_CmdTask_Ai_MD_Switch,         0},
    #ifdef FACE_SUPPORT
    {app_ipcam_CmdTask_Ai_FD_Switch,         0},
    #else
    {app_ipcam_CmdTask_None,                 0},
    #endif
    #else
    {app_ipcam_CmdTask_None,                 0},
    {app_ipcam_CmdTask_None,                 0},
    {app_ipcam_CmdTask_None,                 0},
    #endif
    {app_ipcam_CmdTask_Auto_Rgb_Ir_Switch,   0},
    {app_ipcam_CmdTask_Setect_Pq,            0},
    {app_ipcam_CmdTask_Flip_Switch,          0},
    {app_ipcam_CmdTask_Mirror_Switch,        0},
    {app_ipcam_CmdTask_Flip_Mirror_Switch,   0},
    {app_ipcam_CmdTask_Rotate_Switch,        0},
    #ifdef RECORD_SUPPORT
    {app_ipcam_CmdTask_Record_Task,          0},
    #else
    {app_ipcam_CmdTask_None,                 0},
    #ifdef MP3_SUPPORT
    {app_ipcam_CmdTask_Mp3_Play,             0}, 
    #else
    {app_ipcam_CmdTask_None,                 0},
    #endif
#endif
};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/


static CVI_S32 _Cmd_Mq_CallBack(
    CVI_MQ_ENDPOINT_HANDLE_t ep, 
    CVI_MQ_MSG_t *msg, 
    CVI_VOID *ep_arg) 
{
    (CVI_VOID)(ep);

    printf("cmd_cb: rx, target_id = %08x, len = %d, ep_arg = %p\n", msg->target_id, msg->len, ep_arg);
    printf("cmd_cb:     arg1 = 0x%08x, arg2 = 0x%08x\n", msg->arg1, msg->arg2);
    printf("cmd_cb:     seq_no = 0x%04x, time = %"PRId64"\n", msg->seq_no, msg->crete_time);
    if (msg->len > (int)CVI_MQ_MSG_HEADER_LEN) {
        printf("cmd_cb:     payload [%02x %02x %02x %02x %02x]\n", msg->payload[0], msg->payload[1],
               msg->payload[2], msg->payload[3], msg->payload[4]);
        printf("msg->payload === %s \n", msg->payload);
    }

    int cmd_id = msg->arg1;
    if (cmd_id >= 0 && cmd_id < (int)(sizeof(CmdCbTbl) / sizeof(APP_PARAM_CMD_CB_T))) {
        if (CmdCbTbl[cmd_id].CmdCallBack == NULL) {
            printf("cmd_id %d not handled\n", cmd_id);
            return -1;
        }

        return CmdCbTbl[cmd_id].CmdCallBack(msg, ep_arg);
    } else {
        printf("cmd_id %d %d out of range\n", cmd_id, (int)(sizeof(CmdCbTbl) / sizeof(APP_PARAM_CMD_CB_T)));
        return 0;
    }
    
}

static CVI_VOID *app_ipcam_CmdEvent_Entry(CVI_VOID *arg)
{
    APP_PARAM_CMD_CTX_HANDLE_T pCmdHandle = (APP_PARAM_CMD_CTX_HANDLE_T)arg;

    // start mq
    CVI_MQ_ENDPOINT_CONFIG_t mq_config = {0};
    mq_config.name = "cmd_mq";
    mq_config.id = CVI_MQ_ID(CVI_CMD_CLIENT_ID_MT_TOOL, CVI_CMD_CHANNEL_ID_MT(0));
    mq_config.recv_cb = _Cmd_Mq_CallBack;
    mq_config.recv_cb_arg = (CVI_VOID *)pCmdHandle;

    int rc = app_ipcam_MqEndpoint_Create(&mq_config, &pCmdHandle->mq_ep);
    if (rc != CVI_OSAL_SUCCESS) {
        printf("app_ipcam_MqEndpoint_Create failed\n");
        exit(-1);
    }

    while (!pCmdHandle->shutdown) {
        app_ipcam_task_sleep(10000);  // 10 ms
    }

    // cleanup mq
    app_ipcam_MqEndpoint_Destroy(pCmdHandle->mq_ep);

    return NULL;
}

static CVI_S32 app_ipcam_CmdEvent_Start(APP_PARAM_CMD_CTX_HANDLE_T pCmdHandle)
{
    APP_PARAM_TASK_ATTR_T t_attr;
    t_attr.name = "cmd_event";
    t_attr.entry = app_ipcam_CmdEvent_Entry;
    t_attr.param = (CVI_VOID *)pCmdHandle;
    t_attr.priority = 80;
    t_attr.detached = false;
    
    /*create event_task parm*/
    CVI_S32 rc = app_ipcam_task_create(&t_attr, &pCmdHandle->event_task);
    if (rc != CVI_OSAL_SUCCESS) {
        printf("cmd_event task create failed, %d\n", rc);
        return -1;
    }

    return 0;
}

int app_ipcam_CmdTask_Create(void)
{
    APP_PARAM_CMD_CTX_HANDLE_T pCmdHandle = NULL;
    
    pCmdHandle = (APP_PARAM_CMD_CTX_HANDLE_T)calloc(sizeof(APP_PARAM_CMD_CTX_T), 1);

    pthread_mutex_init(&pCmdHandle->mt_mutex, NULL);

    app_ipcam_CmdEvent_Start(pCmdHandle);

    pCmdHdl = (CVI_CMD_HANDLE_T)pCmdHandle;

    printf("CmdTask Socket Service create success!\n");

    return CVI_SUCCESS;
}


static CVI_S32 cmd_stop_event_task(APP_PARAM_CMD_CTX_HANDLE_T mt)
{
    CVI_S32 rc = app_ipcam_task_join(mt->event_task);
    if (rc != CVI_OSAL_SUCCESS) {
        printf("cmd_event task join failed, %d\n", rc);
        return -1;
    }
    app_ipcam_task_destroy(&mt->event_task);

    return CVI_SUCCESS;
}

int app_ipcam_CmdTask_Destroy(CVI_CMD_HANDLE_T hdl)
{
    APP_PARAM_CMD_CTX_HANDLE_T pCmdHandle = (APP_PARAM_CMD_CTX_HANDLE_T)hdl;
    // send shutdown to self
    pCmdHandle->shutdown = 1;
    CVI_S32 s32Ret = 0;

    // wait for exit
    while (!pCmdHandle->shutdown) {
        app_ipcam_task_sleep(20000);
    }

    pthread_mutex_destroy(&pCmdHandle->mt_mutex);

    s32Ret = cmd_stop_event_task(pCmdHandle);
    if (s32Ret != CVI_SUCCESS) {

    }

    free(pCmdHandle);

    printf("app_IPC_Cmd Service destroy success\n");

    return CVI_SUCCESS;
}

int app_ipcam_CmdTask_None(CVI_MQ_MSG_t *msg, CVI_VOID *userdate)
{
    return 0;
}


