#include <stdio.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "cvi_comm_video.h"
#include "cvi_hal_face_datatype.h"

//#include "face_wrapper.h"
#include "facepass_wrapper.h"

static FaceGroupHandle faceGroupHandle = nullptr;

static int trans_image_type(VIDEO_FRAME_INFO_S *frm, Image *image) {
    if (PIXEL_FORMAT_BGR_888 != frm->stVFrame.enPixelFormat) {
        printf("Frame pixel_format error:%d\n", frm->stVFrame.enPixelFormat);
        return -1;
    }
    if (frm->stVFrame.u32Stride[0] != frm->stVFrame.u32Width * 3) {
        printf("Megvii only support frame without stride! width[%d] stride[%d]\n",
               frm->stVFrame.u32Width, frm->stVFrame.u32Stride[0]);
        return -1;
    }
    if (frm->stVFrame.pu8VirAddr[0] == nullptr) {
        printf("vir addr is null\n");
        return -1;
    }
    image->phy_addr     = nullptr;
    image->vir_addr     = frm->stVFrame.pu8VirAddr[0];
    image->fd = -1;
    image->width        = frm->stVFrame.u32Width;
    image->height       = frm->stVFrame.u32Height;
    image->wstride      = frm->stVFrame.u32Stride[0];
    image->hstride      = image->height;
    image->pixel_format = Format_BGR888;
    return 0;
}

static int trans_image_type(cv::Mat *mat, Image *image) {
    image->phy_addr = nullptr;
    image->vir_addr = mat->data;
    image->fd = -1;
    image->width        = mat->cols;
    image->height       = mat->rows;
    image->wstride      = mat->cols * 3;
    image->hstride      = mat->rows;
    image->pixel_format = Format_BGR888;
    return 0;
}

static void sort_face(aal_facepass_face_t *faces) {
    int max_face_idx = 0;
    float max_face_area = 0.0l;
    for (int i = 0; i < faces->face_num; ++i) {
        FaceRect face_rect;
        getFaceRect(faces->info[i].face_handle, &face_rect);
        float area = (face_rect.right - face_rect.left) *
                     (face_rect.bottom - face_rect.top);
        if (max_face_area < area) {
            max_face_idx = i;
            max_face_area = area;
        }
    }
    if (max_face_idx != 0) {
        aal_facepass_face_info tmp = faces->info[max_face_idx];
        faces->info[max_face_idx] = faces->info[0];
        faces->info[0] = tmp;
    }
}

int FACELIB_INIT(const hal_facelib_config_t *facelib_config) {
    FaceModels models;
    models.detect_model = facelib_config->model_face_fd;
    models.anchor_path = facelib_config->model_face_anchor; 
    models.postfilter_model = facelib_config->model_face_postfilter;
    models.pose_blur_model = facelib_config->model_face_poseblur;
    models.refine_model = facelib_config->model_face_landmark;
    models.liveness_bgr_model = facelib_config->model_face_liveness_bgr;
    models.liveness_ir_model = "";
    models.liveness_bgrir_model =facelib_config->model_face_liveness ;
    models.age_gender_model = "";
    models.rc_model = facelib_config->model_face_attr;
    models.occl_model = "";
    models.stn_model = facelib_config->model_face_stn;
    models.feature_model = facelib_config->model_face_extr;
    models.group_model_path = facelib_config->model_face_search;

    int ret = ::init(models);
    if (RET_OK != ret) {
        printf("init failed return:%d\n", ret);
        return -1;
    }
    set_detect_config(facelib_config->attr.threshold_det, 0.5);
    //set_match_config(1.00758, -52.66727, 1.00936, 12.62161, 0.4);
    //setLogLevel(LOG_LEVEL_INFO);
    return 0;
}

int FACELIB_RELEASE() {
    release();
    return 0;
}

hal_face_t FACELIB_CREATE_FACE() {
    aal_facepass_face_t *faces = reinterpret_cast<aal_facepass_face_t *>(
        malloc(sizeof(aal_facepass_face_t)));
    if (!faces) {
        return nullptr;
    }
    faces->handle_num = FACEPASS_MAX_FACE_NUM;
    faces->face_num = 0;
    faces->width = 0;
    faces->height = 0;
    faces->info = reinterpret_cast<aal_facepass_face_info *>(
        malloc(sizeof(aal_facepass_face_info) * faces->handle_num));
    if (!faces->info) {
        free(faces);
        return nullptr;
    }
    memset(faces->info, 0x00, sizeof(aal_facepass_face_info) * faces->handle_num);
    return reinterpret_cast<hal_face_t>(faces);
}

void FACELIB_FREE_FACE(hal_face_t faces) {
    aal_facepass_face_t *_faces = reinterpret_cast<aal_facepass_face_t *>(faces);
    for (int i = 0; i < _faces->face_num; ++i) {
        releaseFace(_faces->info[i].face_handle);
        if (_faces->info[i].feature) {
            releaseFeature(reinterpret_cast<char *>(_faces->info[i].feature));
        }
    }
    free(_faces->info);
    free(_faces);
}

void FACELIB_CLEAR_FACE(hal_face_t faces) {
    aal_facepass_face_t *_faces = reinterpret_cast<aal_facepass_face_t *>(faces);
    for (int i = 0; i < _faces->face_num; ++i) {
        releaseFace(_faces->info[i].face_handle);
        if (_faces->info[i].feature) {
            releaseFeature(reinterpret_cast<char *>(_faces->info[i].feature));
        }
    }
    memset(_faces->info, 0x00, sizeof(aal_facepass_face_info) * _faces->handle_num);
    _faces->width = 0;
    _faces->height = 0;
    _faces->face_num = 0;
}

int FACELIB_DEL_FACE(hal_face_t faces, int idx) {
    aal_facepass_face_t *_faces = reinterpret_cast<aal_facepass_face_t *>(faces);
    if (idx >= _faces->face_num) {
        printf("idx[%d] out of range, face num[%d]\n", idx, _faces->face_num);
        return -1;
    }
    int last_idx = _faces->face_num - 1;
    if (idx != last_idx) {
        aal_facepass_face_info tmp;
        tmp = _faces->info[idx];
        _faces->info[idx] = _faces->info[last_idx];
        _faces->info[last_idx] = tmp;
    }
    _faces->face_num -= 1;
    releaseFace(_faces->info[last_idx].face_handle);
    _faces->info[last_idx].face_handle = nullptr;
    if (_faces->info[last_idx].feature) {
        releaseFeature(reinterpret_cast<char *>(_faces->info[last_idx].feature));
        _faces->info[last_idx].feature = nullptr;
    }
    _faces->info[last_idx].feature_length = 0;
    _faces->info[last_idx].liveness_score = 0.0l;
    return 0;
}

int FACELIB_GET_FACE_NUM(hal_face_t faces) {
    return reinterpret_cast<aal_facepass_face_t *>(faces)->face_num;
}

int FACELIB_GET_FACE_RECT(hal_face_t faces, int idx, hal_face_bbox_t *rect) {
    aal_facepass_face_t *_faces = reinterpret_cast<aal_facepass_face_t *>(faces);
    if (idx >= _faces->face_num) {
        printf("idx[%d] out of range, face num[%d]\n", idx, _faces->face_num);
        return -1;
    }
    FaceRect face_rect;
    getFaceRect(_faces->info[idx].face_handle, &face_rect);
    rect->x1 = face_rect.left;
    rect->y1 = face_rect.top;
    rect->x2 = face_rect.right;
    rect->y2 = face_rect.bottom;
    return 0;
}

int FACELIB_GET_IMAGE_SIZE(hal_face_t faces, int *width, int *height) {
    *width = reinterpret_cast<aal_facepass_face_t *>(faces)->width;
    *height = reinterpret_cast<aal_facepass_face_t *>(faces)->height;
    return 0;
}

int FACELIB_GET_FEATURE(hal_face_t faces, int idx, hal_face_feature_t *feature) {
    aal_facepass_face_t *_faces = reinterpret_cast<aal_facepass_face_t *>(faces);
    if (idx >= _faces->face_num) {
        printf("idx[%d] out of range, face num[%d]\n", idx, _faces->face_num);
        return -1;
    }
    feature->ptr = _faces->info[idx].feature;
    feature->size = _faces->info[idx].feature_length;
    feature->type = FT_INT8;
    return 0;
}

int FACELIB_GET_NAME(hal_face_t faces, int idx, char **name, size_t *size) {
    aal_facepass_face_t *_faces = reinterpret_cast<aal_facepass_face_t *>(faces);
    if (idx >= _faces->face_num) {
        printf("idx[%d] out of range, face num[%d]\n", idx, _faces->face_num);
        return -1;
    }
    *name = _faces->info[idx].attr.name;
    *size = sizeof(_faces->info[idx].attr.name);
    return 0;
}

int FACELIB_DETECT(VIDEO_FRAME_INFO_S *frame, hal_face_t faces, float min_face_h) {
    aal_facepass_face_t *_faces = reinterpret_cast<aal_facepass_face_t *>(faces);
    int ret = 0;
    if (_faces->face_num) {
        FACELIB_CLEAR_FACE(faces);
    }
    Image bgr_image;
    if (0 != trans_image_type(frame, &bgr_image)) {
        printf("trans_image_type failed\n");
        return -1;
    }
    int face_num = FACEPASS_MAX_FACE_NUM;
    std::vector<FaceHandle> detHandleArr(face_num, nullptr);
    ret = detect(&bgr_image, 0, min_face_h, detHandleArr.data(), &face_num);
    if (RET_NO_FACE != ret && RET_OK != ret) {
        printf("aal_facepass_face_detect failed! ret:%d\n", ret);
        return ret;
    }
    _faces->face_num = face_num;
    _faces->width = bgr_image.width;
    _faces->height = bgr_image.height;
    for (int i = 0; i < face_num; ++i) {
        _faces->info[i].face_handle = detHandleArr[i];
        _faces->info[i].feature = nullptr;
    }
    sort_face(_faces);
    return 0;
}

int FACELIB_DETECT(cv::Mat *mat, hal_face_t faces, float min_face_h) {
    aal_facepass_face_t *_faces = reinterpret_cast<aal_facepass_face_t *>(faces);
    int ret = 0;
    if (_faces->face_num) {
        FACELIB_CLEAR_FACE(faces);
    }
    Image bgr_image;
    if (0 != trans_image_type(mat, &bgr_image)) {
        printf("trans_image_type failed\n");
        return -1;
    }
    int face_num = FACEPASS_MAX_FACE_NUM;
    std::vector<FaceHandle> detHandleArr(face_num, nullptr);
    ret = detect(&bgr_image, 0, 0, detHandleArr.data(), &face_num);
    if (RET_NO_FACE != ret && RET_OK != ret) {
        printf("aal_facepass_face_detect failed! ret:%d\n", ret);
        return ret;
    }
    _faces->face_num = face_num;
    _faces->width = bgr_image.width;
    _faces->height = bgr_image.height;
    for (int i = 0; i < face_num; ++i) {
        _faces->info[i].face_handle = detHandleArr[i];
        _faces->info[i].feature = nullptr;
    }
    sort_face(_faces);
    return 0;
}

int FACELIB_RECOG(VIDEO_FRAME_INFO_S *frame, hal_face_t faces) {
    aal_facepass_face_t *_faces = reinterpret_cast<aal_facepass_face_t *>(faces);
    int ret = 0;
    Image bgr_image;
    if (0 != trans_image_type(frame, &bgr_image)) {
        printf("trans_image_type failed\n");
        return -1;
    }
    for (int i = 0; i < _faces->face_num; ++i) {
        char *feature = nullptr;
        size_t size = 0;
        ret = extract(&bgr_image, _faces->info[i].face_handle, &feature, reinterpret_cast<int *>(&size));
        if (RET_OK != ret) {
            printf("aal_facepass_face_recog failed! ret:%d\n", ret);
            continue;
        }
        _faces->info[i].feature = reinterpret_cast<int8_t *>(feature);
        _faces->info[i].feature_length = size;
    }
    return 0;
}

int FACELIB_RECOG(cv::Mat *mat, hal_face_t faces) {
    aal_facepass_face_t *_faces = reinterpret_cast<aal_facepass_face_t *>(faces);
    int ret = 0;
    Image bgr_image;
    if (0 != trans_image_type(mat, &bgr_image)) {
        printf("trans_image_type failed\n");
        return -1;
    }
    for (int i = 0; i < _faces->face_num; ++i) {
        char *feature = nullptr;
        size_t size = 0;
        ret = extract(&bgr_image, _faces->info[i].face_handle, &feature, reinterpret_cast<int *>(&size));
        if (RET_OK != ret) {
            printf("aal_facepass_face_recog failed! ret:%d\n", ret);
            continue;
        }
        _faces->info[i].feature = reinterpret_cast<int8_t *>(feature);
        _faces->info[i].feature_length = size;
    }
    return 0;
}

int FACELIB_LIVENESS(VIDEO_FRAME_INFO_S *bgrFrame, VIDEO_FRAME_INFO_S *irFrame, hal_face_t faces) {
    aal_facepass_face_t *_faces = reinterpret_cast<aal_facepass_face_t *>(faces);
    int ret = 0;
    Image bgr_image;
    Image ir_image;
    if (0 != trans_image_type(bgrFrame, &bgr_image)) {
        printf("trans_image_type[bgr frame] failed\n");
        return -1;
    }
    if (0 != trans_image_type(irFrame, &ir_image)) {
        printf("trans_image_type[ir frame] failed\n");
        return -1;
    }
    int ir_face_num = FACEPASS_MAX_FACE_NUM;
    std::vector<FaceHandle> irHandleArr(ir_face_num, nullptr);
    ret = detect(&ir_image, 0, 0, irHandleArr.data(), &ir_face_num);
    if (RET_OK != ret) {
        printf("aal_facepass_liveness detect ir face failed! ret:%d\n", ret);
        return ret;
    }
    irHandleArr.resize(ir_face_num);

    std::vector<FaceHandle> bgrHandleArr(_faces->face_num, nullptr);
    for (int i = 0; i < _faces->face_num; ++i) {
        bgrHandleArr[i] = _faces->info[i].face_handle;
    }

    std::vector<int> corrIrIndex(_faces->face_num, ir_face_num);
    BGR_IR_match(bgrHandleArr.data(), _faces->face_num, irHandleArr.data(), ir_face_num, corrIrIndex.data());

    for (int i = 0; i < _faces->face_num; ++i) {
        float score = 0.0l;
        if (corrIrIndex[i] != ir_face_num) {
            ret = getLiveness_bgrir(&bgr_image, bgrHandleArr[i], &ir_image, irHandleArr[corrIrIndex[i]], &score);
            if (RET_OK == ret) {
                _faces->info[i].liveness_score = score;
            }
        } else {
            _faces->info[i].liveness_score = 0.0l;
        }
    }
    for (auto& handle : irHandleArr) {
        releaseFace(handle);
    }
    return 0;
}

int FACELIB_LIVENESS_BGR(VIDEO_FRAME_INFO_S *bgrFrame, hal_face_t faces) {
    aal_facepass_face_t *_faces = reinterpret_cast<aal_facepass_face_t *>(faces);
    int ret = 0;
    Image bgr_image;
    if (0 != trans_image_type(bgrFrame, &bgr_image)) {
        printf("trans_image_type[bgr frame] failed\n");
        return -1;
    }

    std::vector<FaceHandle> bgrHandleArr(_faces->face_num, nullptr);
    for (int i = 0; i < _faces->face_num; ++i) {
        bgrHandleArr[i] = _faces->info[i].face_handle;
    }

    for (int i = 0; i < _faces->face_num; ++i) {
        float score = 0.0l;
        ret = getLiveness_bgr(&bgr_image, bgrHandleArr[i], &score);
        if (RET_OK == ret) {
            _faces->info[i].liveness_score = score;
        } else {
            _faces->info[i].liveness_score = 0.0l;
        }
    }
    return 0;
}

int FACELIB_GET_FACE_INFO(const char *image_path, hal_face_t faces) {
    cv::Mat image = cv::imread(image_path);
    int s32Ret = FACELIB_DETECT(&image, faces, 0);
    if (s32Ret != CVI_SUCCESS) {
        printf("aal_facepass_face_detect failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    if (0 == FACELIB_GET_FACE_NUM(faces)) {
	printf("aal_facepass_face_detect no face!\n");
        s32Ret = CVI_FAILURE;
        return s32Ret;
    }
    s32Ret = FACELIB_RECOG(&image, faces);
    if (s32Ret != CVI_SUCCESS) {
        printf("aal_facepass_face_recog failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    return 0;
}

int FACELIB_GET_LIVENESS_SCORE(hal_face_t faces, int idx, float *score) {
    aal_facepass_face_t *_faces = reinterpret_cast<aal_facepass_face_t *>(faces);
    if (idx >= _faces->face_num) {
        printf("idx[%d] out of range, face num[%d]\n", idx, _faces->face_num);
        return -1;
    }
    *score = _faces->info[idx].liveness_score;
    return 0;
}

int FACELIB_RECT_RESCALE(float width, float height, hal_face_t faces,
                              int idx, bool keep_aspect, hal_face_bbox_t *box) {
    aal_facepass_face_t *_faces = reinterpret_cast<aal_facepass_face_t *>(faces);
    hal_face_bbox_t rect, face_rect;
    FACELIB_GET_FACE_RECT(faces, idx, &rect);

    int det_width = _faces->width;
    int det_height = _faces->height;
    float ratio_x, ratio_y, bbox_y_height, bbox_x_height, bbox_padding_top, bbox_padding_left;

    if (width == det_width && height == det_height) {
        *box = rect;
        return 0;
    } else {
        if (width >= height) {
            ratio_x          = width / det_width;
            bbox_y_height    = det_width * height / width;
            ratio_y          = height / bbox_y_height;
            bbox_padding_top = (det_height - bbox_y_height) / 2;
        } else {
            ratio_y           = height / det_height;
            bbox_x_height     = det_height * width / height;
            ratio_x           = width / bbox_x_height;
            bbox_padding_left = (det_width - bbox_x_height) / 2;
        }

        float x1, x2, y1, y2;

        if (width >= height) {
            x1 = rect.x1 * ratio_x;
            x2 = rect.x2 * ratio_x;
            y1 = (rect.y1 - bbox_padding_top) * ratio_y;
            y2 = (rect.y2 - bbox_padding_top) * ratio_y;
        } else {
            x1 = (rect.x1 - bbox_padding_left) * ratio_x;
            x2 = (rect.x2 - bbox_padding_left) * ratio_x;
            y1 = rect.y1 * ratio_y;
            y2 = rect.y2 * ratio_y;
        }

        face_rect.x1 = std::max(std::min(x1, width - 1), (float)0);
        face_rect.x2 = std::max(std::min(x2, width - 1), (float)0);
        face_rect.y1 = std::max(std::min(y1, height - 1), (float)0);
        face_rect.y2 = std::max(std::min(y2, height - 1), (float)0);

        *box = face_rect;
        return 0;
    }
}

int FACELIB_IMPORT_FEATURE_GROUP(int8_t *feature_ptr, int feature_dim, int feature_type, int feature_num) {
    if (nullptr != faceGroupHandle) {
        releaseFaceGroup(faceGroupHandle);
        faceGroupHandle = nullptr;
    }
    createFaceGroup(&faceGroupHandle);
    printf("import feature\n");
    for (int i = 0; i < feature_num; ++i) {
        if (RET_OK != insertFaceGroup(faceGroupHandle, (const char *)feature_ptr, feature_dim, i)) {
            printf("import face group failed\n");
            releaseFaceGroup(faceGroupHandle);
            return -1;
        }
        feature_ptr += feature_dim;
    }
    printf("import success\n");
    return 0;
}

int FACELIB_INSERT_FEATURE_GROUP(int8_t *feature_ptr, int feature_dim, int feature_type, int feature_num) {
    for (int i = 0; i < feature_num; ++i) {
        if (RET_OK != insertFaceGroup(faceGroupHandle, (const char *)feature_ptr, feature_dim, i)) {
            printf("insert face group failed\n");
            releaseFaceGroup(faceGroupHandle);
            return -1;
        }
    }
    return 0;
}

int FACELIB_DELETE_FEATURE_GROUP(int idx) {
    if (RET_OK != removeFaceGroup(faceGroupHandle, idx)) {
        printf("delete face group failed idx:%d\n", idx);
        return -1;
    }
    return 0;
}

int FACELIB_FREE_FEATURE_GROUP() {
    if (faceGroupHandle) {
        releaseFaceGroup(faceGroupHandle);
    }
    faceGroupHandle = nullptr;
    return 0;
}

int FACELIB_FEATURE_MATCH(int8_t *feature, int8_t *precached, float *unit_precached_arr,
                              uint32_t *k_index, float *k_value, float *buffer,
                              const uint32_t data_length, const uint32_t data_num, const uint32_t k) {
    std::vector<uint64_t> k_index_tmp(k, -1);
    if (RET_OK != top_k_FromFaceGroup(faceGroupHandle, (const char *)feature, data_length, k, k_value, k_index_tmp.data())) {
        printf("feature matching failed\n");
        return -1;
    }
    for (uint32_t i = 0; i < k; ++i) {
        k_index[i] = k_index_tmp[i];
        k_value[i] /= 100;
    }
    return 0;
}

int FACELIB_COMPARE(int8_t *ptr1, int8_t *ptr2, int feature_dim, float *score) {
    compare((const char *)ptr1, (const char *)ptr2, feature_dim, score);
    *score /= 100;
    return 0;
}
