#ifndef __CVI_HAL_FACE_DATATYPE_H__
#define __CVI_HAL_FACE_DATATYPE_H__
#include <stdint.h>
#include <stdbool.h>
#include "cvi_hal_db_repo.h"

typedef void* hal_facelib_handle_t;
typedef uint32_t hal_face_id_t;
typedef void* hal_face_t;

#define FACE_PTS                5
#define MAX_NAME_LEN            128
#define NUM_FACE_FEATURE_DIM    256
#define NUM_EMOTION_FEATURE_DIM 7
#define NUM_GENDER_FEATURE_DIM  2
#define NUM_RACE_FEATURE_DIM    3
#define NUM_AGE_FEATURE_DIM     101

typedef struct {
  float x1;
  float y1;
  float x2;
  float y2;
  float score;
} hal_face_bbox_t;

typedef struct {
    char name[MAX_NAME_LEN];
} hal_face_name_t;

typedef struct {
    void *internal_data;
} hal_face_image_t;

typedef enum {
    RGB_FORMAT_888,
    RGB_FORMAT_565,
    BGR_FORMAT_888,
    BGR_FORMAT_565,
} hal_face_rgb_type;

typedef enum _FACE_DETECT_RET {
  SUCCESS,
  FAIL_FILE_INVALID,
  FAIL_NO_FACE,
  FAIL_MULTI_FACE,
  FAIL_LOW_FACE_QUALITY,
  FAIL_DUPLICATE,
}HAL_FACE_DETECT_RET;

typedef struct {
	float threshold_1v1;
	float threshold_1vN;
	float threshold_facereg;
	float pitch;
	float yaw;
	float roll;
	float min_face_h;
	int face_quality;
	int face_pixel_min;
	int light_sens;
	int move_sens;
	float threshold_liveness;
	float threshold_det;
	bool wdr_en;
} hal_facelib_attr_t;

typedef struct {
	bool fd_en;
	bool facereg_en;
	bool face_matching_en;
	bool yolo_en;
	bool config_yolo; //decide whether or not support yolo
	bool config_liveness;//decide whether or not support liveness
	bool is_cv_mat_mode;
	bool is_face_quality;
	char *model_face_fd;
	char *model_face_quality;
	char *model_face_stn;
	char *model_face_extr;
	char *model_face_liveness;
	char *model_face_liveness_bgr;
	char *model_face_landmark;
	char *model_face_postfilter;
	char *model_face_poseblur;
	char *model_face_anchor;
	char *model_face_attr;
	char *model_face_search;
	char *model_yolo3;
	char *repo_path;
	char *db_repo_path;
	char *face_lib_path;
	hal_facelib_attr_t attr;
} hal_facelib_config_t;

typedef struct {
    int id;
    int remote_id; // this field is for cloud
    char type[8]; // this field is for cloud
    char name[128];
    char identifier[64];
    char serial[64];
    char ic_card[64];
    char image_path[64];
} hal_person_info_t;

typedef struct cvi_db_feature {
    int8_t *raw;
    float *raw_unit;
    uint32_t size;
    uint32_t num;
    float *db_buffer;
} hal_db_feature_t;

typedef struct {
    //unique_ptr<Repository> repo;
    cvi_hal_db_repo_t db_repo;
    hal_facelib_attr_t attr;
    hal_db_feature_t *db_feature;
} hal_facelib_context_t;


typedef enum {
  FT_INT8 = 0, /**< Equals to int8_t. */
  FT_UINT8,    /**< Equals to uint8_t. */
  FT_INT16,    /**< Equals to int16_t. */
  FT_UINT16,   /**< Equals to uint16_t. */
  FT_INT32,    /**< Equals to int32_t. */
  FT_UINT32,   /**< Equals to uint32_t. */
  FT_BF16,     /**< Equals to bf17. */
  FT_FLOAT     /**< Equals to float. */
} hal_face_feature_type_e;

typedef struct {
  int8_t* ptr;
  uint32_t size;
  hal_face_feature_type_e type;
} hal_face_feature_t;

#endif /*__CVI_HAL_FACE_DATETYPE*/