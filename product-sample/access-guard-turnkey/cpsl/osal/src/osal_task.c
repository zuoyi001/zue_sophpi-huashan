#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

#include "cvi_osal.h"

static void *cvi_task_entry(void *arg)
{
    cvi_osal_task_handle_t task = (cvi_osal_task_handle_t)arg;
    struct sched_param sched_param;
    int policy;

    prctl(PR_SET_NAME, (unsigned long int)task->attr.name, 0, 0, 0);
    pthread_getschedparam(pthread_self(), &policy, &sched_param);
    printf("%s: PID:%d,TID:%ld, PTHREAD:0x%lx, POLICY=%d, PRIORITY=%d\n",
        task->attr.name, getpid(), syscall(SYS_gettid), pthread_self(),
        policy, sched_param.sched_priority);
    task->attr.entry(task->attr.param);

    if (task->attr.detached) {
        free(arg);
    }

    return NULL;
}

int cvi_osal_task_create(
    cvi_osal_task_attr_t          *attr,
    cvi_osal_task_handle_t        *task)
{
    cvi_osal_task_handle_t hdl;
    pthread_attr_t pthread_attr;
    struct sched_param sched_param;

    hdl = calloc(sizeof(cvi_osal_task_t), 1);
    if (hdl == NULL) {
        return CVI_OSAL_ERR_NOMEM;
    }
    memcpy(&hdl->attr, attr, sizeof(cvi_osal_task_attr_t));

    pthread_attr_init(&pthread_attr);
    if (attr->detached) {
        pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED);
    } else {
        pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE);
    }
    if (attr->priority != CVI_OSAL_PRI_NORMAL) {
        pthread_attr_getschedparam(&pthread_attr, &sched_param);
        sched_param.sched_priority = attr->priority;
        pthread_attr_setschedparam(&pthread_attr, &sched_param);
        pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
    } else {
        pthread_attr_setschedpolicy(&pthread_attr, SCHED_OTHER);
    }
    pthread_create((pthread_t *)&hdl->task, &pthread_attr, cvi_task_entry, hdl);
    pthread_attr_destroy(&pthread_attr);
    printf("PID:%d, TID:%ld, pPID: %ld\n",
        getpid(), syscall(SYS_gettid), pthread_self());
    *task = hdl;

    return CVI_OSAL_SUCCESS;
}

int cvi_osal_task_destroy(cvi_osal_task_handle_t* task)
{
    if (*task == NULL) {
        return CVI_OSAL_ERR_FAILURE;
    }

    free(*task);
    *task = NULL;

    return CVI_OSAL_SUCCESS;
}

int cvi_osal_task_join(
	cvi_osal_task_handle_t         task)
{
    int rc;
    rc = pthread_join((pthread_t)task->task, NULL);
    return rc < 0 ? CVI_OSAL_ERR_FAILURE : CVI_OSAL_SUCCESS;
}

void cvi_osal_task_sleep(int64_t time_us)
{
    usleep(time_us);
}

void cvi_osal_task_resched(void)
{
    sched_yield();
}
