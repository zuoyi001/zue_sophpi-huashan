#include <memory>
#include <vector>
#include <string>
#include <unistd.h>
#include <iostream>

#include "cvi_buffer.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vi.h"
#include "cvi_vpss.h"
#include "cviai_core.h"
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include "cvi_hal_db_repo.h"
#include <assert.h>

#include "cvi_hal_face_api.h"
#include "cviai.h"
#include "cviai_media.h"
#include "cviai_service_types.h"
#include "cvai_face_types.h"
#include "cviai_service.h"

//#include "cvi_feature_matching/cached_repo.hpp"
//#include "face_wrapper_proxy.h"


using namespace std;
using namespace cv;

static cviai_handle_t aisdk_handle = NULL; 
static cviai_service_handle_t service_handle = NULL;


#define AI_SDK_USE_VPSS_GRP 0

/*
int HAL_FACE_InsertFaceGroup(hal_facelib_handle_t facelib_handle��int count)
{
    cvai_service_feature_array_t add_array;
    memset(&add_array, 0, sizeof(cvai_service_feature_array_t));
    cvai_service_feature_array_t add_array;
    memcpy(&add_array,featureArray,sizeof(cvai_service_feature_array_t));

    int ret = CVI_AI_Service_RegisterFeatureArray(service_handle,add_array,COS_SIMILARITY);
    //CVI_S32 CVI_AI_Service_RegisterFeatureArray(cviai_service_handle_t handle, const 
    //cvai_service_feature_array_t featureArray, const cvai_service_feature_matching_emethod);
    return ret;
}
*/

int HAL_FACE_LibOpen(const hal_facelib_config_t *facelib_config)
{
    int ret = 0;
    if (aisdk_handle) {
        printf("handle already created\n");
        return ret;
    }
    // ret = CVI_AI_CreateHandle(&aisdk_handle);
    ret = CVI_AI_CreateHandle2(&aisdk_handle, AI_SDK_USE_VPSS_GRP, 0);
    if (ret) {
        printf("CVI_AI_CreateHandle failed, ret:%x\n", ret);
        return ret;
    }
    printf("AiSdk create handle\n");
    if (facelib_config->model_face_fd != NULL) {
        if (CVI_AI_OpenModel(aisdk_handle, CVI_AI_SUPPORTED_MODEL_RETINAFACE,
            facelib_config->model_face_fd)) {
                printf("CVI_AI_OpenModel failed\n");
                return -1;
        }
        CVI_AI_SetSkipVpssPreprocess(aisdk_handle, CVI_AI_SUPPORTED_MODEL_RETINAFACE, false);
        printf("init_network_retina done! cvimodel:%s\n", facelib_config->model_face_fd);
    }

    if (facelib_config->is_face_quality) {
        if (CVI_AI_OpenModel(aisdk_handle, CVI_AI_SUPPORTED_MODEL_FACEQUALITY,
            facelib_config->model_face_quality)) {
                printf("CVI_AI_OpenModel failed\n");
                return -1;
        }
        printf("init_face_quality done! cvimodel:%s\n", facelib_config->model_face_quality);
    }

    if (facelib_config->model_face_extr) {

        if (CVI_AI_OpenModel(aisdk_handle, CVI_AI_SUPPORTED_MODEL_FACERECOGNITION,
            facelib_config->model_face_extr)) {
                printf("CVI_AI_OpenModel failed\n");
                return -1;
        }
        CVI_AI_SetSkipVpssPreprocess(aisdk_handle, CVI_AI_SUPPORTED_MODEL_FACERECOGNITION, false);
        //CVI_AI_SetSkipVpssPreprocess(aisdk_handle, CVI_AI_SUPPORTED_MODEL_FACEATTRIBUTE, false);
        printf("init_network_face_attribute done! cvimodel:%s\n", facelib_config->model_face_extr);
    }

    if (facelib_config->config_liveness && (facelib_config->model_face_liveness != NULL))
    {
        if (CVI_AI_OpenModel(aisdk_handle, CVI_AI_SUPPORTED_MODEL_LIVENESS,
            facelib_config->model_face_liveness)) {
                printf("CVI_AI_OpenModel failed\n");
                return -1;
        }
        CVI_AI_SetSkipVpssPreprocess(aisdk_handle, CVI_AI_SUPPORTED_MODEL_LIVENESS, false);
        printf("init_network_liveness done! cvimodel:%s\n", facelib_config->model_face_liveness);
    }

    if (facelib_config->config_yolo && (facelib_config->model_yolo3 != NULL))
    {
        if (CVI_AI_OpenModel(aisdk_handle, CVI_AI_SUPPORTED_MODEL_YOLOV3,
            facelib_config->model_yolo3)) {
                printf("CVI_AI_OpenModel failed\n");
                return -1;
        }
        printf("init_network_yolov3 done\n");
    }

    CVI_AI_Service_CreateHandle(&service_handle, aisdk_handle);

    printf("aisdk_handle = %p, service_handle = 0x%p.\n", aisdk_handle, service_handle);
    return 0;
}

int HAL_FACE_LibClose(void)
{
    CVI_AI_DestroyHandle(aisdk_handle);
    aisdk_handle = NULL;
    printf("CviFaceLibClose\n");
    return 0;
}

int HAL_FACE_Detect(VIDEO_FRAME_INFO_S *stfdFrame, hal_face_t faces, float min_face_h) {
    cvai_face_t *_faces = reinterpret_cast<cvai_face_t *>(faces);
    int ret = CVI_AI_RetinaFace(aisdk_handle, stfdFrame, _faces);
    if (0 != ret) {
        printf("CVI_AI_RetinaFace failed ret:%d\n", ret);
    }
    for (uint32_t i = 0; i < _faces->size;) {
        if (_faces->info[i].bbox.y2 - _faces->info[i].bbox.y1 < min_face_h) {
            HAL_FACE_DelFace(faces, i);
        } else {
            ++i;
        }
    }
    return ret;
}

int HAL_FACE_LivenessDetect(VIDEO_FRAME_INFO_S *rgbFrame,VIDEO_FRAME_INFO_S *irFrame,
         hal_face_t faces) {
    cvai_face_t *_faces = reinterpret_cast<cvai_face_t *>(faces);
    int ret = 0;
    //CVI_SYS_TraceBegin("Retina IR");
    cvai_face_t ir_face = {0};
    ret = CVI_AI_RetinaFace(aisdk_handle, irFrame, &ir_face);
    if (ret != 0) {
        printf("CVI_AI_RetinaFace failed ret:%d\n", ret);
        return ret;
    }

    #if 1
    if (ir_face.size <= 0) {
        _faces->info[0].liveness_score = -2.0;
        CVI_AI_Free(&ir_face);
        return -1;
    }
    //CVI_SYS_TraceEnd();
    //CVI_SYS_TraceBegin("liveness");
    ret = CVI_AI_Liveness(aisdk_handle, rgbFrame, irFrame, _faces, &ir_face);
    if (ret != 0) {
        printf("CVI_AI_Liveness failed, ret:%d\n", ret);
    }
    #else
    _faces->info[0].liveness_score = 0.99;
    #endif

    CVI_AI_Free(&ir_face);
    //CVI_SYS_TraceEnd();
    return ret;
}

int HAL_FACE_LivenessDetectBgr(VIDEO_FRAME_INFO_S *rgbFrame, hal_face_t faces) {
    int ret = 0;
    HAL_FACE_LivenessDetect(rgbFrame, rgbFrame, faces);
    return ret;
}

int HAL_FACE_Recognize(VIDEO_FRAME_INFO_S *stfrFrame, hal_face_t faces) {
    cvai_face_t *_faces = reinterpret_cast<cvai_face_t *>(faces);
    int ret = CVI_AI_FaceRecognition(aisdk_handle, stfrFrame, _faces);
    if (0 != ret) {
        printf("CVI_AI_FaceRecognition failed ret:%d\n", ret);
    }
    return ret;
}

int HAL_FACE_RectRescale(float width, float height, hal_face_t faces, int idx, bool keep_aspect, hal_face_bbox_t *box) {
    float ratio_x, ratio_y, bbox_y_height, bbox_x_height, bbox_padding_top, bbox_padding_left;
    cvai_face_t *_faces = reinterpret_cast<cvai_face_t *>(faces);

    if (width >= height) {
        ratio_x          = width / _faces->width;
        bbox_y_height    = _faces->height * height / width;
        ratio_y          = height / bbox_y_height;
        bbox_padding_top = (_faces->height - bbox_y_height) / 2;
    } else {
        ratio_y           = height / _faces->height;
        bbox_x_height     = _faces->width * width / height;
        ratio_x           = width / bbox_x_height;
        bbox_padding_left = (_faces->width - bbox_x_height) / 2;
    }
    cvai_bbox_t& bbox = _faces->info[idx].bbox;
    float x1, x2, y1, y2;

    if (width >= height) {
        x1 = bbox.x1 * ratio_x;
        x2 = bbox.x2 * ratio_x;
        y1 = (bbox.y1 - bbox_padding_top) * ratio_y;
        y2 = (bbox.y2 - bbox_padding_top) * ratio_y;
    } else {
        x1 = (bbox.x1 - bbox_padding_left) * ratio_x;
        x2 = (bbox.x2 - bbox_padding_left) * ratio_x;
        y1 = bbox.y1 * ratio_y;
        y2 = bbox.y2 * ratio_y;
    }

    box->x1 = std::max(std::min(x1, width - 1), (float)0);
    box->x2 = std::max(std::min(x2, width - 1), (float)0);
    box->y1 = std::max(std::min(y1, height - 1), (float)0);
    box->y2 = std::max(std::min(y2, height - 1), (float)0);
    return 0;
}

int HAL_FACE_Create(hal_face_t *faces) {
    cvai_face_t *_faces = reinterpret_cast<cvai_face_t *>(
        malloc(sizeof(cvai_face_t)));
    if (!_faces) {
        return -1;
    }
    memset(_faces, 0x00, sizeof(cvai_face_t));
    *faces = reinterpret_cast<hal_face_t>(_faces);
    return 0;
}

int HAL_FACE_Free(hal_face_t faces) {
    cvai_face_t *_faces = reinterpret_cast<cvai_face_t *>(faces);
    CVI_AI_Free(_faces);
    free(_faces);
    return 0;
}

int HAL_FACE_Clear(hal_face_t faces) {
    cvai_face_t *_faces = reinterpret_cast<cvai_face_t *>(faces);
    CVI_AI_Free(_faces);
    memset(_faces, 0x00, sizeof(cvai_face_t));
    return 0;
}

int HAL_FACE_DelFace(hal_face_t faces, int idx) {
    cvai_face_t *_faces = reinterpret_cast<cvai_face_t *>(faces);
    if ((uint32_t)idx >= _faces->size) {
        printf("idx[%d] out of range, face num[%d]\n", idx, _faces->size);
        return -1;
    }
    int last_idx = _faces->size - 1;
    if (idx != last_idx) {
        cvai_face_info_t tmp;
        tmp = _faces->info[idx];
        _faces->info[idx] = _faces->info[last_idx];
        _faces->info[last_idx] = tmp;
    }
    _faces->size -= 1;
    CVI_AI_Free(&_faces->info[last_idx]);
    return 0;
}

int HAL_FACE_GetNum(hal_face_t faces) {
    cvai_face_t *_faces = reinterpret_cast<cvai_face_t *>(faces);
    return _faces->size;
}

int HAL_FACE_GetFaceRect(hal_face_t faces, int idx, hal_face_bbox_t *rect) {
    cvai_face_t *_faces = reinterpret_cast<cvai_face_t *>(faces);
    if ((uint32_t)idx >= _faces->size) {
        printf("idx[%d] out of range, face num[%d]\n", idx, _faces->size);
        return -1;
    }
    rect->x1 = _faces->info[idx].bbox.x1;
    rect->x2 = _faces->info[idx].bbox.x2;
    rect->y1 = _faces->info[idx].bbox.y1;
    rect->y2 = _faces->info[idx].bbox.y2;
    return 0;
}

int HAL_FACE_GetImageSize(hal_face_t faces, int *width, int *height) {
    cvai_face_t *_faces = reinterpret_cast<cvai_face_t *>(faces);
    *width = _faces->width;
    *height = _faces->height;
    return 0;
}

int HAL_FACE_GetFeature(hal_face_t faces, int idx, hal_face_feature_t *feature) {
    cvai_face_t *_faces = reinterpret_cast<cvai_face_t *>(faces);
    if ((uint32_t)idx >= _faces->size) {
        printf("idx[%d] out of range, face num[%d]\n", idx, _faces->size);
        return -1;
    }
    feature->ptr = _faces->info[idx].feature.ptr;
    feature->size = _faces->info[idx].feature.size;
    switch (_faces->info[idx].feature.type) {
        case TYPE_INT8:
            feature->type = FT_INT8;
            break;
        case TYPE_UINT8:
            feature->type = FT_UINT8;
            break;
        case TYPE_INT16:
            feature->type = FT_INT16;
            break;
        case TYPE_UINT16:
            feature->type = FT_UINT16;
            break;
        case TYPE_INT32:
            feature->type = FT_INT32;
            break;
        case TYPE_UINT32:
            feature->type = FT_UINT32;
            break;
        case TYPE_BF16:
            feature->type = FT_BF16;
            break;
        case TYPE_FLOAT:
            feature->type = FT_FLOAT;
            break;
        default :
            printf("undefined feature type:%d\n", _faces->info[idx].feature.type);
            return -1;
    }
    return 0;
}

int HAL_FACE_GetLivenessScore(hal_face_t faces, int idx, float *score) {
    cvai_face_t *_faces = reinterpret_cast<cvai_face_t *>(faces);
    if ((uint32_t)idx >= _faces->size) {
        printf("idx[%d] out of range, face num[%d]\n", idx, _faces->size);
        return -1;
    }
    *score = _faces->info[idx].liveness_score;
    return 0;
}

int HAL_FACE_LibRepoOpen(const hal_facelib_config_t *facelib_config,hal_facelib_handle_t *handle)
{
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
}

static int CviImportFeatureGroup(int8_t *feature_ptr, int feature_dim, int feature_type, int feature_num) {
    cvai_service_feature_array_t feature_array;
    int ret = CVI_SUCCESS;

    //printf("feature_dim = %d, feature_num = %d, feature_type = %d .\n",feature_dim, feature_num, feature_type);
    feature_array.ptr = feature_ptr;
    feature_array.feature_length  = feature_dim;
    feature_array.data_num = feature_num;
    feature_array.type = feature_type_e(feature_type);

    ret = CVI_AI_Service_RegisterFeatureArray(service_handle, feature_array, COS_SIMILARITY);
    printf("CVI_AI_Service_RegisterFeatureArray, ret = 0x%x.\n", ret);
    if(ret != CVI_SUCCESS) {
        return -1;
    }
        
    return 0;
}

static int CviFreeFeatureGroup() {
    return 0;
}

static int ResetVerify(hal_facelib_handle_t handle)
{
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
    CviFreeFeatureGroup();
    return 0;
}

int HAL_FACE_LibRepoClose(hal_facelib_handle_t handle)
{

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
    return 0;
}



int CviLoadIdentify(hal_facelib_handle_t handle)
{
    hal_facelib_context_t *context = static_cast<hal_facelib_context_t*>(handle);
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
    
    CviImportFeatureGroup(dbmeta->raw, NUM_FACE_FEATURE_DIM, 0, dbmeta->num);
/*
    if (context->db_feature->raw != nullptr ) {
        free(context->db_feature->raw);
        context->db_feature->raw = nullptr;
    }
*/
    return 0;
}

int HAL_GetFaceLibAttr(const hal_facelib_handle_t handle, hal_facelib_attr_t *facelib_attr) {
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);

    *facelib_attr = ctx->attr;
	return 0;
}

int HAL_UpdateFaceLibAttr(const hal_facelib_handle_t handle, const hal_facelib_attr_t *facelib_attr) {
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);

    ctx->attr = *facelib_attr;
	return 0;
}


int Cvi_person_Info(const hal_facelib_handle_t facelib_handle, int face_id, cvi_person_t *person)
{
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(facelib_handle);

    int ret = -1;
    ret =  cvi_hal_repo_get_person(&(ctx->db_repo), face_id, person);
    return ret;
}

int HAL_FACE_Verify1v1(const hal_facelib_handle_t handle, const hal_face_feature_t *feature1,
        const hal_face_feature_t *feature2, int *match, float *score) {
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);
    int8_t *ptr1 = feature1->ptr;
    int8_t *ptr2 = feature2->ptr;
    int32_t value1 = 0, value2 = 0, value3 = 0;
    for (uint32_t i = 0; i < NUM_FACE_FEATURE_DIM; i++) {
        value1 += (short)ptr1[i] * ptr2[i];
        value2 += (short)ptr1[i] * ptr2[i];
        value3 += (short)ptr1[i] * ptr2[i];
    }
    *score = (float)value3 / (sqrt(value1) * sqrt(value2));
    if (*score >= ctx->attr.threshold_1v1) {
        *match = 1;
    } else {
        *match = 0;
    }
    return 0;
}

int FACELIB_GET_NAME(hal_face_t faces, int idx, char **name, size_t *size) {
    cvai_face_t *_faces = reinterpret_cast<cvai_face_t *>(faces);
    if ((uint32_t)idx >= _faces->size) {
        printf("idx[%d] out of range, face num[%d]\n", idx, _faces->size);
        return -1;
    }
    *name = _faces->info[idx].name;
    *size = sizeof(_faces->info[idx].name);
    return 0;
}

int HAL_FACE_Identify1vN(const hal_facelib_handle_t handle, hal_face_t meta, float threshold) {
    cvai_face_t *_faces = reinterpret_cast<cvai_face_t *>(meta);

    int face_num = HAL_FACE_GetNum(meta);
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
        uint32_t k_index[1] = {0};
        float k_value;
        int face_id = 0;

        cvai_face_info_t tmp;
        uint32_t matchedNum = 0;
        tmp = _faces->info[i];
        CVI_AI_Service_FaceInfoMatching(service_handle, &tmp, 1, threshold, k_index, &k_value, &matchedNum);
        if (matchedNum == 0)
            continue;
        if (static_cast<int32_t>(k_index[0]) != -1) {
            char * name = nullptr;
            size_t size = 0;
            FACELIB_GET_NAME(meta, i, &name, &size);
            cvi_hal_repo_get_face_by_offset(&(ctx->db_repo), k_index[0], &face_id, name, size);
            printf("fd name %s\n", name);
            return face_id;
        }
    }

    return -1;
}

#if 0
static void set_person_info(cvi_person_t *person,int id,char *name,char *identifier,
                            char *serial,char *ic_card,char *image_path)
{
    memset(person, 0, sizeof(cvi_person_t));
    person->id = id;
    strncpy(person->name ,name, strlen(name));
    strncpy(person->identifier , identifier, strlen(identifier));
    strncpy(person->serial , serial,strlen(serial));
    strncpy(person->ic_card , ic_card,strlen(ic_card));
    strncpy(person->image_path, image_path,strlen(image_path));
}
#endif

int CviFaceRepoAddIdentity_with_PersonInfo(hal_facelib_handle_t handle, hal_face_t face_info,const cvi_person_t person)
{
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);
    //hal_db_feature_t *dbmeta = ctx->db_feature;

    char *name = (char *)person.name;

    hal_face_feature_t feature;
    if (HAL_FACE_GetFeature(face_info, 0, &feature)) {
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

    return 0;
}

int HAL_FACE_RepoAddIdentity(hal_facelib_handle_t handle, hal_face_t face_info, hal_face_id_t *id) {
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);

    hal_face_feature_t feature;
    if (HAL_FACE_GetFeature(face_info, 0, &feature)) {
        return -1;
    }

    char * name = nullptr;
    size_t size = 0;
    FACELIB_GET_NAME(face_info, 0, &name, &size);

    int face_id = cvi_hal_repo_add_face(&(ctx->db_repo),
            name, (unsigned char *)feature.ptr, feature.size);

    if (0 > face_id) {
        return -1;
    }

    *id = face_id;

    return 0;
}

int HAL_FACE_RepoGetIdentityName(const hal_facelib_handle_t handle, const hal_face_id_t id, hal_face_name_t *name) {
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);
    int8_t features[NUM_FACE_FEATURE_DIM];

    if (0 > cvi_hal_repo_find_face(&(ctx->db_repo), id, name->name, sizeof(name->name), (unsigned char*)features, sizeof(features))) {
        return -1;
    }
    return 0;
}

int HAL_FACE_RepoGetIdentityNameFeature(const hal_facelib_handle_t handle, const hal_face_id_t id,
                                         hal_face_name_t *name, hal_face_feature_t *feature) {

    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);
    if (0 > cvi_hal_repo_find_face(&(ctx->db_repo), id, name->name, sizeof(name->name), (unsigned char *)feature->ptr, sizeof(feature->size))) {
        return -1;
    }

    return 0;
}

int HAL_FACE_RepoUpdateIdentity(const hal_facelib_handle_t handle, const hal_face_id_t id,
                                 const hal_face_name_t *name, const hal_face_feature_t *features) {
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);
    return cvi_hal_repo_update_face(&(ctx->db_repo), id,
                    name? name->name: NULL,
                    features? (unsigned char*)features->ptr: NULL,
                    features? features->size: 0);
}

int HAL_FACE_RepoRemoveIdentity(const hal_facelib_handle_t handle, const hal_face_id_t id) {
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);

    return cvi_hal_repo_delete_face(&(ctx->db_repo), id);
}

int HAL_FACE_RepoRemoveAllIdentities(const hal_facelib_handle_t handle) {
    hal_facelib_context_t *ctx = static_cast<hal_facelib_context_t*>(handle);

    char path[128] = {0};
    snprintf(path, sizeof(path), "%s", ctx->db_repo.path);

    cvi_hal_repo_close(&(ctx->db_repo));
    unlink(path);

    return cvi_hal_repo_open(&(ctx->db_repo), path);
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
}

int  Cvi_face_Save(cv::Mat src_img, hal_face_t face, int face_idx, char *face_name)
{
    if (src_img.empty()) {
        printf("face img is empty\n");
        return -1;
    }
    hal_face_bbox_t rect;
    HAL_FACE_RectRescale(src_img.cols, src_img.rows, face, 0, true, &rect);
    int x1 = rect.x1;
    int y1 = rect.y1;
    int x2 = rect.x2;
    int y2 = rect.y2;

    //printf("x1 = %d, y1 = %d, x2 = %d, y2 = %d\n",x1,y1,x2,y2);

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
    return 0;
}

static int Cvi_face_getinfo(char *image_path, hal_face_t faces)
{
    int ret = 0;
    // VB_BLK blk_fd = VB_INVALID_HANDLE;
    VIDEO_FRAME_INFO_S fdFrame;
    //float matched_score = 0;
    //int max_index = 0;

    cvai_face_t *_faces = reinterpret_cast<cvai_face_t *>(faces);
    ret = CVI_AI_ReadImage(image_path, &fdFrame, PIXEL_FORMAT_RGB_888);
    if (ret != CVI_SUCCESS) {
        printf("Read image failed with %#x!\n", ret);
        return ret;
    }

    ret = HAL_FACE_Detect(&fdFrame, _faces, 0);
    if (ret != CVI_SUCCESS) {
        printf("Cv183xFaceDetect failed with %#x!\n", ret);
        return ret;
    }
    if (0 == _faces->size) {
        ret = CVI_FAILURE;
        goto END;
    }

    ret = HAL_FACE_Recognize(&fdFrame, _faces);
    if (ret != CVI_SUCCESS) {
        printf("Cv183xFaceRecognize failed with %#x!\n", ret);
        goto END;
    }
    ret = CVI_AI_FaceQuality(aisdk_handle, &fdFrame, _faces, NULL);
    if (ret != CVI_SUCCESS) {
        printf("CVI_AI_FaceQuality failed with %#x!\n", ret);
        goto END;
    }
END:
    // if (blk_fd != VB_INVALID_HANDLE)
    //     CVI_VB_ReleaseBlock(blk_fd);
    return ret;
}

int HAL_FACE_Register(hal_facelib_handle_t facelib_handle, char *image_path,
                             cvi_person_t person)
{
    int ret;
    hal_face_t face = NULL;
    if (HAL_FACE_Create(&face) != 0)
        return -1;

    ret = Cvi_face_getinfo(image_path, face);
    if (ret != CVI_SUCCESS) {
        printf("Cvi_face_getinfo failed with %#x!\n", ret);
        HAL_FACE_Free(face);
        return ret;
    }
    int face_num = HAL_FACE_GetNum(face);
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
            HAL_FACE_Free(face);
            return FAIL_DUPLICATE;
        }
    }

	char face_img[64]={0};
    cv::Mat src_img = cv::imread(image_path);

    sprintf(face_img,"./repo/%s_%d.jpg",(char *)person.name, person_num);
    if (0 != Cvi_face_Save(src_img, face, 0, face_img)) {
        printf("Cvi_face_save failed\n");
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

    if(-1 != CviFaceRepoAddIdentity_with_PersonInfo(facelib_handle, face, person))
    {
        CviLoadIdentify(facelib_handle);// reload data to memory
    }
    HAL_FACE_Free(face);
    return ret ;
}

void HAL_FACE_Encode(VIDEO_FRAME_INFO_S *stOutFrame, hal_face_t face,char *capture_img,char *recored_img)
{
    cv::Mat rgb_frame(stOutFrame->stVFrame.u32Height, stOutFrame->stVFrame.u32Width, CV_8UC3);
    stOutFrame->stVFrame.pu8VirAddr[0] = (CVI_U8 *)CVI_SYS_MmapCache(stOutFrame->stVFrame.u64PhyAddr[0],
                                                           stOutFrame->stVFrame.u32Length[0]);
    char *va_rgb = (char *)stOutFrame->stVFrame.pu8VirAddr[0];
    for (int i = 0; i < rgb_frame.rows; i++) {
        memcpy(rgb_frame.ptr(i, 0), va_rgb + stOutFrame->stVFrame.u32Stride[0] * i, rgb_frame.cols * 3);
    }
    if (stOutFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_RGB_888) {
        cvtColor(rgb_frame, rgb_frame, CV_RGB2BGR);
    }

    CVI_SYS_Munmap((void *)stOutFrame->stVFrame.pu8VirAddr[0], stOutFrame->stVFrame.u32Length[0]);

    if(face != NULL)
    {
        hal_face_bbox_t face_bbox;
        memset(&face_bbox, 0, sizeof(hal_face_bbox_t));
        HAL_FACE_RectRescale(stOutFrame->stVFrame.u32Width, stOutFrame->stVFrame.u32Height, face, 0, true, &face_bbox);

        int x1 = face_bbox.x1;
        int y1 = face_bbox.y1;
        int x2 = face_bbox.x2;
        int y2 = face_bbox.y2;

        // printf("x1 = %d, y1 = %d, x2 = %d, y2 = %d\n",x1,y1,x2,y2);

        int x = x1-5>0?x1-5:0;
        int y = y1-5>0?y1-5:0;
        int width = x2-x1+10;
        int height = y2-y1+10;

        if(x+width>rgb_frame.cols)
            width = rgb_frame.cols-x;
        if(y+height>rgb_frame.rows)
            height = rgb_frame.rows-y;
        // printf("colom = %d, row = %d, x = %d, y = %d, width = %d, height = %d\n",rgb_frame.cols, rgb_frame.rows,x,y,width,height);
        cv::Mat result_image(rgb_frame, cv::Rect(x,y, width, height));
        if (capture_img != NULL) {
            imwrite(capture_img,result_image);
        }
        // cv::resize(result_image,result_image,Size(300,300), CV_INTER_LINEAR);
        if (recored_img != NULL) {
            imwrite(recored_img, result_image);
        }
    } else {
        imwrite(capture_img, rgb_frame);
    }
}

int Cvi_Jpg2PNG(char *src_jpg,char *dst_png,int *width,int *height)
{
    cv::Mat face = imread(src_jpg,1);
    int s32Ret = CVI_SUCCESS;
    if(face.data == NULL)
    {
        printf("%s: invalid file\n", __FUNCTION__);
        return CVI_FAILURE;
    }
    imwrite(dst_png,face);
    *width = face.cols;
    *height = face.rows;
    return s32Ret;
}

int CviGetPersonList(const hal_facelib_handle_t facelib_handle, cvi_person_t **person_list)
{
    hal_facelib_context_t *context = static_cast<hal_facelib_context_t*>(facelib_handle);
    return cvi_hal_repo_get_person_list(&(context->db_repo), 0, -1, person_list);
}

int CviSetPerson(const hal_facelib_handle_t facelib_handle, cvi_person_t *person)
{
    hal_facelib_context_t *context = static_cast<hal_facelib_context_t*>(facelib_handle);
    return cvi_hal_repo_set_person(&(context->db_repo), person->id, person);
}

void CviSaveFrame(VIDEO_FRAME_INFO_S *frm, const char * name) {
    if (frm->stVFrame.pu8VirAddr[0]) {
        cv::Mat rgb_frame = cv::Mat(frm->stVFrame.u32Height,
                            frm->stVFrame.u32Width,
                            CV_8UC3,
                            frm->stVFrame.pu8VirAddr[0],
                            frm->stVFrame.u32Stride[0]);
        cv::imwrite(name, rgb_frame);
    }

}

