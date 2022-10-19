#ifndef __CVI_LOG_H__
#define __CVI_LOG_H__

#include <stdlib.h>

#if defined(CVI_LOG_LEVEL)
	#define ZF_LOG_LEVEL CVI_LOG_LEVEL
#endif
#include "zf_log.h"

#define CVI_LOG_VERBOSE ZF_LOG_VERBOSE // 1
#define CVI_LOG_DEBUG   ZF_LOG_DEBUG   // 2
#define CVI_LOG_INFO    ZF_LOG_INFO    // 3
#define CVI_LOG_WARN    ZF_LOG_WARN    // 4
#define CVI_LOG_ERROR   ZF_LOG_ERROR   // 5
#define CVI_LOG_FATAL   ZF_LOG_FATAL   // 6
#define CVI_LOG_NONE    ZF_LOG_NONE    // 0xFF

/* Message logging macros:
 * - CVI_LOGV("format string", args, ...)
 * - CVI_LOGD("format string", args, ...)
 * - CVI_LOGI("format string", args, ...)
 * - CVI_LOGW("format string", args, ...)
 * - CVI_LOGE("format string", args, ...)
 * - CVI_LOGF("format string", args, ...)
 */
#define CVI_LOGV(...)   ZF_LOGV(__VA_ARGS__)
#define CVI_LOGD(...)   ZF_LOGD(__VA_ARGS__)
#define CVI_LOGI(...)   ZF_LOGI(__VA_ARGS__)
#define CVI_LOGW(...)   ZF_LOGW(__VA_ARGS__)
#define CVI_LOGE(...)   ZF_LOGE(__VA_ARGS__)
#define CVI_LOGF(...)   ZF_LOGF(__VA_ARGS__)

/*
 * Memory logging macros:
 * - CVI_LOGV_MEM(data_ptr, data_sz, "format string", args, ...)
 * - CVI_LOGD_MEM(data_ptr, data_sz, "format string", args, ...)
 * - CVI_LOGI_MEM(data_ptr, data_sz, "format string", args, ...)
 * - CVI_LOGW_MEM(data_ptr, data_sz, "format string", args, ...)
 * - CVI_LOGE_MEM(data_ptr, data_sz, "format string", args, ...)
 * - CVI_LOGF_MEM(data_ptr, data_sz, "format string", args, ...)
 */
#define CVI_LOGV_MEM(...)   ZF_LOGV_MEM(__VA_ARGS__)
#define CVI_LOGD_MEM(...)   ZF_LOGD_MEM(__VA_ARGS__)
#define CVI_LOGI_MEM(...)   ZF_LOGI_MEM(__VA_ARGS__)
#define CVI_LOGW_MEM(...)   ZF_LOGW_MEM(__VA_ARGS__)
#define CVI_LOGE_MEM(...)   ZF_LOGE_MEM(__VA_ARGS__)
#define CVI_LOGF_MEM(...)   ZF_LOGF_MEM(__VA_ARGS__)

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * when called, setup log according to env variables
 * otherwise, logging to stdout, and start with CVI_LOG_LEVEL by default
 *
 * Supported env variables are
 *   - CVI_LOG_SYSLOG : true|false
 *   - CVI_LOG_FILE   : filename
 *   - CVI_LOG_LEVEL  : F|E|W|I|D|V|N
 */
#define CVI_LOG_LEVEL_VAR_NAME   "CVI_LOG_LEVEL"
#define CVI_LOG_FILE_VAR_NAME    "CVI_LOG_FILE"
#define CVI_LOG_SYSLOG_VAR_NAME  "CVI_LOG_SYSLOG"
void CVI_LOG_INIT(void);

/*
 * to set the logging output level
 * by default, CVI_LOG_INFO is used
 */
void CVI_LOG_SET_LEVEL(const int lvl);

/*
 * (Optional) to set a TAG
 */
void CVI_LOG_SET_TAG(const char *const tag);

#ifdef __cplusplus
}
#endif

#ifndef CVI_LOG_ASSERT
#define CVI_LOG_ASSERT(x, ...)     \
    do {                           \
        if (!(x)) {                \
            CVI_LOGE(__VA_ARGS__); \
			abort();               \
        }                          \
    } while(0)
#endif

#ifndef UNUSED
# define UNUSED(x) (void)(x)
#endif

#endif
