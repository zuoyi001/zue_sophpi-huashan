#include <pthread.h>
#include <sys/prctl.h>
#include <string.h>
#include "cvi_hfsm.h"
#include "cvi_log.h"
#include "queue.h"

typedef struct CviHfsmNum {
    CVI_STATE_S                 *parent;
    CVI_STATE_S                 *CurState;
}CVI_HFSM_NUM_S;

typedef struct CviHfsmInfo {
    pthread_t                   taskid;
    pthread_mutex_t             mutex;
    pthread_mutex_t             MessageMutex;
    pthread_cond_t              MessageCond;
    QueueHandle                 queueHdl;
    CVI_HFSM_EVENT_CALLBACK_FN  EventCallback;
    bool                        run;
    bool                        stop;
    uint32_t                    MaxStateCnt;
    void                        **StateNode;
}CVI_HFSM_S;

uint32_t g_SysStateCnt = 0;
int32_t g_iSysCurState = -1;
int32_t g_iSysPrevState = -1;
int32_t g_iSysNextState = -1;

static int32_t CVI_HFSM_StateOpen(CVI_HFSM_S* hfsm, uint32_t mode_id)
{
    CVI_STATE_S* pCurrMode = NULL;
    uint32_t i = 0;
    for (i = 0; i < g_SysStateCnt; i++) {
        CVI_HFSM_NUM_S *hfsm_num = (CVI_HFSM_NUM_S *)hfsm->StateNode[i];
        pCurrMode = hfsm_num->CurState;
        if (pCurrMode->stateID == mode_id) {
            break;
        }
    }
    if (i >= g_SysStateCnt) {
        CVI_LOGE("State id = %d is invalid!\n", mode_id);
        return -1;
    }
    CVI_LOGD("Mode {%s} Open begin\n", pCurrMode->name);
    if(pCurrMode->open)
        pCurrMode->open();
    g_iSysCurState = i;
    CVI_LOGD("Mode {%s} Open end\n", pCurrMode->name);

    return 0;

}

static int32_t CVI_HFSM_StateClose(CVI_HFSM_S* hfsm)
{
    CVI_STATE_S* pCurrMode;

    if((g_iSysCurState >= (int32_t)g_SysStateCnt)
    || (g_iSysCurState < 0) )
    {
        if(g_iSysCurState != -1)
        CVI_LOGE("Current mode (id: 0x%02x) is not exist! ignored.\n", g_iSysCurState);
        return -1;
    }
    CVI_HFSM_NUM_S *hfsm_num = (CVI_HFSM_NUM_S *)hfsm->StateNode[g_iSysCurState];
    pCurrMode = hfsm_num->CurState;
    CVI_LOGD("Mode {%s} Close begin\n",pCurrMode->name);
    if(pCurrMode->close)
        pCurrMode->close();
    CVI_LOGD("Mode {%s} Close end\n", pCurrMode->name);
    g_iSysPrevState = g_iSysCurState; // Save state
    g_iSysCurState  = -1;

    return 0;
}

static int32_t System_ChangeMode(CVI_HFSM_S* hfsm, uint32_t mode_id)
{
    CVI_LOGD("Mode Begin (id: 0x%02x) ...\n", mode_id);
    int32_t s32Ret = 0;

    //close current
    s32Ret = CVI_HFSM_StateClose(hfsm);
    if (s32Ret != 0) {
        CVI_LOGE("close state failed !\n");
        return -1;
    }

    //open new
    s32Ret = CVI_HFSM_StateOpen(hfsm, mode_id);
    if (s32Ret != 0) {
        CVI_LOGE("open state failed !\n");
        return -1;
    }

    return 0;
}

static void* Message_TaskQueue_Proc(void* pvParam)
{
    int32_t s32Ret = 0;
    prctl(PR_SET_NAME, __FUNCTION__, 0, 0, 0);
    CVI_HFSM_S* hfsm = (CVI_HFSM_S*)pvParam;
    QueueHandle queue = hfsm->queueHdl;

    while (hfsm->run)
    {
        if (0 == HFSM_Queue_GetLen(queue) || hfsm->stop == true)
        {
            usleep(500 * 1000);
            continue;
        }
        else
        {
            CVI_MESSAGE_S msg = {0};
            s32Ret = HFSM_Queue_Pop(queue, &msg);

            if (0 != s32Ret) {
                CVI_LOGE("Queue_Pop failed! \n");
                continue;
            }

            CVI_HFSM_EVENT_INFO_S eventInfo = {0};
            CVI_HFSM_NUM_S *hfsm_num = (CVI_HFSM_NUM_S *)hfsm->StateNode[g_iSysCurState];
            CVI_STATE_S *state = hfsm_num->CurState;
            if (state == NULL) {
                CVI_LOGE("State is null! \n");
                continue;
            }
            uint32_t stateID = state->stateID;
            s32Ret = state->processMessage(&msg, state->argv, &stateID);
            if (s32Ret != 0) {
                CVI_STATE_S *parent = hfsm_num->parent;
                if (parent != NULL) {
                    s32Ret = parent->processMessage(&msg, parent->argv, &stateID);
                    if (s32Ret != 0) {
                        eventInfo.enEventCode = CVI_HFSM_EVENT_UNHANDLE_MSG;
                    }
                } else {
                    eventInfo.enEventCode = CVI_HFSM_EVENT_UNHANDLE_MSG;
                }
            }
            if (stateID != state->stateID) {
                s32Ret = System_ChangeMode(hfsm, stateID);
                if (s32Ret != 0) {
                    eventInfo.enEventCode = CVI_HFSM_EVENT_TRANSTION_ERROR;
                }
            }
            eventInfo.pstunHandlerMsg = &msg;
            hfsm->EventCallback(hfsm, &eventInfo);

        }
    }

    return NULL;
}

int32_t CVI_HFSM_Create(CVI_HFSM_ATTR_S *fsmAttr, CVI_HFSM_HANDLE *hfsm)
{
    int32_t s32Ret = 0;
    uint32_t i = 0;

    if (fsmAttr == NULL || hfsm == NULL) {
        CVI_LOGE("Create Hfsm failed! \n");
        return -1;
    }

    CVI_HFSM_S *t = malloc(sizeof(CVI_HFSM_S));
    if (t == NULL) {
        CVI_LOGE("Create Hfsm failed! malloc failed!\n");
        return -1;
    }
    t->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    t->MessageMutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    t->MessageCond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    t->EventCallback = fsmAttr->fnHfsmEventCallback;
    t->MaxStateCnt = fsmAttr->u32StateMaxAmount;
    t->StateNode = (void **)malloc(sizeof(void *) * fsmAttr->u32StateMaxAmount);
    t->run = true;
    t->stop = true;

    for (i = 0; i < fsmAttr->u32StateMaxAmount; i++) {
        t->StateNode[i] = (void *)malloc(sizeof(CVI_HFSM_NUM_S));
    }
    t->queueHdl = HFSM_Queue_Create(sizeof(CVI_MESSAGE_S), fsmAttr->u32MessageQueueSize);
    if (t->queueHdl == NULL) {
        CVI_LOGE("Queue_Create failed!\n");
        return -1;
    }

    s32Ret = pthread_create(&t->taskid, NULL, Message_TaskQueue_Proc, (void*)t);
    if (s32Ret != 0) {
        CVI_LOGE("pthread_create failed!\n");
        return -1;
    }

    *hfsm = (void*)t;

    return 0;
}

int32_t CVI_HFSM_Destroy(CVI_HFSM_HANDLE hfsm)
{
    CVI_HFSM_S *t = (CVI_HFSM_S *)hfsm;
    if (NULL == t) {
        CVI_LOGE("CVI_HFSM_Destroy failed!\n");
        return -1;
    }
    LOCK(t->mutex);
    t->run = false;
    pthread_join(t->taskid, NULL);
    HFSM_Queue_Destroy(t->queueHdl);

    uint32_t i = 0;

    for (i = 0; i < t->MaxStateCnt; i++) {
        SAFE_FREE(t->StateNode[i]);
    }
    SAFE_FREE(t->StateNode);
    UNLOCK(t->mutex);
    DESTROY(t->mutex);
    DESTROY(t->MessageMutex);
    COND_DESTROY(t->MessageCond);
    SAFE_FREE(t);

    return 0;
}

int32_t CVI_HFSM_AddState(CVI_HFSM_HANDLE hfsm, CVI_STATE_S *state, CVI_STATE_S *parent)
{
    uint32_t i = g_SysStateCnt, j = 0;
    CVI_HFSM_S *t = (CVI_HFSM_S *)hfsm;
    if(state == NULL || t == NULL) {
        CVI_LOGE("This state is invalid!\n");
        return -1;
    }
    LOCK(t->mutex);
    CVI_HFSM_NUM_S *hfsm_num = (CVI_HFSM_NUM_S *)t->StateNode[i];
    hfsm_num->CurState = state;
    if (parent != NULL) {
        for (j = 0; j < t->MaxStateCnt; j++) {
            CVI_HFSM_NUM_S *hfsm = (CVI_HFSM_NUM_S *)t->StateNode[j];
            if (hfsm->CurState == parent) {
                hfsm_num->parent = parent;
                break;
            }
        }
        if (j >= t->MaxStateCnt) {
            CVI_LOGE("Add parent state failed, This state not in hfsm!\n");
        }

    }

    g_SysStateCnt++;

    UNLOCK(t->mutex);

    return 0;
}

int32_t CVI_HFSM_SetInitialState(CVI_HFSM_HANDLE hfsm, uint32_t stateID)
{
    CVI_HFSM_S *t = (CVI_HFSM_S *)hfsm;
    if(t == NULL) {
        CVI_LOGE("This state is invalid!\n");
        return -1;
    }
    LOCK(t->mutex);

    uint32_t i = 0;
    for (i = 0; i < t->MaxStateCnt; i++) {
        CVI_HFSM_NUM_S *hfsm_num = (CVI_HFSM_NUM_S *)t->StateNode[i];
        CVI_STATE_S *state = hfsm_num->CurState;
        if (state->stateID == stateID) {
            g_iSysCurState = i;
            state->open();
            break;
        }
    }
    if (i >= t->MaxStateCnt) {
        CVI_LOGE("Not invalid state %d\n", stateID);
        UNLOCK(t->mutex);
        return -1;
    }

    UNLOCK(t->mutex);
    return 0;
}

int32_t CVI_HFSM_GetCurrentState(CVI_HFSM_HANDLE hfsm, CVI_STATE_S *state)
{
    (void)state;
    CVI_HFSM_S *t = (CVI_HFSM_S *)hfsm;
    if (NULL == t) {
        CVI_LOGE("CVI_HFSM_GetCurrentState failed!\n");
        return -1;
    }
    LOCK(t->mutex);
    // TODO
    // CVI_HFSM_NUM_S *hfsm_num = (CVI_HFSM_NUM_S *)t->StateNode[g_iSysCurState];
    // memcpy(state, hfsm_num->CurState, sizeof(CVI_STATE_S));

    UNLOCK(t->mutex);

    return 0;
}

int32_t CVI_HFSM_Start(CVI_HFSM_HANDLE hfsm)
{
    CVI_HFSM_S *t = (CVI_HFSM_S *)hfsm;
    if (NULL == t) {
        CVI_LOGE("CVI_HFSM_Start failed!\n");
        return -1;
    }
    LOCK(t->mutex);

    t->stop = false;

    UNLOCK(t->mutex);

    return 0;
}

int32_t CVI_HFSM_Stop(CVI_HFSM_HANDLE hfsm)
{
    CVI_HFSM_S *t = (CVI_HFSM_S *)hfsm;
    if (NULL == t) {
        CVI_LOGE("CVI_HFSM_Stop failed!\n");
        return -1;
    }
    LOCK(t->mutex);

    t->stop = true;

    UNLOCK(t->mutex);
    return 0;
}

int32_t CVI_HFSM_SendAsyncMessage(CVI_HFSM_HANDLE hfsm, CVI_MESSAGE_S *msg)
{
    int32_t s32Ret = -1;
    CVI_HFSM_S *t = (CVI_HFSM_S *)hfsm;
    if (NULL == t) {
        CVI_LOGE("CVI_HFSM_SendAsyncMessage failed!\n");
        return -1;
    }
    LOCK(t->mutex);

    s32Ret = HFSM_Queue_Push(t->queueHdl, msg);
    if (s32Ret != 0) {
        CVI_LOGE("Queue_Push failed!\n");
        UNLOCK(t->mutex);
        return -1;
    }

    UNLOCK(t->mutex);
    return 0;
}