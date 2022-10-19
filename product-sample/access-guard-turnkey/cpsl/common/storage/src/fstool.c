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
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#include "fstool.h"
#include "cvi_stg.h"
#include "cvi_sysutils.h"

#define FORMAT_TOOL_PATH "/bin/format.sh"
#if 0
static int32_t STG_FSTOOL_TestPartition(const char *pszPartition, const char *pszMountPath, const char *pszDevPath)
{
    int32_t s32Ret = 0;
    int32_t fd = 0;
    char aszTestFile[CVI_STG_PATH_LEN_MAX + 16] = { 0 };
    char aszTestData[STG_FSTOOL_TEST_PARTITION_DATA_LEN] = { 0 };


    (void)Stg_Fstool_Umount(pszDevPath);

    s32Ret = STG_FSTOOL_Mount(pszPartition, pszMountPath);
    if (0 != s32Ret) {
        printf("%s  %d  device(%s) is exception, system errno(%d)\n", __FUNCTION__, __LINE__,
                       pszPartitionPath, errno);
        return s32Ret;
    }

    s32Ret = snprintf_s(aszTestFile, CVI_STG_PATH_LEN_MAX + 16, CVI_STG_PATH_LEN_MAX + 16 - 1, "%s/%s",
                        pszMountPath, STG_FSTOOL_TEST_PARTITION_FILE_NAME);
    if (s32Ret < 0) {
        printf("aszTestFile snprintf_s failed!\n");
        return -1;
    }
    fd = open(aszTestFile, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, S_IRWXU);
    if (0 > fd) {
        (void)Stg_Fstool_Umount(pszDevPath);
        printf("%s  %d  open test file(%s) fail, system errno(%d)\n", __FUNCTION__, __LINE__, aszTestFile, errno);
        return -2;
    }

    s32Ret = memset(aszTestData, STG_FSTOOL_TEST_PARTITION_DATA_LEN, 0x00, STG_FSTOOL_TEST_PARTITION_DATA_LEN);
    if (EOK != s32Ret) {
        printf("aszTestData memset_s failed! s32Ret = 0x%x\n", s32Ret);
        s32Ret = -3;
    }

    if (0 > write(fd, aszTestData, STG_FSTOOL_TEST_PARTITION_DATA_LEN)) {
        if (errno != ENOSPC) {
            printf("%s  %d  write test file(%s) fail, system errno(%d)!\n", __FUNCTION__, __LINE__,
                           pszPartitionPath, errno);
            s32Ret = -4;
        } else {
            // space is full not return erron
            printf("%s  %d  write test file(%s) fail, system errno(%d) for space is full!\n", __FUNCTION__, __LINE__,
                           pszPartitionPath, errno);
        }
    }

    if (0 != close(fd)) {
        printf("%s  %d  close file failure, system errno(%d)\n",
                       __FUNCTION__, __LINE__, errno);
        s32Ret = -4;
    }

    if (0 != remove(aszTestFile)) {
        printf("%s  %d  device(%s) is exception, system errno(%d)\n", __FUNCTION__, __LINE__,
                       pstFSTool->aszPartitionPath, errno);
        s32Ret = -5;
    }

    (void)Stg_Fstool_Umount(pszDevPath);
    return s32Ret;
}
#endif
#define RETRY_TIMES 3
static int Stg_Fstool_ReadPartition(const char *pszPartition, off_t offset,
				    size_t nmemb, char *pFsType)
{
	char *pRealPath = NULL;
	char aszRealPath[STG_PATH_LEN_MAX + 1] = {0};
	FILE *pFile = NULL;
	int i = 0;
	pRealPath = realpath(pszPartition, aszRealPath);
	if (NULL == pRealPath) {
		printf("%s  %d  real path(%s) failed, system errno(%d)!\n",
		       __FUNCTION__, __LINE__, pszPartition, errno);
		for(i = 0; i < RETRY_TIMES; i ++) {
			usleep(20 * 1000);

			pRealPath = realpath(pszPartition, aszRealPath);
			printf("%s  %d  real path(%s) failed, system errno(%d)! retry %d\n",
		       __FUNCTION__, __LINE__, pszPartition, errno, i);
			if(NULL != pRealPath) {
				break;
			}
		}
		if( i >= RETRY_TIMES)
			return -1;
	}
	pFile = fopen(aszRealPath, "r");
	if (NULL == pFile) {
		printf("%s  %d read path error, system errno(%d)\n", __FUNCTION__,
		       __LINE__, errno);
		return -2;
	}
	if (0 != fseeko(pFile, offset, SEEK_SET) ||
	    nmemb != fread(pFsType, 1, nmemb, pFile)) {
		printf("%s  %d  read file(%s) failed , system errno(%d)!\n",
		       __FUNCTION__, __LINE__, aszRealPath, errno);
	} else {
		pFsType[nmemb] = '\0';
	}

	if (0 != fclose(pFile)) {
		printf("%s  %d  read file(%s) failed , system errno(%d)!\n",
		       __FUNCTION__, __LINE__, aszRealPath, errno);
		return -1;
	}
	return 0;
}

static int Stg_Fstool_IsExFat(const char *pszBootSector, uint32_t bootSecLen)
{
	if (bootSecLen <
	    (STG_FSTOOL_FSTYPE_ID_OFFSET_EXFAT + STG_FSTOOL_FSTYPE_ID_LENGTH)) {
		return -1;
	}
	if (strncmp(pszBootSector + STG_FSTOOL_FSTYPE_ID_OFFSET_EXFAT,
		    STG_FSTOOL_FSTYPE_ID_EXFAT, STG_FSTOOL_FSTYPE_ID_LENGTH) == 0) {
		return 0;
	}
	return -1;
}

static int Stg_Fstool_IsFat32(const char *pszBootSector, uint32_t len)
{
	// if 1 condition of 2 is agree,then think it is fat32 system.
	int s32Ret = 0;
	int s32Index = 0;
	uint32_t fat32FileTypeOffeset = 82;
	if (len < STG_FSTOOL_FAT_BOOT_SEC_LEN) {
		printf("boot sector len is short\n");
		return -1;
	}
	// check BS_FilSysType equals "FAT32   ",if equals then return;
	char aszBSFilSysType[8] = {0x46, 0x41, 0x54, 0x33, 0x32, 0x20, 0x20, 0x20};
	for (s32Index = 0; s32Index < 8; s32Index++) {
		if (aszBSFilSysType[s32Index] !=
		    pszBootSector[s32Index + fat32FileTypeOffeset]) {
			break;
		}
	}
	if (s32Index == 8) {
		return 0;
	}

	// accord to fat32 check bpb_bkbootsec and bpb reserved,if equal also judge
	// fat32 system;
	uint32_t fat32BPBBkBootSec = 50;
	char aszFat32Identify[14] = {0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				    };
	for (s32Index = 0; s32Index < 14; s32Index++) {
		if (aszFat32Identify[s32Index] !=
		    pszBootSector[s32Index + fat32BPBBkBootSec]) {
			s32Ret = -1;
			break;
		}
	}
	return s32Ret;
}

int Stg_FsTool_GetFsType(const char *pszPartition, char *pszFSType,
			 uint32_t typeLen)
{
	int s32Ret = 0;
	char aszBootSector[STG_FSTOOL_FAT_BOOT_SEC_LEN + 1] = {0};
	s32Ret = Stg_Fstool_ReadPartition(
			 pszPartition, 0, STG_FSTOOL_FAT_BOOT_SEC_LEN, aszBootSector);
	if (0 != s32Ret) {
		printf("%s  %d  get fs type of partition(%s) failed , and ret:%x!\n",
		       __FUNCTION__, __LINE__, pszPartition, s32Ret);
		return s32Ret;
	}

	s32Ret = Stg_Fstool_IsFat32(aszBootSector, STG_FSTOOL_FAT_BOOT_SEC_LEN + 1);
	if (s32Ret == 0) {
		s32Ret = snprintf(pszFSType, typeLen, "%s", STG_FSTOOL_FSTYPE_VFAT);
		if (s32Ret < 0) {
			printf("%s  %d  snprintf failed,and ret:%x!\n", __FUNCTION__,
			       __LINE__, s32Ret);
			return -1;
		}
		return 0;
	}

	s32Ret = Stg_Fstool_IsExFat(aszBootSector, STG_FSTOOL_FAT_BOOT_SEC_LEN + 1);
	if (s32Ret == 0) {
		s32Ret = snprintf(pszFSType, typeLen, "%s", STG_FSTOOL_FSTYPE_EXFAT);
		if (s32Ret < 0) {
			printf("%s  %d  snprintf failed,and ret:%x!\n", __FUNCTION__,
			       __LINE__, s32Ret);
			return -1;
		}
		return 1;
	}

	// get type from other  file system support should add here,and fill
	// pszFSType for mount and format ops.
	printf("%s  %d  unknow fs type of partition(%s)\n", __FUNCTION__, __LINE__,
	       pszPartition);
	return -2;
}

int Stg_Fstool_Mount(const char *pszPartition, const char *pszMountPath)
{
	int s32Ret = 0;
	char *pRealPath = NULL;
	char aszFSType[STG_PATH_LEN_MAX] = {0};
	char aszRealPath[STG_PATH_LEN_MAX + 1] = {0};

	pRealPath = realpath(pszMountPath, aszRealPath);
	if (NULL == pRealPath) {
		printf("%s  %d  real path(%s) failed, system errno(%d)!\n",
		       __FUNCTION__, __LINE__, pszMountPath, errno);
		if (0 != mkdir(pszMountPath, 0755)) {
			printf("mkdir %s failed\n", pszMountPath);
		}
	}
	s32Ret = Stg_FsTool_GetFsType(pszPartition, aszFSType, STG_PATH_LEN_MAX);
	if (0 != s32Ret) {
		printf("%s  %d  do mount(%s) failed, and ret:%x!\n", __FUNCTION__,
		       __LINE__, pszPartition, s32Ret);
		return -1;
	}

	if (0 != mount(pszPartition, pszMountPath, aszFSType, MS_NOATIME|MS_NODIRATIME, 0)) {
		printf("%s  %d  mount %s on %s failureerrno(%d)!\n", __FUNCTION__,
		       __LINE__, pszPartition, pszMountPath, errno);
		return -1;
	}

	return 0;
}

int Stg_Fstool_GetFsInfo(const char *pszMntPath, STG_FS_INFO_S *pstInfo)
{
	struct statfs stFSbuf = {0};
	int ret = 0;

	ret = statfs(pszMntPath, &stFSbuf);
	if (ret != 0) {
		printf("get sd card fsinfo failed %s \n", pszMntPath);
		return ret;
	}

	pstInfo->u64ClusterSize = (uint64_t)stFSbuf.f_bsize;
	pstInfo->u64TotalSize = (uint64_t)(stFSbuf.f_bsize * stFSbuf.f_blocks);
	pstInfo->u64AvailableSize = (uint64_t)(stFSbuf.f_bsize * stFSbuf.f_bavail);
	pstInfo->u64UsedSize =
		(uint64_t)(pstInfo->u64TotalSize - pstInfo->u64AvailableSize);
	return ret;
}

static int Stg_Fstool_CheckMMCBlkPath(const char *partionPath)
{
	char aszSrcPath[STG_PATH_LEN_MAX + 1] = {0};
	char aszBlkPath[STG_PATH_LEN_MAX + 1] = {0};
	uint32_t i = 0;
	size_t partionLen = strlen(partionPath);
	for (i = 0; i < (partionLen - 1); i++) {
		// /dev/mmcblk0p1
		if (*(partionPath + i) == 'p' &&
		    (*(partionPath + i + 1) > '0' && *(partionPath + i + 1) <= '9')) {
			break;
		}
	}

	if ((i <= 0) || (i >= (partionLen - 1))) {
		printf("cannot parse blk path\n");
		return -1;
	}
	strncpy(aszSrcPath, partionPath, i);

	if (!realpath(aszSrcPath, aszBlkPath)) {
		printf("dev path (%s) is not exist\n", aszSrcPath);
		return -1;
	}

	return 0;
}

int Stg_Fstool_DetectPartition(const char *pszPartition)
{
	char *pRealPath = NULL;
	char aszRealPath[STG_PATH_LEN_MAX + 1] = {0};
	uint32_t u32DetectCnt = 0;

	while (u32DetectCnt < STG_FSTOOL_PARTITION_DETECT_CNT) {
		pRealPath = realpath(pszPartition, aszRealPath);
		if (NULL == pRealPath) {
			u32DetectCnt++;
			usleep(STG_FSTOOL_PARTITION_DETECT_INTERVAL);
			continue;
		} else {
			break;
		}
	}
	if (pRealPath != NULL) {
		return 0;
	}
	if (Stg_Fstool_CheckMMCBlkPath(pszPartition) == 0) {
		printf("%s %d dev block exist, partion %s not exist!\n", __FUNCTION__,
		       __LINE__, pszPartition);
		return -2;
	} else {
		printf("%s %d partion %s and dev block not exist!\n", __FUNCTION__,
		       __LINE__, pszPartition);
		return -3;
	}
}

int Stg_Fstool_Umount(const char *pszDevPath)
{
	if (0 != umount(pszDevPath)) {
		printf("%s  %d  umount %s failure, errno(%d)!\n", __FUNCTION__,
		       __LINE__, pszDevPath, errno);
		return -1;
	}

	return 0;
}

// /dev/mmcblk1p0
int Stg_Fstool_Format(const char *pszDevPath)
{
	char blkname[STG_PATH_LEN_MAX + 1] = {0};
	char *pRealPath = NULL;
	char aszRealPath[STG_PATH_LEN_MAX + 1] = {0};
	char cmd[STG_PATH_LEN_MAX] = {0};
	snprintf(blkname, 13, "%s", pszDevPath);
	pRealPath = realpath(blkname, aszRealPath);
	if (NULL == pRealPath) {
		return -1;
	}

	sprintf(cmd, "%s --device %s ", FORMAT_TOOL_PATH, pRealPath);
	printf("%s", cmd);
	return cvi_system(cmd);
}

int Stg_Fstool_Format_with_Label(const char *pszDevPath, char *labelname)
{
	char blkname[STG_PATH_LEN_MAX + 1] = {0};
	char *pRealPath = NULL;
	char aszRealPath[STG_PATH_LEN_MAX + 1] = {0};
	char cmd[STG_PATH_LEN_MAX] = {0};
	snprintf(blkname, 13, "%s", pszDevPath);
	pRealPath = realpath(blkname, aszRealPath);
	if (NULL == pRealPath) {
		return -1;
	}
	sprintf(cmd, "%s --device %s --label %s ", FORMAT_TOOL_PATH, pRealPath,
		labelname);
	printf("%s", cmd);
	return cvi_system(cmd);
}
