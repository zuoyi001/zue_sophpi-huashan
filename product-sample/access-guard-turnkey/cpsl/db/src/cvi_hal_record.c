#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sqlite3.h>
#include <syslog.h>
#include <sys/stat.h>
#include <time.h>
//#include <record.h>
#include "private_utils.h"
#include "db_schema.h"
#include "cvi_hal_record.h"


int cvi_hal_record_open(cvi_record_ctx_t *ctx, const char *path)
{
    int ret = -1;
    sqlite3 *db = NULL;
    char *errmsg = NULL;
    int err = SQLITE_ERROR;

    if (ctx == NULL || path == NULL) {
        goto END;
    }

    if (SQLITE_OK != (err = sqlite3_open(path, &db))) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        goto END;
    }

    if (SQLITE_OK != sqlite3_exec(db, RECORD_SCHEMA, NULL, NULL, &errmsg)) {
        syslog(LOG_ERR, "%s", errmsg);
        goto END;
    }

    snprintf(ctx->path, sizeof(ctx->path), "%s", path);
    ctx->db = (void *)db;

    ret = 0;
END:
    if (errmsg) {
        sqlite3_free(errmsg);
    }

    return ret;
}

static void notify_webserver()
{
    int fd = -1;
    int pid = -1;
    char buf[32] = {0};
    struct stat st = {0};

    if (0 != stat(WEBSERVER_PID_FILE, &st)) {
        goto END;
    }

    if (-1 == (fd = open(WEBSERVER_PID_FILE, O_RDONLY))) {
        goto END;
    }

    if (0 == read(fd, buf, sizeof(buf) - 1)) {
        goto END;
    }

    if (0 == (pid = atoi(buf))) {
        goto END;
    }

    kill(pid, SIGUSR1);
END:
    if (fd != -1) {
        close(fd);
    }
}

int cvi_hal_record_add(const cvi_record_ctx_t *ctx, const cvi_record_t *record)
{
    int ret = -1;
    int err = SQLITE_ERROR;
    char snapshot_name[128] = {0};
    sqlite3_stmt *stmt = NULL;

    if (ctx == NULL || (sqlite3 *)ctx->db == NULL || record == NULL) {
        goto END;
    }

    err = sqlite3_prepare_v2((sqlite3 *)ctx->db,
        "INSERT INTO "RECORD_TABLE"(remote_id, person_type, name, serial, identifier, ic_card, verification_type, temperature, timestamp, status, reserve, image_path) VALUES (?,?,?,?,?,?,?,?,?,?,?,?);", -1, &stmt, NULL);
    if (SQLITE_OK != err) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        goto END;
    }

    sqlite3_bind_int(stmt, 1, record->remote_id);
    sqlite3_bind_text(stmt, 2, record->type, strlen(record->type), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, record->name, strlen(record->name), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, record->serial, strlen(record->serial), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, record->identifier, strlen(record->identifier), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, record->ic_card, strlen(record->ic_card), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, record->verification_type);
    sqlite3_bind_double(stmt, 8, record->temperature);
    sqlite3_bind_int64(stmt, 9, get_ms_timestamp());
	sqlite3_bind_int(stmt, 10, record->status);
	sqlite3_bind_int(stmt, 11, record->reserve);
    sqlite3_bind_text(stmt, 12, record->image_path, strlen(record->image_path), SQLITE_STATIC);

    if (SQLITE_DONE != (err = sqlite3_step(stmt))) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        goto END;
    }

    //notify_webserver();

    ret = 0;

END:

    if (stmt) {
        sqlite3_finalize(stmt);
    }

    return ret;
}

static char *get_query_cmd(int offset, int limit, const cvi_record_filter_t *filter, int is_count)
{
    int count = 0;
    char *cmd = NULL;
    char fmt[1024] = {0};
    const char *count_fmt = "SELECT count(*) FROM "RECORD_TABLE;
    const char *select_fmt = "SELECT * FROM "RECORD_TABLE;

    if (filter == NULL) {
        return is_count?
            sqlite3_mprintf("%s;",count_fmt):
            sqlite3_mprintf("%s ORDER BY timestamp DESC limit %d offset %d;", select_fmt, limit, offset);
    }

    snprintf(fmt, sizeof(fmt), "%s", is_count? count_fmt: select_fmt);

    if (filter->verification_type != 0) {
        cmd = sqlite3_mprintf("%s %s verification_type=%d",
            fmt,
            count++ == 0? "WHERE": "AND",
            filter->verification_type);
        snprintf(fmt, sizeof(fmt), "%s", cmd);
        sqlite3_free(cmd);
    }

    if (filter->floor_temperature != 0) {
        cmd = sqlite3_mprintf("%s %s %.1f <= temperature",
            fmt,
            count++ == 0? "WHERE": "AND",
            filter->floor_temperature);
        snprintf(fmt, sizeof(fmt), "%s", cmd);
        sqlite3_free(cmd);
    }

    if (filter->ceiling_temperature != 0) {
        cmd = sqlite3_mprintf("%s %s temperature <= %.1f",
            fmt,
            count++ == 0? "WHERE": "AND",
            filter->ceiling_temperature);
        snprintf(fmt, sizeof(fmt), "%s", cmd);
        sqlite3_free(cmd);
    }

    if (filter->start_time != 0) {
        cmd = sqlite3_mprintf("%s %s %lu <= timestamp",
            fmt,
            count++ == 0? "WHERE": "AND",
            filter->start_time);
        snprintf(fmt, sizeof(fmt), "%s", cmd);
        sqlite3_free(cmd);
    }

    if (filter->end_time != 0) {
        cmd = sqlite3_mprintf("%s %s timestamp <= %lu",
            fmt,
            count++ == 0? "WHERE": "AND",
            filter->end_time);
        snprintf(fmt, sizeof(fmt), "%s", cmd);
        sqlite3_free(cmd);
    }

    if (strlen(filter->name) != 0) {
        cmd = sqlite3_mprintf("%s %s name='%s'",
            fmt,
            count++ == 0? "WHERE": "AND",
            filter->name);
        snprintf(fmt, sizeof(fmt), "%s", cmd);
        sqlite3_free(cmd);
    }

    if (strlen(filter->serial) != 0) {
        cmd = sqlite3_mprintf("%s %s serial='%s'",
            fmt,
            count++ == 0? "WHERE": "AND",
            filter->serial);
        snprintf(fmt, sizeof(fmt), "%s", cmd);
        sqlite3_free(cmd);
    }

    if (strlen(filter->ic_card) != 0) {
        cmd = sqlite3_mprintf("%s %s ic_card='%s'",
            fmt,
            count++ == 0? "WHERE": "AND",
            filter->ic_card);
        snprintf(fmt, sizeof(fmt), "%s", cmd);
        sqlite3_free(cmd);
    }

    cmd = is_count?
        sqlite3_mprintf("%s;", fmt):
        sqlite3_mprintf("%s ORDER BY timestamp DESC limit %d offset %d;", fmt, limit, offset);

    return cmd;
}

int cvi_hal_record_total_count(const cvi_record_ctx_t *ctx, const cvi_record_filter_t *filter)
{
    int total = -1;
    char *cmd = NULL;

    if (ctx == NULL || (sqlite3 *)ctx->db == NULL) {
        goto END;
    }

    cmd = get_query_cmd(0, -1, filter, 1);
    total = get_count((sqlite3 *)ctx->db, cmd);

END:
    if (cmd) {
        sqlite3_free(cmd);
    }

    return total;
}

int cvi_hal_record_get(const cvi_record_ctx_t *ctx, int offset, int limit, cvi_record_filter_t *filter, cvi_record_t **records)
{
    int i = 0;
    int total = 0;
    int count = -1;
    char *cmd = NULL;
    char *count_cmd = NULL;
    int err = SQLITE_ERROR;
    sqlite3_stmt *stmt = NULL;

    if (ctx == NULL || (sqlite3 *)ctx->db == NULL || records == NULL) {
        goto END;
    }

    cmd = get_query_cmd(offset, limit, filter, 0);
    count_cmd = get_query_cmd(offset, limit, filter, 1);
    total = get_count((sqlite3 *)ctx->db, count_cmd);

    if (total == 0 || offset >= total) {
        count = 0;
        goto END;
    }

    if (SQLITE_OK != (err = sqlite3_prepare_v2((sqlite3 *)ctx->db, cmd, -1, &stmt, NULL))) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        goto END;
    }

    count = total > offset + limit? limit: (total) - offset;

    *records = malloc(sizeof(cvi_record_t) * count);
    if (*records == NULL) {
        syslog(LOG_ERR, "Out of memory");
        goto END;
    }
    memset(*records, 0, sizeof(cvi_record_t) * count);

    while (SQLITE_ROW == (err = sqlite3_step(stmt))) {
        (*records)[i].id = sqlite3_column_int(stmt, 0);
        (*records)[i].remote_id = sqlite3_column_int(stmt, 1);

        snprintf((*records)[i].name, sizeof((*records)[i].name), "%s",
            sqlite3_column_text(stmt, 3));

        snprintf((*records)[i].serial, sizeof((*records)[i].serial), "%s",
            sqlite3_column_text(stmt, 4));

        snprintf((*records)[i].identifier, sizeof((*records)[i].identifier), "%s",
            sqlite3_column_text(stmt, 5));

        snprintf((*records)[i].ic_card, sizeof((*records)[i].ic_card), "%s",
            sqlite3_column_text(stmt, 6));

        (*records)[i].verification_type = sqlite3_column_int(stmt, 7);
        (*records)[i].temperature = sqlite3_column_double(stmt, 8);
        (*records)[i].timestamp = sqlite3_column_int64(stmt, 9);
		(*records)[i].status = sqlite3_column_int(stmt, 10);
		(*records)[i].reserve = sqlite3_column_int(stmt, 11);

        snprintf((*records)[i].image_path, sizeof((*records)[i].image_path), "%s",
            sqlite3_column_text(stmt, 12));

        i++;
    }

END:
    if (cmd) {
        sqlite3_free(cmd);
    }

     if (count_cmd) {
        sqlite3_free(count_cmd);
    }

    if (stmt) {
        sqlite3_finalize(stmt);
    }

    return count;
}

int cvi_hal_record_delete(const cvi_record_ctx_t *ctx, int record_id)
{
    int ret = -1;
    char *cmd = NULL;
    char *errmsg = NULL;

    if (ctx == NULL || (sqlite3 *)ctx->db == NULL) {
        goto END;
    }

    cmd = sqlite3_mprintf("DELETE FROM %s WHERE id=%d;", RECORD_TABLE, record_id);
    if (SQLITE_OK != sqlite3_exec((sqlite3 *)ctx->db, cmd, NULL, NULL, &errmsg)) {
        syslog(LOG_ERR, "%s", errmsg);
        goto END;
    }

    ret = 0;
END:
    if (cmd) {
        sqlite3_free(cmd);
    }

    if (errmsg) {
        sqlite3_free(errmsg);
    }

    return ret;
}

int cvi_hal_record_delete_by_date(const cvi_record_ctx_t *ctx, unsigned long start_time, unsigned long end_time)
{
    int ret = -1;
    char *cmd = NULL;
    char *errmsg = NULL;

    if (ctx == NULL || (sqlite3 *)ctx->db == NULL) {
        goto END;
    }

    if (start_time != 0 && end_time != 0) {
        cmd = sqlite3_mprintf("DELETE FROM %s WHERE %lu <= timestamp AND timestamp <= %lu;", RECORD_TABLE, start_time, end_time);
    } else if (start_time != 0) {
        cmd = sqlite3_mprintf("DELETE FROM %s WHERE %lu <= timestamp;", RECORD_TABLE, start_time);
    } else {
        cmd = sqlite3_mprintf("DELETE FROM %s WHERE timestamp <= %lu;", RECORD_TABLE, end_time);
    }
    if (SQLITE_OK != sqlite3_exec((sqlite3 *)ctx->db, cmd, NULL, NULL, &errmsg)) {
        syslog(LOG_ERR, "%s", errmsg);
        goto END;
    }

    ret = 0;
END:
    if (cmd) {
        sqlite3_free(cmd);
    }

    if (errmsg) {
        sqlite3_free(errmsg);
    }

    return ret;
}

int cvi_hal_record_delete_all(const cvi_record_ctx_t *ctx)
{
    int ret = -1;
    char *errmsg = NULL;
    const char *cmd = "DELETE FROM "RECORD_TABLE";VACCUM;";

    if (SQLITE_OK != sqlite3_exec((sqlite3 *)ctx->db, cmd, NULL, NULL, &errmsg)) {
        syslog(LOG_ERR, "%s", errmsg);
        goto END;
    }

    ret = 0;

END:

    if (errmsg) {
        sqlite3_free(errmsg);
    }

    return ret;
}

int cvi_hal_record_close(cvi_record_ctx_t *ctx)
{
    if (ctx== NULL) {
        return -1;
    }

    memset(ctx->path, 0, sizeof(ctx->path));

    if ((sqlite3 *)ctx->db == NULL) {
        return 0;
    }

    if (SQLITE_OK != sqlite3_close((sqlite3 *)ctx->db)) {
        return -1;
    }

    ctx->db = NULL;

    return 0;
}

int cvi_hal_get_record_list(cvi_record_t **record_list, int offset, int limit )
{
    cvi_record_ctx_t ctx = {0};
	int count = 0;
    if (0 > cvi_hal_record_open(&ctx, RECORD_PATH)) {
        printf("repo open fail\n");
        return -1;
    }

    count = cvi_hal_record_get(&ctx, offset, limit, NULL, record_list);

    cvi_hal_record_close(&ctx);

    //unlink(RECORD_PATH);
    return count;
}

