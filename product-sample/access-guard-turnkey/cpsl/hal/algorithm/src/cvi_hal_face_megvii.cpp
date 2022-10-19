#include <memory>
#include <vector>
#include <string>
#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include "cvi_hal_db_repo.h"
#include <assert.h>
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vpss.h"
#include "cvi_hal_face_api.h"
#include "face_sdk.h"

using namespace std;
using namespace cv;

#if 1
extern int FACELIB_INIT(const hal_facelib_config_t *facelib_config);
extern int FACELIB_RELEASE();
extern hal_face_t FACELIB_CREATE_FACE();
extern void FACELIB_FREE_FACE(hal_face_t faces);
extern void FACELIB_CLEAR_FACE(hal_face_t faces);
extern int FACELIB_DETECT(VIDEO_FRAME_INFO_S *frame, hal_face_t faces, float min_face_h);
extern int FACELIB_RECOG(VIDEO_FRAME_INFO_S *frame, hal_face_t faces);
extern int FACELIB_LIVENESS(VIDEO_FRAME_INFO_S *bgrFrame, VIDEO_FRAME_INFO_S *irFrame, hal_face_t faces);
extern int FACELIB_LIVENESS_BGR(VIDEO_FRAME_INFO_S *bgrFrame, hal_face_t faces);
extern int FACELIB_GET_FACE_INFO(const char *image_path, hal_face_t faces);
extern int FACELIB_GET_FACE_NUM(hal_face_t faces);
extern int FACELIB_GET_FACE_RECT(hal_face_t faces, int idx, hal_face_bbox_t *rect);
extern int FACELIB_GET_IMAGE_SIZE(hal_face_t faces, int *width, int *height);
extern int FACELIB_GET_FEATURE(hal_face_t faces, int idx, hal_face_feature_t *feature);
extern int FACELIB_GET_NAME(hal_face_t faces, int idx, char **name, size_t *size);
extern int FACELIB_GET_LIVENESS_SCORE(hal_face_t faces, int idx, float *score);
extern int FACELIB_RECT_RESCALE(float width, float height, hal_face_t faces,
                                int idx, bool keep_aspect, hal_face_bbox_t *box);
extern int FACELIB_DEL_FACE(hal_face_t faces, int idx);
extern int FACELIB_IMPORT_FEATURE_GROUP(int8_t *feature_ptr, int feature_dim, int feature_type, int feature_num);
extern int FACELIB_FREE_FEATURE_GROUP();
extern int FACELIB_FEATURE_MATCH(int8_t *feature, int8_t *precached, float *unit_precached_arr,
                              uint32_t *k_index, float *k_value, float *buffer,
                              const uint32_t data_length, const uint32_t data_num, const uint32_t k);
extern int FACELIB_COMPARE(int8_t *ptr1, int8_t *ptr2, int feature_dim, float *score);
#endif
#if 0
//typedef CachedRepo<512> Repository;
typedef struct  {
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
    bool enable_feature_tpu;
} hal_facelib_context_t;
#endif

#define AI_SDK_USE_VPSS_GRP 5
static bool bFaceLibInited = false;
//static cviai_handle_t cviai_handle = NULL;

//static FaceFunc face_lib;

//sensor calibrate 

#if 1
enum CALIBRATION_STATE {CALIBRATION_NONE,  CALIBRATION_IN_PROGRESS, CALIBRATION_FINISHED};

static int   g_calibration_state = CALIBRATION_NONE;
static float g_left_right_k;
static float g_left_right_b;
static float g_top_bottom_k;
static float g_top_bottom_b;

void resetRgbirCalibration(){
    g_calibration_state = CALIBRATION_NONE;
    g_left_right_k = 0.0;
    g_left_right_b = 0.0;
    g_top_bottom_k = 0.0;
    g_top_bottom_b = 0.0;
}

static unsigned char Rgb_Sensor_bgr_Buf[576 * 1024 * 3] = {0}; 
static unsigned char Ir_Sensor_bgr_Buf[576 * 1024 * 3] = {0}; 
#endif

int HAL_FACE_RectRescale(float width, float height, hal_face_t faces, int idx, bool keep_aspect, hal_face_bbox_t *box) {
    #if 0
    if (!face_lib.rect_rescale) {
        printf("function[rect rescale] not loaded!");
        return -1;
    }
    return face_lib.rect_rescale(width, height, faces, idx, keep_aspect, box);
    #endif
    return FACELIB_RECT_RESCALE(width, height, faces, idx, keep_aspect, box);
}

int HAL_FACE_LibOpen(const hal_facelib_config_t *facelib_config)
{
    #if 0
    int ret = 0;
    if(bFaceLibInited)
    {
        printf("FaceLib already initialized.Return directly.\n");
        return ret;
    }
    ret = FaceProxyOpenLib(facelib_config->face_lib_path, face_lib);
    if (ret != 0) {
        printf("FaceProxyOpenLib failed.\n");
        return ret;
    }
    ret = face_lib.init(facelib_config);
    if (ret != 0) {
        printf("face lib init failed ret:%d\n", ret);
        return ret;
    }
    bFaceLibInited = true;
    return 0;
    #endif
    int ret = 0;
    if(bFaceLibInited)
    {
        printf("FaceLib already initialized.Return directly.\n");
        return ret;
    }
    ret = FACELIB_INIT(facelib_config);

    if (ret != 0) {
        printf("face lib init failed ret:%d\n", ret);
        return ret;
    }
    bFaceLibInited = true;

    const char* versionPtr = getVersion();
    printf("megvii algo version: %s.\n",versionPtr);
    return 0;
}

int HAL_FACE_LibClose(void)
{
    #if 1
    if(!bFaceLibInited)
        return 0;
    //face_lib.release();
    FACELIB_RELEASE();
    //FaceProxyCloseLib(face_lib);
    bFaceLibInited = false;
    printf("CviFaceLibClose\n");
    return 0;
    #endif
}

int HAL_FACE_Detect(VIDEO_FRAME_INFO_S *stfdFrame, hal_face_t faces, float min_face_h) {
    #if 1
    //if (!face_lib.detect) {
    //    printf("function[detect] not loaded!");
    //    return -1;
    //}
    int ret = 0;

    //ret = face_lib.detect(stfdFrame, faces);
    ret = FACELIB_DETECT(stfdFrame, faces, min_face_h);
    if (ret != 0) {
        printf("HAL_FACE_Detect failed ret:%d\n", ret);
    }
    return ret;
    #endif
}

int HAL_FACE_LivenessDetect(VIDEO_FRAME_INFO_S *rgbFrame,VIDEO_FRAME_INFO_S *irFrame,
         hal_face_t faces) {
    #if 1
    //if (!face_lib.liveness) {
    //    printf("function[liveness] not loaded!");
    //    return -1;
    //}
    int ret = 0;

    //ret = face_lib.liveness(rgbFrame, irFrame, faces);
    ret =  FACELIB_LIVENESS(rgbFrame, irFrame, faces);
    if (ret) {
        printf("HAL_FACE_LivenessDetect failed, ret:%d\n", ret);
    }
    return ret;
    #endif
}

int HAL_FACE_LivenessDetectBgr(VIDEO_FRAME_INFO_S *rgbFrame, hal_face_t faces) {
    #if 1
    //if (!face_lib.liveness_bgr) {
    //    printf("function[liveness_bgr] not loaded!");
    //    return -1;
    //}
    int ret = 0;

    //ret = face_lib.liveness_bgr(rgbFrame, faces);
    ret = FACELIB_LIVENESS_BGR(rgbFrame, faces);
    if (ret) {
        printf("HAL_FACE_LivenessDetectBgr failed, ret:%d\n", ret);
    }
    return ret;
    #endif
}

int HAL_FACE_Recognize(VIDEO_FRAME_INFO_S *stfrFrame, hal_face_t face) {
    #if 1
    //if (!face_lib.recog) {
    //    printf("function[recog] not loaded!");
    //    return -1;
    //}
    int ret = 0;

    //ret = face_lib.recog(stfrFrame, face);
    ret = FACELIB_RECOG(stfrFrame, face);
    if (ret) {
        printf("HAL_FACE_Recognize failed, ret:%d\n", ret);
    }
    return ret;
    #endif
}

int HAL_FACE_Create(hal_face_t *face) {
    #if 1
    //if (!face_lib.create_face) {
    //    printf("function[create_face] not loaded!");
    //   return -1;
    //}
    //*face = face_lib.create_face();
    *face = FACELIB_CREATE_FACE();
    if (face) {
        return 0;
    } else {
        printf("HAL_FACE_Create failed\n");
        return -1;
    }
    #endif
}

int HAL_FACE_Free(hal_face_t face) {
    #if 1
    //if (!face_lib.free_face) {
    //    printf("function[free_face] not loaded!");
    //    return -1;
    //}
    //face_lib.free_face(face);

    FACELIB_FREE_FACE(face);
    return 0;
    #endif
}

int HAL_FACE_Clear(hal_face_t face) {
    #if 1
    //if (!face_lib.clear_face) {
    //    printf("function[clear_face] not loaded!");
    //    return -1;
    //}
    //face_lib.clear_face(face);
    FACELIB_CLEAR_FACE(face);
    return 0;
    #endif
}

int HAL_FACE_DelFace(hal_face_t face, int idx) {
    #if 1
    //if (!face_lib.del_face) {
    //    printf("function[del_face] not loaded!");
    //    return -1;
    //}
    //face_lib.del_face(face, idx);
    FACELIB_DEL_FACE(face, idx);
    return 0;
    #endif
}

int HAL_FACE_GetNum(hal_face_t face) {
    #if 0
    if (!face_lib.get_face_num) {
        printf("function[get_face_num] not loaded!");
        return -1;
    }
    return face_lib.get_face_num(face);
    #endif
    return FACELIB_GET_FACE_NUM(face);
}

int HAL_FACE_GetFaceRect(hal_face_t face, int idx, hal_face_bbox_t *rect) {
    #if 0
    if (!face_lib.get_face_rect) {
        printf("function[get_face_rect] not loaded!");
        return -1;
    }
    return face_lib.get_face_rect(face, idx, rect);
    #endif
    return FACELIB_GET_FACE_RECT(face, idx, rect);
}

int HAL_FACE_GetImageSize(hal_face_t face, int *width, int *height) {
    #if 0
    if (!face_lib.get_image_size) {
        printf("function[get_image_size] not loaded!");
        return -1;
    }
    return face_lib.get_image_size(face, width, height);
    #endif
    return -1;
}

int HAL_FACE_GetFeature(hal_face_t face, int idx, hal_face_feature_t *feature) {
    #if 0
    if (!face_lib.get_feature) {
        printf("function[get_feature] not loaded!");
        return -1;
    }
    return face_lib.get_feature(face, idx, feature);
    #endif

    return FACELIB_GET_FEATURE(face, idx, feature);
}

int HAL_FACE_GetLivenessScore(hal_face_t face, int idx, float *score) {
    #if 0
    if (!face_lib.get_liveness_score) {
        printf("function[get_liveness_score] not loaded!");
        return -1;
    }
    return face_lib.get_liveness_score(face, idx, score);
    #endif

    return FACELIB_GET_LIVENESS_SCORE(face, idx, score);
}

int HAL_FACE_LibRepoOpen(const hal_facelib_config_t *facelib_config,hal_facelib_handle_t *handle)
{
    #if 1
    hal_facelib_context_t *context = new hal_facelib_context_t;
    if (cvi_hal_repo_open(&(context->db_repo), facelib_config->db_repo_path)) {
        return -1;
    }

    // Init DB
    context->db_feature = new hal_db_feature_t;
    // Init value
    context->db_feature->raw = nullptr;
    context->db_feature->raw_unit = nullptr;
    context->db_feature->db_buffer = nullptr;

    context->db_feature->size = 0;
    context->db_feature->num = 0;

   *handle = context;
    context->attr = facelib_config->attr;

    return 0;
    #endif
}

int ResetVerify(hal_facelib_handle_t handle)
{
    #if 1
    hal_facelib_context_t *context = static_cast<hal_facelib_context_t*>(handle);

    if (context->db_feature->raw != nullptr ) {
        free(context->db_feature->raw);
    }
    if (context->db_feature->raw_unit != nullptr) {

        delete[] context->db_feature->raw_unit;
    }

    if (context->db_feature->db_buffer != nullptr) {

        delete[] context->db_feature->db_buffer;
    }

    context->db_feature->raw = nullptr;
    context->db_feature->raw_unit = nullptr;
    context->db_feature->db_buffer = nullptr;

    context->db_feature->size = 0;
    context->db_feature->num = 0;
    FACELIB_FREE_FEATURE_GROUP();
    #endif
    return 0;
    
}

int HAL_FACE_LibRepoClose(hal_facelib_handle_t handle)
{
    #if 1
    hal_facelib_context_t *fctx = static_cast<hal_facelib_context_t*>(handle);

    if(fctx == nullptr)
        return 0;

    cvi_hal_repo_close(&fctx->db_repo);

    if(fctx->db_feature != nullptr)
    {
        ResetVerify(handle);
    }
    else
    {
       return 0;
    }

    if(fctx->db_feature != NULL)
    {
        delete fctx->db_feature;
        fctx->db_feature = nullptr;
    }

    delete fctx;
    fctx = nullptr;
    #endif
    return 0;
}

int CviLoadIdentify(hal_facelib_handle_t handle)
{
    #if 1
    hal_facelib_context_t *context = static_cast<hal_facelib_context_t*>(handle);
    //context->enable_feature_tpu = use_tpu;
    ResetVerify(handle);
    hal_db_feature_t *dbmeta = context->db_feature;
    // Get from db function.
    uint8_t *from_db = nullptr;
    dbmeta->num = cvi_hal_repo_get_features(&(context->db_repo), 0, -1, &from_db);

    if (dbmeta->num == 0) {
        printf("Error! DB is empty.\n");
        return -1;
    }
    dbmeta->size = NUM_FACE_FEATURE_DIM * dbmeta->num;
    dbmeta->raw = (int8_t *)from_db;

    /*
    dbmeta->raw_unit = new float[dbmeta->num * sizeof(float)];
    cvm_gen_db_i8_unit_length(dbmeta->raw, dbmeta->raw_unit, NUM_FACE_FEATURE_DIM, dbmeta->num);
    // Create buffer
    dbmeta->db_buffer = new float[dbmeta->num];
    */
    FACELIB_IMPORT_FEATURE_GROUP(dbmeta->raw, NUM_FACE_FEATURE_DIM, 1, dbmeta->num);
    if (context->db_feature->raw != nullptr ) {
        free(context->db_feature->raw);
        context->db_feature->raw = nullptr;
    }
    return 0;
    #endif
}

int AAL_GetFaceLibAttr(const hal_facelib_handle_t handle, hal_facelib_attr_t *facelib_attr) {
    #if 1
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);

    *facelib_attr = ctx->attr;
    #endif
	return 0;
}

int CviUpdateFaceLibAttr(const hal_facelib_handle_t handle, const hal_facelib_attr_t *facelib_attr) {
    #if 1
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);

    ctx->attr = *facelib_attr;
    #endif
	return 0;
}

int Cvi_person_Info(const hal_facelib_handle_t facelib_handle, int face_id, cvi_person_t *person)
{
    #if 1
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(facelib_handle);

    int ret = -1;
    ret =  cvi_hal_repo_get_person(&(ctx->db_repo), face_id, person);
    return ret;
    #endif
}

int CviFeatureMatching(const hal_facelib_handle_t handle, int8_t *feature,
                          uint32_t *k_index, float *k_value, float *buffer, const int k,
                          const float threshold) {
    #if 1
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);
    hal_db_feature_t *dbmeta = ctx->db_feature;
    FACELIB_FEATURE_MATCH(feature, dbmeta->raw, dbmeta->raw_unit, k_index,
                           k_value, buffer, NUM_FACE_FEATURE_DIM, dbmeta->num, k);
    for (int i = 0; i < k; i++) {
      if (k_value[i] < threshold) {
          k_index[i] = -1;
          //printf("cmp point1 = %f\n",k_value[i]);

          k_value[i] = 0;

      }
      else
      {
          printf("cmp point = %f\n",k_value[i]);
          return 0;
      }
    }
    #endif
    return -1;
}

//please use megvii api 'compare'
int HAL_FACE_Verify1v1(const hal_facelib_handle_t handle, const hal_face_feature_t *feature1,
        const hal_face_feature_t *feature2, int *match, float *score) {
    #if 1
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);
    FACELIB_COMPARE(feature1->ptr, feature2->ptr, NUM_FACE_FEATURE_DIM, score);
    if (*score >= ctx->attr.threshold_1v1) {
        *match = 1;
    } else {
        *match = 0;
    }
    #endif
    return 0;
}

//please use megvii api 'identifyFromFaceGroup'
int HAL_FACE_Identify1vN(const hal_facelib_handle_t handle, hal_face_t meta, float threshold) {
    #if 1
    int face_num = FACELIB_GET_FACE_NUM(meta);
    if (!face_num) {
        return -1;
    }
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);
    hal_db_feature_t *dbmeta = ctx->db_feature;
    if (dbmeta->num == 0) {
        printf("Error. DB is empty.\n");
        return -1;
    }
    for (int i = 0; i < face_num; ++i) {
        uint32_t index[1] = {0};
        float score[1] = {0};
        int face_id = 0;

        hal_face_feature_t feature;
        //if (!face_lib.get_feature(meta, i, &feature)) {
        if (!FACELIB_GET_FEATURE(meta, i, &feature)) {
            if (0 > CviFeatureMatching(handle, feature.ptr, index, score,
                                          ctx->db_feature->db_buffer, 1, threshold)) {
                continue;
            }
            if (static_cast<int32_t>(index[0]) != -1) {
                char * name = nullptr;
                size_t size = 0;
                //face_lib.get_name(meta, i, &name, &size);
                FACELIB_GET_NAME(meta, i, &name, &size);
                cvi_hal_repo_get_face_by_offset(&(ctx->db_repo), index[0], &face_id, name, size);
                printf("fd name %s\n", name);
                return face_id;
            }
        }
    }
    #endif
    return -1;
}

/*
static void set_person_info(cvi_person_t *person,int id,char *name,char *identifier,
                            char *serial,char *ic_card,char *image_path)
{
    #if 1
    memset(person, 0, sizeof(cvi_person_t));
    person->id = id;
    strncpy(person->name ,name, strlen(name));
    strncpy(person->identifier , identifier, strlen(identifier));
    strncpy(person->serial , serial,strlen(serial));
    strncpy(person->ic_card , ic_card,strlen(ic_card));
    strncpy(person->image_path, image_path,strlen(image_path));
    #endif
}
*/

int HAL_FACE_RepoAddIdentity_with_PersonInfo(hal_facelib_handle_t handle, hal_face_t face_info,const cvi_person_t person)
{
    #if 1
    //if (!face_lib.get_feature) {
    //    printf("function[get_feature] not loaded!");
    //    return -1;
    //}
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);

    char *name = (char *)person.name;

    hal_face_feature_t feature;
    //if (face_lib.get_feature(face_info, 0, &feature)) {
    if (FACELIB_GET_FEATURE(face_info, 0, &feature)) {
        return -1;
    }
    int face_id = cvi_hal_repo_add_face(&(ctx->db_repo),
            name, (unsigned char *)feature.ptr, feature.size);

    if (0 > face_id) {
        return -1;
    }

    int ret;
    ret = cvi_hal_repo_set_person(&(ctx->db_repo), face_id,&person);

    if(ret<0)
    {
        printf("set person info error\n");
        cvi_hal_repo_delete_face(&(ctx->db_repo),face_id);
        return -1;
    }
    #endif
    return 0;
}

int HAL_FACE_RepoAddIdentity(hal_facelib_handle_t handle, hal_face_t face_info, hal_face_id_t *id) {
    #if 1
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);
    //if (!face_lib.get_feature) {
    //    printf("function[get_feature] not loaded!");
    //    return -1;
    //}

    hal_face_feature_t feature;
    //if (face_lib.get_feature(face_info, 0, &feature)) {
    if (FACELIB_GET_FEATURE(face_info, 0, &feature)) {
        return -1;
    }

    char * name = nullptr;
    size_t size = 0;
    //face_lib.get_name(face_info, 0, &name, &size);
    FACELIB_GET_NAME(face_info, 0, &name, &size);

    int face_id = cvi_hal_repo_add_face(&(ctx->db_repo),
            name, (unsigned char *)feature.ptr, feature.size);

    if (0 > face_id) {
        return -1;
    }

    *id = face_id;
    #endif
    return 0;
}

int HAL_FACE_RepoGetIdentityName(const hal_facelib_handle_t handle, const hal_face_id_t id, hal_face_name_t *name) {
    #if 1
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);
    int8_t features[NUM_FACE_FEATURE_DIM];

    if (0 > cvi_hal_repo_find_face(&(ctx->db_repo), id, name->name, sizeof(name->name), (unsigned char*)features, sizeof(features))) {
        return -1;
    }
    #endif
    return 0;
}

int HAL_FACE_RepoGetIdentityNameFeature(const hal_facelib_handle_t handle, const hal_face_id_t id,
                                         hal_face_name_t *name, hal_face_feature_t *feature) {
    #if 1
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);
    if (0 > cvi_hal_repo_find_face(&(ctx->db_repo), id, name->name, sizeof(name->name), (unsigned char *)feature->ptr, sizeof(feature->size))) {
        return -1;
    }
    #endif
    return 0;
}

int HAL_FACE_RepoUpdateIdentity(const hal_facelib_handle_t handle, const hal_face_id_t id,
                                 const hal_face_name_t *name, const hal_face_feature_t *features) {
    #if 1
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);
    return cvi_hal_repo_update_face(&(ctx->db_repo), id,
                    name? name->name: NULL,
                    features? (unsigned char*)features->ptr: NULL,
                    features? features->size: 0);
    #endif
}

int HAL_FACE_RepoRemoveIdentity(const hal_facelib_handle_t handle, const hal_face_id_t id) {
    #if 1
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);

    return cvi_hal_repo_delete_face(&(ctx->db_repo), id);
    #endif
}

int HAL_FACE_RepoRemoveAllIdentities(const hal_facelib_handle_t handle) {
    #if 1
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);

    char path[128] = {0};
    snprintf(path, sizeof(path), "%s", ctx->db_repo.path);

    cvi_hal_repo_close(&(ctx->db_repo));
    unlink(path);

    return cvi_hal_repo_open(&(ctx->db_repo), path);
    #endif
}

int HAL_FACE_RepoShowList(const hal_facelib_handle_t handle)
{
    #if 0
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);

    std::vector<size_t> id_table;
    id_table = ctx->repo->id_list();

    cout << "repo size = " << id_table.size() << endl;
    for (int i=0; i < id_table.size() ; i++) {
        size_t id = id_table[i];
        cout << "id = " << id << " , name = " << ctx->repo->get_name(id).value_or("") << endl;
    }
    #endif
    return 0;
}

int HAL_FACE_GetRepoNum(hal_facelib_handle_t facelib_handle)
{
    #if 1
    hal_facelib_context_t *context = static_cast<hal_facelib_context_t*>(facelib_handle);

    hal_db_feature_t *dbmeta = context->db_feature;
    // Get from db function.
    uint8_t *from_db = nullptr;
    dbmeta->num = cvi_hal_repo_get_features(&(context->db_repo), 0, -1, &from_db);

    if(from_db != nullptr){
        free(from_db);
        from_db = nullptr;
    }

    return dbmeta->num;
    #endif
}

int  hal_face_Save(cv::Mat src_img, hal_face_t face, int face_idx, char *face_name)
{
    #if 1
    //if (!face_lib.rect_rescale) {
    //    printf("function[rect_rescale] not loaded!");
    //    return;
    //}
    hal_face_bbox_t rect;
    HAL_FACE_RectRescale(src_img.cols, src_img.rows, face, 0, true, &rect);
    int x1 = rect.x1;
    int y1 = rect.y1;
    int x2 = rect.x2;
    int y2 = rect.y2;

    // printf("x1 = %d, y1 = %d, x2 = %d, y2 = %d\n",x1,y1,x2,y2);

    int x = x1-5>0?x1-5:0;
    int y = y1-5>0?y1-5:0;
    int width = x2-x1+10;
    int height = y2-y1+10;

    if (!width || !height) {
        printf("face rect error[%d %d %d %d]\n", x, y, width, height);
        return -1;
    }

    if(x+width>src_img.cols)
        width = src_img.cols-x;
    if(y+height>src_img.rows)
        height = src_img.rows-y;
    // printf("colom = %d, row = %d, x = %d, y = %d, width = %d, height = %d\n",rgb_frame.cols, rgb_frame.rows,x,y,width,height);

    cv::Mat result_image(src_img, cv::Rect(x,y, width, height));

    cv::resize(result_image, result_image, cv::Size(width*150/height, 150));  // resize to popmenu image icon size


    if(face_name != NULL)
    {
        imwrite(face_name,result_image);
    }
    system("sync;sync");
    #endif
    printf("%s:%d\n",__FUNCTION__,__LINE__);
    return 0;
}

static int hal_face_getinfo(char *image_path, hal_face_t face)
{
    #if 1
    //if (!face_lib.get_face_info) {
    //    printf("function[get_face_info] not loaded!");
    //    return -1;
    //}
    int s32Ret = CVI_SUCCESS;
    //s32Ret = face_lib.get_face_info(image_path, face);
    s32Ret = FACELIB_GET_FACE_INFO(image_path, face);
    if (0 != s32Ret) {
        printf("hal_face_getinfo failed with:%#x\n", s32Ret);
    }
    return s32Ret;
    #endif
}

int HAL_FACE_Register(hal_facelib_handle_t facelib_handle, char *image_path,
                             cvi_person_t person)
{
    #if 1
    //if (!face_lib.free_face) {
    //    printf("function[free_face] not loaded!");
    //    return -1;
    //}
    int ret;
    //hal_face_t face = face_lib.create_face();
    hal_face_t face = FACELIB_CREATE_FACE();

    ret = hal_face_getinfo(image_path, face);
    if (ret != CVI_SUCCESS) {
        printf("hal_face_getinfo failed with %#x!\n", ret);
        //face_lib.free_face(face);
        FACELIB_FREE_FACE(face);
        return ret;
    }
    //int face_num = face_lib.get_face_num(face);
    int face_num = FACELIB_GET_FACE_NUM(face);
    if(face_num != 1) {
        if(face_num == 0) {
            std::cerr << "no face detect" << std::endl;
            return FAIL_NO_FACE;
        } else {
            std::cerr << "multiple face detect" << std::endl;
            return FAIL_MULTI_FACE;
        }
    }

    int person_num = HAL_FACE_GetRepoNum(facelib_handle);
    printf("total person num = %d\n",person_num);
    if(person_num != 0)
    {
        int matched_id = HAL_FACE_Identify1vN(facelib_handle, face, 0.6);
        if(matched_id >= 0) // there is matched person
        {
            printf("Person is already in repo, don't register again.\n");
            //face_lib.free_face(face);
            FACELIB_FREE_FACE(face);
            return FAIL_DUPLICATE;
        }
    }

	char face_img[64]={0};
    cv::Mat src_img = cv::imread(image_path);

    sprintf(face_img,"./repo/%s_%d.jpg",(char *)person.name, person_num);
    if (0 != hal_face_Save(src_img, face, 0, face_img)) {
        printf("hal_face_save failed\n");
        return FAIL_NO_FACE;
    }
    memset(person.image_path,0,sizeof(person.image_path));
    strncpy(person.image_path, face_img,strlen(face_img));

    int tmp = 20;
    char identifier[50] = "\0";
    sprintf(identifier, "%d", tmp);

    char ic_card[50] = "\0";
    sprintf(ic_card, "%d", tmp);

    strncpy(person.identifier, identifier, strlen(identifier));
    strncpy(person.ic_card, ic_card, strlen(ic_card));

    if(-1 != HAL_FACE_RepoAddIdentity_with_PersonInfo(facelib_handle, face, person))
    {
        CviLoadIdentify(facelib_handle);// reload data to memory
    }
    //face_lib.free_face(face);
    FACELIB_FREE_FACE(face);
    return ret ;
    #endif
}

void HAL_FACE_Encode(VIDEO_FRAME_INFO_S *stOutFrame, hal_face_t face,char *capture_img,char *recored_img)
{
    #if 1
    cv::Mat rgb_frame;
    size_t length = 0;
    cv::Rect face_rect;
    if (face != NULL) {
        hal_face_bbox_t rect;
        HAL_FACE_RectRescale(stOutFrame->stVFrame.u32Width, stOutFrame->stVFrame.u32Height, face, 0, true, &rect);

        int x1 = rect.x1;
        int y1 = rect.y1;
        int x2 = rect.x2;
        int y2 = rect.y2;

        // printf("x1 = %d, y1 = %d, x2 = %d, y2 = %d\n",x1,y1,x2,y2);

        int x = x1-5>0?x1-5:0;
        int y = y1-5>0?y1-5:0;
        int width = x2-x1+10;
        int height = y2-y1+10;

        int frame_w = stOutFrame->stVFrame.u32Width;
        int frame_h = stOutFrame->stVFrame.u32Height;

        if (x + width > frame_w)
            width = frame_w - x;
        if (y + height > frame_h)
            height = frame_h - y;
        // printf("colom = %d, row = %d, x = %d, y = %d, width = %d, height = %d\n",rgb_frame.cols, rgb_frame.rows,x,y,width,height);

        face_rect = cv::Rect(x,y, width, height);
    }
    bool need_mmap = NULL == stOutFrame->stVFrame.pu8VirAddr[0] ? true : false;
    if (stOutFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_RGB_888_PLANAR) {
        if (need_mmap) {
            for (int i = 0; i < 3; ++i) {
                length += stOutFrame->stVFrame.u32Length[i];
            }
            stOutFrame->stVFrame.pu8VirAddr[0] = (CVI_U8 *)CVI_SYS_MmapCache(stOutFrame->stVFrame.u64PhyAddr[0],
                                                                             length);
            stOutFrame->stVFrame.pu8VirAddr[1] = stOutFrame->stVFrame.pu8VirAddr[0] + stOutFrame->stVFrame.u32Length[0];
            stOutFrame->stVFrame.pu8VirAddr[2] = stOutFrame->stVFrame.pu8VirAddr[1] + stOutFrame->stVFrame.u32Length[1];
        }
        std::vector<cv::Mat> mats(3, cv::Mat());
        for (int i = 0; i < 3; ++i) {
            mats[i] = cv::Mat(stOutFrame->stVFrame.u32Height,
                                stOutFrame->stVFrame.u32Width,
                                CV_8UC1,
                                stOutFrame->stVFrame.pu8VirAddr[2 - i],
                                stOutFrame->stVFrame.u32Stride[2 - i]);
        }
        if (face != NULL) {
            std::vector<cv::Mat> tmp_mats(3, cv::Mat());
            for (int i = 0; i < 3; ++i) {
                tmp_mats[i] = mats[i](face_rect);
            }
            cv::Mat result_image;
            cv::merge(tmp_mats, result_image);
            if (capture_img != NULL) {
                imwrite(capture_img, result_image);
            }
            // cv::resize(result_image,result_image,Size(300,300), CV_INTER_LINEAR);
            if (recored_img != NULL) {
                imwrite(recored_img, result_image);
            }
        }
        else
        {
            cv::merge(mats, rgb_frame);
            imwrite(capture_img, rgb_frame);
        }
    } else if (stOutFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_BGR_888) {
        if (need_mmap) {
            length = stOutFrame->stVFrame.u32Length[0];
            stOutFrame->stVFrame.pu8VirAddr[0] = (CVI_U8 *)CVI_SYS_MmapCache(stOutFrame->stVFrame.u64PhyAddr[0],
                                                                             length);
        }
        rgb_frame = cv::Mat(stOutFrame->stVFrame.u32Height,
                            stOutFrame->stVFrame.u32Width,
                            CV_8UC3,
                            stOutFrame->stVFrame.pu8VirAddr[0],
                            stOutFrame->stVFrame.u32Stride[0]);
        cv::Mat result_image = rgb_frame(face_rect);
        if (face != NULL) {
            if (capture_img != NULL) {
                imwrite(capture_img, result_image);
            }
            // cv::resize(result_image,result_image,Size(300,300), CV_INTER_LINEAR);
            if (recored_img != NULL) {
                imwrite(recored_img, result_image);
            }
        } else {
            imwrite(capture_img, rgb_frame);
        }
    } else {
        printf("error format, face encode failed\n");
        return;
    }

    if (need_mmap) {
        CVI_SYS_Munmap((void *)stOutFrame->stVFrame.pu8VirAddr[0], length);
    }
    #endif
}

int Cvi_Jpg2PNG(char *src_jpg,char *dst_png,int *width,int *height)
{
    #if 1
    cv::Mat face = imread(src_jpg,1);
    int s32Ret = CVI_SUCCESS;
    if(face.data == NULL)
    {
        printf("invalid file\n");
        return CVI_FAILURE;
    }
    imwrite(dst_png,face);
    *width = face.cols;
    *height = face.rows;
    return s32Ret;
    #endif
}

int CviGetPersonList(const hal_facelib_handle_t facelib_handle, cvi_person_t **person_list)
{
    #if 1
    hal_facelib_context_t *context = static_cast<hal_facelib_context_t*>(facelib_handle);
    return cvi_hal_repo_get_person_list(&(context->db_repo), 0, -1, person_list);
    #endif
}

int CviSetPerson(const hal_facelib_handle_t facelib_handle, cvi_person_t *person)
{
    #if 1
    hal_facelib_context_t *context = static_cast<hal_facelib_context_t*>(facelib_handle);
    return cvi_hal_repo_set_person(&(context->db_repo), person->id, person);
    #endif
}

void CviSaveFrame(VIDEO_FRAME_INFO_S *frm, const char * name) {
    #if 1
    if (frm->stVFrame.pu8VirAddr[0]) {
        cv::Mat rgb_frame = cv::Mat(frm->stVFrame.u32Height,
                            frm->stVFrame.u32Width,
                            CV_8UC3,
                            frm->stVFrame.pu8VirAddr[0],
                            frm->stVFrame.u32Stride[0]);
        cv::imwrite(name, rgb_frame);
    }
    #endif
}

void CviGetVPSSChnBgrFrame(void)
{
    int s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame;

	//RGB
	memset(&stVideoFrame, 0, sizeof(VIDEO_FRAME_INFO_S));
	s32Ret = CVI_VPSS_GetChnFrame(0, 0, &stVideoFrame, 1000);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VPSS_GetChnFrame failed with %#x\n", s32Ret);
	}
	stVideoFrame.stVFrame.pu8VirAddr[0] = (CVI_U8*)CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], stVideoFrame.stVFrame.u32Length[0]);
	memcpy(Rgb_Sensor_bgr_Buf, stVideoFrame.stVFrame.pu8VirAddr[0], stVideoFrame.stVFrame.u32Length[0]);
	CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[0], stVideoFrame.stVFrame.u32Length[0]);
	if (CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame) != 0) {
		printf("CVI_VPSS_ReleaseChnFrame NG\n");
	}

	//IR
	memset(&stVideoFrame, 0, sizeof(VIDEO_FRAME_INFO_S));
	s32Ret = CVI_VPSS_GetChnFrame(1, 0, &stVideoFrame, 1000);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VPSS_GetChnFrame failed with %#x\n", s32Ret);
	}
	stVideoFrame.stVFrame.pu8VirAddr[0] = (CVI_U8*)CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], stVideoFrame.stVFrame.u32Length[0]);
	memcpy(Ir_Sensor_bgr_Buf, stVideoFrame.stVFrame.pu8VirAddr[0], stVideoFrame.stVFrame.u32Length[0]);
	CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[0], stVideoFrame.stVFrame.u32Length[0]);
	if (CVI_VPSS_ReleaseChnFrame(1, 0, &stVideoFrame) != 0) {
		printf("CVI_VPSS_ReleaseChnFrame NG\n");
	}
}

/***************************************************sensor calibrate***********************************************/

static int doCalibrationVerify(vector<Point2f> &pointbuf_rgb, vector<Point2f> &pointbuf_ir){
    if (pointbuf_rgb.size() != pointbuf_ir.size()) {
        printf("doCalibrationVerify:input size mismatch (%d vs %d)\n", (int)(pointbuf_rgb.size()), (int)(pointbuf_ir.size()));
        return CALIBRATION_IN_PROGRESS;
    }
    if (pointbuf_rgb.size() <= 1) {
        printf("input size should large than 1\n" );
        return CALIBRATION_IN_PROGRESS;
    }

    g_calibration_state = CALIBRATION_IN_PROGRESS;

    vector<float>  x_rgb_calibration;
    vector<float>  y_rgb_calibration;
    vector<float>  x_ir;
    vector<float>  y_ir;
    for(auto &item : pointbuf_rgb) {
        x_rgb_calibration.push_back(item.x * g_left_right_k + g_left_right_b);
        y_rgb_calibration.push_back(item.y * g_top_bottom_k + g_top_bottom_b);
    }
    for(auto &item: pointbuf_ir) {
        x_ir.push_back(item.x);
        y_ir.push_back(item.y);
    }

    float x_deviation_sum = 0.0;
    float y_deviation_sum = 0.0;
    int point_num = x_ir.size();
    for(int i = 0; i < point_num; ++i) {
        x_deviation_sum += pow(x_rgb_calibration[i] - x_ir[i], 2);
        y_deviation_sum += pow(y_rgb_calibration[i] - y_ir[i], 2);
    }
    float x_standard_deviation = sqrt(x_deviation_sum / point_num);
    float y_standard_deviation = sqrt(y_deviation_sum / point_num);

    printf(" x_standard_deviation: %.2f, y_standard_deviation: %.2f\n", x_standard_deviation, y_standard_deviation);

    if (x_standard_deviation < 8.0 && y_standard_deviation < 8.0) {
        g_calibration_state = CALIBRATION_FINISHED;
        printf("calibration state changed:  CALIBRATION_IN_PROGRESS  >>>>  CALIBRATION_FINISHED\n");
    } else {
        g_calibration_state = CALIBRATION_NONE;
        printf("reset calibration to CALIBRATION_NONE >>>>\n");
    }
    return  g_calibration_state;
}

static int  findCornersPoints(Mat &cvimg, Size board_size, vector<Point2f> &pointbuf){

    bool found = findChessboardCorners(cvimg, board_size, pointbuf, CALIB_CB_FAST_CHECK);
    const size_t checkerNum = board_size.width * board_size.height;
    if (!found || (pointbuf.size() < checkerNum)) {
        printf("found Chessboard Corners error, point buffer size:%d !!\n", (int)(pointbuf.size()));
        return -1;
    }

    return 0;
}

/* use ordinary least squares(ols) for linear regression */
static void linearRegressionOLS(const std::vector<float>& x,
        const std::vector<float>& y, float &k, float &b){
    if (x.size() != y.size()) {
        printf("linearRegressionOLS:input size mismatch (%d vs %d)\n", (int)(x.size()), (int)(y.size()));
        return;
    }
    if (x.size() <= 1) {
        printf("input size should large than 1\n");
        return;
    }

    float y_mean = 0.0;
    float x_mean = 0.0;
    for (size_t i = 0; i < x.size(); ++i) {
        x_mean += x[i];
        y_mean += y[i];
    }
    x_mean /= static_cast<float>(x.size());
    y_mean /= static_cast<float>(x.size());
    float A_fractions = 0;
    float A_denominator = 0;
    for (size_t i = 0; i < x.size(); ++i) {
        A_fractions += (x[i] * y[i]);
        A_denominator += pow(x[i], 2);
    }
    A_fractions -= (static_cast<float>(x.size()) * x_mean * y_mean);
    A_denominator -= (static_cast<float>(x.size()) * pow(x_mean, 2));
    k = A_fractions / A_denominator;
    b = y_mean - k * x_mean;

}

static int linearRegression(vector<Point2f> &pointbuf_rgb, vector<Point2f> &pointbuf_ir) {
    if (pointbuf_rgb.size() != pointbuf_ir.size()) {
        printf("linearRegression:input size mismatch (%d vs %d)\n", (int)(pointbuf_rgb.size()), (int)(pointbuf_ir.size()));
        return CALIBRATION_NONE;
    }
    if (pointbuf_rgb.size() <= 1) {
        printf("input size should large than 1\n");
        return CALIBRATION_NONE;
    }

    vector<float>  x_rgb;
    vector<float>  y_rgb;
    vector<float>  x_ir;
    vector<float>  y_ir;

    for(auto &item : pointbuf_rgb) {
        x_rgb.push_back(item.x);
        y_rgb.push_back(item.y);
    }

    for(auto &item : pointbuf_ir) {
        x_ir.push_back(item.x);
        y_ir.push_back(item.y);
    }

    linearRegressionOLS(x_rgb, x_ir, g_left_right_k, g_left_right_b);
    linearRegressionOLS(y_rgb, y_ir, g_top_bottom_k, g_top_bottom_b);

    g_calibration_state = CALIBRATION_IN_PROGRESS;
    printf("calibration state changed:  CALIBRATION_NONE>>>>CALIBRATION_IN_PROGRESS\n");
    printf("(left_right_k, left_right_b, top_bottom_k, top_bottom_b): (%.2f, %.2f, %.2f, %.2f)\n",
            g_left_right_k, g_left_right_b, g_top_bottom_k, g_top_bottom_b);

    return g_calibration_state;
}

int HAL_FACE_LibSensorCalibrate(void)
{
    const int width = 576;
    const int height = 1024;

    CviGetVPSSChnBgrFrame();

    //highe, width
    cv::Mat bgrImg(height, width, CV_8UC3);
    cv::Mat irImg(height, width, CV_8UC3);

    memcpy(bgrImg.data, Rgb_Sensor_bgr_Buf, height*width*3);
    memcpy(irImg.data, Ir_Sensor_bgr_Buf, height*width*3);

    // cv::imwrite("bgr.bmp", bgrImg);
    // cv::imwrite("ir.bmp", irImg);

    if(bgrImg.empty()){
        printf("bgr_image is empty!\n");
        return -1;
    }
    if(irImg.empty()){
        printf("ir_image is empty!\n");
        return -1;
    }
    vector<Point2f> points_rgb;
    vector<Point2f> points_ir;
    Size board_size;
    board_size.width = 9;
    board_size.height = 6;
    resetRgbirCalibration();

    findCornersPoints(bgrImg, board_size, points_rgb);
    printf("bgrImg corner size = %d\n", points_rgb.size());

    findCornersPoints(irImg, board_size, points_ir);
    printf("irImg corner size = %d\n", points_ir.size());


    if (g_calibration_state == CALIBRATION_NONE) {
        g_calibration_state = linearRegression(points_rgb, points_ir);
    }

    if (g_calibration_state == CALIBRATION_IN_PROGRESS) {
        g_calibration_state = doCalibrationVerify(points_rgb, points_ir);
    }

    return 0;
}