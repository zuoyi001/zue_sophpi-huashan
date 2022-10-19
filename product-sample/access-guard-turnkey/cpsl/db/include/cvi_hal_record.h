#ifndef __CVI_HAL_RECORD_H__
#define __CVI_HAL_RECORD_H__

#ifdef  __cplusplus
extern  "C" {
#endif

#define WEBSERVER_PID_FILE "/var/run/webserver.pid"
/* "/mnt/data/record.db" */
#define RECORD_PATH "./record.db"

typedef struct {
    char path[128];
    void *db;
} cvi_record_ctx_t;

typedef struct {
    int id;
    // this field is only for user who is added from cloud
    int remote_id;
	char type[8]; // this field is for cloud
    char name[128];
    char serial[64];
    char identifier[64];
    char ic_card[64];
    unsigned long long timestamp;
    float temperature;
    int verification_type;
	int status;
	int reserve;
    char image_path[64];
} cvi_record_t;

typedef struct {
    char name[128];
    char serial[64];
    char ic_card[64];
    float ceiling_temperature;
    float floor_temperature;
    unsigned long start_time;
    unsigned long end_time;
    int verification_type;
} cvi_record_filter_t;

int cvi_hal_record_open(cvi_record_ctx_t *ctx, const char *path);
int cvi_hal_record_close(cvi_record_ctx_t *ctx);
int cvi_hal_record_add(const cvi_record_ctx_t *ctx, const cvi_record_t *record);
int cvi_hal_record_total_count(const cvi_record_ctx_t *ctx, const cvi_record_filter_t *filter);
int cvi_hal_record_get(const cvi_record_ctx_t *ctx, int offset, int limit, cvi_record_filter_t *filter, cvi_record_t **records);
int cvi_hal_record_delete(const cvi_record_ctx_t *ctx, int record_id);
int cvi_hal_record_delete_by_date(const cvi_record_ctx_t *ctx, unsigned long start_time, unsigned long end_time);
int cvi_hal_record_delete_all(const cvi_record_ctx_t *ctx);
int cvi_hal_get_record_list(cvi_record_t **record_list, int offset, int limit );


#ifdef  __cplusplus
}
#endif

#endif
