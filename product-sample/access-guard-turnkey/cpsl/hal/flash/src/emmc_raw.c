#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cvi_flash.h"
#include "nand.h"
#include "emmc_raw.h"

struct EMMC_FLASH_S g_stEmmcFlash;

#define EMMC_RAW_AREA_START 0  // EMMC_SECTOR_SIZE

CVI_S32 find_part_from_devname(CVI_CHAR *media_name, CVI_CHAR *bootargs,
							  CVI_CHAR *devname, CVI_U64 *start, CVI_U64 *size)
{
	CVI_U8 partnum = 0;
	CVI_CHAR *tmp;

	if (!devname || (*devname == '\0')) {
		return CVI_FAILURE;
	}

	tmp = strstr(bootargs, "blkdevparts=");
	if (!(tmp)) {
		return CVI_FAILURE;
	}

	tmp += strlen("blkdevparts=");

	if (!strstr(bootargs, media_name)) {
		return CVI_FAILURE;
	}

	tmp = strstr(devname, "mmcblk0p");
	if (!(tmp)) {
		return CVI_FAILURE;
	}

	tmp += strlen("mmcblk0p");
	partnum = (CVI_U8)strtol(tmp, NULL, 10);

	if (get_part_info(partnum, start, size)) {
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 emmc_raw_init(CVI_CHAR *bootargs)
{
	CVI_U8 aucBuf[512];
	CVI_S32 dev_fd = -1;
	CVI_S32 tmp_fd = -1;
	CVI_S32 ret = -1;

	if (!bootargs) {
		printf("Invalid parameter, bootargs NULL\n");
		return CVI_FAILURE;
	}

	ret = cmdline_parts_init(bootargs);

	if (ret < 0) {
#ifdef EMMC_RAW_DBG
		printf("cmdline parts init failed, ret=%d, bootargs:%s\n", ret, bootargs);
#endif
		return CVI_FAILURE;
	}

#if defined(ANDROID)

	dev_fd = open("/dev/block/mmcblk0", O_RDWR);
	if ((dev_fd) == -1)
#else
	if ((dev_fd) == -1)
#endif
	{
		printf("open mmcblk0 failed, errno=%d\n", errno);
		return CVI_FAILURE;
	}

	g_stEmmcFlash.u32EraseSize = EMMC_SECTOR_SIZE;

	memset(aucBuf, 0, sizeof(aucBuf));
	if (((ssize_t)sizeof(aucBuf) - 1) != read(dev_fd, aucBuf, sizeof(aucBuf) - 1)) {
		printf("Failed to read dev, errno=%d\n", errno);
		close(dev_fd);
		return CVI_FAILURE;
	}

	close(dev_fd);

	/* Raw area start from 512, after MBR.. */
	g_stEmmcFlash.u64RawAreaStart = EMMC_RAW_AREA_START;
	tmp_fd = open("/sys/block/mmcblk0/size", O_RDONLY);

	if (tmp_fd < 0) {
		printf("Fail to open the size of mmcblk0\n");
		return CVI_FAILURE;
	}

	memset(aucBuf, 0, sizeof(aucBuf));
	if (read(tmp_fd, aucBuf, sizeof(aucBuf) - 1) < 0) {
		printf("Failed to read the size of mmcblk0\n");
		close(tmp_fd);
		return CVI_FAILURE;
	}
	close(tmp_fd);

	g_stEmmcFlash.u64RawAreaSize = (CVI_U64)strtoull((CVI_CHAR *)aucBuf, NULL, 10)
								   * EMMC_SECTOR_SIZE - EMMC_RAW_AREA_START;

	return CVI_SUCCESS;
}

static CVI_S32 emmc_flash_probe(CVI_VOID)
{
	CVI_S32 dev;
#if defined(ANDROID)
	dev = open("/dev/block/mmcblk0", O_RDWR | O_SYNC);
	if (dev == -1)
#else
	dev = open("/dev/block/mmcblk0", O_RDWR | O_SYNC);
	if (dev == -1)
#endif
	{
		printf("Failed to open device '/dev/mmcblk0'.");
		return CVI_FAILURE;
	}

	return dev;
}

struct EMMC_CB_S *emmc_raw_open(CVI_U64 u64Addr,
						 CVI_U64 u64Length)
{
	struct EMMC_CB_S *pstEmmcCB;
	CVI_S32 fd;

	fd = emmc_flash_probe();
	if (-1 == fd) {
		printf("no devices available.");
		return NULL;
	}

	/* Reject open, which are not block aligned */
	if ((u64Addr & (g_stEmmcFlash.u32EraseSize - 1))
		|| (u64Length & (g_stEmmcFlash.u32EraseSize - 1))) {
		//printf("Attempt to open non block aligned,
		//		 eMMC blocksize: 0x%x, address: 0x%08llx, length: 0x%08llx.",
		//		 g_stEmmcFlash.u32EraseSize,
		//		 u64Addr,
		//		 u64Length);
		close(fd);
		return NULL;
	}

	if ((u64Addr > g_stEmmcFlash.u64RawAreaStart + g_stEmmcFlash.u64RawAreaSize)
		|| (u64Length > g_stEmmcFlash.u64RawAreaStart + g_stEmmcFlash.u64RawAreaSize)
		|| ((u64Addr + u64Length) > g_stEmmcFlash.u64RawAreaStart + g_stEmmcFlash.u64RawAreaSize)) {
		//printf("Attempt to open outside the flash area,
		//		 eMMC chipsize: 0x%08llx, address: 0x%08llx, length: 0x%08llx\n",
		//		 g_stEmmcFlash.u64RawAreaStart + g_stEmmcFlash.u64RawAreaSize, u64Addr, u64Length);
		close(fd);
		return NULL;
	}

	pstEmmcCB = (struct EMMC_CB_S *)malloc(sizeof(struct EMMC_CB_S));
	if (pstEmmcCB == NULL) {
		printf("no many memory.");
		close(fd);
		return NULL;
	}
	memset(pstEmmcCB, 0, sizeof(struct EMMC_CB_S));

	pstEmmcCB->u64Address = u64Addr;
	pstEmmcCB->u64PartSize = u64Length;
	pstEmmcCB->u32EraseSize = g_stEmmcFlash.u32EraseSize;
	pstEmmcCB->fd = fd;
	pstEmmcCB->enPartType = EMMC_PART_TYPE_RAW;

	return pstEmmcCB;
}

struct EMMC_CB_S *emmc_node_open(const CVI_U8 *pu8Node)
{
	struct EMMC_CB_S *pstEmmcCB;
	CVI_S32 fd;

	if (pu8Node == NULL) {
		return NULL;
	}

	fd = open((const CVI_CHAR *)pu8Node, O_RDWR);
	if (fd == -1) {
		printf("no devices available.");
		return NULL;
	}

	pstEmmcCB = (struct EMMC_CB_S *)malloc(sizeof(struct EMMC_CB_S));
	if (pstEmmcCB == NULL) {
		printf("No enough space.");
		close(fd);
		return NULL;
	}
	memset(pstEmmcCB, 0, sizeof(struct EMMC_CB_S));

	pstEmmcCB->u64Address = 0;
	pstEmmcCB->u64PartSize = 0;
	pstEmmcCB->u32EraseSize = g_stEmmcFlash.u32EraseSize;
	pstEmmcCB->fd = fd;
	pstEmmcCB->enPartType = EMMC_PART_TYPE_LOGIC;

	return pstEmmcCB;
}

CVI_S32 emmc_block_read(CVI_S32 fd,
					   CVI_U64 u64Start,
					   CVI_U32 u32Len,
					   CVI_VOID *buff)
{
	CVI_S32 s32Ret = CVI_FAILURE;
	ssize_t length;

	if (lseek64(fd, (off64_t)u64Start, SEEK_SET) == -1) {
		printf("Failed to lseek64.");
		return s32Ret;
	}
	if (buff == NULL) {
		return CVI_FAILURE;
	}

	length = read(fd, buff, u32Len);

	if (length < 0) {
		return s32Ret;
	}

	return (CVI_S32)length;
}

CVI_S32 emmc_block_write(CVI_S32 fd,
						CVI_U64 u64Start,
						CVI_U32 u32Len,
						const CVI_VOID *buff)
{
	CVI_S32 s32Ret;

	if (lseek64(fd, (off64_t)u64Start, SEEK_SET) == -1) {
		printf("Failed to lseek64.");
		return CVI_FAILURE;
	}
	if (buff == NULL) {
		return CVI_FAILURE;
	}

	s32Ret = write(fd, buff, u32Len);

	if (s32Ret < 0) {
		return CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 emmc_raw_read(const struct EMMC_CB_S *pstEmmcCB,
			 CVI_U64 u64Offset, /* should be alignment with emmc block size */
				 CVI_U32 u32Length, /* should be alignment with emmc block size */
					 CVI_U8 *buf)
{
	CVI_S32 S32Ret;
	CVI_U64 u64Start;

	if (pstEmmcCB == NULL || buf == NULL) {
		printf("Pointer is null.");
		return CVI_FAILURE;
	}

	/* Reject read, which are not block aligned */
	if (pstEmmcCB->enPartType == EMMC_PART_TYPE_RAW) {
		if ((u64Offset > pstEmmcCB->u64PartSize)
			|| (u32Length > pstEmmcCB->u64PartSize)
			|| ((u64Offset + u32Length) > pstEmmcCB->u64PartSize)) {
			//printf("Attempt to write outside the flash handle area,
			//		 eMMC part size: 0x%08llx, offset: 0x%08llx,
			//		 length: 0x%08x.",
			//		 pstEmmcCB->u64PartSize,
			//		 u64Offset,
			//		 u32Length);

			return CVI_FAILURE;
		}
	}

	u64Start = pstEmmcCB->u64Address + u64Offset;
	S32Ret = emmc_block_read(pstEmmcCB->fd, u64Start, u32Length, buf);
	return S32Ret;
}

CVI_S32 emmc_raw_write(const struct EMMC_CB_S *pstEmmcCB,
			CVI_U64 u64Offset, /* should be alignment with emmc block size */
				CVI_U32 u32Length, /* should be alignment with emmc block size */
					const CVI_U8 *buf)
{
	CVI_S32 S32Ret;
	CVI_U64 u64Start;

	if (pstEmmcCB == NULL || buf == NULL) {
		printf("Pointer is null.");
		return CVI_FAILURE;
	}

	if (pstEmmcCB->enPartType == EMMC_PART_TYPE_RAW) {
		if ((u64Offset > pstEmmcCB->u64PartSize)
			|| (u32Length > pstEmmcCB->u64PartSize)
			|| ((u64Offset + u32Length) > pstEmmcCB->u64PartSize)) {
		//	printf("Attempt to write outside the flash handle area,
		//			 eMMC part size: 0x%08llx, offset: 0x%08llx, length: 0x%08x\n",
		//			 pstEmmcCB->u64PartSize, u64Offset, u32Length);

			return CVI_FAILURE;
		}
	}

	u64Start = pstEmmcCB->u64Address + u64Offset;
	S32Ret = emmc_block_write(pstEmmcCB->fd, u64Start, u32Length, buf);
	return S32Ret;
}

CVI_S32 emmc_raw_close(struct EMMC_CB_S *pstEmmcCB)
{
	if (pstEmmcCB == NULL) {
		printf("Pointer is null.");
		return CVI_FAILURE;
	}

	close(pstEmmcCB->fd);
	free(pstEmmcCB);
	pstEmmcCB = NULL;

	return CVI_SUCCESS;
}

#if defined(CVI_EMMC_ERASE_SUPPORT)
CVI_S32 emmc_raw_erase(struct EMMC_CB_S *pstEmmcCB,
					  CVI_U64 u64Offset,
					  CVI_U64 u64Length)
{
	CVI_S32 S32Ret;
	CVI_U64 u64Start;
	struct mmc_erase_cmd cmd;

	if (pstEmmcCB == NULL) {
		printf("Pointer is null.");
		return CVI_FAILURE;
	}

	if (pstEmmcCB->enPartType == EMMC_PART_TYPE_RAW) {
		if ((u64Offset > pstEmmcCB->u64PartSize)
			|| (u64Length > pstEmmcCB->u64PartSize)
			|| ((u64Offset + u64Length) > pstEmmcCB->u64PartSize)) {
		//	printf("Attempt to write outside the flash handle area,
		//			 eMMC part size: 0x%08llx, offset: 0x%08llx,
		//			 length: 0x%08llx\n",
		//			 pstEmmcCB->u64PartSize,
		//			 u64Offset,
		//			 u64Length);

			return CVI_FAILURE;
		}
	}

	u64Start = pstEmmcCB->u64Address + u64Offset;
	// printf("[%s][%d][%s] u64Offset=0x%08llx, u32Length=0x%08llx, u64Start=0x%08llx, fd=%d\n", __FILE__,
	//	__LINE__, __FUNCTION__, u64Offset, u64Length, u64Start, pstEmmcCB->fd);
	memset(&cmd, 0, sizeof(cmd));

	cmd.from = (CVI_U32)(u64Start >> 9);
	cmd.nr = (CVI_U32)(u64Length >> 9);
	cmd.arg = 0x01;
	// printf("from=0x%x, nr=0x%x\n", cmd.from, cmd.nr);
	S32Ret = ioctl(pstEmmcCB->fd, MMC_ERASE_CMD, &cmd);
	if (S32Ret) {
		printf("ioctl: ret=%d\n", S32Ret);
	}

	return S32Ret;
}
#endif

