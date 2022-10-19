#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/prctl.h>

#include "threadpool.h"

static void *worker_thread_func(void *arg){
    char szThreadName[20]="worker_thread";
	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);
    worker_t *worker = (worker_t*)arg;
    thread_pool_t *pool = worker->pool;
    while(1){
        pthread_mutex_lock(&pool->mutex);
        while(pool->jobs == NULL){
            if(worker->isstop) break;//no need to sleep and wait signal
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }

        if(worker->isstop) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }

        job_t *job = pool->jobs;
        if(pool->jobs!=NULL) LL_RM(pool->jobs, job);
        pthread_mutex_unlock(&pool->mutex);
        if(job == NULL) continue;
        if(job->job_function){
            job->job_function(job);
        }
        else
            printf("error:null function ptr.\n");
        free(job);
    }

    free(worker);
    pthread_exit(NULL);
}

int thread_pool_init(thread_pool_t *pool, int size)
{
    pthread_cond_t default_cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t default_mutex = PTHREAD_MUTEX_INITIALIZER;

    if(size<1) size = 1;
    memset(pool, 0, sizeof(thread_pool_t));
    memcpy(&pool->cond, &default_cond, sizeof(default_cond));
    memcpy(&pool->mutex, &default_mutex, sizeof(default_mutex));
    int i;

    for(i=0; i<size; i++){
        worker_t *worker = (worker_t *)malloc(sizeof(worker_t));
        if(worker == NULL) {
            thread_pool_destroy(pool);
            return -1;
        }
        memset(worker, 0, sizeof(worker_t));
        worker->pool = pool;
        if(pthread_create(&worker->worker_thread, NULL, worker_thread_func, (void*)worker)){
            free(worker);
            thread_pool_destroy(pool);
            return -2;
        }
        LL_ADD(pool->workers, worker);
    }
    return 0;
}

int thread_pool_destroy(thread_pool_t *pool){
    if(!pool) return 0;
    worker_t *worker;
    job_t *job;

    pthread_mutex_lock(&pool->mutex);
    for(worker=pool->workers; worker != NULL; worker=worker->next){
        worker->isstop = true;
    }

    pthread_cond_broadcast(&pool->cond);
    pool->workers = NULL;
    job = pool->jobs;
    while(job){
        free(job);
        job = job->next;
    }
    pool->jobs = NULL;
    pthread_mutex_unlock(&pool->mutex);

    return 0;
}

int thread_pool_add(thread_pool_t *pool, job_t *job){
    if(job){
        pthread_mutex_lock(&pool->mutex);
        LL_ADD(pool->jobs, job);
        pthread_cond_signal(&pool->cond);
        pthread_mutex_unlock(&pool->mutex);
        return 0;
    }
    return -1;
}