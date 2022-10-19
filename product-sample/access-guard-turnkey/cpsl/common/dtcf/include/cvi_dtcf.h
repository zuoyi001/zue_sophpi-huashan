#ifndef CVI_DTCF_H_
#define CVI_DTCF_H_
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include <stdint.h>

#define MODULE_NAME_DTCF  "DTCF"

#define CVI_FILE_PATH_LEN_MAX         (256)
#define CVI_DIR_LEN_MAX               (64)

/** DataTime Error number base */
#define CVI_DTCF_ERRNO_BASE 0x2A00
/** Parameter is invalid */
#define CVI_DTCF_ERR_EINVAL_PAEAMETER    (CVI_DTCF_ERRNO_BASE + 1)
/** Null pointer*/
#define CVI_DTCF_ERR_NULL_PTR            (CVI_DTCF_ERRNO_BASE + 2)
/** failure caused by malloc memory */
#define CVI_DTCF_ERR_NOMEM               (CVI_DTCF_ERRNO_BASE + 3)
/** not support file path */
#define CVI_DTCF_UNSUPPORT_PATH          (CVI_DTCF_ERRNO_BASE + 4)
/** the same directory */
#define CVI_DTCF_SAME_DIR_PATH           (CVI_DTCF_ERRNO_BASE + 5)
/** Status error, operation not allowed*/
#define CVI_DTCF_ERR_STATUS_ERROR        (CVI_DTCF_ERRNO_BASE + 6)
/** Empty directory*/
#define CVI_DTCF_ERR_EMPTEY_DIR          (CVI_DTCF_ERRNO_BASE + 7)
/** directory not define*/
#define CVI_DTCF_ERR_UNDEFINE_DIR        (CVI_DTCF_ERRNO_BASE + 8)
/** System error*/
#define CVI_DTCF_ERR_SYSTEM_ERROR        (CVI_DTCF_ERRNO_BASE + 9)
/** the same filename */
#define CVI_DTCF_SAME_FILENAME_PATH      (CVI_DTCF_ERRNO_BASE + 10)
/** the path is not a directory */
#define CVI_DTCF_PATH_IS_NOT_DIR_ERROR   (CVI_DTCF_ERRNO_BASE + 11)

/**
 * File Directory Type
 */
typedef enum cviDTCF_DIR_E
{
    DTCF_DIR_EMR_FRONT = 0,
    DTCF_DIR_EMR_FRONT_SUB,
    DTCF_DIR_NORM_FRONT,
    DTCF_DIR_NORM_FRONT_SUB,
    DTCF_DIR_PARK_FRONT,
    DTCF_DIR_PARK_FRONT_SUB,
    DTCF_DIR_EMR_REAR,
    DTCF_DIR_EMR_REAR_SUB,
    DTCF_DIR_NORM_REAR,
    DTCF_DIR_NORM_REAR_SUB,
    DTCF_DIR_PARK_REAR,
    DTCF_DIR_PARK_REAR_SUB,
    DTCF_DIR_PHOTO_FRONT,
    DTCF_DIR_PHOTO_REAR,
    DTCF_DIR_BUTT
} CVI_DTCF_DIR_E;

/**
 * File Type.
 */
typedef enum cviDTCF_FILE_TYPE_E
{
    CVI_DTCF_FILE_TYPE_MP4,
    CVI_DTCF_FILE_TYPE_JPG,
    CVI_DTCF_FILE_TYPE_TS,
    CVI_DTCF_FILE_TYPE_MOV,
    CVI_DTCF_FILE_TYPE_BUTT
} CVI_DTCF_FILE_TYPE_E;


int32_t CVI_DTCF_Init(const char *pazRootDir, const char azDirNames[DTCF_DIR_BUTT][CVI_DIR_LEN_MAX]);
int32_t CVI_DTCF_GetDirNames(char (*pazDirNames)[CVI_DIR_LEN_MAX], uint32_t u32DirAmount);
int32_t CVI_DTCF_DeInit(void);
int32_t CVI_DTCF_Scan(CVI_DTCF_DIR_E enDirs[DTCF_DIR_BUTT], uint32_t u32DirCount, uint32_t *pu32FileAmount);
int32_t CVI_DTCF_GetFileByIndex(uint32_t u32Index, char *pazFileName, uint32_t u32Length, CVI_DTCF_DIR_E *penDir);
int32_t CVI_DTCF_DelFileByIndex(uint32_t u32Index, uint32_t *pu32FileAmount);
int32_t CVI_DTCF_AddFile(const char *pazSrcFilePath, CVI_DTCF_DIR_E enDir);
int32_t CVI_DTCF_GetOldestFileIndex(CVI_DTCF_DIR_E enDirs[DTCF_DIR_BUTT], uint32_t u32DirCount, uint32_t *pu32Index);
int32_t CVI_DTCF_GetFileAmount(uint32_t *pu32FileAmount);
int32_t CVI_DTCF_GetOldestFilePath(CVI_DTCF_DIR_E enDir, char *pazFilePath, uint32_t u32Length);
int32_t CVI_DTCF_CreateFilePath(CVI_DTCF_FILE_TYPE_E enFileType, CVI_DTCF_DIR_E enDir, char *pazFilePath, uint32_t u32Length);
int32_t CVI_DTCF_GetRelatedFilePath(const char *pazSrcFilePath, CVI_DTCF_DIR_E enDir, char *pazDstFilePath, uint32_t u32Length);
int32_t CVI_DTCF_GetEmrFilePath(const char *pazSrcFilePath, char *pazDstFilePath, uint32_t u32Length);
int32_t CVI_DTCF_GetFileDirType(const char *pazSrcFilePath, CVI_DTCF_DIR_E *penDir);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* CVI_DTCF_H_ */
