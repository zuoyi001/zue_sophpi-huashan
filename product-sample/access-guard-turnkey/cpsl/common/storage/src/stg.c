#include <errno.h>
#include <fcntl.h>
#include <limits.h>
//#include <linux/fs.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <unistd.h>

#include "fstool.h"
#include "cvi_sysutils.h"
#include "cvi_stg.h"

pthread_mutex_t mDevProcLock;

static void STG_DEVINFO_ClassTypeLevel(const char *speedClass, CVI_STG_TRANSMISSION_SPEED_E *speed)
{
	if ((strstr(speedClass, STG_DEVINFO_SPEED_CLASS_10) != NULL)) {
		*speed = CVI_STG_TRANSMISSION_SPEED_10_30M;
	} else if (strstr(speedClass, STG_DEVINFO_SPEED_CLASS_6) != NULL) {
		*speed = CVI_STG_TRANSMISSION_SPEED_4_10M;
	} else if ((strstr(speedClass, STG_DEVINFO_SPEED_CLASS_4) != NULL) ||
		   (strstr(speedClass, STG_DEVINFO_SPEED_CLASS_2) != NULL)) {
		*speed = CVI_STG_TRANSMISSION_SPEED_1_4M;
	} else {
		printf("%s %d speedClass not recongize!\n", __FUNCTION__, __LINE__);
		*speed = CVI_STG_TRANSMISSION_SPEED_BUTT;
	}
	return;
}

static void STG_DEVINFO_WorkModeLevel(const char *workMode, CVI_STG_TRANSMISSION_SPEED_E *speed)
{
	if ((strstr(workMode, STG_DEVINFO_HS) != NULL) &&
	    (strstr(workMode, STG_DEVINFO_UHS) == NULL)) {
		*speed = CVI_STG_TRANSMISSION_SPEED_1_4M;
		return;
	}

	if (strstr(workMode, STG_DEVINFO_UHS_SDR12) != NULL) {
		*speed = CVI_STG_TRANSMISSION_SPEED_1_4M;
	} else if (strstr(workMode, STG_DEVINFO_UHS_SDR25) != NULL) {
		*speed = CVI_STG_TRANSMISSION_SPEED_4_10M;
	} else if ((strstr(workMode, STG_DEVINFO_UHS_SDR50) != NULL) ||
		   (strstr(workMode, STG_DEVINFO_UHS_DDR50) != NULL) ||
		   (strstr(workMode, STG_DEVINFO_UHS_SDR104) != NULL)) {
		*speed = CVI_STG_TRANSMISSION_SPEED_10_30M;
	} else if (strstr(workMode, STG_DEVINFO_UHS_DDR200) != NULL) {
		*speed = CVI_STG_TRANSMISSION_SPEED_30_50M;
	} else {
		printf("%s  %d workmode not recongize!\n", __FUNCTION__, __LINE__);
		*speed = CVI_STG_TRANSMISSION_SPEED_BUTT;
	}
	return;
}

static void STG_DEVINFO_SpeedGradeLevel(const char *speedGrade, CVI_STG_TRANSMISSION_SPEED_E *speed)
{
	if ((strstr(speedGrade, STG_DEVINFO_SPEED_GRADE_HIGH) != NULL)) {
		*speed = CVI_STG_TRANSMISSION_SPEED_10_30M;
	} else if (strstr(speedGrade, STG_DEVINFO_SPEED_GRADE_LOW) != NULL) {
		*speed = CVI_STG_TRANSMISSION_SPEED_4_10M;
	} else {
		printf("%s %d speedGrade not recongize!\n", __FUNCTION__, __LINE__);
		*speed = CVI_STG_TRANSMISSION_SPEED_BUTT;
	}
	return;
}

static void STG_DEVINFO_CalcTransSpeed(const STG_DEVINFO_S *devInfo, CVI_STG_TRANSMISSION_SPEED_E *speed)
{
    CVI_STG_TRANSMISSION_SPEED_E enBaseSpeed;
    CVI_STG_TRANSMISSION_SPEED_E enSpeed;
    STG_DEVINFO_WorkModeLevel(devInfo->workMode, &enBaseSpeed);
    STG_DEVINFO_ClassTypeLevel(devInfo->speedClass, &enSpeed);
	//switch the max value both two num
	if (enSpeed != CVI_STG_TRANSMISSION_SPEED_BUTT && enBaseSpeed != CVI_STG_TRANSMISSION_SPEED_BUTT) {
		enBaseSpeed = enSpeed > enBaseSpeed ? enSpeed : enBaseSpeed;
	} else {
		enBaseSpeed = enSpeed > enBaseSpeed ? enBaseSpeed : enSpeed;
	}
    STG_DEVINFO_SpeedGradeLevel(devInfo->speedGrade, &enSpeed);
	if (enSpeed != CVI_STG_TRANSMISSION_SPEED_BUTT && enBaseSpeed != CVI_STG_TRANSMISSION_SPEED_BUTT) {
		enBaseSpeed = enSpeed > enBaseSpeed ? enSpeed : enBaseSpeed;
	} else {
		enBaseSpeed = enSpeed > enBaseSpeed ? enBaseSpeed : enSpeed;
	}
    *speed = enBaseSpeed;
}

static int32_t STG_DEVINFO_Route(uint32_t procLineNo, char *value,
				 STG_DEVINFO_S *pstInfo)
{
	int32_t s32FmtCount = 0;
	char *end = NULL;
	switch (procLineNo) {
	case STG_DEVINFO_DEV_TYPE_LINE_NO:
		end = strchr(value, ' ');
		if (end != NULL && (end - value) > 0) {
			strncpy(pstInfo->aszDevType, value, (end - value));
		}
		break;
	case STG_DEVINFO_WORK_MODE_LINE_NO:
		s32FmtCount = snprintf(pstInfo->workMode, STG_DEVINFO_PROC_LINE_LEN,
				       "%s", value);
		break;
	case STG_DEVINFO_SPEED_CLASS_LINE_NO:
		s32FmtCount = snprintf(pstInfo->speedClass,
				       STG_DEVINFO_PROC_LINE_LEN, "%s", value);
		break;
	case STG_DEVINFO_SPEED_GRADE_LINE_NO:
		s32FmtCount = snprintf(pstInfo->speedGrade,
				       STG_DEVINFO_PROC_LINE_LEN, "%s", value);
		break;
	default:
		break;
	}

	if (s32FmtCount < 0) {
		printf("snprintf failed, procLineNo = %d\n", procLineNo);
		return -1;
	}

	return 0;
}

int STG_ReadProc(STG_DEVINFO_S *pstDevInfo)
{
	uint8_t u8LineNo = 0;
	int32_t s32Ret = 0;
	FILE *pFDevProc = NULL;
	size_t szProcLen = 0;
	char *pTmp = NULL;
	char aszProcLineStr[STG_DEVINFO_PROC_LINE_LEN] = {0};

	// STG_THREAD_MUTEX_LOCK(&gReadMciInfoLock);
	pthread_mutex_lock(&mDevProcLock);
	pFDevProc = fopen(PROC_FILE_PATH, "r");
	if (NULL == pFDevProc) {
		(void)pthread_mutex_unlock(&mDevProcLock);
		printf("%s  %d  open file failure, system errno(%d)!\n",
		       __FUNCTION__, __LINE__, errno);
		return -1;
	}

	while (NULL !=
	       fgets(aszProcLineStr, STG_DEVINFO_PROC_LINE_LEN - 1, pFDevProc)) {
		pTmp = strstr(aszProcLineStr, pstDevInfo->aszDevPort);
		if (pTmp) {
			break;
		}
	}

	if (!pTmp) {
		s32Ret = -2;
		printf("%s  %d  dev port is not found,!\n", __FUNCTION__, __LINE__);
		goto CLOSE_FILE;
	}

	if (strstr(aszProcLineStr, STG_DEVINFO_STATE_UNPLUGGED_CONNECTED)) {
		pstDevInfo->devState = STG_DEV_STATE_UNPLUGGED;
	} else if (strstr(aszProcLineStr, STG_DEVINFO_STATE_CONNECTED)) {
		pstDevInfo->devState = STG_DEV_STATE_CONNECTED;
		for (u8LineNo = 1; u8LineNo < STG_DEVINFO_PROC_LINES; u8LineNo++) {
			if (NULL == fgets(aszProcLineStr, STG_DEVINFO_PROC_LINE_LEN - 1,
					  pFDevProc)) {
				s32Ret = -1;
				printf("%s %d read file failure,system errno(%d)!\n",
				       __FUNCTION__, __LINE__, errno);
				goto CLOSE_FILE;
			}

			szProcLen = strlen(aszProcLineStr);
			if (szProcLen >= 1 && (szProcLen - 1) < STG_DEVINFO_PROC_LINE_LEN) {
				aszProcLineStr[szProcLen - 1] = '\0';
			} else {
				aszProcLineStr[0] = '\0';
			}
			pTmp = strstr(aszProcLineStr, STG_DEVINFO_VALUE_PREFIX);
			if (pTmp && (pTmp = pTmp + 2)) {  // case ": " has occupied two Bit
				s32Ret = STG_DEVINFO_Route(u8LineNo, pTmp, pstDevInfo);
				if (0 != s32Ret) {
					goto CLOSE_FILE;
				}
			}
		}
	} else if (strstr(aszProcLineStr, STG_DEVINFO_STATE_UNPLUGGED)) {
		pstDevInfo->devState = STG_DEV_STATE_UNPLUGGED;
	} else {
		pstDevInfo->devState = STG_DEV_STATE_CONNECTING;
	}

CLOSE_FILE:
	if (0 != fclose(pFDevProc)) {
		printf("%s  %d  close file failure, system errno(%d)!\n", __FUNCTION__,
		       __LINE__, errno);
	}
	(void)pthread_mutex_unlock(&mDevProcLock);
	return s32Ret;
}

int CVI_STG_Format(CVI_STG_HANDLE_T stg_hdl)
{
	STG_DEVINFO_S *stgdev;

	stgdev = (STG_DEVINFO_S *)stg_hdl;
	return Stg_Fstool_Format(stgdev->aszDevPath);
}

int CVI_STG_Format_with_Label(CVI_STG_HANDLE_T stg_hdl, char *labelname)
{
	STG_DEVINFO_S *stgdev;

	stgdev = (STG_DEVINFO_S *)stg_hdl;
	return Stg_Fstool_Format_with_Label(stgdev->aszDevPath, labelname);
}

int CVI_STG_Mount(CVI_STG_HANDLE_T stg_hdl)
{
	STG_DEVINFO_S *stgdev;
	int ret = 0;
	stgdev = (STG_DEVINFO_S *)stg_hdl;
	//if (stgdev->stgState != CVI_STG_STATE_MOUNTED) {
		ret = Stg_Fstool_Umount(stgdev->aszMntPath);
		if (ret != 0) {
			printf("umount failed\n");
		}
		ret = Stg_Fstool_Mount(stgdev->aszDevPath, stgdev->aszMntPath);
		if (ret == 0) {
			stgdev->devState = CVI_STG_STATE_MOUNTED;
		}
	//}
	return ret;
}

int CVI_STG_Umount(CVI_STG_HANDLE_T stg_hdl)
{
	STG_DEVINFO_S *stgdev;
	int ret = 0;
	stgdev = (STG_DEVINFO_S *)stg_hdl;
	if (stgdev->stgState == CVI_STG_STATE_MOUNTED) {
		ret = Stg_Fstool_Umount(stgdev->aszMntPath);
		if (ret == 0) {
			stgdev->devState = CVI_STG_STATE_UNMOUNTED;
		}
	} else {
		ret = -1;
	}
	return ret;
}

int CVI_STG_GetDevState(CVI_STG_HANDLE_T stg_hdl, STG_DEV_STATE_E *state)
{
	STG_DEVINFO_S *stgdev;

	stgdev = (STG_DEVINFO_S *)stg_hdl;
	*state = stgdev->devState;
	return 0;
}

int CVI_STG_GetFsInfo(CVI_STG_HANDLE_T stg_hdl, STG_FS_INFO_S *fsinfo)
{
	STG_DEVINFO_S *stgdev;

	stgdev = (STG_DEVINFO_S *)stg_hdl;
	return Stg_Fstool_GetFsInfo(stgdev->aszMntPath, fsinfo);
}
// caution call this function when sd is umount
int CVI_STG_RepairFAT32(CVI_STG_HANDLE_T stg_hdl)
{
	STG_DEVINFO_S *stgdev;
	int ret = 0;
	char cmd[50] = {0};
	stgdev = (STG_DEVINFO_S *)stg_hdl;
	if (stgdev->stgState != CVI_STG_STATE_MOUNTED) {
		snprintf(cmd, sizeof(cmd), "fsck.fat -a %s", stgdev->aszDevPath);
		ret = cvi_system(cmd);
		if (ret == -1) {
			printf("repair file system failed ret:%d cmd:%s\n", ret, cmd);
		}
	}
	return ret;
}

int CVI_STG_GetFsType(CVI_STG_HANDLE_T stg_hdl)
{
	STG_DEVINFO_S *stgdev;
	int ret = 0;
	stgdev = (STG_DEVINFO_S *)stg_hdl;
	ret = Stg_FsTool_GetFsType(stgdev->aszDevPath, stgdev->fsType,
				    STG_PATH_LEN_MAX);
	if(ret == 0) {
		stgdev->fsType_e = STG_FS_TYPE_FAT32;
	} else if(ret == 1) {
		stgdev->fsType_e = STG_FS_TYPE_EXFAT;
	} else {
		stgdev->fsType_e = STG_FS_TYPE_UNKNOWN;
	}
	return ret;
}

int CVI_STG_GetSDInfo(CVI_STG_HANDLE_T stg_hdl)
{
	STG_DEVINFO_S *stgdev;

	stgdev = (STG_DEVINFO_S *)stg_hdl;
	return STG_ReadProc(stgdev);
}

int CVI_STG_GetInfo(CVI_STG_HANDLE_T stg_hdl, CVI_STG_DEV_INFO_S *pInfo)
{
    STG_DEVINFO_S *pstDevInfo = (STG_DEVINFO_S *)stg_hdl;
    int s32Ret = 0;

    s32Ret = STG_ReadProc(pstDevInfo);
    if (0 != s32Ret) {
        printf("%s  %d  read dev info failure, and ret:%x!\n", __FUNCTION__,
                       __LINE__, s32Ret);
        return s32Ret;
    }

    if (STG_DEV_STATE_CONNECTED != pstDevInfo->devState) {
        printf("%s  %d  get info failure, and stg disable!\n", __FUNCTION__, __LINE__);
        return -1;
    }

    STG_DEVINFO_CalcTransSpeed(pstDevInfo, &pInfo->enTranSpeed);
    s32Ret = snprintf(pInfo->aszWorkMode, CVI_STG_PATH_LEN_MAX, "%s", pstDevInfo->workMode);
    if (s32Ret < 0) {
        return -1;
    }
    s32Ret = snprintf(pInfo->aszDevType, CVI_STG_PATH_LEN_MAX, "%s", pstDevInfo->aszDevType);
    if (s32Ret < 0) {
        return -1;
    }

    return 0;
}

int CVI_STG_Init(STG_DEVINFO_S *stgdev, CVI_STG_HANDLE_T *stg_hdl)
{
	*stg_hdl = stgdev;
	pthread_mutex_init(&mDevProcLock, NULL);
	return 0;
}

int CVI_STG_DeInit(CVI_STG_HANDLE_T stg_hdl)
{
	STG_DEVINFO_S *stgdev;

	stgdev = (STG_DEVINFO_S *)stg_hdl;
	if (!stgdev) {
		pthread_mutex_destroy(&mDevProcLock);
	}
	stg_hdl = NULL;
	return 0;
}
