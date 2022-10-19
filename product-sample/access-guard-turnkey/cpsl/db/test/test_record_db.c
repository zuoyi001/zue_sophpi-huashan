#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <sqlite3.h>
#include <record.h>

#define TEST_RECORD_PATH "record.db"

static unsigned long get_ms_timestamp()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return (unsigned long)(tv.tv_sec) * 1000 + (unsigned long)(tv.tv_usec) / 1000;
}

int test_record_add()
{
    cvi_hal_record_ctx_t ctx = {0};
    if (0 > cvi_hal_record_open(&ctx, TEST_RECORD_PATH)) {
        printf("repo open fail\n");
        return -1;
    }

    cvi_hal_record_t record = {0};
    snprintf(record.name, sizeof(record.name), "%s", "test");
    snprintf(record.serial, sizeof(record.serial), "%s", "A1234");
    snprintf(record.ic_card, sizeof(record.ic_card), "%s", "0204");
    record.verification_type = 1;
    snprintf(record.image_path, sizeof(record.image_path), "%s", "snapshot.jpg");

    if (0 > cvi_hal_record_add(&ctx, &record)) {
        printf("cvi_record_add fail\n");
        return -1;
    }

    cvi_hal_record_close(&ctx);
    unlink(TEST_RECORD_PATH);
    return 0;
}

int test_record_get()
{
    cvi_hal_record_ctx_t ctx = {0};
    if (0 > cvi_hal_record_open(&ctx, TEST_RECORD_PATH)) {
        printf("repo open fail\n");
        return -1;
    }

    for (int i = 0; i < 10; i++) {
        cvi_hal_record_t record = {0};
        snprintf(record.name, sizeof(record.name), "user_%d", i);
        snprintf(record.serial, sizeof(record.serial), "serial100%d", i);
        snprintf(record.ic_card, sizeof(record.ic_card), "ic_card00000%d", i+10);
        record.verification_type = i % 5 == 0? -1: i % 5;
        snprintf(record.image_path, sizeof(record.image_path), "img_%d", i);
        record.temperature = 30.5 + i;

        if (0 > cvi_hal_record_add(&ctx, &record)) {
            printf("cvi_record_add fail\n");
            return -1;
        }
    }

    cvi_record_t *records = NULL;
    int count = cvi_hal_record_get(&ctx, 0, 5, NULL, &records);
    for (int i = 0; i < count; i++) {
        char name[64] = {0};
        char serial[64] = {0};
        char ic_card[64] = {0};
        snprintf(name, sizeof(name), "user_%d", 10-i-1);
        snprintf(serial, sizeof(serial), "serial100%d", 10-i-1);
        snprintf(ic_card, sizeof(ic_card), "ic_card00000%d", 20 - i - 1);
        int type = (10-i-1) % 5 == 0? -1: (10-i-1) % 5;

        if (strcmp(name, records[i].name) != 0) {
            printf("%d, name different: %s, %s\n", i, name, records[i].name);
            return -1;
        }
        if (strcmp(serial, records[i].serial) != 0) {
            printf("serila different, %s, %s\n", serial, records[i].serial);
            return -1;
        }
        if (strcmp(ic_card, records[i].ic_card) != 0) {
            printf("ic card different, %s, %s\n", ic_card, records[i].ic_card);
            return -1;
        }
        if (type != records[i].verification_type) {
            printf("type different, %d, %d\n", type, records[i].verification_type);
            return -1;
        }
    }

    free(records);
    cvi_hal_record_close(&ctx);

    unlink(TEST_RECORD_PATH);
    return 0;
}

int test_record_filter_get()
{
    cvi_record_ctx_t ctx = {0};
    if (0 > cvi_hal_record_open(&ctx, TEST_RECORD_PATH)) {
        printf("repo open fail\n");
        return -1;
    }

    int i = 0;
    unsigned long first_time = get_ms_timestamp();
    for (i = 0; i < 10; i++) {
        cvi_record_t record = {0};
        snprintf(record.name, sizeof(record.name), "user_%d", i);
        snprintf(record.serial, sizeof(record.serial), "serial100%d", i);
        snprintf(record.ic_card, sizeof(record.ic_card), "ic_card00000%d", i+10);
        record.verification_type = i % 5 == 0? -1: i % 5;
        snprintf(record.image_path, sizeof(record.image_path), "img_%d", i);
        record.temperature = 30.5 + i;

        if (0 > cvi_hal_record_add(&ctx, &record)) {
            printf("cvi_record_add fail\n");
            return -1;
        }
    }

    cvi_record_t *records = NULL;
    cvi_record_filter_t filter = {0};
    snprintf(filter.name, sizeof(filter.name), "%s", "user_1");

    int total = 0;
    int count = cvi_hal_record_get(&ctx, 0, 20, &filter, &records);
    if (count != 1 && strcmp(records[0].name, "user_1")) {
        printf("record count error\n");
        return -1;
    }
    free(records);

    sleep(1);
    unsigned long second_time = get_ms_timestamp();
    sleep(1);
    for (i; i < 20; i++) {
        cvi_record_t record = {0};
        snprintf(record.name, sizeof(record.name), "user_%d", i);
        snprintf(record.serial, sizeof(record.serial), "serial100%d", i);
        snprintf(record.ic_card, sizeof(record.ic_card), "ic_card00000%d", i+10);
        record.verification_type = i % 5 == 0? -1: i % 5;
        snprintf(record.image_path, sizeof(record.image_path), "img_%d", i);
        record.temperature = 30.5 + i;

        if (0 > cvi_hal_record_add(&ctx, &record)) {
            printf("cvi_record_add fail\n");
            return -1;
        }
    }

    sleep(1);
    memset(&filter, 0, sizeof(filter));
    filter.start_time = first_time;
    count = cvi_hal_record_get(&ctx, 0, 20, &filter, &records);
    if (count != 20) {
        printf("start time filter count error\n");
        return -1;
    }
    free(records);

    memset(&filter, 0, sizeof(filter));
    filter.end_time = get_ms_timestamp();
    count = cvi_hal_record_get(&ctx, 0, 50, &filter, &records);
    if (count != 20) {
        printf("end time filter count error\n");
        return -1;
    }
    free(records);

    memset(&filter, 0, sizeof(filter));
    filter.start_time = first_time;
    filter.end_time = second_time;
    count = cvi_hal_record_get(&ctx, 0, 50, &filter, &records);
    if (count != 10) {
        printf("between time filter count error\n");
        return -1;
    }
    free(records);

    cvi_hal_record_close(&ctx);
    unlink(TEST_RECORD_PATH);

    return 0;
}

int test_record_delete_by_date()
{
    cvi_record_ctx_t ctx = {0};
    if (0 > cvi_hal_record_open(&ctx, TEST_RECORD_PATH)) {
        printf("repo open fail\n");
        return -1;
    }

    int i = 0;
    unsigned long first_time = get_ms_timestamp();
    for (i = 0; i < 10; i++) {
        cvi_record_t record = {0};
        snprintf(record.name, sizeof(record.name), "user_%d", i);
        snprintf(record.serial, sizeof(record.serial), "serial100%d", i);
        snprintf(record.ic_card, sizeof(record.ic_card), "ic_card00000%d", i+10);
        record.verification_type = i % 5 == 0? -1: i % 5;
        snprintf(record.image_path, sizeof(record.image_path), "img_%d", i);
        record.temperature = 30.5 + i;

        if (0 > cvi_hal_record_add(&ctx, &record)) {
            printf("cvi_record_add fail\n");
            return -1;
        }
    }

    sleep(1);
    unsigned long second_time = get_ms_timestamp();
    sleep(1);
    for (i; i < 20; i++) {
        cvi_record_t record = {0};
        snprintf(record.name, sizeof(record.name), "user_%d", i);
        snprintf(record.serial, sizeof(record.serial), "serial100%d", i);
        snprintf(record.ic_card, sizeof(record.ic_card), "ic_card00000%d", i+10);
        record.verification_type = i % 5 == 0? -1: i % 5;
        snprintf(record.image_path, sizeof(record.image_path), "img_%d", i);
        record.temperature = 30.5 + i;

        if (0 > cvi_hal_record_add(&ctx, &record)) {
            printf("cvi_record_add fail\n");
            return -1;
        }
    }

    sleep(1);

    if (0 > cvi_hal_record_delete_by_date(&ctx, second_time, get_ms_timestamp())) {
        printf("delete by time fail\n");
        return -1;
    }

    int total = 0;
    cvi_record_t *records = NULL;
    int count = cvi_hal_record_get(&ctx, 0, 50, NULL, &records);
    if (count != 10) {
        printf("delete by end time result error\n");
        return -1;
    }
    free(records);

    cvi_hal_record_close(&ctx);
    unlink(TEST_RECORD_PATH);
    return 0;
}

int test_record_delete_by_id()
{
    cvi_record_ctx_t ctx = {0};
    if (0 > cvi_hal_record_open(&ctx, TEST_RECORD_PATH)) {
        printf("repo open fail\n");
        return -1;
    }
    int i = 0;
    for (i = 0; i < 10; i++) {
        cvi_record_t record = {0};
        snprintf(record.name, sizeof(record.name), "user_%d", i);
        snprintf(record.serial, sizeof(record.serial), "serial100%d", i);
        snprintf(record.ic_card, sizeof(record.ic_card), "ic_card00000%d", i+10);
        record.verification_type = i % 5 == 0? -1: i % 5;
        snprintf(record.image_path, sizeof(record.image_path), "img_%d", i);
        record.temperature = 30.5 + i;

        if (0 > cvi_hal_record_add(&ctx, &record)) {
            printf("cvi_record_add fail\n");
            return -1;
        }
    }

    cvi_record_t *records = NULL;
    int count = cvi_hal_record_get(&ctx, 0, 50, NULL, &records);
    if (count <= 0) {
        printf("get record error\n");
        return -1;
    }

    int record_id = records[0].id;
    free(records);

    if (0 > cvi_hal_record_delete(&ctx, record_id)) {
        printf("record delete by id fail\n");
        return -1;
    }

    int count2 = cvi_hal_record_get(&ctx, 0, 50, NULL, &records);
    if (count2 != count -1) {
        printf("delete result error\n");
        return -1;
    }

    cvi_hal_record_close(&ctx);
    unlink(TEST_RECORD_PATH);

    return 0;
}

int test_record_total_count()
{
    cvi_record_ctx_t ctx = {0};
    if (0 > cvi_hal_record_open(&ctx, TEST_RECORD_PATH)) {
        printf("repo open fail\n");
        return -1;
    }

    for (int i = 0; i < 10; i++) {
        cvi_record_t record = {0};
        snprintf(record.name, sizeof(record.name), "user_%d", i);
        snprintf(record.serial, sizeof(record.serial), "serial100%d", i);
        snprintf(record.ic_card, sizeof(record.ic_card), "ic_card00000%d", i+10);
        record.verification_type = i % 5 == 0? -1: i % 5;
        snprintf(record.image_path, sizeof(record.image_path), "img_%d", i);
        record.temperature = 30.5 + i;

        if (0 > cvi_hal_record_add(&ctx, &record)) {
            printf("cvi_hal_record_add fail\n");
            return -1;
        }
    }

    int total = cvi_hal_record_total_count(&ctx, NULL);
    if (total != 10) {
        printf("total count result error, %d\n", total);
        return -1;
    }

    cvi_hal_record_close(&ctx);
    unlink(TEST_RECORD_PATH);

    return 0;
}

int main(int argc, const char *argv[])
{
    unlink(TEST_RECORD_PATH);

    if (0 > test_record_add()) {
        printf("test_record_add fail\n");
        return -1;
    }

    if (0 > test_record_total_count()) {
        printf("test_record_total_count fail\n");
        return -1;
    }

    if (0 > test_record_get()) {
        printf("test_record_get fail\n");
        return -1;
    }

    if (0 > test_record_filter_get()) {
        printf("test_record_filter_get fail\n");
        return -1;
    }

    if (0 > test_record_delete_by_id()) {
        printf("test_record_delete_by_id fail\n");
        return -1;
    }

    if (0 > test_record_delete_by_date()) {
        printf("test_record_delete_by_date fail\n");
        return -1;
    }

    return 0;
}
