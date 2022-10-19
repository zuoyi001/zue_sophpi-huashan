#ifndef __CVI_STG__
#define __CVI_STG__
#include <pthread.h>
#include <limits.h>
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define CVI_STG_PATH_LEN_MAX (256)

#define PROC_FILE_PATH "/proc/cvi/cvi_info"
#define STG_DEVINFO_PROC_LINE_LEN (64)

#define STG_DEVINFO_PORT_INVALID "invalid"
#define STG_DEVINFO_STATE_UNPLUGGED "unplugged_disconnected"
#define STG_DEVINFO_STATE_UNPLUGGED_CONNECTED "unplugged"
#define STG_DEVINFO_STATE_CONNECTING "plugged_disconnected"
#define STG_DEVINFO_STATE_CONNECTED "plugged_connected"
#define STG_DEVINFO_VALUE_RESERVED "Reserved"
#define STG_DEVINFO_VALUE_PREFIX ":"
#define STG_PATH_LEN_MAX (PATH_MAX)

#define STG_FSTOOL_FSTYPE_ID_OFFSET_EXFAT (3)
#define STG_FSTOOL_FSTYPE_ID_OFFSET_VFAT (82)
#define STG_FSTOOL_FSTYPE_ID_LENGTH (8)
#define STG_FSTOOL_FSTYPE_ID_EXFAT "EXFAT   "
#define STG_FSTOOL_FSTYPE_ID_VFAT "FAT32   "
#define STG_FSTOOL_FSTYPE_EXFAT "exfat"
#define STG_FSTOOL_FSTYPE_VFAT "vfat"

typedef struct {
	uint64_t u64ClusterSize;   /**<DEV partition cluster size(unit bytes) */
	uint64_t u64TotalSize;     /**<DEV partition total space size(unit bytes) */
	uint64_t u64AvailableSize; /**<DEV partition free space size(unit bytes) */
	uint64_t u64UsedSize;      /**<DEV partition used space size(unit bytes) */
} STG_FS_INFO_S;

typedef enum tagSTG_DEV_STATE_E {
	STG_DEV_STATE_UNPLUGGED = 0x00,
	STG_DEV_STATE_CONNECTING,
	STG_DEV_STATE_CONNECTED,
	STG_DEV_STATE_IDLE
} STG_DEV_STATE_E;


typedef enum tagSTG_FS_TYPE_E {
	STG_FS_TYPE_FAT32 = 0x00,
	STG_FS_TYPE_EXFAT,
	STG_FS_TYPE_UNKNOWN
} CVI_STG_FS_TYPE_E;

typedef enum STORAGE_STATE_E {
	CVI_STG_STATE_DEV_UNPLUGGED = 0x00, /**<device already plugout */
	CVI_STG_STATE_DEV_CONNECTING,       /**<device connecting */
	CVI_STG_STATE_DEV_ERROR,            /**<sd card error */
	CVI_STG_STATE_FS_CHECKING,          /**<device doing fscheck */
	CVI_STG_STATE_FS_CHECK_FAILED,      /**<device fscheck failed */
	CVI_STG_STATE_FS_EXCEPTION,         /**<device file system exception */
	CVI_STG_STATE_MOUNTED,              /**<device mounted */
	CVI_STG_STATE_UNMOUNTED,            /**<device unmounted */
	CVI_STG_STATE_MOUNT_FAILED,         /**<device mount fail */
	CVI_STG_STATE_FORMATING,         	/**<device foramating */
	CVI_STG_STATE_FORMAT_SUCCESSED,     /**<device foramat successed */
	CVI_STG_STATE_FORMAT_FAILED,     	/**<device foramat failed */
	CVI_STG_STATE_IDLE                  /**init state */
} CVI_STG_STATE_E;

typedef struct {
	STG_DEV_STATE_E devState;
	CVI_STG_STATE_E stgState;
	char fsType[STG_PATH_LEN_MAX];
	CVI_STG_FS_TYPE_E fsType_e;
	char workMode[STG_DEVINFO_PROC_LINE_LEN];
	char aszDevPort[STG_DEVINFO_PROC_LINE_LEN];
	char speedClass[STG_DEVINFO_PROC_LINE_LEN];
	char speedGrade[STG_DEVINFO_PROC_LINE_LEN];
	char aszDevType[STG_DEVINFO_PROC_LINE_LEN];
	char aszDevPath[STG_PATH_LEN_MAX];
	char aszMntPath[STG_PATH_LEN_MAX];
} STG_DEVINFO_S;

typedef enum cviSTG_TRANSMISSION_SPEED_E {
    CVI_STG_TRANSMISSION_SPEED_1_4M = 0x00, /**1-4   MB/s */
    CVI_STG_TRANSMISSION_SPEED_4_10M,       /**4-10  MB/s */
    CVI_STG_TRANSMISSION_SPEED_10_30M,      /**10-30 MB/s */
    CVI_STG_TRANSMISSION_SPEED_30_50M,      /**30-50 MB/s */
    CVI_STG_TRANSMISSION_SPEED_50_100M,     /**50-100MB/s */
    CVI_STG_TRANSMISSION_SPEED_EXCEED_100M, /**100MB/s and faster */
    CVI_STG_TRANSMISSION_SPEED_BUTT         /***others**/
} CVI_STG_TRANSMISSION_SPEED_E;

typedef struct cviSTG_DEV_INFO_S {
    char aszDevType[CVI_STG_PATH_LEN_MAX];   /**device type,such as SD or MMC */
    char aszWorkMode[CVI_STG_PATH_LEN_MAX];  /**device work mode */
    CVI_STG_TRANSMISSION_SPEED_E enTranSpeed;   /**device transmission rate info */
} CVI_STG_DEV_INFO_S;

typedef void* CVI_STG_HANDLE_T;

#define STG_DEVINFO_HS "HS"
#define STG_DEVINFO_UHS "UHS"

#define STG_DEVINFO_UHS_SDR12 "SDR12"
#define STG_DEVINFO_UHS_SDR25 "SDR25"
#define STG_DEVINFO_UHS_SDR50 "SDR50"
#define STG_DEVINFO_UHS_DDR50 "DDR50"
#define STG_DEVINFO_UHS_SDR104 "SDR104"
#define STG_DEVINFO_UHS_DDR200 "DDR200"

#define STG_DEVINFO_SPEED_GRADE_LOW "Less than 10MB/sec(0h)"
#define STG_DEVINFO_SPEED_GRADE_HIGH "10MB/sec and above(1h)"

#define STG_DEVINFO_SPEED_CLASS_10 "Class 10"
#define STG_DEVINFO_SPEED_CLASS_6 "Class 6"
#define STG_DEVINFO_SPEED_CLASS_4 "Class 4"
#define STG_DEVINFO_SPEED_CLASS_2 "Class 2"

#define STG_DEVINFO_DEV_TYPE_LINE_NO (1)
#define STG_DEVINFO_WORK_MODE_LINE_NO (2)
#define STG_DEVINFO_SPEED_CLASS_LINE_NO (3)
#define STG_DEVINFO_SPEED_GRADE_LINE_NO (4)
#define STG_DEVINFO_PROC_LINES (5)

int CVI_STG_DeInit(CVI_STG_HANDLE_T stg_hdl);
int CVI_STG_Init(STG_DEVINFO_S *stgdev, CVI_STG_HANDLE_T *stg_hdl);
int CVI_STG_GetFsInfo(CVI_STG_HANDLE_T stg_hdl, STG_FS_INFO_S *fsinfo);
int CVI_STG_GetSDInfo(CVI_STG_HANDLE_T stg_hdl);
int CVI_STG_GetFsType(CVI_STG_HANDLE_T stg_hdl);
int CVI_STG_GetDevState(CVI_STG_HANDLE_T stg_hdl, STG_DEV_STATE_E *state);
int CVI_STG_Umount(CVI_STG_HANDLE_T stg_hdl);
int CVI_STG_Mount(CVI_STG_HANDLE_T stg_hdl);
int CVI_STG_Format(CVI_STG_HANDLE_T stg_hdl);
int CVI_STG_Format_with_Label(CVI_STG_HANDLE_T stg_hdl, char *labelname);
int STG_ReadProc(STG_DEVINFO_S *pstDevInfo);
int CVI_STG_GetInfo(CVI_STG_HANDLE_T stg_hdl, CVI_STG_DEV_INFO_S *pInfo);
int CVI_STG_RepairFAT32(CVI_STG_HANDLE_T stg_hdl);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif