#ifndef __CVI_OSAL_H__
#define __CVI_OSAL_H__

#include <stdint.h>
#include <stdbool.h>

#define CVI_OSAL_SUCCESS           ((int)(0))
#define CVI_OSAL_ERR_FAILURE       ((int)(-1001))
#define CVI_OSAL_ERR_NOMEM         ((int)(-1002))
#define CVI_OSAL_ERR_TIMEOUT       ((int)(-1003))

#define CVI_OSAL_NO_WAIT           ((int64_t)(0))
#define CVI_OSAL_WAIT_FOREVER      ((int64_t)(-1))

#define CVI_OSAL_PRI_NORMAL        ((int)0)
#define CVI_OSAL_PRI_RT_LOWEST     ((int)1)
#define CVI_OSAL_PRI_RT_LOW        ((int)9)
#define CVI_OSAL_PRI_RT_MID        ((int)49)
#define CVI_OSAL_PRI_RT_HIGH       ((int)89)
#define CVI_OSAL_PRI_RT_HIGHEST    ((int)99)

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * time
 */
int cvi_osal_get_boot_time(
    uint64_t                      *time_us);

/*
 * mutex
 */
typedef struct {
    const char                    *name;
} cvi_osal_mutex_attr_t;

typedef struct {
    cvi_osal_mutex_attr_t          attr;
    void                          *mutex;
} cvi_osal_mutex_t, *cvi_osal_mutex_handle_t;

int cvi_osal_mutex_create(
    cvi_osal_mutex_attr_t         *attr,
    cvi_osal_mutex_handle_t       *mutex);

int cvi_osal_mutex_destroy(
    cvi_osal_mutex_handle_t        mutex);

int cvi_osal_mutex_lock(
    cvi_osal_mutex_handle_t        mutex,
    int64_t                        timeout_us);

int cvi_osal_mutex_unlock(
    cvi_osal_mutex_handle_t        mutex);

/*
 * task
 */
typedef void (*cvi_osal_task_entry_t)(
    void                          *param);

typedef struct {
    const char                    *name;
    cvi_osal_task_entry_t          entry;
    void                          *param;
    int                            priority;
    bool                           detached;
} cvi_osal_task_attr_t;

typedef struct {
    cvi_osal_task_attr_t           attr;
    void                          *task;
} cvi_osal_task_t, *cvi_osal_task_handle_t;

int cvi_osal_task_create(
    cvi_osal_task_attr_t          *attr,
    cvi_osal_task_handle_t        *task);

int cvi_osal_task_destroy(cvi_osal_task_handle_t* task);

int cvi_osal_task_join(
	cvi_osal_task_handle_t         task);

void cvi_osal_task_sleep(int64_t time_us);
void cvi_osal_task_resched(void);

#ifdef __cplusplus
}
#endif

#endif
