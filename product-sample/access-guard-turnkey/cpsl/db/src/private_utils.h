#ifndef __PRIVATE_UTILS_H__
#define __PRIVATE_UTILS_H__

#include <sqlite3.h>
#include <time.h>

#ifdef  __cplusplus
extern  "C" {
#endif

int get_count(sqlite3 *cvi_hal_db, const char *cmd);
unsigned long long get_ms_timestamp();
#ifdef  __cplusplus
}
#endif

#endif
