#pragma once
#include "face_sdk.h"
#define FACEPASS_MAX_FACE_NUM 10

struct aal_facepass_face_attr {
    char name[128];
};

struct aal_facepass_face_info {
    FaceHandle face_handle;
    int8_t *feature;
    size_t feature_length;
    float liveness_score;
    aal_facepass_face_attr attr;
};

struct aal_facepass_face_t {
    aal_facepass_face_info *info;
    int handle_num;
    int face_num;
    int width;
    int height;
};