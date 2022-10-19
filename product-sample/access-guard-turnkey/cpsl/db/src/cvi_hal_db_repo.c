#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <syslog.h>
#include <sys/stat.h>
#include <time.h>
#include "cvi_hal_db_repo.h"
#include "private_utils.h"
#include "db_schema.h"

int cvi_hal_repo_open(cvi_hal_db_repo_t *repo, const char *path)
{
    int ret = -1;
    sqlite3 *cvi_hal_db = NULL;
    char *errmsg = NULL;
    int err = SQLITE_ERROR;

    if (repo == NULL || path == NULL) {
        goto END;
    }

    if (SQLITE_OK != (err = sqlite3_open(path, &cvi_hal_db))) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        goto END;
    }

    if (SQLITE_OK != sqlite3_exec(cvi_hal_db, ENABLE_FK REPO_SCHEMA FEATURE_SCHEMA, NULL, NULL, &errmsg)) {
        syslog(LOG_ERR, "%s", errmsg);
        goto END;
    }

    snprintf(repo->path, sizeof(repo->path), "%s", path);
    repo->cvi_hal_db = (void *)cvi_hal_db;

    ret = 0;
END:
    if (errmsg) {
        sqlite3_free(errmsg);
    }

    return ret;
}

int cvi_hal_repo_close(cvi_hal_db_repo_t *repo)
{
    if (repo == NULL) {
        return -1;
    }

    memset(repo->path, 0, sizeof(repo->path));

    if ((sqlite3 *)repo->cvi_hal_db == NULL) {
        return 0;
    }

    if (SQLITE_OK != sqlite3_close((sqlite3 *)repo->cvi_hal_db)) {
        return -1;
    }

    repo->cvi_hal_db = NULL;

    return 0;
}

int add_face(sqlite3 *cvi_hal_db, const char *name)
{
    char *cmd = NULL;
    char *errmsg = NULL;
    int face_id = 0;

    unsigned long long timestamp = get_ms_timestamp();
    cmd = sqlite3_mprintf("INSERT INTO "FACE_TABLE" (name, add_time, update_time) VALUES ('%s', %lu, %lu);", name, timestamp, timestamp);
    if (SQLITE_OK != sqlite3_exec(cvi_hal_db, cmd, NULL, NULL, &errmsg)) {
        syslog(LOG_ERR, "%s", errmsg);
        goto END;
    }

    face_id = sqlite3_last_insert_rowid(cvi_hal_db);

END:
    if (cmd) {
        sqlite3_free(cmd);
    }

    if (errmsg) {
        sqlite3_free(errmsg);
    }

    return face_id;
}

int add_feature(sqlite3 *cvi_hal_db, int face_id, const unsigned char *feature, size_t feature_size)
{
    int ret = -1;
    int err = SQLITE_ERROR;
    sqlite3_stmt *stmt = NULL;

    if (SQLITE_OK != (err = sqlite3_prepare_v2(cvi_hal_db, "INSERT INTO "FEATURE_TABLE" (face_id, feature) VALUES (?, ?);", -1, &stmt, NULL))) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        goto END;
    }

    sqlite3_bind_int(stmt, 1, face_id);
    sqlite3_bind_blob(stmt, 2, feature, feature_size, SQLITE_STATIC);

    if (SQLITE_DONE != sqlite3_step(stmt)) {
        syslog(LOG_ERR, "step fail");
        goto END;
    }

    ret = 0;

END:
    if (stmt != NULL) {
        sqlite3_finalize(stmt);
    }

    return ret;
}

int cvi_hal_repo_add_face(const cvi_hal_db_repo_t *repo, const char *name, unsigned char *feature, size_t feature_size)
{
    int err = -1;
    char *errmsg = NULL;
    int face_id = 0;

    if (repo == NULL || (sqlite3 *)repo->cvi_hal_db == NULL || name == NULL || feature == NULL) {
        return -1;
    }

    if (SQLITE_OK != sqlite3_exec((sqlite3 *)repo->cvi_hal_db, "BEGIN TRANSACTION;" , NULL, NULL, &errmsg)) {
        goto END;
    }

    if (0 == (face_id = add_face((sqlite3 *)repo->cvi_hal_db, name))) {
        syslog(LOG_ERR, "add face fail");
        goto END;
    }

    if (0 > add_feature((sqlite3 *)repo->cvi_hal_db, face_id, feature, feature_size)) {
        syslog(LOG_ERR, "add face fail");
        goto END;
    }

    if (SQLITE_OK != sqlite3_exec((sqlite3 *)repo->cvi_hal_db, "COMMIT;" , NULL, NULL, &errmsg)) {
        sqlite3_exec((sqlite3 *)repo->cvi_hal_db, "ROLLBACK;" , NULL, NULL, &errmsg);
        goto END;
    }

    err = 0;

END:

    if (errmsg) {
        sqlite3_free(errmsg);
    }

    return err == 0? face_id: -1;
}

int update_face(sqlite3 *cvi_hal_db, int face_id, const char *name)
{
    int ret = -1;
    char *errmsg = NULL;
    char *cmd = NULL;

    unsigned long long timestamp = get_ms_timestamp();
    cmd = sqlite3_mprintf("UPDATE "FACE_TABLE" SET name='%s', update_time=%lu WHERE id=%d;", name, timestamp, face_id);
    if (SQLITE_OK != sqlite3_exec(cvi_hal_db, cmd, NULL, NULL, &errmsg)) {
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

static int get_feature_count(sqlite3 *cvi_hal_db, int face_id)
{
    int count = -1;
    int err = SQLITE_ERROR;
    sqlite3_stmt *stmt = NULL;

    if (SQLITE_OK != (err = sqlite3_prepare_v2(cvi_hal_db, "SELECT count(*) FROM "FEATURE_TABLE" WHERE face_id=?;", -1, &stmt, NULL))) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, face_id);

    if (SQLITE_ROW == sqlite3_step(stmt)) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return count;
}

static int update_feature(sqlite3 *cvi_hal_db, int face_id, const unsigned char *feature, size_t feature_size)
{
    int err = SQLITE_ERROR;
    sqlite3_stmt *stmt = NULL;

    if (PER_USER_FEATURE_LIMIT > get_feature_count(cvi_hal_db, face_id)) {
        return add_feature(cvi_hal_db, face_id, feature, feature_size);
    }

    err = sqlite3_prepare_v2(cvi_hal_db, "UPDATE "FEATURE_TABLE" SET feature=? WHERE face_id=? ORDER BY update_time ASC limit 1;", -1, &stmt, NULL);
     if (SQLITE_OK != err) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        return -1;
    }

    sqlite3_bind_blob(stmt, 1, feature, feature_size, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, face_id);

    if (SQLITE_DONE !=  (err = sqlite3_step(stmt))) {
        syslog(LOG_ERR, "step fail");
    }

    sqlite3_step(stmt);

    return SQLITE_DONE == err? 0: -1;
}

int cvi_hal_repo_update_face(const cvi_hal_db_repo_t *repo, int face_id, const char *name, const unsigned char *feature, size_t feature_size)
{
    int ret = -1;
    int err = SQLITE_ERROR;
    char *errmsg = NULL;

    if (repo == NULL || (sqlite3 *)repo->cvi_hal_db == NULL) {
        goto END;
    }

    if (SQLITE_OK != sqlite3_exec((sqlite3 *)repo->cvi_hal_db, "BEGIN TRANSACTION;" , NULL, NULL, &errmsg)) {
        goto END;
    }

    if (name != NULL) {
        update_face((sqlite3 *)repo->cvi_hal_db, face_id, name);
    }

    if (feature != NULL) {
        update_feature((sqlite3 *)repo->cvi_hal_db, face_id, feature, feature_size);
    }

    if (SQLITE_OK != sqlite3_exec((sqlite3 *)repo->cvi_hal_db, "COMMIT;" , NULL, NULL, &errmsg)) {
        goto END;
    }

    ret = 0;
END:
    if (errmsg) {
        sqlite3_free(errmsg);
    }

    return ret;
}

int cvi_hal_repo_delete_face(const cvi_hal_db_repo_t *repo, int face_id)
{
    int ret = -1;
    char *errmsg = NULL;
    char *cmd = NULL;

    if (repo == NULL || (sqlite3 *)repo->cvi_hal_db == NULL) {
        goto END;
    }

    cmd = sqlite3_mprintf("DELETE FROM %s WHERE id=%d;DELETE FROM %s WHERE face_id=%d;",
        FACE_TABLE, face_id,
        FEATURE_TABLE, face_id);

    if (SQLITE_OK != sqlite3_exec((sqlite3 *)repo->cvi_hal_db, cmd, NULL, NULL, &errmsg)) {
        syslog(LOG_ERR, "%s", errmsg);
        goto END;
    }

    ret = 0;

END:
    if (cmd != NULL) {
        sqlite3_free(cmd);
    }

    if (errmsg != NULL) {
        sqlite3_free(errmsg);
    }

    return ret;
}

int cvi_hal_repo_find_face(const cvi_hal_db_repo_t *repo, int face_id, char *name, size_t name_size, unsigned char *feature, size_t feature_size)
{
    int ret = -1;
    int err = SQLITE_ERROR;
    char *cmd = NULL;
    const char *_name = NULL;
    const unsigned char *_feature = NULL;
    sqlite3_stmt *stmt = NULL;

    if (repo == NULL || (sqlite3 *)repo->cvi_hal_db == NULL) {
        goto END;
    }

    // select the last
    cmd = sqlite3_mprintf("SELECT %s.name, %s.feature FROM %s INNER JOIN %s ON %s.id=%s.face_id WHERE %s.id=%d ORDER BY %s.update_time DESC limit 1;",
            FACE_TABLE, FEATURE_TABLE,
            FACE_TABLE, FEATURE_TABLE,
            FACE_TABLE, FEATURE_TABLE,
            FACE_TABLE, face_id,
            FEATURE_TABLE);

    if (SQLITE_OK != (err = sqlite3_prepare_v2((sqlite3 *)repo->cvi_hal_db, cmd, -1, &stmt, NULL))) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        goto END;
    }

    // TODO check behavior
    if (SQLITE_ROW != (err = sqlite3_step(stmt))) {
        ret = err == SQLITE_DONE? 0: -1;
        goto END;
    }

    _name = sqlite3_column_text(stmt, 0);
    _feature = sqlite3_column_blob(stmt, 1);

    if (_name != NULL) {
        snprintf(name, name_size, "%s", _name);
    }

    if (_feature != NULL) {
        memcpy(feature, _feature, feature_size);
    }

    ret = 0;
END:
    if (stmt != NULL) {
        sqlite3_finalize(stmt);
    }

    if (cmd != NULL) {
        sqlite3_free(cmd);
    }

    return ret;
}

int cvi_hal_repo_get_features(const cvi_hal_db_repo_t *repo, int offset, int limit, unsigned char **features)
{
    int shift = 0;
    int ret = -1;
    int total = 0;

    int count = -1;
    int err = SQLITE_ERROR;
    char *query_cmd = NULL;
    sqlite3_stmt *stmt = NULL;

    if (repo == NULL || (sqlite3 *)repo->cvi_hal_db == NULL) {
        goto END;
    }

	query_cmd = sqlite3_mprintf("SELECT length(feature) as size, feature from "FEATURE_TABLE" limit %d offset %d;", limit, offset);
    if (0 >= (total = get_count((sqlite3 *)repo->cvi_hal_db, "SELECT count(*) FROM "FEATURE_TABLE";")) ||
        offset >= total) {
        ret = 0;
        goto END;
    }

    if (limit > 0) {
        count = total > offset + limit? limit: total - offset;
    } else {
        count = total - offset;
    }

    if (NULL == (*features = (unsigned char *)malloc(sizeof(unsigned char) * 512 * count))) {
        syslog(LOG_ERR, "Out of memory");
        goto END;
    }
    memset(*features, 0, sizeof(unsigned char) * 512 * count);

    if (SQLITE_OK != (err = sqlite3_prepare_v2((sqlite3 *)repo->cvi_hal_db, query_cmd, -1, &stmt, NULL))) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        goto END;
    }

    while (SQLITE_ROW == (err = sqlite3_step(stmt))) {
        int size = sqlite3_column_int(stmt, 0);
        const unsigned char *feature = sqlite3_column_blob(stmt, 1);
        if (feature == NULL) {
            syslog(LOG_ERR, "null feature");
            continue;
        }
        memcpy(*features + shift, feature, size);
        shift += size;
    }

    ret = count;
END:
    if (stmt) {
        sqlite3_finalize(stmt);
    }

    if (query_cmd) {
        sqlite3_free(query_cmd);
    }

    return ret;
}

int cvi_hal_repo_get_face_by_offset(const cvi_hal_db_repo_t *repo, int feature_offset, int *face_id, char *name, size_t name_size)
{
    int ret = -1;
    int err = SQLITE_ERROR;
    char cmd[128] = {0};
    sqlite3_stmt *stmt = NULL;

    if (repo == NULL || (sqlite3 *)repo->cvi_hal_db == NULL) {
        goto END;
    }

    sqlite3_snprintf(sizeof(cmd), cmd,
        "SELECT id, name from %s WHERE id=(SELECT face_id FROM %s limit 1 offset %d);",
        FACE_TABLE, FEATURE_TABLE, feature_offset);

    if (SQLITE_OK != (err = sqlite3_prepare_v2((sqlite3 *)repo->cvi_hal_db, cmd, -1, &stmt, NULL))) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        goto END;
    }

    if (SQLITE_ROW == (err = sqlite3_step(stmt))) {
        *face_id = sqlite3_column_int(stmt, 0);
        snprintf(name, name_size, "%s", sqlite3_column_text(stmt, 1));
    }

    ret = 0;
END:

    if (stmt) {
        sqlite3_finalize(stmt);
    }

    return ret;
}
 
int cvi_hal_repo_set_person(const cvi_hal_db_repo_t *repo, int face_id, const cvi_person_t *person)
{
    int ret = -1;
    int err = SQLITE_ERROR;
    sqlite3_stmt *stmt = NULL;

    if (repo == NULL || (sqlite3 *)repo->cvi_hal_db == NULL || person == NULL) {
        goto END;
    }

    if (SQLITE_OK != (err = sqlite3_prepare_v2((sqlite3 *)repo->cvi_hal_db, "UPDATE "FACE_TABLE" SET name=?, identifier=?, serial=?, ic_card=?, image_path=?, type=?, remote_id=? WHERE id=?;", -1, &stmt, NULL))) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        goto END;
    }

    sqlite3_bind_text(stmt, 1, person->name, strlen(person->name), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, person->identifier, strlen(person->identifier), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, person->serial, strlen(person->serial), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, person->ic_card, strlen(person->ic_card), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, person->image_path, strlen(person->image_path), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, person->type, strlen(person->type), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, person->remote_id);
    sqlite3_bind_int(stmt, 8, face_id);

    if (SQLITE_DONE != (err = sqlite3_step(stmt))) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        goto END;
    }

    ret = 0;

END:
    if (stmt) {
        sqlite3_finalize(stmt);
    }

    return ret;
}

int cvi_hal_repo_get_person(const cvi_hal_db_repo_t *repo, int face_id, cvi_person_t *person)
{
    int ret = -1;
    int err = SQLITE_ERROR;
    char cmd[512] = {0};
    sqlite3_stmt *stmt = NULL;

    if (repo == NULL || (sqlite3 *)repo->cvi_hal_db == NULL) {
        goto END;
    }

    memset(person, 0, sizeof(cvi_person_t));

    sqlite3_snprintf(sizeof(cmd), cmd, "SELECT name, identifier, serial, ic_card, image_path, type, remote_id FROM %s WHERE id=%d;", FACE_TABLE, face_id);
    if (SQLITE_OK != (err = sqlite3_prepare_v2((sqlite3 *)repo->cvi_hal_db, cmd, -1, &stmt, NULL))) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        goto END;
    }

    while (SQLITE_ROW == (err = sqlite3_step(stmt))) {
        person->id = face_id;
        snprintf(person->name, sizeof(person->name), "%s",
            sqlite3_column_text(stmt, 0));

        snprintf(person->identifier, sizeof(person->identifier), "%s",
            sqlite3_column_text(stmt, 1));

        snprintf(person->serial, sizeof(person->serial), "%s",
            sqlite3_column_text(stmt, 2));

        snprintf(person->ic_card, sizeof(person->ic_card), "%s",
            sqlite3_column_text(stmt, 3));

        snprintf(person->image_path, sizeof(person->image_path), "%s",
            sqlite3_column_text(stmt, 4));

        snprintf(person->type, sizeof(person->type), "%s",
            sqlite3_column_text(stmt, 5));

        person->remote_id = sqlite3_column_int(stmt, 6);
    }

    if (err != SQLITE_DONE) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        goto END;
    }

    ret = 0;

END:
    if (stmt) {
        sqlite3_finalize(stmt);
    }

    return ret;
}

int cvi_hal_repo_get_person_list(const cvi_hal_db_repo_t *repo, int offset, int limit, cvi_person_t **person)
{
    int i = 0;
    int ret = -1;
    int total = 0;

    int count = 0;
    int err = SQLITE_ERROR;
    char cmd[512] = {0};
    sqlite3_stmt *stmt = NULL;

    if (repo == NULL || (sqlite3 *)repo->cvi_hal_db == NULL) {
        goto END;
    }

    sqlite3_snprintf(sizeof(cmd), cmd,
        "SELECT id, name, identifier, serial, ic_card, image_path, type, remote_id FROM %s LIMIT %d OFFSET %d;", FACE_TABLE, limit, offset);

	if (0 >= (total = get_count((sqlite3 *)repo->cvi_hal_db, "SELECT count(*) FROM "FACE_TABLE";")) ||
        offset >= total) {
        ret = 0;
        goto END;
    }

    if (SQLITE_OK != (err = sqlite3_prepare_v2((sqlite3 *)repo->cvi_hal_db, cmd, -1, &stmt, NULL))) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        goto END;
    }

	if (limit > 0) {
        count = total > offset + limit? limit: total - offset;
    } else {
        count = total - offset;
    }


    if (NULL == (*person = malloc(sizeof(cvi_person_t) * count))) {
        syslog(LOG_ERR, "Out of memory");
        goto END;
    }
    memset(*person, 0, sizeof(cvi_person_t) * count);

    while (SQLITE_ROW == (err = sqlite3_step(stmt))) {
        (*person)[i].id = sqlite3_column_int(stmt, 0);
        snprintf((*person)[i].name, sizeof((*person)[i].name), "%s",
            sqlite3_column_text(stmt, 1));

        snprintf((*person)[i].identifier, sizeof((*person)[i].identifier), "%s",
            sqlite3_column_text(stmt, 2));

        snprintf((*person)[i].serial, sizeof((*person)[i].serial), "%s",
            sqlite3_column_text(stmt, 3));

        snprintf((*person)[i].ic_card, sizeof((*person)[i].ic_card), "%s",
            sqlite3_column_text(stmt, 4));

        snprintf((*person)[i].image_path, sizeof((*person)[i].image_path), "%s",
            sqlite3_column_text(stmt, 5));

        snprintf((*person)[i].type, sizeof((*person)[i].type), "%s",
            sqlite3_column_text(stmt, 6));

        (*person)[i].remote_id = sqlite3_column_int(stmt, 7);

        i++;
    }

    if (err != SQLITE_DONE) {
        syslog(LOG_ERR, "%s", sqlite3_errstr(err));
        goto END;
    }

    ret = count;
END:
    if (stmt) {
        sqlite3_finalize(stmt);
    }

    return ret;
}

int cvi_hal_repo_person_total_count(const cvi_hal_db_repo_t *repo)
{
    if (repo == NULL || (sqlite3 *)repo->cvi_hal_db == NULL) {
        return -1;
    }
    return get_count((sqlite3 *)repo->cvi_hal_db, "SELECT count(*) FROM "FACE_TABLE";");
}

int cvi_hal_repo_feature_total_count(const cvi_hal_db_repo_t *repo)
{
    if (repo == NULL || (sqlite3 *)repo->cvi_hal_db == NULL) {
        return -1;
    }
    return get_count((sqlite3 *)repo->cvi_hal_db, "SELECT count(*) FROM "FEATURE_TABLE";");
}

