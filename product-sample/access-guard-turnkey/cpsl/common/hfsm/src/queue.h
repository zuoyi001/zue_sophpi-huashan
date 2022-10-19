#ifndef __QUEUE_H__
#define __QUEUE_H__
#include <stdint.h>
#include <unistd.h>

#define LOCK(mutex)                  \
    do {                                      \
        (void)pthread_mutex_lock(&mutex); \
    } while (0)
#define UNLOCK(mutex)                  \
    do {                                        \
        (void)pthread_mutex_unlock(&mutex); \
    } while (0)
#define DESTROY(mutex)                  \
    do {                                         \
        (void)pthread_mutex_destroy(&mutex); \
    } while (0)

#define SAFE_FREE(p) \
    do {                        \
        if (NULL != (p)) {      \
            free(p);            \
            (p) = NULL;         \
        }                       \
    } while (0)
#define COND_WAIT(cond, mutex)                   \
    do {                                            \
        (void)pthread_cond_wait(&cond, &mutex); \
    } while (0)
#define COND_SIGNAL(cond)                  \
    do {                                      \
        (void)pthread_cond_signal(&cond); \
    } while (0)
#define COND_DESTROY(cond)                  \
    do {                                       \
        (void)pthread_cond_destroy(&cond); \
    } while (0)

typedef void * QueueHandle;

QueueHandle HFSM_Queue_Create(uint32_t nodeSize, uint32_t maxLen);

void HFSM_Queue_Destroy(QueueHandle queueHdl);

void HFSM_Queue_Clear(QueueHandle queueHdl);

int HFSM_Queue_GetLen(QueueHandle queueHdl);

int HFSM_Queue_Push(QueueHandle queueHdl, const void *node);

int HFSM_Queue_Pop(QueueHandle queueHdl, void *node);

#endif  // __QUEUE_H__