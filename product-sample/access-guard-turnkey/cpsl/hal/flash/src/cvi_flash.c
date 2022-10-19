#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "cvi_flash.h"
#include "nand.h"
#include "spi_raw.h"
#include "nand_raw.h"
#include "emmc_raw.h"
#include "cvi_log.h"
//#include "cvi_appcomm_log.h"

#define MAX_HANDLE		 MAX_PARTS /* Flash max handle number */
#define MAX_BOOTARGS_LEN 1024	   /* The max length of bootargs */

/* Expand hiflash handle fd, only CVI_Flash_OpenByTypeAndName() use it */
#define SPAN_PART_HANDLE	  1000

/** Flash Information */
struct CVI_FLASH_INTER_INFO_S {
	CVI_U64 TotalSize;				 /**< flash total size */
	CVI_U64 PartSize;				 /**< flash partition size */
	CVI_U32 BlockSize;				 /**< flash block size */
	CVI_U32 PageSize;				 /**< flash page size */
	CVI_U32 OobSize;				 /**< flash OOB size */
	CVI_VOID *fd;					 /**< file handle */
	CVI_U64 OpenAddr;				 /**< flash open address */
	CVI_U64 OpenLeng;				 /**< flash open length */
	CVI_FLASH_TYPE_E FlashType;		 /**< flash type */
	struct FLASH_OPT_S *pFlashopt;			/**< operation callbacks on this flash */
	struct CVI_Flash_PartInfo_S *pPartInfo; /**< parition descriptions on this flash */
};

enum CVI_FLASH_DEV_STAT {
	CVI_FLASH_STAT_INSTALL,
	CVI_FLASH_STAT_UNINSTALL,
	/*lint -save -e749*/
	CVI_FLASH_STAT_BUTT
};

static struct CVI_FLASH_INTER_INFO_S gFlashInfo[MAX_HANDLE];
static struct CVI_Flash_PartInfo_S gPartInfo[MAX_PARTS];
static CVI_BOOL gInitFlag = CVI_FALSE;
static CVI_BOOL gInitMutex = CVI_FALSE;
static CVI_U8 g_au8Bootargs[MAX_BOOTARGS_LEN];

static CVI_CHAR *pszPos[CVI_FLASH_TYPE_BUTT];
static enum CVI_FLASH_DEV_STAT g_eDevStat[CVI_FLASH_TYPE_BUTT];
static struct FLASH_OPT_S g_stFlashOpt[CVI_FLASH_TYPE_BUTT];

CVI_U32 check_flash_init(CVI_HANDLE hFlash)
{
	if (!gInitFlag) {
		printf("NOT init yet!\n");
		return CVI_FAILURE;
	}
	if (hFlash >= MAX_HANDLE) {
		return CVI_FAILURE;
	}
	if (gFlashInfo[hFlash].fd == (CVI_VOID *)INVALID_FD) {
		return CVI_FAILURE;
	}
	return 0;
}

CVI_U32 check_addr_len_valid(CVI_U64 Address, CVI_U64 Len, CVI_U64 LimitLeng)
{
	if ((Address >= LimitLeng) || ((Address + Len) > LimitLeng)) {
		return CVI_FAILURE;
	}
	return 0;
}

static CVI_S8 *skip_space(CVI_S8 *line)
{
	CVI_S8 *p = line;
	while (*p == ' ' || *p == '\t') {
		p++;
	}
	return p;
}

static CVI_S8 *skip_word(CVI_S8 *line)
{
	CVI_S8 *p = line;

	while (*p != '\t' && *p != ' ' && *p != '\n' && *p != 0) {
		p++;
	}

	return p;
}

static CVI_S8 *get_word(CVI_S8 *line, CVI_S8 *value)
{
	CVI_S8 *p = line;

	p = skip_space(p);
	while (*p != '\t' && *p != ' ' && *p != '\n' && *p != 0) {
		*value++ = *p++;
	}

	*value = 0;
	return p;
}

static CVI_FLASH_TYPE_E get_flashtype_by_bootargs(CVI_CHAR *pszPartitionName)
{
	CVI_CHAR *pszPartitionPos = CVI_NULL;
	CVI_CHAR *pszTmpPos = CVI_NULL;
	CVI_CHAR pszTmpStr[64] = { 0 };
	CVI_U32 i;
	CVI_FLASH_TYPE_E enFlashType = CVI_FLASH_TYPE_BUTT;

	if (pszPartitionName == CVI_NULL) {
		return CVI_FLASH_TYPE_BUTT;
	}

	if (strchr(pszPartitionName, ' ')) {
		printf("Invalid pszPartitionName, should not include ' '.\n");
		return CVI_FLASH_TYPE_BUTT;
	}

	memset(pszTmpStr, 0, sizeof(pszTmpStr));
	(CVI_VOID)snprintf(pszTmpStr, sizeof(pszTmpStr) - 1, "(%s)", pszPartitionName);
	pszPartitionPos = strstr((CVI_CHAR *)g_au8Bootargs, pszTmpStr);

	if (pszPartitionPos == CVI_NULL) {
		return CVI_FLASH_TYPE_BUTT;
	}

	for (i = 0; i < CVI_FLASH_TYPE_BUTT; i++) {
		if (pszPos[i] == CVI_NULL) {
			continue;
		}

		/* pszTmpPos is used to be a cursor */
		/*lint -save -e613*/
		if ((pszPartitionPos >= pszPos[i]) && (pszPos[i] >= pszTmpPos)) {
			enFlashType = (CVI_FLASH_TYPE_E)i;
			pszTmpPos = pszPos[i];
		}
	}

	return enFlashType;
}

CVI_S32 permission_check(CVI_FLASH_TYPE_E enFlashType,
						CVI_U64 u64StartAddress,
						CVI_U64 u64Len)
{
	CVI_U64 u64EndAddress = u64StartAddress + u64Len - 1;
	CVI_U32 i;

	for (i = 0; i < MAX_PARTS; i++) {
		if (gPartInfo[i].FlashType != enFlashType) {
			continue;
		}

		if (gPartInfo[i].perm != ACCESS_NONE) {
			continue;
		}

		if ((gPartInfo[i].StartAddr >= u64StartAddress) && (gPartInfo[i].StartAddr <= u64EndAddress)) {
			printf("%s(%s) is not permitted to be opened.\n", gPartInfo[i].DevName, gPartInfo[i].PartName);
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 Flash_Init(CVI_VOID)
{
	CVI_CHAR line[512];
	FILE *fp = CVI_NULL;
	CVI_U32 i = 0;
	CVI_U64 u64StartAddr[CVI_FLASH_TYPE_BUTT];
	CVI_S32 ret = CVI_SUCCESS;

	for (i = 0; i < MAX_PARTS; i++) {
		gPartInfo[i].StartAddr = 0;
		gPartInfo[i].PartSize = 0;
		gPartInfo[i].BlockSize = 0;
		gPartInfo[i].FlashType = CVI_FLASH_TYPE_BUTT;
		gPartInfo[i].perm = ACCESS_BUTT;
		memset(gPartInfo[i].DevName, '\0', FLASH_NAME_LEN);
		memset(gPartInfo[i].PartName, '\0', FLASH_NAME_LEN);
	}

	for (i = 0; i < MAX_HANDLE; i++) {
		//(CVI_VOID)pthread_mutex_lock(&gFlashMutex);
		gFlashInfo[i].fd = (CVI_VOID *)INVALID_FD;
		gFlashInfo[i].OpenAddr = 0;
		gFlashInfo[i].OpenLeng = 0;
		gFlashInfo[i].pPartInfo = NULL;
		gFlashInfo[i].FlashType = CVI_FLASH_TYPE_BUTT;
		//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
	}

	if ((g_eDevStat[CVI_FLASH_TYPE_SPI_0] == CVI_FLASH_STAT_INSTALL)
		|| (g_eDevStat[CVI_FLASH_TYPE_NAND_0]) == CVI_FLASH_STAT_INSTALL) {
		fp = fopen("/proc/mtd", "r");

		if (fp) {
			if (fgets(line, sizeof(line), fp) == NULL) { // skip first line
				fclose(fp);
				return CVI_FAILURE;
			}

			for (i = 0; i < CVI_FLASH_TYPE_BUTT; i++) {
				u64StartAddr[i] = 0;
			}

			for (i = 0; fgets(line, sizeof(line), fp) != 0; i++) {
				CVI_S8 argv[4][32];
				CVI_S8 *p;
				CVI_S32 fd = -1;

				p = (CVI_S8 *)&line[0];

				p = skip_space(p);
				p = skip_word(p);
				p = get_word(p, argv[1]);
				p = get_word(p, argv[2]);
				p = get_word(p, argv[3]);

				if (i >= MAX_PARTS) {
					printf("Detected there has more than %d partitions.\n"
							 "You should encrease MAX_PARTS in order to use left partitions!\n",
							 MAX_PARTS);
					break;
				}

				gPartInfo[i].PartSize = (CVI_U64)(CVI_S64)strtoull((const CVI_CHAR *)argv[1],
					(CVI_CHAR **)NULL, 16);
				gPartInfo[i].BlockSize = (CVI_U64)(CVI_S64)strtol((const CVI_CHAR *)argv[2],
						(CVI_CHAR **)NULL, 16); // erase size
				memset(gPartInfo[i].PartName, 0, sizeof(gPartInfo[i].PartName));
				strncpy(gPartInfo[i].PartName, (CVI_CHAR *)(argv[3] + 1), (strlen((CVI_CHAR *)
						argv[3]) - 2));
				//gPartInfo[i].FlashType = get_flashtype_by_bootargs(gPartInfo[i].PartName);
				gPartInfo[i].FlashType = CVI_FLASH_TYPE_NAND_0;
				gPartInfo[i].StartAddr = u64StartAddr[gPartInfo[i].FlashType];
				u64StartAddr[gPartInfo[i].FlashType] += gPartInfo[i].PartSize;
#if defined(ANDROID)
				snprintf(gPartInfo[i].DevName, sizeof(gPartInfo[i].DevName), DEV_MTDBASE "%d", i);
#else
				snprintf(gPartInfo[i].DevName, sizeof(gPartInfo[i].DevName), "/dev/mtd%d", i);
#endif
				fd = open(gPartInfo[i].DevName, O_RDWR);
				if (fd == -1) {
					fd = open(gPartInfo[i].DevName, O_RDONLY);
					if (fd == -1) {
						// CVI_ERR_FLASH("Can't open \"%s\"\n", gPartInfo[i].DevName);
						// fclose(fp);
						gPartInfo[i].perm = ACCESS_NONE;
						continue;
					}

					printf("access %s readonly!\n", gPartInfo[i].DevName);
					gPartInfo[i].perm = ACCESS_RD;
				} else {
				//	printf("access %s read and write, i:%d, fd:%x!\n", gPartInfo[i].DevName, i, fd);
					gPartInfo[i].perm = ACCESS_RDWR;
				}

				close(fd);

#ifdef FLASH_DEBUG
				printf("\n");
				printf("gPartInfo[%d].PartSize = 0x%lx\n", i, gPartInfo[i].PartSize);
				printf("gPartInfo[%d].BlockSize = 0x%x\n", i, gPartInfo[i].BlockSize);
				printf("gPartInfo[%d].PartName = %s\n", i, gPartInfo[i].PartName);
				printf("gPartInfo[%d].FlashType = %d\n",	i, gPartInfo[i].FlashType);
				printf("gPartInfo[%d].StartAddr = 0x%lx\n",  i, gPartInfo[i].StartAddr);
				printf("gPartInfo[%d].DevName = %s\n", i, gPartInfo[i].DevName);
#endif
			}
		} else {
			printf("open /proc/mtd file failure!\n");
			return CVI_FAILURE;
		}

		if ((g_eDevStat[CVI_FLASH_TYPE_SPI_0] == CVI_FLASH_STAT_INSTALL)
			|| (g_eDevStat[CVI_FLASH_TYPE_NAND_0] == CVI_FLASH_STAT_INSTALL)
			|| (g_eDevStat[CVI_FLASH_TYPE_EMMC_0] == CVI_FLASH_STAT_INSTALL)) {
			g_stFlashOpt[CVI_FLASH_TYPE_NAND_0].raw_erase = nand_raw_erase;
			g_stFlashOpt[CVI_FLASH_TYPE_NAND_0].raw_read = nand_raw_read;
			g_stFlashOpt[CVI_FLASH_TYPE_NAND_0].raw_write = nand_raw_write;
		}
		if (fclose(fp)) {
			return CVI_FAILURE;
		}
	}

	return ret;
}

static CVI_S32 All_FLash_Init(void)
{
	CVI_U8 u8Loop = 0;

	if (gInitMutex) {
		while (gInitMutex && (u8Loop < 100)) {
			usleep(100);
			u8Loop++;
		}
	} else {
		gInitMutex = CVI_TRUE;
	}

	if (!gInitFlag) {
		for (u8Loop = 0; u8Loop < CVI_FLASH_TYPE_BUTT; u8Loop++) {
			g_eDevStat[u8Loop] = CVI_FLASH_STAT_UNINSTALL;
		}

		if (nand_raw_init() == CVI_SUCCESS) {
			g_eDevStat[CVI_FLASH_TYPE_NAND_0] = CVI_FLASH_STAT_INSTALL;
		}else if (pszPos[CVI_FLASH_TYPE_NAND_0] != CVI_NULL) {
			(CVI_VOID)spi_raw_destroy();
			printf("nand init fail!\n");
			gInitMutex = CVI_FALSE;
			return CVI_FAILURE;
		}

		if (Flash_Init()) {
			(CVI_VOID)nand_raw_destroy();
			printf("Flash init fail!\n");
			gInitMutex = CVI_FALSE;
			return CVI_FAILURE;
		}
	}
	gInitFlag = CVI_TRUE;
	gInitMutex = CVI_FALSE;
	return CVI_SUCCESS;
}

CVI_S32 CVI_Flash_UnusedHandle(void)
{
	CVI_S8 u8Loop;

	for (u8Loop = 0; u8Loop < MAX_HANDLE; u8Loop++) {
		if (gFlashInfo[u8Loop].fd == (CVI_VOID *)INVALID_FD) {
			break;
		}
	}

	return u8Loop;
}

CVI_HANDLE CVI_Flash_OpenByTypeAndAddr(CVI_FLASH_TYPE_E enFlashType,
									 CVI_U64 u64Address,
									 CVI_U64 u64Len)
{
	CVI_U64 TotalSize = 0;
	CVI_U32 PageSize = 0;
	CVI_U32 BlockSize = 0;
	CVI_U32 OobSize = 0;
	CVI_U32 BlockShift = 0;
	CVI_U32 hFlash = 0;

	if (All_FLash_Init()) {
		return (CVI_HANDLE)INVALID_FD;
	}

	if (enFlashType >= CVI_FLASH_TYPE_BUTT) {
		printf("FlashType error!\n");
		return (CVI_HANDLE)INVALID_FD;
	}

	if (g_eDevStat[enFlashType] != CVI_FLASH_STAT_INSTALL) {
		printf("No config flash[type:%d].", enFlashType);
		return (CVI_HANDLE)INVALID_FD;
	}

	if (enFlashType == CVI_FLASH_TYPE_EMMC_0) {
//		emmc_raw_get_info(&TotalSize, &PageSize, &BlockSize, &OobSize, &BlockShift);
	} else if (enFlashType == CVI_FLASH_TYPE_SPI_0) {
//		spi_raw_get_info(&TotalSize, &PageSize, &BlockSize, &OobSize, &BlockShift);
	} else {
		nand_raw_get_info(&TotalSize, &PageSize, &BlockSize, &OobSize, &BlockShift);
	}

	if (BlockSize == 0) {
		printf("BlockSize shouldn't equal 0!\n");
		return (CVI_HANDLE)INVALID_FD;
	}

	if ((CVI_S32)(u64Address % BlockSize) || (CVI_S32)(u64Len % BlockSize)) {
		printf("Open Address(%#lx) and Len(%#lx) should be align with BlockSize(0x%X)!\n",
					 u64Address, u64Len, BlockSize);
		return (CVI_HANDLE)INVALID_FD;
	}

	if ((u64Address >= TotalSize) || (u64Address + u64Len) > TotalSize) {
		printf("Open Address(%#lx) and Len(%#lx) should be smaller than TotalSize(0x%lX)!\n",
					 u64Address, u64Len, TotalSize);
		return (CVI_HANDLE)INVALID_FD;
	}

	if (permission_check(enFlashType, u64Address, u64Len) != CVI_SUCCESS) {
		printf("not permission to be opened.\n");
		return (CVI_HANDLE)INVALID_FD;
	}

	hFlash = (CVI_U32)enFlashType;
	if (hFlash == MAX_HANDLE) {
		printf("flash array full!\n");
		return (CVI_HANDLE)INVALID_FD;
	}

	gFlashInfo[hFlash].fd = (CVI_SL *)(CVI_SL)(SPAN_PART_HANDLE + hFlash);
	gFlashInfo[hFlash].OpenAddr = u64Address;
	gFlashInfo[hFlash].OpenLeng = u64Len;
	gFlashInfo[hFlash].pPartInfo = NULL;
	gFlashInfo[hFlash].FlashType = enFlashType;
	gFlashInfo[hFlash].PageSize = PageSize;
	gFlashInfo[hFlash].OobSize = OobSize;
	gFlashInfo[hFlash].BlockSize = BlockSize;

	if (enFlashType == CVI_FLASH_TYPE_SPI_0) {
		gFlashInfo[hFlash].pFlashopt = (struct FLASH_OPT_S *)&g_stFlashOpt[CVI_FLASH_TYPE_SPI_0];
	}

	if (enFlashType == CVI_FLASH_TYPE_NAND_0) {
		gFlashInfo[hFlash].pFlashopt = (struct FLASH_OPT_S *)&g_stFlashOpt[CVI_FLASH_TYPE_NAND_0];
	}
	if (enFlashType == CVI_FLASH_TYPE_EMMC_0) {
		gFlashInfo[hFlash].pFlashopt = (struct FLASH_OPT_S *)&g_stFlashOpt[CVI_FLASH_TYPE_EMMC_0];
	}

#ifdef FLASH_DEBUG
	printf("\n");
	printf("gFlashInfo[%d].OpenAddr = 0x%lx\n", hFlash, gFlashInfo[hFlash].OpenAddr);
	printf("gFlashInfo[%d].OpenLeng = 0x%lx\n", hFlash, gFlashInfo[hFlash].OpenLeng);
	printf("gFlashInfo[%d].FlashType = %d\n", hFlash, gFlashInfo[hFlash].FlashType);
	printf("gFlashInfo[%d].PageSize = 0x%x\n",	hFlash, gFlashInfo[hFlash].PageSize);
	printf("gFlashInfo[%d].OobSize = 0x%x\n",  hFlash, gFlashInfo[hFlash].OobSize);
	printf("gFlashInfo[%d].BlockSize = 0x%x\n", hFlash, gFlashInfo[hFlash].BlockSize);
	printf("end.\n");
#endif
	//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);

	return (CVI_HANDLE)hFlash;
}

static CVI_HANDLE CVI_Flash_OpenByName_Emmc(CVI_CHAR *pPartitionName)
{
	CVI_U64 u64Address = 0;
	CVI_U64 u64Len = 0;
	CVI_U32 hFlash = 0;
	CVI_CHAR *ptr;
	CVI_CHAR media_name[20] = { 0 };
	struct EMMC_CB_S *pstEmmcCB;

	//(CVI_VOID)pthread_mutex_lock(&gFlashMutex);

#if defined(ANDROID)
	if (strncmp(pPartitionName, "/dev/block/mmcblk0p", strlen("/dev/block/mmcblk0p")) == 0)
#else
	if (strncmp(pPartitionName, "/dev/mmcblk0p", strlen("/dev/mmcblk0p")) == 0)
#endif
	{
		pstEmmcCB = emmc_node_open((CVI_U8 *)pPartitionName);
		if (pstEmmcCB == NULL) {
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			return (CVI_HANDLE)INVALID_FD;
		}

		hFlash = (CVI_U32)CVI_Flash_UnusedHandle();
		if (hFlash == MAX_HANDLE) {
			printf("flash array full!\n");
			(CVI_VOID)emmc_raw_close(pstEmmcCB);
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			return (CVI_HANDLE)INVALID_FD;
		}

		memset(media_name, 0, sizeof(media_name));
		strncpy(media_name, "mmcblk0", sizeof(media_name) - 1);
		media_name[sizeof(media_name) - 1] = '\0';
		if (find_part_from_devname(media_name, (CVI_CHAR *)g_au8Bootargs,
								   pPartitionName, &u64Address, &u64Len)) {
			printf("Cannot find partiton from %s\n", pPartitionName);
			(CVI_VOID)emmc_raw_close(pstEmmcCB);
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			return (CVI_HANDLE)INVALID_FD;
		}

		pstEmmcCB->u64PartSize = u64Len;
		gFlashInfo[hFlash].fd = pstEmmcCB;
		gFlashInfo[hFlash].FlashType = CVI_FLASH_TYPE_EMMC_0;
		//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
		return (CVI_HANDLE)hFlash;
	}

	ptr = strstr((CVI_CHAR *)g_au8Bootargs, "mmcblk0:");
	if (ptr != NULL) {
		memset(media_name, 0, sizeof(media_name));
		strncpy(media_name, "mmcblk0", sizeof(media_name) - 1);
		media_name[sizeof(media_name) - 1] = '\0';
		if (find_flash_part(ptr, media_name, pPartitionName, &u64Address, &u64Len) == 0) {
			printf("Cannot find partition: %s\n", pPartitionName);
		} else if (u64Len == (CVI_U64)(-1)) {
			printf("Can not contain char '-'\n");
		} else {
			pstEmmcCB = emmc_raw_open(u64Address, u64Len);
			if (pstEmmcCB == NULL) {
				//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
				return (CVI_HANDLE)INVALID_FD;
			}

			hFlash = (CVI_U32)CVI_Flash_UnusedHandle();
			if (hFlash == MAX_HANDLE) {
				printf("flash array full!\n");
				(CVI_VOID)emmc_raw_close(pstEmmcCB);
				//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
				return (CVI_HANDLE)INVALID_FD;
			}

			gFlashInfo[hFlash].fd = pstEmmcCB;
			gFlashInfo[hFlash].FlashType = CVI_FLASH_TYPE_EMMC_0;
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			return (CVI_HANDLE)hFlash;
		}
	}

	//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
	return ((CVI_HANDLE)INVALID_FD);
}

static CVI_S32 check_flash_part(CVI_FLASH_TYPE_E enFlashType,
							   CVI_CHAR *pPartitionName)
{
	CVI_S32 i, j;

	for (i = 0; i < MAX_PARTS; i++) {
		if (!strncmp(gPartInfo[i].DevName, pPartitionName, strlen(pPartitionName) + 1) &&
			(gPartInfo[i].FlashType == enFlashType)) {	// eg: "/dev/mtd* "
			break;
		}
		if (!strncmp(gPartInfo[i].PartName, pPartitionName, strlen(pPartitionName) + 1) &&
			(gPartInfo[i].FlashType == enFlashType)) {
			break;
		}
	}

	for (j = MAX_PARTS - 1; j >= 0; j--) {
		if (!strncmp(gPartInfo[j].DevName, pPartitionName, strlen(pPartitionName) + 1) &&
			(gPartInfo[j].FlashType == enFlashType)) {	// eg: "/dev/mtd* "
			break;
		}
		if (!strncmp(gPartInfo[j].PartName, pPartitionName, strlen(pPartitionName) + 1) &&
			(gPartInfo[j].FlashType == enFlashType)) {
			break;
		}
	}
	// add i < 0 test in if branch to avoid pclint warning:
	// Warning 676: Possibly negative subscript (-1) in operator '['

	if ((i == MAX_PARTS) || (i != j) || i < 0) {
		printf("can not find a right flash part(i=%d, j=%d)!\n", i, j);
		return -1;
	}

	return i;
}

static CVI_HANDLE CVI_Flash_OpenByName_Spi_Nand(CVI_FLASH_TYPE_E enFlashType,
											  CVI_CHAR *pPartitionName)
{
	CVI_CHAR DevName[FLASH_NAME_LEN] = { 0 };
	CVI_U32 hPart = 0;
	CVI_U32 hFlash = 0;
	CVI_S64 fd = (CVI_S64)INVALID_FD;
	CVI_U64 TotalSize = 0;
	CVI_U32 PageSize = 0;
	CVI_U32 BlockSize = 0;
	CVI_U32 OobSize = 0;
	CVI_U32 BlockShift = 0;
	CVI_S32 i = 0;

	//(CVI_VOID)pthread_mutex_lock(&gFlashMutex);

	//if (strstr((char *)g_au8Bootargs, "ubi.mtd")
	//	|| strstr((char *)g_au8Bootargs, "cvi_sfc:"))
		{
		/* add i < 0 test in if branch to avoid pclint warning:
		 * Warning 676: Possibly negative subscript (-1) in operator '['
		 */
		i = check_flash_part(enFlashType, pPartitionName);
		if (i < 0) {
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			return (CVI_HANDLE)INVALID_FD;
		}

		memset(DevName, 0, FLASH_NAME_LEN);
		strncpy(DevName, gPartInfo[i].DevName, sizeof(DevName));
		DevName[sizeof(DevName) - 1] = '\0';

		hPart = (CVI_U32)i;

#ifdef FLASH_DEBUG
	printf("hpart = %d\n", hPart);
#endif
		//hFlash = (CVI_U32)CVI_Flash_UnusedHandle();
		hFlash = (CVI_U32)enFlashType;
		if (hFlash == MAX_HANDLE) {
			printf("flash array full!\n");
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			return (CVI_HANDLE)INVALID_FD;
		}

		for (i = 0; i < MAX_HANDLE; i++) {
			if ((gFlashInfo[i].pPartInfo != CVI_NULL) &&
				(!strncmp(gFlashInfo[i].pPartInfo->DevName, DevName, strlen(DevName) + 1))) {
				if (gFlashInfo[i].fd != (CVI_VOID *)INVALID_FD) {
					printf("fd = %p, DevName =\"%s\"(%s)\n", gFlashInfo[i].fd,
						 gFlashInfo[i].pPartInfo->DevName, gFlashInfo[i].pPartInfo->PartName);
					//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
					return (CVI_HANDLE)i;
				}
			}
		}

		if (gPartInfo[hPart].perm == ACCESS_RDWR) {
			fd = open(DevName, O_RDWR);
		} else if (gPartInfo[hPart].perm == ACCESS_RD) {
			fd = open(DevName, O_RDONLY);
		} else if (gPartInfo[hPart].perm == ACCESS_WR) {
			fd = open(DevName, O_WRONLY);
		} else {
			printf("Device \"%s\"(%s) can not be opened\n", gPartInfo[hPart].DevName,
						 gPartInfo[hPart].PartName);
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			return ((CVI_HANDLE)INVALID_FD);
		}

		if ((fd < 0) || (fd >= SPAN_PART_HANDLE)) {
			printf("Open %s flash partition failure(fd = %ld)!\n", DevName, (CVI_S64)fd);
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			if (fd >= 0) {
				close(fd);
			}
			return (CVI_HANDLE)INVALID_FD;
		}

		if (enFlashType == CVI_FLASH_TYPE_SPI_0) {
			spi_raw_get_info(&TotalSize, &PageSize, &BlockSize, &OobSize, &BlockShift);
		} else {
			nand_raw_get_info(&TotalSize, &PageSize, &BlockSize, &OobSize, &BlockShift);
		}

		gFlashInfo[hFlash].fd = (long *)(long)fd;
		// gFlashInfo[hFlash].OpenAddr = 0;
		gFlashInfo[hFlash].OpenLeng = 0;
		gFlashInfo[hFlash].pPartInfo = &gPartInfo[hPart];
		gFlashInfo[hFlash].OpenAddr = gFlashInfo[hFlash].pPartInfo->StartAddr;
		gFlashInfo[hFlash].FlashType = enFlashType;
		gFlashInfo[hFlash].PageSize = PageSize;
		gFlashInfo[hFlash].OobSize = OobSize;
		gFlashInfo[hFlash].BlockSize = BlockSize;

		if (enFlashType == CVI_FLASH_TYPE_SPI_0) {
			gFlashInfo[hFlash].pFlashopt = (struct FLASH_OPT_S *)&g_stFlashOpt[CVI_FLASH_TYPE_SPI_0];
		}
		if (enFlashType == CVI_FLASH_TYPE_NAND_0) {
			gFlashInfo[hFlash].pFlashopt = (struct FLASH_OPT_S *)&g_stFlashOpt[CVI_FLASH_TYPE_NAND_0];
		}
	//	printf("fd = %p, DevName =\"%s\"(%s)\n", gFlashInfo[hFlash].fd,
	//		 gFlashInfo[hFlash].pPartInfo->DevName, gFlashInfo[hFlash].pPartInfo->PartName);
#ifdef FLASH_DEBUG
		printf("\n");
		printf("gFlashInfo[%d].OpenAddr = 0x%lx\n", hFlash, gFlashInfo[hFlash].OpenAddr);
		printf("gFlashInfo[%d].OpenLeng = 0x%lx\n", hFlash, gFlashInfo[hFlash].OpenLeng);
		printf("gFlashInfo[%d].FlashType = %d\n", hFlash, gFlashInfo[hFlash].FlashType);
		printf("gFlashInfo[%d].PageSize = 0x%x\n",	hFlash, gFlashInfo[hFlash].PageSize);
		printf("gFlashInfo[%d].OobSize = 0x%x\n",  hFlash, gFlashInfo[hFlash].OobSize);
		printf("gFlashInfo[%d].BlockSize = 0x%x\n", hFlash, gFlashInfo[hFlash].BlockSize);
		printf("end.\n");
#endif
		//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
		return (CVI_HANDLE)hFlash;
	}

	//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
	return ((CVI_HANDLE)INVALID_FD);
}

CVI_HANDLE CVI_Flash_OpenByTypeAndName(CVI_FLASH_TYPE_E enFlashType,
									 CVI_CHAR *pPartitionName)
{
	if (enFlashType == CVI_FLASH_TYPE_BUTT) {
		printf("FlashType error(FlashType=%d)!\n", enFlashType);
		return (CVI_HANDLE)INVALID_FD;
	}

	if (!pPartitionName) {
		printf("pPartitionName is null!\n");
		return (CVI_HANDLE)INVALID_FD;
	}

	if (All_FLash_Init()) {
		return (CVI_HANDLE)INVALID_FD;
	}

	if (g_eDevStat[enFlashType] != CVI_FLASH_STAT_INSTALL) {
		printf("No config flash[type:%d].", enFlashType);
		return (CVI_HANDLE)INVALID_FD;
	}

	if (enFlashType == CVI_FLASH_TYPE_EMMC_0) {
		return CVI_Flash_OpenByName_Emmc(pPartitionName);
	}

	if ((enFlashType == CVI_FLASH_TYPE_SPI_0) || (enFlashType == CVI_FLASH_TYPE_NAND_0)) {
		return CVI_Flash_OpenByName_Spi_Nand(enFlashType, pPartitionName);
	}

	return (CVI_HANDLE)INVALID_FD;
}

CVI_HANDLE CVI_Flash_OpenByName(CVI_CHAR *pPartitionName)
{
	CVI_FLASH_TYPE_E enFlashType = CVI_FLASH_TYPE_BUTT;
	CVI_S32 i = 0;
	CVI_U32 hFlash = 0;
	CVI_U64 u64Address = 0;
	CVI_U64 u64Len = 0;
	CVI_CHAR *ptr = NULL;
	CVI_CHAR media_name[20];
	struct EMMC_CB_S *pstEmmcCB;

	if (!pPartitionName) {
		printf("pPartitionName is null!\n");
		return (CVI_HANDLE)INVALID_FD;
	}

	if (All_FLash_Init()) {
		return (CVI_HANDLE)INVALID_FD;
	}

	enFlashType = get_flashtype_by_bootargs(pPartitionName);
	if (enFlashType == CVI_FLASH_TYPE_BUTT) {
		printf("Invalid partition name: %s\n", pPartitionName);
		return (CVI_HANDLE)INVALID_FD;
	}

	ptr = strstr((CVI_CHAR *)g_au8Bootargs, "mmcblk0:");
	if ((enFlashType == CVI_FLASH_TYPE_EMMC_0) && (ptr != NULL)) {
		memset(media_name, 0, sizeof(media_name));
		strncpy(media_name, "mmcblk0", sizeof(media_name) - 1);
		media_name[sizeof(media_name) - 1] = '\0';
		if (find_flash_part(ptr, media_name, pPartitionName, &u64Address, &u64Len) == 0) {
			printf("Cannot find partition: %s\n", pPartitionName);
			//return (CVI_HANDLE)INVALID_FD;
		} else if (u64Len == (CVI_U64)(-1)) {
			printf("Can not contain char '-'\n");
			//return (CVI_HANDLE)INVALID_FD;
		} else {
			//(CVI_VOID)pthread_mutex_lock(&gFlashMutex);
			pstEmmcCB = emmc_raw_open(u64Address, u64Len);
			if (pstEmmcCB == NULL) {
				//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
				//return (CVI_HANDLE)INVALID_FD;
			}

			hFlash = (CVI_U32)CVI_Flash_UnusedHandle();
			if (hFlash == MAX_HANDLE) {
				printf("flash array full!\n");
				(CVI_VOID)emmc_raw_close(pstEmmcCB);
				//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
				//return (CVI_HANDLE)INVALID_FD;
			}

			gFlashInfo[hFlash].fd = pstEmmcCB;
			gFlashInfo[hFlash].FlashType = CVI_FLASH_TYPE_EMMC_0;
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			return (CVI_HANDLE)hFlash;
		}
		return 0;
	}

	i = check_flash_part(enFlashType, pPartitionName);
	if (strstr((CVI_CHAR *)g_au8Bootargs, "hinand:")
		|| strstr((CVI_CHAR *)g_au8Bootargs, "cvi_sfc:")) {
		if (i < 0) {
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			return (CVI_HANDLE)INVALID_FD;
		}

		enFlashType = gPartInfo[i].FlashType;
		hFlash = CVI_Flash_OpenByTypeAndName(enFlashType, pPartitionName);
		return hFlash;
	}

	return ((CVI_HANDLE)INVALID_FD);
}

CVI_HANDLE CVI_Flash_Open(CVI_FLASH_TYPE_E enFlashType, CVI_CHAR *pPartitionName,
		CVI_U64 u64Address, CVI_U64 u64Len)
{
	CVI_U32 hFlash = 0;

	// printf("FlashType=%d, PartitionName=%s, Address=0x%lx,
		//Len=0x%lx\n", enFlashType, pPartitionName, u64Address, u64Len);

	if (pPartitionName == NULL) {
		hFlash = CVI_Flash_OpenByTypeAndAddr(enFlashType, u64Address, u64Len);
	} else {
			hFlash = CVI_Flash_OpenByTypeAndName(enFlashType, pPartitionName);
	}

	return hFlash;
}

CVI_S32 CVI_Flash_Close(CVI_HANDLE hFlash)
{
	CVI_U32 i;
	CVI_S32 ret;
	struct EMMC_CB_S *pstEmmcCB;

	check_flash_init(hFlash);

	if (gFlashInfo[hFlash].FlashType == CVI_FLASH_TYPE_EMMC_0) {
		//(CVI_VOID)pthread_mutex_lock(&gFlashMutex);
		pstEmmcCB = ((struct EMMC_CB_S *)(gFlashInfo[hFlash].fd));
		(CVI_VOID)emmc_raw_close(pstEmmcCB);
		gFlashInfo[hFlash].fd = (CVI_S64 *)(CVI_S64)INVALID_FD;
		gFlashInfo[hFlash].OpenAddr = 0;
		gFlashInfo[hFlash].OpenLeng = 0;
		gFlashInfo[hFlash].pPartInfo = NULL;
		gFlashInfo[hFlash].FlashType = CVI_FLASH_TYPE_BUTT;

		//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
		return CVI_SUCCESS;
	}

	//(CVI_VOID)pthread_mutex_lock(&gFlashMutex);
	printf("fd = %p\n", gFlashInfo[hFlash].fd);

	if (gFlashInfo[hFlash].fd >= (CVI_VOID *)SPAN_PART_HANDLE) {
		gFlashInfo[hFlash].fd = (CVI_S64 *)(CVI_S64)INVALID_FD;
		gFlashInfo[hFlash].OpenAddr = 0;
		gFlashInfo[hFlash].OpenLeng = 0;
		gFlashInfo[hFlash].pPartInfo = NULL;
	} else {
		ret = close((long)gFlashInfo[hFlash].fd);

		if (ret != 0) {
			printf("Close %s flash partition failure %d!\n", gFlashInfo[hFlash].pPartInfo->DevName, ret);
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			return CVI_FAILURE;
		}

		gFlashInfo[hFlash].fd = (CVI_S64 *)(CVI_S64)INVALID_FD;
		gFlashInfo[hFlash].OpenAddr = 0;
		gFlashInfo[hFlash].OpenLeng = 0;
		gFlashInfo[hFlash].pPartInfo = NULL;
	}

	for (i = 0; i < MAX_HANDLE; i++) {
		if (gFlashInfo[i].fd != (CVI_VOID *)INVALID_FD) {
			break;
		}
	}

	if (i == MAX_HANDLE) {
		(CVI_VOID)spi_raw_destroy();
		(CVI_VOID)nand_raw_destroy();
		gInitFlag = CVI_FALSE;
	}

	printf("end.\n");
	//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);

	return CVI_SUCCESS;
}

static CVI_S32 Compensate_Nand_Address(CVI_U64 u64Address, CVI_U64 *StartAddr)
{
	CVI_U64 TotalSize = 0;
	CVI_U32 PageSize = 0;
	CVI_U32 BlockSize = 0;
	CVI_U32 OobSize = 0;
	CVI_U32 BlockShift = 0;
	CVI_S32 idx = 0;
	CVI_S32 ret;

	nand_raw_get_info(&TotalSize, &PageSize, &BlockSize, &OobSize, &BlockShift);
	if (BlockSize == 0) {
		printf("BlockSize shouldn't equal 0!\n");
		return CVI_FAILURE;
	}
	idx = (CVI_S32)(u64Address >> BlockShift);
	ret = nand_raw_get_physical_index(*StartAddr, &idx, (CVI_S32)BlockSize);
	if (ret != 0) {
		printf("logical addr change to physical addr error!\n");
		return ret;
	}
	*StartAddr += (CVI_U64)(((CVI_U32)idx) << BlockShift);
	*StartAddr += (u64Address % BlockSize);

	return CVI_SUCCESS;
}

CVI_S32 CVI_Flash_Erase(CVI_HANDLE hFlash, CVI_U64 u64Address, CVI_U64 u64Len)
{
	CVI_U64 StartAddr = 0;
	CVI_U64 LimitLeng = 0;
	CVI_S32 ret = 0;

	/* u64Len bigger than 2GB should use CVI_Flash_Erase64 instead. */
	if (u64Len >= 0x80000000) {
		printf("u64Len is too big(0x%lx), should use CVI_Flash_Erase64() instead.\n", u64Len);
		return CVI_FAILURE;
	}

	check_flash_init(hFlash);

	if (gFlashInfo[hFlash].FlashType == CVI_FLASH_TYPE_EMMC_0) {
#if defined(CVI_EMC_ERASE_SUPPORT)
		// printf("CVI_Flash_Erase -> emc_raw_erase\nu64Address=0x%08llx,
			//u64Len=0x%08llx\n", u64Address, u64Len);
		return emc_raw_erase((struct EMMC_CB_S *)gFlashInfo[hFlash].fd, u64Address, u64Len);
#else
		return CVI_SUCCESS;
#endif
	}

	//(CVI_VOID)pthread_mutex_lock(&gFlashMutex);

	if (gFlashInfo[hFlash].fd >= (CVI_VOID *)SPAN_PART_HANDLE) {
		StartAddr = gFlashInfo[hFlash].OpenAddr;
		LimitLeng = gFlashInfo[hFlash].OpenLeng;
		printf("StartAddr = 0x%lx, LimitLeng =0x%lx\n", StartAddr, LimitLeng);
	} else {
		StartAddr = gFlashInfo[hFlash].pPartInfo->StartAddr;
		LimitLeng = gFlashInfo[hFlash].pPartInfo->PartSize;
		printf("StartAddr = 0x%lx, LimitLeng =0x%lx\n", StartAddr, LimitLeng);
	}

#ifdef FLASH_DEBUG
	printf("gFlashInfo[hFlash].OpenAddr = 0x%lx\n", gFlashInfo[hFlash].OpenAddr);
#endif
	check_addr_len_valid(u64Address, u64Len, LimitLeng);

	u64Address -= gFlashInfo[hFlash].OpenAddr;
	if (gFlashInfo[hFlash].FlashType == CVI_FLASH_TYPE_NAND_0) {
		ret = Compensate_Nand_Address(u64Address, &StartAddr);
		if (ret != CVI_SUCCESS) {
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			return CVI_FAILURE;
		}
	} else {
		StartAddr += u64Address;
	}

	printf("HANDLE=%d, Address=0x%lx, Len=0x%lx\n", hFlash, StartAddr, u64Len);

	if (!gFlashInfo[hFlash].pFlashopt->raw_erase) {
		printf("flash service function ptr(raw_erase) is NULL!\n");
		//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
		return CVI_FAILURE;
	}
	ret = (CVI_S32)gFlashInfo[hFlash].pFlashopt->raw_erase((CVI_S32)(CVI_SL)gFlashInfo[hFlash].fd,
		 (unsigned long long)StartAddr, u64Len, gFlashInfo[hFlash].OpenAddr, LimitLeng);

	printf("end.\n");
	//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);

	return ret;
}

CVI_S64 CVI_Flash_Erase64(CVI_HANDLE hFlash, CVI_U64 u64Address, CVI_U64 u64Len)
{
	CVI_U64 StartAddr = 0;
	CVI_U64 LimitLeng = 0;
	CVI_S64 ret;

	check_flash_init(hFlash);

	if (gFlashInfo[hFlash].FlashType == CVI_FLASH_TYPE_EMMC_0) {
		return CVI_SUCCESS;
	}

	//(CVI_VOID)pthread_mutex_lock(&gFlashMutex);

	if (gFlashInfo[hFlash].fd >= (CVI_VOID *)SPAN_PART_HANDLE) {
		StartAddr = gFlashInfo[hFlash].OpenAddr;
		LimitLeng = gFlashInfo[hFlash].OpenLeng;
	} else {
		StartAddr = gFlashInfo[hFlash].pPartInfo->StartAddr;
		LimitLeng = gFlashInfo[hFlash].pPartInfo->PartSize;
	}
	check_addr_len_valid(u64Address, u64Len, LimitLeng);

	u64Address -= gFlashInfo[hFlash].OpenAddr;
	if (gFlashInfo[hFlash].FlashType == CVI_FLASH_TYPE_NAND_0) {
		ret = (CVI_S64)Compensate_Nand_Address(u64Address, &StartAddr);
		if (ret != CVI_SUCCESS) {
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			return CVI_FAILURE;
		}
	} else {
		StartAddr += u64Address;
	}

	printf("HANDLE=%d, Address=0x%lx, Len=0x%lx\n", hFlash, StartAddr, u64Len);

	if (!gFlashInfo[hFlash].pFlashopt->raw_erase) {
		printf("flash service function ptr(raw_erase) is NULL!\n");
		//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
		return CVI_FAILURE;
	}

	ret = (CVI_S64)gFlashInfo[hFlash].pFlashopt->raw_erase((CVI_S32)(CVI_SL)gFlashInfo[hFlash].fd,
		 (unsigned long long)StartAddr, u64Len, gFlashInfo[hFlash].OpenAddr, LimitLeng);
	printf("end.\n");
	//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);

	return ret;
}

CVI_S32 CVI_Flash_Read(CVI_HANDLE hFlash, CVI_U64 u64Address, CVI_U8 *pBuf, CVI_U32 u32Len, CVI_U32 u32Flags)
{
	CVI_U64 StartAddr = 0;
	CVI_U64 LimitLeng = 0;
	CVI_S32 ret = 0;
	CVI_S32 wtihoob;
	struct EMMC_CB_S *pstEmmcCB;

	if (pBuf == NULL) {
		return CVI_FAILURE;
	}

	check_flash_init(hFlash);

	//(CVI_VOID)pthread_mutex_lock(&gFlashMutex);

	if (gFlashInfo[hFlash].FlashType == CVI_FLASH_TYPE_EMMC_0) {
		pstEmmcCB = (struct EMMC_CB_S *)(gFlashInfo[hFlash].fd);
		ret = emmc_raw_read(pstEmmcCB, u64Address, u32Len, pBuf);
		//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
		/* union return value to CVI_FAILURE. */
		if (ret < 0) {
			return CVI_FAILURE;
		}
		return ret;
	}

	if (gFlashInfo[hFlash].fd >= (CVI_VOID *)SPAN_PART_HANDLE) {
		StartAddr = gFlashInfo[hFlash].OpenAddr;
		LimitLeng = gFlashInfo[hFlash].OpenLeng;
		printf("gFlashInfo[hFlash].OpenAddr = 0x%lx\n", gFlashInfo[hFlash].OpenAddr);
	} else {
		StartAddr = gFlashInfo[hFlash].pPartInfo->StartAddr;
		LimitLeng = gFlashInfo[hFlash].pPartInfo->PartSize;
	}

#ifdef FLASH_DEBUG
	printf("gFlashInfo[hFlash].OpenAddr = 0x%lx\n", gFlashInfo[hFlash].OpenAddr);
#endif
	if ((u32Flags & CVI_FLASH_RW_FLAG_WITH_OOB) == CVI_FLASH_RW_FLAG_WITH_OOB) {
		CVI_U32 u32LenWithoutOOB = (u32Len / (gFlashInfo[hFlash].OobSize
			 + gFlashInfo[hFlash].PageSize)) * gFlashInfo[hFlash].PageSize;

		if (u32Len % (gFlashInfo[hFlash].OobSize
				  + gFlashInfo[hFlash].PageSize)) {
			u32LenWithoutOOB += gFlashInfo[hFlash].PageSize;
		}

		check_addr_len_valid(u64Address, (CVI_U64)u32LenWithoutOOB, LimitLeng);
	} else {
		check_addr_len_valid(u64Address, (CVI_U64)u32Len, LimitLeng);
	}

	u64Address -= gFlashInfo[hFlash].OpenAddr;
	if (gFlashInfo[hFlash].FlashType == CVI_FLASH_TYPE_NAND_0) {
		ret = Compensate_Nand_Address(u64Address, &StartAddr);
		if (ret != CVI_SUCCESS) {
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			return CVI_FAILURE;
		}
	} else {
		StartAddr += u64Address;
	}

	printf("HANDLE=%d, StartAddr Address=0x%lx, Len=0x%x, Flag=%d\n", hFlash, StartAddr, u32Len, u32Flags);

	if ((u32Flags & CVI_FLASH_RW_FLAG_WITH_OOB) == CVI_FLASH_RW_FLAG_WITH_OOB) {
		wtihoob = 1;
	} else {
		wtihoob = 0;
	}

	if (!gFlashInfo[hFlash].pFlashopt->raw_read) {
		printf("flash service function ptr(raw_read) is NULL!\n");
		//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
		return CVI_FAILURE;
	}

	ret = gFlashInfo[hFlash].pFlashopt->raw_read((CVI_S32)(CVI_SL)gFlashInfo[hFlash].fd,
		 (CVI_U64 *)&StartAddr, pBuf, u32Len, gFlashInfo[hFlash].OpenAddr, LimitLeng, wtihoob, 1);
	printf("totalread =0x%x, end.\n", ret);
	//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);

	return ret;
}

CVI_S32 CVI_Flash_Write(CVI_HANDLE hFlash, CVI_U64 u64Address, CVI_U8 *pBuf, CVI_U32 u32Len, CVI_U32 u32Flags)
{
	CVI_U64 StartAddr = 0;
	CVI_U64 LimitLeng = 0;
	CVI_S32 ret = 0;
	CVI_S32 wtihoob, erase;

	CVI_U64 TotalSize = 0;
	CVI_U32 PageSize = 0;
	CVI_U32 BlockSize = 0;
	CVI_U32 OobSize = 0;
	CVI_U32 BlockShift = 0;

	CVI_U32 u32EraseLen = 0;
	CVI_U32 BlockSize_new = 0;
	struct EMMC_CB_S *pstEmmcCB;

	if (pBuf == NULL) {
		return CVI_FAILURE;
	}

	check_flash_init(hFlash);
	//(CVI_VOID)pthread_mutex_lock(&gFlashMutex);

	if (gFlashInfo[hFlash].FlashType == CVI_FLASH_TYPE_EMMC_0) {
		pstEmmcCB = (struct EMMC_CB_S *)(gFlashInfo[hFlash].fd);
		ret = emmc_raw_write(pstEmmcCB, u64Address, u32Len, pBuf);
		//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
		if (ret < 0) {
			return CVI_FAILURE;
		}
		return ret;
	}

	if (gFlashInfo[hFlash].fd >= (CVI_VOID *)SPAN_PART_HANDLE) {
		StartAddr = gFlashInfo[hFlash].OpenAddr;
		LimitLeng = gFlashInfo[hFlash].OpenLeng;
	} else {
		StartAddr = gFlashInfo[hFlash].pPartInfo->StartAddr;
		LimitLeng = gFlashInfo[hFlash].pPartInfo->PartSize;
	}
#ifdef FLASH_DEBUG
	printf("gFlashInfo[hFlash].OpenAddr = 0x%lx\n", gFlashInfo[hFlash].OpenAddr);
#endif

	if ((u32Flags & CVI_FLASH_RW_FLAG_WITH_OOB) == CVI_FLASH_RW_FLAG_WITH_OOB) {
		CVI_U32 u32LenWithoutOOB = (u32Len / (gFlashInfo[hFlash].OobSize
			  + gFlashInfo[hFlash].PageSize))
				  * gFlashInfo[hFlash].PageSize;

		if (u32Len % (gFlashInfo[hFlash].OobSize
					  + gFlashInfo[hFlash].PageSize)) {
			u32LenWithoutOOB += gFlashInfo[hFlash].PageSize;
		}
		check_addr_len_valid(u64Address, (CVI_U64)u32LenWithoutOOB, LimitLeng);
	} else {
		check_addr_len_valid(u64Address, (CVI_U64)u32Len, LimitLeng);
	}

	if (gFlashInfo[hFlash].FlashType == CVI_FLASH_TYPE_NAND_0) {
		ret = Compensate_Nand_Address(u64Address, &StartAddr);
		if (ret != CVI_SUCCESS) {
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			return CVI_FAILURE;
		}
	} else {
		StartAddr += u64Address;
	}
#ifdef FLASH_DEBUG
	printf("HANDLE=%d, Address=0x%lx, Len=0x%x, Flag=%d\n", hFlash, StartAddr, u32Len, u32Flags);
#endif
	if (gFlashInfo[hFlash].FlashType == CVI_FLASH_TYPE_SPI_0) {
		spi_raw_get_info(&TotalSize, &PageSize, &BlockSize, &OobSize, &BlockShift);
	} else if (gFlashInfo[hFlash].FlashType == CVI_FLASH_TYPE_NAND_0) {
		nand_raw_get_info(&TotalSize, &PageSize, &BlockSize, &OobSize, &BlockShift);
	}

	if (CVI_FLASH_RW_FLAG_ERASE_FIRST == (u32Flags & CVI_FLASH_RW_FLAG_ERASE_FIRST)) {
		erase = 1;
	} else {
		erase = 0;
	}
	/* avoid pclint div 0 warning */
	if (!PageSize) {
		//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
		return -1;
	}

	if (CVI_FLASH_RW_FLAG_WITH_OOB == (u32Flags & CVI_FLASH_RW_FLAG_WITH_OOB)) {
		BlockSize_new = BlockSize + OobSize * (BlockSize / PageSize);
		wtihoob = 1;
	} else {
		BlockSize_new = BlockSize;
		wtihoob = 0;
	}

	if (erase) {
		/* avoid pclint div 0 warning */
		if (!BlockSize_new) {
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			return -1;
		}

		u32EraseLen = u32Len / BlockSize_new;

		if (u32Len % BlockSize_new) {
			u32EraseLen += 1;
		}
		u32EraseLen = u32EraseLen * BlockSize;
		// printf("> %s: [%d], u32EraseLen=%#x\n", __FUNCTION__, __LINE__, u32EraseLen);

		if (!gFlashInfo[hFlash].pFlashopt->raw_erase) {
			printf("flash service function ptr(raw_erase) is NULL!\n");
			//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
			return CVI_FAILURE;
		}
		ret = gFlashInfo[hFlash].pFlashopt->raw_erase((CVI_SL)gFlashInfo[hFlash].fd, (unsigned long long)
			StartAddr, (unsigned long long)u32EraseLen, gFlashInfo[hFlash].OpenAddr, LimitLeng);
		if (ret <= 0) {
			if (ret != CVI_FLASH_END_DUETO_BADBLOCK) {
				printf("earse fail!\n");
				//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
				return ret;
			}
		}
	}

	if (!gFlashInfo[hFlash].pFlashopt->raw_write) {
		printf("flash service function ptr(raw_write) is NULL!\n");
		//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);
		return CVI_FAILURE;
	}

	ret = gFlashInfo[hFlash].pFlashopt->raw_write((CVI_S32)(CVI_SL)gFlashInfo[hFlash].fd, &StartAddr, pBuf, u32Len,
		  gFlashInfo[hFlash].OpenAddr, LimitLeng, wtihoob);
	printf("totalwrite =0x%x, end.\n", ret);
	//(CVI_VOID)pthread_mutex_unlock(&gFlashMutex);

	return ret;
}

CVI_S32 CVI_Flash_GetInfo(CVI_HANDLE hFlash, struct CVI_Flash_InterInfo_S *pFlashInfo)
{
	CVI_U64 TotalSize = 0;
	CVI_U32 PageSize = 0;
	CVI_U32 BlockSize = 0;
	CVI_U32 OobSize = 0;
	CVI_U32 BlockShift = 0;
	struct EMMC_CB_S *pstEmmcCB;

	check_flash_init(hFlash);
	if (gFlashInfo[hFlash].FlashType == CVI_FLASH_TYPE_EMMC_0) {
		pstEmmcCB = (struct EMMC_CB_S *)(gFlashInfo[hFlash].fd);

		memset(pFlashInfo, 0x00, sizeof(*pFlashInfo));
		pFlashInfo->TotalSize = g_stEmmcFlash.u64RawAreaSize;
		pFlashInfo->OobSize = 0;
		pFlashInfo->PartSize = pstEmmcCB->u64PartSize;
		pFlashInfo->BlockSize = (g_stEmmcFlash.u32EraseSize * 16);
		pFlashInfo->PageSize = 0;
		pFlashInfo->fd = -1;
		pFlashInfo->FlashType = CVI_FLASH_TYPE_EMMC_0;
		pFlashInfo->OpenAddr = pstEmmcCB->u64Address;
		pFlashInfo->pFlashopt = NULL;

		return CVI_SUCCESS;
	}

	switch (gFlashInfo[hFlash].FlashType) {
	case CVI_FLASH_TYPE_SPI_0: {
		spi_raw_get_info(&TotalSize, &PageSize, &BlockSize, &OobSize, &BlockShift);
		break;
	}

	case CVI_FLASH_TYPE_NAND_0: {
		nand_raw_get_info(&TotalSize, &PageSize, &BlockSize, &OobSize, &BlockShift);
		break;
	}

	default:
	{
		break;
	}
	}

	pFlashInfo->TotalSize = TotalSize;

	if (gFlashInfo[hFlash].fd >= (CVI_VOID *)SPAN_PART_HANDLE) {
		pFlashInfo->PartSize = gFlashInfo[hFlash].OpenLeng;
		pFlashInfo->pPartInfo = NULL;
	} else {
		pFlashInfo->PartSize = gFlashInfo[hFlash].pPartInfo->PartSize;
		pFlashInfo->pPartInfo = gFlashInfo[hFlash].pPartInfo;
	}

	pFlashInfo->BlockSize = BlockSize;
	pFlashInfo->PageSize = PageSize;
	pFlashInfo->OobSize = OobSize;
	pFlashInfo->fd = -1;
	pFlashInfo->FlashType = gFlashInfo[hFlash].FlashType;
	pFlashInfo->pFlashopt = NULL;
	pFlashInfo->OpenAddr = gFlashInfo[hFlash].OpenAddr;
	pFlashInfo->OpenLeng = gFlashInfo[hFlash].OpenLeng;

#ifdef FLASH_DEBUG
	printf("pFlashInfo->TotalSize = 0x%lx\n", pFlashInfo->TotalSize);
	printf("pFlashInfo->PartSize = 0x%lx\n", pFlashInfo->PartSize);
	printf("pFlashInfo->BlockSize = 0x%x\n", pFlashInfo->BlockSize);
	printf("pFlashInfo->PageSize = 0x%x\n", pFlashInfo->PageSize);
	printf("pFlashInfo->OobSize = 0x%x\n", pFlashInfo->OobSize);
	printf("pFlashInfo->fd = %d\n", pFlashInfo->fd);
	printf("pFlashInfo->FlashType = %d\n", pFlashInfo->FlashType);
	printf("pFlashInfo->OpenAddr = 0x%lx\n", pFlashInfo->OpenAddr);
	printf("pFlashInfo->OpenLeng = 0x%lx\n", pFlashInfo->OpenLeng);
#endif
	return CVI_SUCCESS;
}

#define PATH_MAX 64
CVI_S32 CVI_Flash_GetStartAddr(CVI_S32 ix, CVI_U64 *startaddr, CVI_U64 *endaddr)
{

	CVI_S32 dev;
	struct mtd_info_user mtdinfo;
	struct CVI_Flash_PartInfo_S *pstPartInfo = NULL;
	CVI_CHAR devname[32];

	CVI_CHAR buf[PATH_MAX];

	memset(buf, 0, PATH_MAX);
	(CVI_VOID)snprintf(buf, PATH_MAX, DEV_MTDBASE"%d", ix);
	dev = open(buf, O_RDWR);
	if (dev == -1) {
		dev = open(buf, O_RDONLY);
		if (dev == -1) {
			printf("Can't open \"%s\", set to access none!\n", buf);
		}
	}

	if (ioctl(dev, MEMGETINFO, &mtdinfo)) {
		printf("Can't get \"%s\" information.\n", buf);
		close(dev);
	}

	if ((mtdinfo.type != MTD_NANDFLASH) && (mtdinfo.type != MTD_MLCNANDFLASH)) {
		close(dev);
	}

	memset(devname, 0, sizeof(devname));
	(CVI_VOID)snprintf(devname, sizeof(devname), "mtd%d", ix);
	flash_partition_info_init();
	pstPartInfo = get_flash_partition_info(CVI_FLASH_TYPE_NAND_0, devname);

	if (pstPartInfo == NULL) {
		printf("Can't get \"%s\" partition information\n", buf);
		close(dev);
		return CVI_FAILURE;
	}
	close(dev);
	*startaddr = pstPartInfo->StartAddr;
	*endaddr = pstPartInfo->StartAddr + mtdinfo.size - 1;

	return 0;
}
