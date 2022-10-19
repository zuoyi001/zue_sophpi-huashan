
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "cvi_log.h"
#include "crc32.h"

#include "cvi_upgrade.h"
#include "cvi_flash.h"
#include "cvi_fip.h"

/** color log macro define */
#define NONE         "\033[m"
#define RED          "\033[0;32;31m"
#define LIGHT_RED    "\033[1;31m"
#define GREEN        "\033[0;32;32m"
#define LIGHT_GREEN  "\033[1;32m"
#define BLUE         "\033[0;32;34m"
#define LIGHT_BLUE   "\033[1;34m"
#define DARY_GRAY    "\033[1;30m"
#define CYAN         "\033[0;36m"
#define LIGHT_CYAN   "\033[1;36m"
#define PURPLE       "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN        "\033[0;33m"
#define YELLOW       "\033[1;33m"
#define LIGHT_GRAY   "\033[0;37m"
#define WHITE        "\033[1;37m"

/** upgrade package information */
typedef struct _UPGRADE_PKG_INFO_S {
	CVI_U32  u32PkgLen;
	CVI_CHAR szSoftVersion[CVI_COMM_STR_LEN]; /* Package version */
	CVI_CHAR szModel[CVI_COMM_STR_LEN];   /* Package model */
	CVI_CHAR szPkgFilePath[CVI_COMM_PATH_MAX_LEN];
} UPGRADE_PKG_INFO_S;

/*
 *	CIMG Header format total 64 bytes
 *	4 Bytes: Magic
 *	4 Bytes: Version
 *	4 Bytes: Chunk header size
 *	4 Bytes: Total chunks
 *	4 Bytes: File size
 *	32 Bytes: Extra Flags
 *	12 Bytes: Reserved
 */
typedef struct cviUPGRADE_PARTITION_HEAD_S {
	CVI_U32  u32Magic;
	CVI_U32  u32Version;
	CVI_U32  u32ChunkHeaderSize;
	CVI_U32  u32TotalChunk;
	CVI_U32  u32DataLen;
	CVI_CHAR szExtraFlags[32];
	CVI_CHAR reserved[12];
} CVI_UPGRADE_PARTITION_HEAD_S;

/*
 *	CIMG Chunk format total 64 bytes
 *	4 Bytes: Chunk Type
 *	4 Bytes: Chunk data size
 *	4 Bytes: Program offset
 *	4 Bytes: Crc32 checksum
 *	48 Bytes: Reserved
 */
typedef struct cviUPGRADE_CHUNK_HEAD_S {
	CVI_U32  u32ChunkType;
	CVI_U32  u32DataSize;
	CVI_U32  u32ProgramOffset;
	CVI_U32  u32Crc32;
	CVI_U32  reserved[12];
} CVI_UPGRADE_CHUNK_HEAD_S;

#define MAX_PARTS 32

/** MTD partition descriptions */
typedef struct cvi_MTDInfo_S {
	CVI_U64 StartAddr;                 /* Partiton start address */
	CVI_U64 PartSize;                  /* Partition size */
	CVI_U32 BlockSize;                 /* The Block size of the flash where this partition at */
	CVI_CHAR DevName[CVI_COMM_STR_LEN];  /* The device node name where this partition relate to */
	CVI_CHAR PartName[CVI_COMM_STR_LEN]; /* The partition name of this partition */
} CVI_MTDInfo_S;

typedef struct {
	CVI_U32 curPercent;
	CVI_U32 offset;
	CVI_U32 totalSize;
} CVI_UPGRADE_Progress;

typedef void (*event_cb_t)(CVI_UPGRADE_EVENT_S *event);

/** upgrade module context */
typedef struct cviUPGRADE_CONTEXT_S {
	CVI_FLASH_TYPE_E FlashType;
	int gMTDPartNum;
	CVI_MTDInfo_S gPartInfo[MAX_PARTS];
	CVI_UPGRADE_Progress gProgress;
	event_cb_t event_cbFunc;
} UPGRADE_CONTEXT_S;
static UPGRADE_CONTEXT_S gUPGRADECtx;

#ifndef MKFOURCC
#define MKFOURCC(a, b, c, d) (a | (b << 8) | (c << 16) | (d << 24))
#endif

#define PROC_MTD_PATH "/proc/mtd"
#define DEV_MMC0_PATh "/sys/class/mmc_host/mmc0"
#define FLASH_ERASE_CMD_FMT "./flash_erase %s 0 0"
#define NANDWRITE_CMD_FMT   "./nandwrite -q -m -p %s -"

#define UPGRADE_IMGDATA_BUFF_MAX_SIZE   (0x40000)
#define UPGRADE_BLOCK_SIZE              (0x10000) /**<64K */
#define CVI_UPGRADE_PACKAGE_HEAD_MAGIC   (0x55667788)
#define CVI_UPGRADE_PART_HEAD_MAGIC   MKFOURCC('C', 'I', 'M', 'G')
#define UPGRADE_PKG_BUFF_SIZE (sizeof(CVI_UPGRADE_PKG_HEAD_S) + sizeof(CVI_UPGRADE_PARTITION_HEAD_S) \
										+ UPGRADE_IMGDATA_BUFF_MAX_SIZE)

/** Pointer Check */
#define CVI_SAFECHECK_POINTER(p, errcode)    \
	do {                                        \
		if (!(p)) {                             \
			CVI_LOGI("pointer[%s] is NULL\n", #p); \
			return errcode;                     \
		}                                       \
	} while (0)

/** Memory Safe Free */
#define CVI_SAFECHECK_FREE(p) \
	do {                        \
		if (NULL != (p)) {      \
			free(p);            \
			(p) = NULL;         \
		}                       \
	} while (0)
static CVI_S32 UPGRADE_ReadPartitionHead(CVI_S32 fd, CVI_UPGRADE_PARTITION_HEAD_S *partitionHead, CVI_U32 *u32StartOffset, CVI_U32 *RawDataLen);
static CVI_S32 UPGRADE_CheckFlashSize(const CVI_UPGRADE_PKG_HEAD_S *pktHead)
{
	UNUSED(pktHead);
#ifdef _FOOLPROOF_
	// TODO
#endif
	return CVI_SUCCESS;
}

static CVI_S32 UPGRADE_CheckALLPartitionsSize(const CVI_S32 pktFd, const CVI_UPGRADE_PKG_HEAD_S *pktHead)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_UPGRADE_PARTITION_HEAD_S partitionHead;
	CVI_U32 u32StartOffset;
	CVI_U32 u32RawDataLen;

	CVI_LOGD("PkgLen=0x%X, PartitionCnt=%d\n", pktHead->u32PkgLen, pktHead->s32PartitionCnt);
	for (int i = 0; i < pktHead->s32PartitionCnt; i++) {
		lseek(pktFd, pktHead->au32PartitionOffSet[i], SEEK_SET);
		s32Ret = UPGRADE_ReadPartitionHead(pktFd, &partitionHead, &u32StartOffset, &u32RawDataLen);
		if (s32Ret != CVI_SUCCESS) {
			CVI_LOGE("UPGRADE_ReadPartitionHead failed\n");
			return CVI_UPGRADE_EINTR;
		}
		s32Ret = CVI_FAILURE;
		for (int j = 0; j < gUPGRADECtx.gMTDPartNum; j++)
		{
			if (strncmp(gUPGRADECtx.gPartInfo[j].PartName, partitionHead.szExtraFlags, 31) == 0) {
				if (gUPGRADECtx.gPartInfo[j].StartAddr == (CVI_U64) u32StartOffset &&
					gUPGRADECtx.gPartInfo[j].PartSize >= (CVI_U64) u32RawDataLen) {
						CVI_LOGI("Part-[%d] %s Matched\n", i, partitionHead.szExtraFlags);
						s32Ret = CVI_SUCCESS;
						break;
				} else {
					CVI_LOGE(RED"Part-[%d] %s Un-matched.\nPKG: Length\t%u\tStartOffset\t0x%x\nDEV: PartSize\t%"PRId64"\tStartAddress\t0x%"PRIx64""NONE"\n"
						, i, partitionHead.szExtraFlags
						, u32RawDataLen, u32StartOffset
						, gUPGRADECtx.gPartInfo[j].PartSize, gUPGRADECtx.gPartInfo[j].StartAddr);
					return CVI_UPGRADE_EPKG_UNMATCH;
				}
			}
		}
		if (s32Ret != CVI_SUCCESS) {
			CVI_LOGE(RED"UPGRADE_CheckALLPartitionsSize failed. Part-[%d] %s Not found."NONE"\n", i, partitionHead.szExtraFlags);
			return CVI_UPGRADE_EPKG_UNMATCH;
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 UPGRADE_CompareVersion(const CVI_CHAR *pszVer1, const CVI_CHAR *pszVer2)
{
	CVI_S32 s32Ret = strncmp(pszVer1, pszVer2, CVI_COMM_STR_LEN);

	if (s32Ret == 0) {
		CVI_LOGI("Ver1[%s] is the same with Ver2[%s]\n", pszVer1, pszVer2);
		return 0;
	} else if (s32Ret < 0) {
		CVI_LOGI("Ver1[%s] is older than Ver2[%s]\n", pszVer1, pszVer2);
		return -1;
	}
	CVI_LOGI("Ver1[%s] is newer than Ver2[%s]\n", pszVer1, pszVer2);
	return 1;
}

static int _Lookup_MTDdev_Index(const CVI_CHAR *name)
{
	int i;

	//Invalid string
	if (!name || name[0] == '\0') {
		return -1;
	}

	for (i = 0; i < gUPGRADECtx.gMTDPartNum; i++) {
		if (strcmp(gUPGRADECtx.gPartInfo[i].PartName, name) == 0) {
			return i;
		}
	}

	return -1;
}

static void _init_progress(CVI_U32 totalSize)
{
	gUPGRADECtx.gProgress.curPercent = 0;
	gUPGRADECtx.gProgress.offset = 0;
	gUPGRADECtx.gProgress.totalSize = totalSize;
}

static CVI_VOID UPGRADE_ReportProgress(CVI_U32 offsetPos, CVI_U32 writtenSize)
{
	CVI_U32 offset = gUPGRADECtx.gProgress.offset;
	CVI_U32 totalSize = gUPGRADECtx.gProgress.totalSize;
	CVI_U32 percentage;

	if (offsetPos > 0) {
		offset = offsetPos;
	} else {
		offset += writtenSize;
	}
	percentage = offset/(totalSize/100);
	if (percentage > 100)
		percentage = 100;

	if (gUPGRADECtx.gProgress.offset == 0 || percentage > gUPGRADECtx.gProgress.curPercent) {
		if (gUPGRADECtx.event_cbFunc) {
			CVI_UPGRADE_EVENT_S event;

			memset(&event, 0, sizeof(CVI_UPGRADE_EVENT_S));
			event.eventID = CVI_UPGRADE_EVENT_PROGRESS;
			event.argv = (CVI_VOID *) (intptr_t) percentage;
			gUPGRADECtx.event_cbFunc(&event);
		}
		CVI_LOGD(LIGHT_BLUE"Total=0x%X, Offset=0x%X, Percentage=%d%%\n"NONE, totalSize, offset, percentage);
	}
	gUPGRADECtx.gProgress.curPercent = percentage;
	gUPGRADECtx.gProgress.offset = offset;
}

static CVI_S32 UPGRADE_ReadPartitionHead(CVI_S32 fd, CVI_UPGRADE_PARTITION_HEAD_S *partitionHead, CVI_U32 *StartOffset, CVI_U32 *RawDataLen)
{
	CVI_S32 readLen = 0;
	CVI_U32 i;

	readLen = read(fd, partitionHead, sizeof(CVI_UPGRADE_PARTITION_HEAD_S));
	if (readLen < 0) {
		CVI_LOGE("read file failed\n");
		return CVI_UPGRADE_EINTR;
	}

	/* Check Partition Head Info */
	if (partitionHead->u32Magic != CVI_UPGRADE_PART_HEAD_MAGIC) {
		CVI_LOGE("Invalid HeadMagic[%#08x] expect[%#08x]\n",
			partitionHead->u32Magic, CVI_UPGRADE_PART_HEAD_MAGIC);
		close(fd);
		return CVI_UPGRADE_EPKG_INVALID;
	}

	CVI_LOGD("Partition Head:\n");
	CVI_LOGD("  PartitionName[%s] ChunkHeaderSize[%u] ChunkNum[%u] DataLen[%u]\n",
		  partitionHead->szExtraFlags, partitionHead->u32ChunkHeaderSize
		  , partitionHead->u32TotalChunk, partitionHead->u32DataLen);
	if (RawDataLen)
		*RawDataLen = partitionHead->u32DataLen - (partitionHead->u32TotalChunk * partitionHead->u32ChunkHeaderSize);

	CVI_UPGRADE_CHUNK_HEAD_S *chunkHead = calloc(1, partitionHead->u32ChunkHeaderSize);

	for (i = 0; i < partitionHead->u32TotalChunk; i++) {
		CVI_S32 s32Ret;

		readLen = read(fd, chunkHead, partitionHead->u32ChunkHeaderSize);
		if (readLen < 0) {
			CVI_LOGE("read file failed\n");
			close(fd);
			CVI_SAFECHECK_FREE(chunkHead);
			return CVI_UPGRADE_EINTR;
		}

		CVI_LOGD("  Chunk[%d] Head:\n", i);
		CVI_LOGD("    ChunkDataSize[%u] ProgramOffset[0x%x]\n"
					, chunkHead->u32DataSize, chunkHead->u32ProgramOffset);
		if (StartOffset && i == 0)
			*StartOffset = chunkHead->u32ProgramOffset;

		s32Ret = lseek(fd, chunkHead->u32DataSize, SEEK_CUR);
		if (s32Ret < 0) {
			CVI_LOGE("seek file failed\n");
			close(fd);
			CVI_SAFECHECK_FREE(chunkHead);
			return CVI_UPGRADE_EINTR;
		}
	}

	CVI_SAFECHECK_FREE(chunkHead);

	return CVI_SUCCESS;
}


static CVI_S32 UPGRADE_WritePartitionFromChunk(CVI_S32 fd, CVI_UPGRADE_PARTITION_HEAD_S *partitionHead
						, const char *partName, CVI_U8 *puReadBuf)
{
	CVI_S32 readLen = 0;
	CVI_U32 i;
	FILE *file = NULL;
	CVI_U32 eraseUnit = 0;
	// CVI_U64 startAddr = 0;

	if (strcmp(partName, "") != 0 && gUPGRADECtx.FlashType == CVI_FLASH_TYPE_NAND_0) {
		int idx = _Lookup_MTDdev_Index(partName);

		if (strcmp(partName, "fip") == 0) {
			CVI_S32 s32Ret;
			CVI_UPGRADE_CHUNK_HEAD_S *chunkHead = calloc(1, partitionHead->u32ChunkHeaderSize);

			readLen = read(fd, chunkHead, partitionHead->u32ChunkHeaderSize);
			if (readLen < (CVI_S32) partitionHead->u32ChunkHeaderSize) {
				CVI_LOGE("read file failed\n");
				close(fd);
				CVI_SAFECHECK_FREE(chunkHead);
				return CVI_UPGRADE_EINTR;
			}
#define MAX_BUFF_RW (4*1024*1024)
			char *FipBin = calloc(1, MAX_BUFF_RW);
			readLen = read(fd, FipBin, chunkHead->u32DataSize);
			if (readLen < (CVI_S32) chunkHead->u32DataSize) {
				CVI_LOGE("read fip.bin failed\n");
				close(fd);
				CVI_SAFECHECK_FREE(chunkHead);
				CVI_SAFECHECK_FREE(FipBin);
				return CVI_UPGRADE_EINTR;
			}
			printf("Read fip.bin size %d bytes\n", readLen);
			s32Ret = spi_nand_check_write_vector(FipBin);
			CVI_SAFECHECK_FREE(chunkHead);
			CVI_SAFECHECK_FREE(FipBin);
			return s32Ret;
		}

		if (idx >= 0) {
#ifdef _USE_NANDWRITE_
			if (access("./flash_erase", X_OK) == 0 && access("./nandwrite", X_OK) == 0) {
				char shellcmd[256];

				snprintf(shellcmd, sizeof(shellcmd)
					, FLASH_ERASE_CMD_FMT, gUPGRADECtx.gPartInfo[idx].DevName);
				// CVI_LOGI(LIGHT_GREEN"system(%s)\n"NONE, shellcmd);
				system(shellcmd);
				snprintf(shellcmd, sizeof(shellcmd)
					, NANDWRITE_CMD_FMT, gUPGRADECtx.gPartInfo[idx].DevName);
				// CVI_LOGI(LIGHT_GREEN"popen(%s)\n"NONE, shellcmd);
				file = popen(shellcmd, "w");
			} else {
				CVI_LOGE(RED"Cannot find flash_erase/nandwrite; required for spinand device\n"NONE);
				close(fd);
				return CVI_UPGRADE_EINTR;
			}
#else
			CVI_S32  ret;

			ret = CVI_Flash_Open(gUPGRADECtx.FlashType, gUPGRADECtx.gPartInfo[idx].DevName
					, gUPGRADECtx.gPartInfo[idx].StartAddr
					, gUPGRADECtx.gPartInfo[idx].PartSize);
			if (ret == CVI_FAILURE) {
				CVI_LOGE(RED"Cannot CVI_Flash_Open %s\n"NONE, gUPGRADECtx.gPartInfo[idx].DevName);
				close(fd);
				return CVI_UPGRADE_EINTR;
			}

			ret = CVI_Flash_Erase(gUPGRADECtx.FlashType
					, gUPGRADECtx.gPartInfo[idx].StartAddr
					, gUPGRADECtx.gPartInfo[idx].PartSize);
			if (ret >= 0) {
				CVI_LOGD(LIGHT_BLUE"CVI_Flash_Erase %s success %d\n"NONE
					, gUPGRADECtx.gPartInfo[idx].DevName, ret);
			} else {
				CVI_LOGE(RED"Cannot CVI_Flash_Erase %s %d\n"NONE
					, gUPGRADECtx.gPartInfo[idx].DevName, ret);
			}
			eraseUnit = gUPGRADECtx.gPartInfo[idx].BlockSize;
			// startAddr =  gUPGRADECtx.gPartInfo[idx].StartAddr;
#endif
		}
	}

	CVI_UPGRADE_CHUNK_HEAD_S *chunkHead = calloc(1, partitionHead->u32ChunkHeaderSize);

	for (i = 0; i < partitionHead->u32TotalChunk; i++) {
		CVI_S32 s32Ret;

		readLen = read(fd, chunkHead, partitionHead->u32ChunkHeaderSize);
		if (readLen < (CVI_S32) partitionHead->u32ChunkHeaderSize) {
			CVI_LOGE("read file failed\n");
			close(fd);
			if (file)
				pclose(file);
			#ifndef _USE_NANDWRITE_
			CVI_Flash_Close(gUPGRADECtx.FlashType);
			#endif
			CVI_SAFECHECK_FREE(chunkHead);
			return CVI_UPGRADE_EINTR;
		}

		UPGRADE_ReportProgress(0, readLen);

#ifdef _USE_NANDWRITE_
		if (file) {
			CVI_U8 *pBuf2Write;
			int iLen2Wrtie;
			CVI_U32 u32Remain2Read = chunkHead->u32DataSize;

			while (u32Remain2Read) {
				int readSize = (u32Remain2Read > UPGRADE_IMGDATA_BUFF_MAX_SIZE) ?
								UPGRADE_IMGDATA_BUFF_MAX_SIZE : u32Remain2Read;

				readLen = read(fd, puReadBuf, readSize);
				u32Remain2Read -= readLen;
				iLen2Wrtie = readLen;
				pBuf2Write = puReadBuf;
				do {
					int ret = fwrite(pBuf2Write, 1, iLen2Wrtie, file);

					if (ret > 0) {
						iLen2Wrtie -= ret;
						pBuf2Write += ret;
						UPGRADE_ReportProgress(0, ret);
					} else if (ret < 0) {
						CVI_LOGE("Write to pipe error: %s\n", strerror(errno));
						continue;
					}
				} while (iLen2Wrtie > 0);
			}
#else
	   if (gUPGRADECtx.FlashType == CVI_FLASH_TYPE_NAND_0) {
			CVI_U32 u32Remain2Read = chunkHead->u32DataSize;
			CVI_U32 offset;

			for (offset = 0; offset < u32Remain2Read && eraseUnit > 0; offset += readLen) {
				memset(puReadBuf, 0xFF, eraseUnit);
				readLen = read(fd, puReadBuf, eraseUnit);

				s32Ret = CVI_Flash_Write(gUPGRADECtx.FlashType
							, offset, puReadBuf, eraseUnit, CVI_FLASH_RW_FLAG_RAW);
				UPGRADE_ReportProgress(0, readLen);
			}
#endif
		} else {
			CVI_LOGW(YELLOW"Skip UPGRADE_WritePartitionFromChunk; FlashType %d not support yet\n"NONE
				, gUPGRADECtx.FlashType);
			s32Ret = lseek(fd, chunkHead->u32DataSize, SEEK_CUR);
			if (s32Ret < 0) {
				CVI_LOGE("seek file failed\n");
				close(fd);
				if (file)
					pclose(file);
			#ifndef _USE_NANDWRITE_
				CVI_Flash_Close(gUPGRADECtx.FlashType);
			#endif
				CVI_SAFECHECK_FREE(chunkHead);
				return CVI_UPGRADE_EINTR;
			}
			UPGRADE_ReportProgress(s32Ret, 0);
		}
	}

	if (file) {
		int ret = pclose(file);

		if (ret > 0) {
			CVI_LOGE("pclose return %d\n", ret);
		}
		file = NULL;
	}
#ifndef _USE_NANDWRITE_
	CVI_Flash_Close(gUPGRADECtx.FlashType);
#endif
	CVI_SAFECHECK_FREE(chunkHead);

	return CVI_SUCCESS;
}

static CVI_S32 UPGRADE_ReadPkgHead(CVI_S32 s32Fd, CVI_UPGRADE_PKG_HEAD_S *pstPkgHead)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32ReadLen = 0;

	s32Ret = lseek(s32Fd, 0, SEEK_SET);
	if (s32Ret < 0) {
		CVI_LOGE("lseek file failed\n");
		return CVI_UPGRADE_EINTR;
	}

	s32ReadLen = read(s32Fd, pstPkgHead, sizeof(CVI_UPGRADE_PKG_HEAD_S));
	if (s32ReadLen < 0)	{
		CVI_LOGE("read file failed\n");
		return CVI_UPGRADE_EINTR;
	}
	CVI_LOGI("Pkg HeadInfo:\n");
	CVI_LOGI("  Magic[%#08x] Crc[%#08x] PkgLen[%u]\n",
		pstPkgHead->u32Magic, pstPkgHead->u32Crc, pstPkgHead->u32PkgLen);
	CVI_LOGI("  Model[%s] Version[%s]\n", pstPkgHead->szPkgModel, pstPkgHead->szPkgSoftVersion);
	CVI_LOGI("  PartitionCnt[%d]\n", pstPkgHead->s32PartitionCnt);
	return CVI_SUCCESS;
}

static CVI_S32 UPGRADE_CheckPkgCrc(const CVI_UPGRADE_PKG_HEAD_S *pstPkgHead,
						CVI_S32 s32Fd, CVI_U8 *pu8Buf, CVI_U32 u32BufSize)
{
	CVI_U32 u32PkgCrc = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;
	UNUSED(u32BufSize);
	u32PkgCrc = update_crc(u32PkgCrc, (CVI_U8 *)&pstPkgHead->u32HeadVer, sizeof(CVI_UPGRADE_PKG_HEAD_S) - 8);

	s32Ret = lseek(s32Fd, sizeof(CVI_UPGRADE_PKG_HEAD_S), SEEK_SET);
	if (s32Ret == -1) {
		CVI_LOGE("lseek file failed\n");
		return CVI_UPGRADE_EINTR;
	}

	CVI_S32 s32RemainReadLen = pstPkgHead->u32PkgLen - sizeof(CVI_UPGRADE_PKG_HEAD_S);
	CVI_S32 s32BytesToRead = 0;
	CVI_S32 s32ReadLen = 0;

	while (s32RemainReadLen > 0) {
		/* read image data */
		s32BytesToRead = (s32RemainReadLen > UPGRADE_BLOCK_SIZE) ?
									UPGRADE_BLOCK_SIZE : s32RemainReadLen;
		s32ReadLen = read(s32Fd, pu8Buf, (size_t)s32BytesToRead);
		if (s32ReadLen < 0) {
			if (errno == EINTR) {
				CVI_LOGW("read file interuptted, retry\n");
				continue;
			}

			CVI_LOGE("read file failed\n");
			return CVI_UPGRADE_EINTR;
		} else if (s32ReadLen == 0) {
			CVI_LOGE("read end of file\n");
			return CVI_UPGRADE_EINTR;
		}
		s32RemainReadLen -= s32ReadLen;

		u32PkgCrc = update_crc(u32PkgCrc, pu8Buf, s32ReadLen);
	}
	CVI_LOGD("PkgCrc[%#08x]\n", u32PkgCrc);

	if (u32PkgCrc != pstPkgHead->u32Crc) {
		CVI_LOGE("CrcDismatch[%#08x, %#08x]\n", u32PkgCrc, pstPkgHead->u32Crc);
		return CVI_UPGRADE_EPKG_INVALID;
	}
	return CVI_SUCCESS;
}

static CVI_S32 UPGRADE_CheckPkgValid(UPGRADE_PKG_INFO_S *pstPkgInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U8 *pu8PkgBuf = (CVI_U8 *) malloc(UPGRADE_PKG_BUFF_SIZE);

	CVI_SAFECHECK_POINTER(pu8PkgBuf, CVI_UPGRADE_EINTR);
	memset(pu8PkgBuf, 0, UPGRADE_PKG_BUFF_SIZE);

	CVI_S32 s32PkgFd = -1;

	s32PkgFd = open(pstPkgInfo->szPkgFilePath, O_RDONLY);
	if (s32PkgFd < 0) {
		CVI_LOGE("open file %s failed\n", pstPkgInfo->szPkgFilePath);
		CVI_SAFECHECK_FREE(pu8PkgBuf);
		return CVI_UPGRADE_EINTR;
	}

	/* Read Pkg Head Info */
	CVI_UPGRADE_PKG_HEAD_S *pstPkgHead = (CVI_UPGRADE_PKG_HEAD_S *) pu8PkgBuf;

	s32Ret = UPGRADE_ReadPkgHead(s32PkgFd, pstPkgHead);
	if (s32Ret != CVI_SUCCESS) {
		CVI_LOGE("UPGRADE_ReadPkgHead failed\n");
		close(s32PkgFd);
		CVI_SAFECHECK_FREE(pu8PkgBuf);
		return CVI_UPGRADE_EINTR;
	}

	/* Check Pkg Head Info */
	if (pstPkgHead->u32Magic != CVI_UPGRADE_PACKAGE_HEAD_MAGIC) {
		CVI_LOGE("Invalid HeadMagic[%#08x] expect[%#08x]\n",
			pstPkgHead->u32Magic, CVI_UPGRADE_PACKAGE_HEAD_MAGIC);
		close(s32PkgFd);
		CVI_SAFECHECK_FREE(pu8PkgBuf);
		return CVI_UPGRADE_EPKG_INVALID;
	}

	if (strncmp(pstPkgHead->szPkgModel, pstPkgInfo->szModel, CVI_COMM_STR_LEN))	{
		CVI_LOGE("Invalid Model[%s] expect[%s]\n", pstPkgHead->szPkgModel, pstPkgInfo->szModel);
		close(s32PkgFd);
		CVI_SAFECHECK_FREE(pu8PkgBuf);
		return CVI_UPGRADE_EPKG_INVALID;
	}
	snprintf(pstPkgInfo->szSoftVersion, CVI_COMM_STR_LEN, "%s", pstPkgHead->szPkgSoftVersion);
	pstPkgInfo->u32PkgLen = pstPkgHead->u32PkgLen;

	/* Crc Check */
	s32Ret = UPGRADE_CheckPkgCrc(pstPkgHead, s32PkgFd,
			pu8PkgBuf + sizeof(CVI_UPGRADE_PKG_HEAD_S), UPGRADE_IMGDATA_BUFF_MAX_SIZE);
	if (s32Ret != CVI_SUCCESS) {
		CVI_LOGE("UPGRADE_CheckPkgCrc failed\n");
		close(s32PkgFd);
		CVI_SAFECHECK_FREE(pu8PkgBuf);
		return CVI_UPGRADE_EPKG_INVALID;
	}

	/* Check Total flash size versus Package size */
	s32Ret = UPGRADE_CheckFlashSize(pstPkgHead);
	if (s32Ret < 0) {
		CVI_LOGE("Get flash Size failed\n");
		close(s32PkgFd);
		CVI_SAFECHECK_FREE(pu8PkgBuf);
		return s32Ret;
	}

	/* Check device partition size versus per partition size of package */
	s32Ret = UPGRADE_CheckALLPartitionsSize(s32PkgFd, pstPkgHead);

	close(s32PkgFd);
	CVI_SAFECHECK_FREE(pu8PkgBuf);

	return s32Ret;
}

CVI_S32 CVI_UPGRADE_CheckPkg(const CVI_CHAR *pszPkgUrl, const CVI_UPGRADE_DEV_INFO_S *pstDevInfo, CVI_BOOL bCheckVer)
{
	UPGRADE_PKG_INFO_S stPkgInfo;

	memset(&stPkgInfo, 0, sizeof(UPGRADE_PKG_INFO_S));
	snprintf(stPkgInfo.szModel, CVI_COMM_STR_LEN, "%s", pstDevInfo->szModel);
	snprintf(stPkgInfo.szPkgFilePath, CVI_COMM_PATH_MAX_LEN, "%s", pszPkgUrl);

	if ((UPGRADE_CheckPkgValid(&stPkgInfo) == CVI_SUCCESS)
		&& (!bCheckVer || UPGRADE_CompareVersion(stPkgInfo.szSoftVersion, pstDevInfo->szSoftVersion) == 1)) {
		CVI_LOGD(GREEN"ValidPkt[%s] Version[%s] Len[%#x]"NONE"\n",
			stPkgInfo.szPkgFilePath, stPkgInfo.szSoftVersion, stPkgInfo.u32PkgLen);
		//TODO Callback
		return CVI_SUCCESS;
	}

	return CVI_FAILURE;
}

CVI_S32 CVI_UPGRADE_DoUpgrade(const CVI_CHAR *pszPkgUrl)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U8 *pu8PkgBuf = (CVI_U8 *)calloc(1, UPGRADE_PKG_BUFF_SIZE);
	CVI_S32 s32PkgFd = -1;
	int i;

	CVI_SAFECHECK_POINTER(pszPkgUrl, CVI_UPGRADE_EINTR);
	CVI_SAFECHECK_POINTER(pu8PkgBuf, CVI_UPGRADE_EINTR);

	s32PkgFd = open(pszPkgUrl, O_RDONLY);
	if (s32PkgFd < 0) {
		CVI_LOGE("open file %s failed\n", pszPkgUrl);
		CVI_SAFECHECK_FREE(pu8PkgBuf);
		return CVI_UPGRADE_EINTR;
	}

	/* Read Pkg Head Info */
	CVI_UPGRADE_PKG_HEAD_S stPkgHead;

	s32Ret = UPGRADE_ReadPkgHead(s32PkgFd, &stPkgHead);
	if (s32Ret != CVI_SUCCESS) {
		CVI_LOGE("UPGRADE_ReadPkgHead failed\n");
		close(s32PkgFd);
		CVI_SAFECHECK_FREE(pu8PkgBuf);
		return CVI_UPGRADE_EINTR;
	}

	s32Ret = UPGRADE_CheckFlashSize(&stPkgHead);
	if (s32Ret < 0) {
		CVI_LOGE("Get flash Size failed\n");
		close(s32PkgFd);
		CVI_SAFECHECK_FREE(pu8PkgBuf);
		return CVI_UPGRADE_EPKG_OVERSIZE;
	}

	CVI_LOGD("PkgLen=0x%X, PartitionCnt=%d\n", stPkgHead.u32PkgLen, stPkgHead.s32PartitionCnt);

	_init_progress(stPkgHead.u32PkgLen);

	CVI_UPGRADE_PARTITION_HEAD_S partitionHead;

	for (i = 0; i < stPkgHead.s32PartitionCnt; i++) {
		CVI_S32 s32CurOffset;

		s32CurOffset = lseek(s32PkgFd, 0, SEEK_CUR);
		if (s32CurOffset <(CVI_S32)stPkgHead.au32PartitionOffSet[i]) {
			CVI_LOGE("File pos smaller than header record?\n");
			close(s32PkgFd);
			CVI_SAFECHECK_FREE(pu8PkgBuf);
			return CVI_UPGRADE_EINTR;
		}

		s32CurOffset = lseek(s32PkgFd, stPkgHead.au32PartitionOffSet[i], SEEK_SET);
		UPGRADE_ReportProgress(s32CurOffset, 0);
		s32Ret = UPGRADE_ReadPartitionHead(s32PkgFd, &partitionHead, NULL, NULL);
		if (s32Ret != CVI_SUCCESS) {
			CVI_LOGE("UPGRADE_ReadPartitionHead failed\n");
			close(s32PkgFd);
			CVI_SAFECHECK_FREE(pu8PkgBuf);
			return CVI_UPGRADE_EINTR;
		}

		s32CurOffset += sizeof(CVI_UPGRADE_PARTITION_HEAD_S);
		s32CurOffset = lseek(s32PkgFd, s32CurOffset, SEEK_SET);
		UPGRADE_ReportProgress(s32CurOffset, 0);
		s32Ret = UPGRADE_WritePartitionFromChunk(s32PkgFd, &partitionHead
					, partitionHead.szExtraFlags, pu8PkgBuf);
		if (s32Ret != CVI_SUCCESS) {
			CVI_LOGE("UPGRADE_WritePartitionFromChunk failed\n");
			close(s32PkgFd);
			CVI_SAFECHECK_FREE(pu8PkgBuf);
			return CVI_UPGRADE_EINTR;
		}
	}

	close(s32PkgFd);
	CVI_SAFECHECK_FREE(pu8PkgBuf);
	CVI_LOGI(GREEN"Finish!"NONE"\n");
	return CVI_SUCCESS;
}

CVI_S32 CVI_UPGRADE_Init(CVI_VOID)
{
	int minor;
	unsigned int size, erasesize;
	char name[65];
	char line[128];
	FILE *fp = fopen(PROC_MTD_PATH, "r");
	CVI_U64 u64StartAddr = 0;
	int i;

	gUPGRADECtx.FlashType = CVI_FLASH_TYPE_BUTT;

	if (fp == NULL)
		return CVI_FAILURE;

	if (fgets(line, sizeof(line), fp) == NULL) { // skip first line
		fclose(fp);
		return CVI_FAILURE;
	}

	for (i = 0; fgets(line, sizeof(line), fp) != 0; i++) {
		if (sscanf(line, "mtd%d: %x %x \"%64[^\"]\"", &minor, &size, &erasesize, name) == 4) {
			if (i == 0)
				CVI_LOGD(" DevName\t  PartSize\t  BlockSize\t  PartName   \t  Startaddr\n");

			gUPGRADECtx.gPartInfo[i].PartSize = (CVI_U64) size;
			gUPGRADECtx.gPartInfo[i].BlockSize = (CVI_U64) erasesize; // erase size
			memset(gUPGRADECtx.gPartInfo[i].PartName, 0, sizeof(gUPGRADECtx.gPartInfo[i].PartName));
			strncpy(gUPGRADECtx.gPartInfo[i].PartName, name, sizeof(gUPGRADECtx.gPartInfo[i].PartName) - 1);
			snprintf(gUPGRADECtx.gPartInfo[i].DevName, sizeof(gUPGRADECtx.gPartInfo[i].DevName)
					, "/dev/mtd%d", minor);
			gUPGRADECtx.gPartInfo[i].StartAddr = u64StartAddr;
			u64StartAddr += gUPGRADECtx.gPartInfo[i].PartSize;
			gUPGRADECtx.gMTDPartNum++;
			CVI_LOGD(" %s\t  %"PRId64"\t  0x%x\t  %s   \t  0x%"PRIx64"\n"
				, gUPGRADECtx.gPartInfo[i].DevName, gUPGRADECtx.gPartInfo[i].PartSize
				, gUPGRADECtx.gPartInfo[i].BlockSize, gUPGRADECtx.gPartInfo[i].PartName
				, gUPGRADECtx.gPartInfo[i].StartAddr);
			gUPGRADECtx.FlashType = CVI_FLASH_TYPE_NAND_0;
		}
	}

	//No mtd device; check for emmc
	if (gUPGRADECtx.gMTDPartNum == 0) {
		CVI_CHAR realPath[128] = {0,};

		if (!realpath(DEV_MMC0_PATh, realPath)) {
			CVI_LOGE("realpath failed:%s", DEV_MMC0_PATh);
			return CVI_UPGRADE_EINVAL;
		}
		if (strstr(realPath, "cv-emmc")) {
			gUPGRADECtx.FlashType = CVI_FLASH_TYPE_EMMC_0;
		}
		CVI_LOGI("Flash type: %d\n", gUPGRADECtx.FlashType);
	}

	fclose(fp);
	return CVI_SUCCESS;
}

CVI_S32 CVI_UPGRADE_Deinit(CVI_VOID)
{
	return CVI_SUCCESS;
}

void CVI_UPGRADE_RegisterEvent(void (*eventCb)(CVI_UPGRADE_EVENT_S *))
{
	if (eventCb)
		gUPGRADECtx.event_cbFunc = eventCb;
}
