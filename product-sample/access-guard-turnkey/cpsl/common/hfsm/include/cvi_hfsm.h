#ifndef CVI_HFSM_H
#define CVI_HFSM_H
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include "cvi_eventhub.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define CVI_STATE_NAME_LEN                64
#define CVI_STATE_MAX_AMOUNT              32
#define CVI_PROCESS_MSG_RESULTE_OK        (0)
#define CVI_PROCESS_MSG_UNHANDLER         (-1)
typedef void *           CVI_HFSM_HANDLE;
typedef CVI_EVENT_S      CVI_MESSAGE_S;

typedef struct CviStateInfo {
    uint32_t stateID;                         /* state id */
    char name[CVI_STATE_NAME_LEN];            /* state name */
    int32_t (*open)(void);             /* call when state open */
    int32_t (*close)(void);              /* call when state close */
    /* call when process a message */
    int32_t (*processMessage)(CVI_MESSAGE_S *msg, void* argv, uint32_t *stateID);
    void* argv;                              /* User private, used by processMessage */
}CVI_STATE_S;

/* hfsm event enum */
typedef enum CviHfsmEventE {
    CVI_HFSM_EVENT_HANDLE_MSG = 0,       /* handler message */
    CVI_HFSM_EVENT_UNHANDLE_MSG,         /* unhandler message */
    CVI_HFSM_EVENT_TRANSTION_ERROR,      /* transtion error */

    CVI_HFSM_EVENT_BUTT
} CVI_HFSM_EVENT_E;

/* HFSM event information */
typedef struct HiHfsmEventInfo {
    int32_t s32ErrorNo;
    CVI_HFSM_EVENT_E enEventCode;
    CVI_MESSAGE_S *pstunHandlerMsg;
} CVI_HFSM_EVENT_INFO_S;

typedef int32_t (*CVI_HFSM_EVENT_CALLBACK_FN)(CVI_HFSM_HANDLE hfsmHandle, const CVI_HFSM_EVENT_INFO_S *eventInfo);

typedef struct CviHfsmAttr {
    CVI_HFSM_EVENT_CALLBACK_FN fnHfsmEventCallback;
    uint32_t u32StateMaxAmount;
    uint32_t u32MessageQueueSize;
} CVI_HFSM_ATTR_S;

int32_t CVI_HFSM_Create(CVI_HFSM_ATTR_S *fsmAttr, CVI_HFSM_HANDLE *hfsm);
int32_t CVI_HFSM_Destroy(CVI_HFSM_HANDLE hfsm);
int32_t CVI_HFSM_AddState(CVI_HFSM_HANDLE hfsm, CVI_STATE_S *state, CVI_STATE_S *parent);
int32_t CVI_HFSM_SetInitialState(CVI_HFSM_HANDLE hfsm, uint32_t stateID);
int32_t CVI_HFSM_GetCurrentState(CVI_HFSM_HANDLE hfsm, CVI_STATE_S *state);
int32_t CVI_HFSM_Start(CVI_HFSM_HANDLE hfsm);
int32_t CVI_HFSM_Stop(CVI_HFSM_HANDLE hfsm);
int32_t CVI_HFSM_SendAsyncMessage(CVI_HFSM_HANDLE hfsm, CVI_MESSAGE_S *msg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif