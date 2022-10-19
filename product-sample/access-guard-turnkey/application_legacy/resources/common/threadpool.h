#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <pthread.h>
#include <stdbool.h>

#define LL_ADD(head, node) {\
    if(head != NULL) head->prev = node; \
    node->next = head; \
    node->prev = NULL; \
    head = node; \
}

#define LL_RM(head, node) {\
    if(node->next != NULL) node->next->prev = node->prev; \
    if(node->prev != NULL) node->prev->next = node->next; \
    if(node == head) head = node->next; \
    node->next = node->prev = NULL; \
}

typedef struct worker{
    pthread_t worker_thread;
    struct thread_pool *pool;
    bool isstop;
    struct worker *next;
    struct worker *prev;
}worker_t;

typedef struct job{
    void *(*job_function)(void *arg);
    void *data;
    struct job *next;
    struct job *prev;
}job_t;

typedef struct thread_pool{
    worker_t *workers;
    job_t *jobs;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
}thread_pool_t;

int thread_pool_add(thread_pool_t *pool, job_t* job);
int thread_pool_init(thread_pool_t *pool, int size);
int thread_pool_destroy(thread_pool_t *pool);

#endif