#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "cvi_flash.h"
#include "nand.h"

/*****************************************************************************/
/* change length to string */
CVI_CHAR *int_to_size(CVI_U64 size)
{
	CVI_S32 ix;
	static CVI_CHAR buffer[20];
	const CVI_CHAR *fmt[] = { "%u", "%uK", "%uM", "%uG", "%uT", "%uT" };

	for (ix = 0; (ix < 5) && !(size & 0x3FF) && size; ix++) {
		size = (size >> 10);
	}
	snprintf(buffer, sizeof(buffer), fmt[ix], (CVI_U32)size);

	return buffer;
}
/*****************************************************************************/
/*
 * Modified for CONNAX CA.
 * When /dev path has no excute permission, stat call will fail.
 * Try parse /proc/mtd to get max partition number.
 */
CVI_S32 get_max_partition(CVI_VOID)
{
	FILE *fd;
#define MAX_PROC_LINE_SZ 1024
	CVI_CHAR bf[MAX_PROC_LINE_SZ];
	CVI_S32 nr = 0;

	fd = fopen(PROC_MTD_FILE, "r");
	if (fd == NULL) {
		printf("Fail to open %s!\n", PROC_MTD_FILE);
		return 0;
	}

	/* skip first prompt line */
	if (fgets(bf, MAX_PROC_LINE_SZ, fd) == NULL) {
		fclose(fd);
		return -1;
	}

	while (fgets(bf, MAX_PROC_LINE_SZ, fd)) {
		nr++;
	}

	fclose(fd);
	printf("max partition nr %d\n", nr);
	// printf("max partition nr %d\n", nr);
	/* keep the way before do */
	return nr - 1;
}
/*****************************************************************************/

CVI_S32 offshift(CVI_UL n)
{
	CVI_S32 shift = -1;

	while (n) {
		n = n >> 1;
		shift++;
	}
	return shift;
}
/*****************************************************************************/

static CVI_CHAR *skip_space(CVI_CHAR *line)
{
	CVI_CHAR *p = line;

	while (*p == ' ' || *p == '\t') {
		p++;
	}
	return p;
}

static CVI_CHAR *get_word(CVI_CHAR *line, CVI_CHAR *value)
{
	CVI_CHAR *p = line;

	p = skip_space(p);
	while (*p != '\t' && *p != ' ' && *p != '\n' && *p != 0) {
		*value++ = *p++;
	}
	*value = 0;
	return p;
}

static CVI_S32 get_bootargs(CVI_CHAR *pu8Bootarags, CVI_U16 u16Len)
{
	FILE *pf;

	if (pu8Bootarags == NULL) {
		printf("Pointer is null.\n");
		return -1;
	}

	pf = fopen("/proc/cmdline", "r");
	if (pf == NULL) {
		printf("Failed to open '/proc/cmdline'.\n");
		return -1;
	}

	if (fgets((CVI_CHAR *)pu8Bootarags, u16Len, pf) == NULL) {
		printf("Failed to fgets string.\n");
		fclose(pf);
		return -1;
	}

	fclose(pf);
	return 0;
}
/*
 *static CVI_FLASH_TYPE_E get_flashtype_by_bootargs(const CVI_CHAR *pszBootargs, CVI_CHAR *pszPartitionName)
 *{
 *	CVI_CHAR *pszPartitionPos = NULL;
 *	CVI_CHAR *pszTmpPos = NULL;
 *	CVI_CHAR pszTmpStr[64];
 *	CVI_S32 i;
 *	CVI_FLASH_TYPE_E enFlashType = CVI_FLASH_TYPE_BUTT;
 *	CVI_CHAR szTypeStr[CVI_FLASH_TYPE_BUTT][16] = {
 *		"cvi_sfc:",
 *		"ubi.mtd",
 *		"mmcblk0:"
 *	};
 *	CVI_CHAR *pszType[CVI_FLASH_TYPE_BUTT];
 *
 *	if (pszPartitionName == NULL) {
 *		return CVI_FLASH_TYPE_BUTT;
 *	}
 *
 *	for (i = 0; i < CVI_FLASH_TYPE_BUTT; i++) {
 *		pszType[i] = strstr((CVI_CHAR *)pszBootargs, szTypeStr[i]);
 *	}
 *
 *	memset(pszTmpStr, 0, sizeof(pszTmpStr));
 *	(CVI_VOID)snprintf(pszTmpStr, sizeof(pszTmpStr) - 1, "(%s)", pszPartitionName);
 *	pszPartitionPos = strstr((CVI_CHAR *)pszBootargs, pszTmpStr);
 *	if (pszPartitionPos == NULL) {
 *		return CVI_FLASH_TYPE_BUTT;
 *	}
 *
 *	for (i = 0; i < CVI_FLASH_TYPE_BUTT; i++) {
 *		if (pszType[i] == NULL) {
 *			continue;
 *		}
 *
 *		if ((pszPartitionPos >= pszType[i]) && (pszType[i] >= pszTmpPos)) {
 *			enFlashType = (CVI_FLASH_TYPE_E)i;
 *			pszTmpPos = pszType[i];
 *		}
 *	}
 *	return enFlashType;
 *}
 */
static struct CVI_Flash_PartInfo_S gPartInfo[MAX_MTD_PARTITION];

CVI_S32 flash_partition_info_init(CVI_VOID)
{
	CVI_S32 ret = -1;
	CVI_S32 i = 0;
	CVI_CHAR bootargs[512];
	FILE *fp = CVI_NULL;
	static CVI_S32 init_flag = -1;
	struct CVI_Flash_PartInfo_S *pstPartInfo = NULL;

	if (init_flag >= 0) {
		return 0;
	}

	for (i = 0; i < MAX_MTD_PARTITION; i++) {
		pstPartInfo = &gPartInfo[i];
		pstPartInfo->StartAddr = 0;
		pstPartInfo->PartSize = 0;
		pstPartInfo->BlockSize = 0;
		pstPartInfo->FlashType = CVI_FLASH_TYPE_BUTT;
		pstPartInfo->perm = ACCESS_BUTT;
		memset(pstPartInfo->DevName, '\0', FLASH_NAME_LEN);
		memset(pstPartInfo->PartName, '\0', FLASH_NAME_LEN);
	}

	ret = get_bootargs(bootargs, sizeof(bootargs) - 1);

	if (ret != 0) {
		return ret;
	}

	fp = fopen("/proc/mtd", "r");
	if (fp) {
		CVI_U64 u64StartAddr[CVI_FLASH_TYPE_BUTT];
		CVI_CHAR line[512];

		if (fgets(line, sizeof(line), fp) == NULL) { // skip first line
			fclose(fp);
			return -1;
		}

		//printf(" DevName\t  PartSize\tBlockSize	PartName	Startaddr\n");
		for (i = 0; i < CVI_FLASH_TYPE_BUTT; i++) {
			u64StartAddr[i] = 0;
		}

		i = 0;

		while (i < MAX_MTD_PARTITION) {
			CVI_CHAR argv[4][32];
			CVI_CHAR *p;

			if (fgets(line, sizeof(line), fp) == NULL) {
				break;
			}

			p = &line[0];
			p = get_word(p, argv[0]);
			p = get_word(p, argv[1]);
			p = get_word(p, argv[2]);
			p = get_word(p, argv[3]);

			pstPartInfo = &gPartInfo[i];
			pstPartInfo->PartSize = (CVI_U64)strtoull(argv[1], NULL, 16);		  // partion size
			pstPartInfo->BlockSize = (CVI_U64)(CVI_U32)strtol(argv[2], NULL, 16);  // erase size
			strncpy(pstPartInfo->DevName, argv[0], (strlen(argv[0]) - 1));
			strncpy(pstPartInfo->PartName, (argv[3] + 1), (strlen(argv[3]) - 2));

			// comment:To avoid bootargs diff in nand/emmc. just for nand usage:
			//pstPartInfo->FlashType = get_flashtype_by_bootargs(bootargs, pstPartInfo->PartName);
			pstPartInfo->FlashType = CVI_FLASH_TYPE_NAND_0;

			pstPartInfo->StartAddr = u64StartAddr[pstPartInfo->FlashType];
			u64StartAddr[pstPartInfo->FlashType] += pstPartInfo->PartSize;

			//printf("pstPartInfo[%d].FlashType: %d\n", i, pstPartInfo->FlashType);
			//printf("pstPartInfo[%d].DevName: %s\n", i, pstPartInfo->DevName);
			//printf("pstPartInfo[%d].PartSize: 0x%lx\n", i, pstPartInfo->PartSize);
			///printf("pstPartInfo[%d].BlockSize: 0x%x\n", i, pstPartInfo->BlockSize);
			//printf("pstPartInfo[%d].PartName: %s\n", i, pstPartInfo->PartName);
			//printf("gPartInfo[%d].StartAddr: 0x%lx\n", i, gPartInfo[i].StartAddr);
			//printf("gPartInfo[%d].DevName: %s\n", i, gPartInfo[i].DevName);
			i++;
		}

		fclose(fp);
	} else {
		return -1;
	}

	init_flag = 0;
	return 0;
}

struct CVI_Flash_PartInfo_S *get_flash_partition_info(CVI_FLASH_TYPE_E FlashType, const CVI_CHAR *devname)
{
	CVI_S32 i;
	struct CVI_Flash_PartInfo_S *pstPartInfo = NULL;

	if (devname == NULL) {
		return NULL;
	}

	for (i = 0; i < MAX_MTD_PARTITION; i++) {
		pstPartInfo = &gPartInfo[i];

		if (pstPartInfo->FlashType != FlashType) {
			continue;
		}

		if (strncmp(pstPartInfo->DevName, devname,
					 strlen(pstPartInfo->DevName) > strlen(devname) ?
					 strlen(pstPartInfo->DevName):strlen(devname)) == 0) {
			return pstPartInfo;
		}
	}

	return NULL;
}

CVI_U64 get_flash_total_size(CVI_FLASH_TYPE_E FlashType)
{
	CVI_S32 i;
	CVI_U64 totalsize = 0;
	struct CVI_Flash_PartInfo_S *pstPartInfo = NULL;

	for (i = 0; i < MAX_MTD_PARTITION; i++) {
		pstPartInfo = &gPartInfo[i];

		if (pstPartInfo->FlashType != FlashType) {
			continue;
		}

		totalsize += pstPartInfo->PartSize;
	}

	return totalsize;
}

