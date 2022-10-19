#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "cvi_osal.h"

int cvi_osal_mutex_create(
    cvi_osal_mutex_attr_t         *attr,
    cvi_osal_mutex_handle_t       *mutex)
{
    cvi_osal_mutex_handle_t hdl;
    hdl = calloc(sizeof(cvi_osal_mutex_t), 1);
    if (hdl == NULL) {
        return CVI_OSAL_ERR_NOMEM;
    }
    if (attr) {
        memcpy(&hdl->attr, attr, sizeof(cvi_osal_mutex_attr_t));
    }
    sem_init((sem_t *)&hdl->mutex, 0, 1);
    *mutex = hdl;

    return CVI_OSAL_SUCCESS;
}


int cvi_osal_mutex_destroy(
    cvi_osal_mutex_handle_t        mutex)
{
    sem_destroy((sem_t *)&mutex->mutex);
    free(mutex);

    return CVI_OSAL_SUCCESS;
}

int cvi_osal_mutex_lock(
    cvi_osal_mutex_handle_t        mutex,
    int64_t                        timeout_us)
{
    struct timespec timeo;

    switch (timeout_us) {
    case CVI_OSAL_NO_WAIT:
        if (sem_trywait((sem_t *)&mutex->mutex)) {
            return CVI_OSAL_ERR_TIMEOUT;
        }
        break;
    case CVI_OSAL_WAIT_FOREVER:
        if (sem_wait((sem_t *)&mutex->mutex)) {
            return CVI_OSAL_ERR_FAILURE;
        }
        break;
    default:
        clock_gettime(CLOCK_REALTIME, &timeo);
        timeo.tv_sec += timeout_us / 1000000;
        timeo.tv_nsec += (timeout_us % 1000000) * 1000;
        if (timeo.tv_nsec > 1000000000) {
            timeo.tv_sec++;
            timeo.tv_nsec -= 1000000000;
        }
        if (sem_timedwait((sem_t *)&mutex->mutex, &timeo)) {
            return CVI_OSAL_ERR_TIMEOUT;
        }
        break;
    }

    return CVI_OSAL_SUCCESS;
}

int cvi_osal_mutex_unlock(
    cvi_osal_mutex_handle_t        mutex)
{
    if (sem_post((sem_t *)&mutex->mutex))
        return CVI_OSAL_ERR_FAILURE;

    return CVI_OSAL_SUCCESS;
}
