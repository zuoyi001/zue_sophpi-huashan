#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "queue.h"
#include "cvi_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct tagQUEUE_S {
    pthread_mutex_t mutex;
    int frontIdx;
    int rearIdx;
    int curLen;
    int maxLen;
    int nodeSize;
    void **node;
} QUEUE_S;

/*create queue and malloc memory*/
QueueHandle HFSM_Queue_Create(uint32_t nodeSize, uint32_t maxLen)
{
    QUEUE_S *queue = malloc(sizeof(QUEUE_S));

    if (!queue) {
        CVI_LOGE("malloc queue failed! \n");
        return NULL;
    }

    int i = 0;
    queue->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    queue->frontIdx = 0;
    queue->rearIdx = 0;
    queue->curLen = 0;
    queue->maxLen = maxLen;
    queue->nodeSize = nodeSize;
    queue->node = (void **)malloc(sizeof(void *) * maxLen);

    for (i = 0; i < queue->maxLen; i++) {
        queue->node[i] = (void *)malloc(nodeSize);
    }

    return (QueueHandle)queue;
}

void HFSM_Queue_Destroy(QueueHandle queueHdl)
{
    if (queueHdl == 0) {
        CVI_LOGE("queueHdl is NULL!\n");
        return;
    }

    QUEUE_S *queue = (QUEUE_S *)queueHdl;
    LOCK(queue->mutex);
    int i = 0;

    for (i = 0; i < queue->maxLen; i++) {
        SAFE_FREE(queue->node[i]);
    }

    SAFE_FREE(queue->node);
    UNLOCK(queue->mutex);
    DESTROY(queue->mutex);
    SAFE_FREE(queue);
}

void HFSM_Queue_Clear(QueueHandle queueHdl)
{
    if (queueHdl == 0) {
        CVI_LOGE("queueHdl is NULL!\n");
        return;
    }

    QUEUE_S *queue = (QUEUE_S *)queueHdl;
    LOCK(queue->mutex);
    queue->curLen = 0;
    queue->frontIdx = 0;
    queue->rearIdx = 0;
    UNLOCK(queue->mutex);
}

int HFSM_Queue_GetLen(QueueHandle queueHdl)
{
    if (queueHdl == 0) {
        CVI_LOGE("queueHdl is NULL!\n");
        return -1;
    }

    QUEUE_S *queue = (QUEUE_S *)queueHdl;
    return (int)queue->curLen;
}

int HFSM_Queue_Push(QueueHandle queueHdl, const void *node)
{
    if (queueHdl == 0) {
        CVI_LOGE("queueHdl is NULL!\n");
        return -1;
    }

    QUEUE_S *queue = (QUEUE_S *)queueHdl;
    LOCK(queue->mutex);

    if (queue->curLen >= queue->maxLen) {
        UNLOCK(queue->mutex);
        CVI_LOGE("queue is full!\n");
        return -2;
    }

    if (node) {
        memcpy(queue->node[queue->rearIdx], node, queue->nodeSize);
    }

    queue->curLen++;
    queue->rearIdx = (queue->rearIdx + 1) % queue->maxLen;
    UNLOCK(queue->mutex);
    return 0;
}

int HFSM_Queue_Pop(QueueHandle queueHdl, void *node)
{
    if (queueHdl == 0) {
        CVI_LOGE("queueHdl is NULL!\n");
        return -1;
    }

    QUEUE_S *queue = (QUEUE_S *)queueHdl;
    LOCK(queue->mutex);

    if (queue->curLen == 0) {
        UNLOCK(queue->mutex);
        CVI_LOGE("queue is empity!\n");
        return -1;
    }

    if (node) {
        memcpy(node, queue->node[queue->frontIdx], queue->nodeSize);
    }

    queue->curLen--;
    queue->frontIdx = (queue->frontIdx + 1) % queue->maxLen;
    UNLOCK(queue->mutex);
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
