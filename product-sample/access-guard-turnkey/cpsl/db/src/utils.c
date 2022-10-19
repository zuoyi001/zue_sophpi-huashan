#include <sqlite3.h>
#include <syslog.h>
#include <stdlib.h>
#include <sys/time.h>
#include "private_utils.h"

int get_count(sqlite3 *db, const char *cmd)
{
    int total = 0;
    int err = SQLITE_ERROR;
    sqlite3_stmt *stmt = NULL;
    if (SQLITE_OK != (err = sqlite3_prepare_v2(db, cmd, -1, &stmt, NULL))) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        goto END;
    }

    while (SQLITE_ROW == (err = sqlite3_step(stmt))) {
        total = sqlite3_column_int(stmt, 0);
    }

END:
    if (stmt) {
        sqlite3_finalize(stmt);
    }

    return total;
}

unsigned long long get_ms_timestamp()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
}
