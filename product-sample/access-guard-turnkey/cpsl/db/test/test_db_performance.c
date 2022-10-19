#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>

#define TEST_DB "test.db"

void generate_feature(unsigned char *feature, size_t size)
{
    for (int i = 0; i < size; i++) {
        feature[i] = rand() % 256;
    }
}

int init_data(const char *filename)
{
    int ret = -1;
    sqlite3 *db = NULL;
    char *errmsg = NULL;
    sqlite3_stmt *stmt = NULL;
    int err = SQLITE_ERROR;

    unlink(filename);
 
    if (SQLITE_OK != (err = sqlite3_open(filename, &db))) {
        printf("%s\n", sqlite3_errstr(err));
        goto END;
    }

    if (SQLITE_OK != sqlite3_exec(db, "CREATE TABLE repo (name TEXT, feature BLOB);CREATE INDEX index_name ON repo(name);" , NULL, NULL, &errmsg)) {
        printf("%s\n", errmsg);
        goto END;
    }

    if (SQLITE_OK != sqlite3_exec(db, "BEGIN TRANSACTION;" , NULL, NULL, &errmsg)) {
        printf("%s\n", errmsg);
        goto END;
    }

    for (int i = 0; i < 20000; i++) {
        if (SQLITE_OK != (err = sqlite3_prepare_v2(db, "INSERT INTO repo VALUES (?, ?);", -1, &stmt, NULL))) {
            printf("%s\n", sqlite3_errstr(err));
            goto END;
        }

        char name[128] = {0};
        char feature[512] = {0};

        generate_feature(feature, sizeof(feature));
        snprintf(name, sizeof(name), "%s_%d", "user", i);

        sqlite3_bind_text(stmt, 1, name, sizeof(name), SQLITE_TRANSIENT);
        sqlite3_bind_blob(stmt, 2, feature, sizeof(feature), SQLITE_TRANSIENT);

        if (SQLITE_DONE !=  sqlite3_step(stmt)) {
            printf("step fail\n");
            goto END;
        }

        sqlite3_reset(stmt);
    }

    if (SQLITE_OK != sqlite3_exec(db, "COMMIT;" , NULL, NULL, &errmsg)) {
        printf("%s\n", errmsg);
        goto END;
    }

    ret = 0;

END:
    if (errmsg) {
        sqlite3_free(errmsg);
    }

    if (stmt != NULL) {
        sqlite3_finalize(stmt);
    }

    if (db) {
        sqlite3_close(db);
    }

    return ret;
}

long long get_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (tv.tv_sec) * 1000 + (tv.tv_usec)/1000 ;
}

void test_select_all_performance(const char *filename)
{
    sqlite3 *db = NULL;
    char *errmsg = NULL;
    sqlite3_stmt *stmt = NULL;
    int err = SQLITE_ERROR;

    if (SQLITE_OK != (err = sqlite3_open(filename, &db))) {
        printf("%s\n", sqlite3_errstr(err));
        goto END;
    }

    if (SQLITE_OK != (err = sqlite3_prepare_v2(db, "SELECT * FROM repo;", -1, &stmt, NULL))) {
        printf("%s\n", sqlite3_errstr(err));
        goto END;
    }

    int count = 0;
    long long start = get_time();
    printf("select all start: %lld\n", start);
    while (err = sqlite3_step(stmt)) {
        if (err != SQLITE_ROW) {
            if (err != SQLITE_DONE) {
                printf("%s\n", sqlite3_errstr(err));
            }
            break;
        }
        count++;
    }

    long long end = get_time();
    printf("select all - total: %d, end: %lld, total: %lld\n", count, end, end - start);

END:
    if (stmt != NULL) {
        sqlite3_finalize(stmt);
    }

    if (db != NULL) {
        sqlite3_close(db);
    }
}

void test_select_one_performance(const char *filename)
{
    sqlite3 *db = NULL;
    char *errmsg = NULL;
    sqlite3_stmt *stmt = NULL;
    int err = SQLITE_ERROR;

    if (SQLITE_OK != (err = sqlite3_open(filename, &db))) {
        printf("%s\n", sqlite3_errstr(err));
        goto END;
    }
 
    char *cmd = sqlite3_mprintf("SELECT * FROM repo WHERE name='user_%d';", rand() % 20000);
    printf("%s\n", cmd);
    if (SQLITE_OK != (err = sqlite3_prepare_v2(db, cmd, -1, &stmt, NULL))) {
        printf("%s\n", sqlite3_errstr(err));
        goto END;
    }

    int count = 0;
    long long start = get_time();
    printf("select one performance start: %lld\n", start);
    while (err = sqlite3_step(stmt)) {
        if (err != SQLITE_ROW) {
            if (err != SQLITE_DONE) {
                printf("%s\n", sqlite3_errstr(err));
            }
            break;
        }
        printf("%s\n", sqlite3_column_text(stmt, 0));
        count++;
    }

    long long end = get_time();
    printf("select one - count: %d, end: %lld, total: %lld\n", count, end, end - start);

END:
    if (stmt != NULL) {
        sqlite3_finalize(stmt);
    }

    if (cmd != NULL) {
        sqlite3_free(cmd);
    }

    if (db != NULL) {
        sqlite3_close(db);
    }
}

int main(int argc, const char *argv[])
{
    srand(time(NULL));
    struct stat st;

    if (stat(TEST_DB, &st) != 0) {
        init_data(TEST_DB);
    }
    test_select_all_performance(TEST_DB);
    test_select_one_performance(TEST_DB);

    return 0;
}
