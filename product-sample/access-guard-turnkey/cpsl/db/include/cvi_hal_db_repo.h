#ifndef __PERSON_DATA_H__
#define __PERSON_DATA_H__

#include <stdlib.h>

#ifdef  __cplusplus
extern  "C" {
#endif

#define PER_USER_FEATURE_LIMIT 3

typedef struct {
    char path[128];
    void *cvi_hal_db;
} cvi_hal_db_repo_t;

typedef struct {
    int id;
    int remote_id; // this field is for cloud
    char type[8]; // this field is for cloud
    char name[128];
    char identifier[64];
    char serial[64];
    char ic_card[64];
    char image_path[64];
} cvi_person_t;

int cvi_hal_repo_open(cvi_hal_db_repo_t *repo, const char *path);
int cvi_hal_repo_close(cvi_hal_db_repo_t *repo);
int cvi_hal_repo_set_person(const cvi_hal_db_repo_t *repo, int face_id, const cvi_person_t *person);
int cvi_hal_repo_get_person(const cvi_hal_db_repo_t *repo, int face_id, cvi_person_t *person);
int cvi_hal_repo_get_person_list(const cvi_hal_db_repo_t *repo, int offset, int limit, cvi_person_t **person);
int cvi_hal_repo_person_total_count(const cvi_hal_db_repo_t *repo);
int cvi_hal_repo_feature_total_count(const cvi_hal_db_repo_t *repo);
int cvi_hal_repo_add_face(const cvi_hal_db_repo_t *repo, const char *name, unsigned char *feature, size_t feature_size);
int cvi_hal_repo_update_face(const cvi_hal_db_repo_t *repo, int face_id, const char *name, const unsigned char *feature, size_t feature_size);
int cvi_hal_repo_delete_face(const cvi_hal_db_repo_t *repo, int face_id);
int cvi_hal_repo_find_face(const cvi_hal_db_repo_t *repo, int face_id, char *name, size_t name_size, unsigned char *feature, size_t feature_size);
int cvi_hal_repo_get_features(const cvi_hal_db_repo_t *repo, int offset, int limit, unsigned char **features);
int cvi_hal_repo_get_face_by_offset(const cvi_hal_db_repo_t *repo, int feature_offset, int *face_id, char *name, size_t name_size);

#ifdef  __cplusplus
}
#endif

#endif
