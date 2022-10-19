#include <time.h>

#include "cvi_osal.h"

int cvi_osal_get_boot_time(
    uint64_t                      *time_us)
{
    struct timespec timeo;
    clock_gettime(CLOCK_MONOTONIC, &timeo);
    *time_us = (uint64_t)timeo.tv_sec * 1000000 + timeo.tv_nsec / 1000;
    return CVI_OSAL_SUCCESS;
}
