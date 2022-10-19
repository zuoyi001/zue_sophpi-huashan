#ifndef __CVI_HAL_FACE_API_H__
#define __CVI_HAL_FACE_API_H__

#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vpss.h"
#include "cvi_hal_face_datatype.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief	Library init
 * @param 	In: config_file, path to config file
 * @return  Ok:0, Error:-1
 * @note
**/
int HAL_FACE_LibOpen(const hal_facelib_config_t *facelib_config);

/**
 * @brief   Library release
 * @return  Ok:0, Error:-1
**/
int HAL_FACE_LibClose(void);

/**
 * @brief   Detect faces in image
 * @param   In: image: the image to be detected
 * @param   Out: faces: facd infos hal_face_t
 * @return  Ok:0, Error:-1
**/
int HAL_FACE_Detect(VIDEO_FRAME_INFO_S *stfdFrame, hal_face_t faces, float min_face_h);

/**
 * @brief   Detect liveness in image
 * @param   In: image1: color image
 * @param   In: image2: ir image
 * @param   Out: faces: facd infos hal_face_t
 * @return  Ok:0, Error:-1
**/
int HAL_FACE_LivenessDetect(VIDEO_FRAME_INFO_S *rgbFrame,VIDEO_FRAME_INFO_S *irFrame, hal_face_t faces);

/**
 * @brief   Detect liveness in image
 * @param   In: image1: color image
 * @param   Out: faces: facd infos hal_face_t
 * @return  Ok:0, Error:-1
**/
int HAL_FACE_LivenessDetectBgr(VIDEO_FRAME_INFO_S *rgbFrame, hal_face_t faces);

/**
 * @brief   Extract features in image of given faces
 * @param   In: image, the image to be processed
 * @param   Out: faces: facd infos hal_face_t
 * @return  Ok:0, Error:-1
**/
int HAL_FACE_Recognize(VIDEO_FRAME_INFO_S *image, hal_face_t face);

int HAL_FACE_RectRescale(float width, float height, hal_face_t faces, int idx, bool keep_aspect, hal_face_bbox_t *box);

void CviSaveFrame(VIDEO_FRAME_INFO_S *frm, const char * name);

int HAL_FACE_Create(hal_face_t *face);

int HAL_FACE_Free(hal_face_t face);

int HAL_FACE_Clear(hal_face_t face);

int HAL_FACE_DelFace(hal_face_t face, int idx);

int HAL_FACE_GetNum(hal_face_t face);

int HAL_FACE_GetFaceRect(hal_face_t face, int idx, hal_face_bbox_t *rect);

int HAL_FACE_GetImageSize(hal_face_t face, int *width, int *height);

int HAL_FACE_GetFeature(hal_face_t face, int idx, hal_face_feature_t *feature);

int HAL_FACE_GetLivenessScore(hal_face_t face, int idx, float *score);

int HAL_FACE_LibRepoOpen(const hal_facelib_config_t *facelib_config,hal_facelib_handle_t *handle);

int HAL_FACE_LibRepoClose(hal_facelib_handle_t handle);

int HAL_FACE_LibSensorCalibrate(void);

/**
 * @brief          Load feature from DB into array for verify functions
 * @param handle   In: handle, handle to the libdrary internal data
 * @param use_tpu  Use TPU to do verify, currently unsupported
 * @return         Ok:0, Error:-1
 */
int CviLoadIdentify(hal_facelib_handle_t handle);


/**
 * @brief   Get face attribute settings
 * @param   In: handle, handle to the libdrary internal data
 * @param   Out: facelib_attr, pointer of face attribute stting data
 * @return  Ok:0, Error:-1
**/
int HAL_GetFaceLibAttr(const hal_facelib_handle_t handle, hal_facelib_attr_t *facelib_attr);

/**
 * @brief   Update face attribute settings
 * @param   In: handle, handle to the libdrary internal data
 * @param   In: facelib_attr, pointer of face attribute stting data
 * @return  Ok:0, Error:-1
**/
int HAL_UpdateFaceLibAttr(const hal_facelib_handle_t handle, const hal_facelib_attr_t *facelib_attr);

/**
 * @brief   Get person info by face_id
 * @param   In: handle, handle to the libdrary internal data
 * @param   In: face_id
 * @param   Out: person pointer to person structure
 * @return  Ok:0, Error:-1
**/
int Cvi_person_Info(const hal_facelib_handle_t facelib_handle, int face_id, cvi_person_t *person);

/**
 * @brief   Compute similarity of two given features and check if score is greater than threshold
 * @param   In: handle, handle to the libdrary internal data
 * @param   In: features
 * @param   In: features2
 * @param   Out: match, 1: when matched, 0: when not matched
 * @param   Out: score, a score in [0, 1] indiciating similarity score between the feature and identity
 * @return  Ok:0, Error:-1
**/
int HAL_FACE_Verify1v1(const hal_facelib_handle_t handle, const hal_face_feature_t *features1, 
	const hal_face_feature_t *features2, int *match, float *score);

/**
 * @brief   Match an extracted face feature against all entries in repositroy, returns the identity with highest score
 * @param   In: handle, handle to the libdrary internal data
 * @param   In: features
 * @param   Out: id, pointer to the matched identity, negative id indicates no matching identity
 * @param   Out: score, a score in [0, 1] indiciating similarity score between the feature and identity
 * @return  Ok:0, Error:-1
**/
int HAL_FACE_Identify1vN(const hal_facelib_handle_t handle, hal_face_t face_info, float threshold);

int HAL_FACE_RepoAddIdentity(hal_facelib_handle_t handle, hal_face_t face_info, hal_face_id_t *id);

/**
 * @brief   Get name of a person from repository
 * @param   In: handle, handle to the libdrary internal data
 * @param   In: id
 * @param   Out: name
 * @return  Ok:0, Error:-1
**/
int HAL_FACE_RepoGetIdentityName(const hal_facelib_handle_t handle, const hal_face_id_t id,
									hal_face_name_t *name);


/**
 * @brief   Get name of a person from repository
 * @param   In: handle, handle to the libdrary internal data
 * @param   In: id
 * @param   Out: name
 * @param   Out: feature
 * @return  Ok:0, Error:-1
**/
int HAL_FACE_RepoGetIdentityNameFeature(const hal_facelib_handle_t handle, const hal_face_id_t id,
										 hal_face_name_t *name, hal_face_feature_t *feature);

/**
 * @brief   Update entry in repository with id, name and features of a person
 * @param   In: handle, handle to the libdrary internal data
 * @param   In: id
 * @param   In: name
 * @param   In: features
 * @return  Ok:0, Error:-1
**/
int HAL_FACE_RepoUpdateIdentity(const hal_facelib_handle_t handle, const hal_face_id_t id,
								 const hal_face_name_t *name, const hal_face_feature_t *features);

/**
 * @brief   Remove a specific entry in repository
 * @param   In: handle, handle to the libdrary internal data
 * @param   In: id
 * @return  Ok:0, Error:-1
**/
int HAL_FACE_RepoRemoveIdentity(const hal_facelib_handle_t handle, const hal_face_id_t id);

/**
 * @brief   Remove all entries in repository
 * @param   In: handle, handle to the libdrary internal data
 * @return  Ok:0, Error:-1
**/
int HAL_FACE_RepoRemoveAllIdentities(const hal_facelib_handle_t handle);

/**
 * @brief   Show all entries in repository
 * @param   In: handle, handle to the libdrary internal data
 * @return  Ok:0, Error:-1
**/
int HAL_FACE_RepoShowList(const hal_facelib_handle_t handle);

/**
 * @brief   Match an extracted face feature against all entries in repositroy, returns the identity with highest score
 * @param   In: handle, handle to the libdrary internal data
 * @return  person number in repo
**/
int HAL_FACE_GetRepoNum(hal_facelib_handle_t facelib_handle);

//FD_RET aal_face_register(cv183x_facelib_handle_t facelib_handle,const char *image_path,cvi_person_t person);
int HAL_FACE_Register(hal_facelib_handle_t facelib_handle, char *image_path, cvi_person_t person);
void HAL_FACE_Encode(VIDEO_FRAME_INFO_S *stOutFrame, hal_face_t face,char *capture_img,char *recored_img);

/**
 * @brief   Convert jpg to png for lvgl display
 * @param   In: src_jpg, path of jpg image
 * @param   In: dst_png, path of out png image
 * @param   In: width, src_jpg img width
 * @param   In: height, src_jpg img height
 * @return  Ok:0, Error:-1
 */
int Cvi_Jpg2PNG(char *src_jpg,char *dst_png,int *width,int *height);

int CviGetPersonList(const hal_facelib_handle_t facelib_handle, cvi_person_t **person_list);

int CviSetPerson(const hal_facelib_handle_t facelib_handle, cvi_person_t *person);
#ifdef __cplusplus
}
#endif

#endif /*__CVI_HAL_FACE_API_H__*/