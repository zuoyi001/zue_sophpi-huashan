#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
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
#include "nand_raw.h"
#include "cvi_log.h"

#define NAND_PAGE_BUF_SIZE 16384
#define NAND_OOB_BUF_SIZE  1200

struct nand_raw_ctrl *nandraw_ctrl;

static CVI_S32 check_skip_badblock(struct mtd_partition *ptn, CVI_S32 *blockindex, CVI_S32 blocksize);
struct CVI_Flash_PartInfo_S *get_flash_partition_info(CVI_FLASH_TYPE_E FlashType, const CVI_CHAR *devname);

/*****************************************************************************/
// To avoid warnings: "Too many leading tabs - consider code refactoring"
CVI_S32 nand_read_refactoring(CVI_S32 read_oob,
						 CVI_UL length,
						 CVI_U8 *buffer,
						 CVI_S32 totalread,
						 CVI_U64 offset,
						 struct mtd_partition *ptn)
{
	if (read_oob) {
	struct mtd_oob_buf oob;

	if (length >= nandraw_ctrl->oobused) {
		oob.start = (CVI_UL)(offset - ptn->start);
		oob.length = nandraw_ctrl->oobused;
		oob.ptr = (CVI_U8 *)buffer;
		if (ioctl(ptn->fd, MEMREADOOB, &oob)) {
			printf("read oob \"%s\" fail. error(%d)\n", ptn->mtddev, errno);
			return -1;
		}
		buffer += nandraw_ctrl->oobused;
		length -= (CVI_UL)nandraw_ctrl->oobused;
		totalread += (CVI_S32)nandraw_ctrl->oobused;
		} else {
			/* read end when length less than oobsize. */
			length = 0;
		}
		return 0;
	}
	return 0;
}

CVI_S32 nand_raw_init(CVI_VOID)
{
	CVI_S32 ix;
	CVI_S32 dev;
	CVI_S32 readonly;
	CVI_S32 max_partition;
	// struct stat status;
	struct mtd_partition *ptn;
	struct mtd_info_user mtdinfo;
	struct CVI_Flash_PartInfo_S *pstPartInfo = NULL;
	char *ptr;
	CVI_CHAR devname[32];

	CVI_CHAR buf[PATH_MAX];

	if (nandraw_ctrl) {
		return 0;
	}

	max_partition = get_max_partition();
	if (max_partition < 0) {
#ifdef NAND_RAW_DBG
		printf("Can't find mtd device at /dev/mtdx.\n");
#endif
		return -1;
	}

	if (++max_partition >= MAX_MTD_PARTITION) {
		printf("partition maybe more than %d, please increase MAX_MTD_PARTITION.\n", MAX_MTD_PARTITION);
	}

	if (flash_partition_info_init() != 0) {
		printf("Initial partition information failure.\n");
		return -1;
	}
	ptr = (char *)malloc(sizeof(struct nand_raw_ctrl)
						 + (CVI_U32) max_partition * sizeof(struct mtd_partition));
	if (!ptr) {
		printf("Not enough memory.\n");
		return -1;
	}
	nandraw_ctrl = (struct nand_raw_ctrl *)ptr;
	memset(nandraw_ctrl, 0, sizeof(struct nand_raw_ctrl)
		   + (CVI_U32) max_partition * sizeof(struct mtd_partition));

	nandraw_ctrl->num_partition = 0;
	nandraw_ctrl->size = 0;
	ptn = nandraw_ctrl->partition;

	for (ix = 0; ix < max_partition; ix++) {
		readonly = 0;

		ptn->fd = INVALID_FD;
		memset(buf, 0, PATH_MAX);
		(CVI_VOID)snprintf(buf, PATH_MAX, DEV_MTDBASE"%d", ix);
		dev = open(buf, O_RDWR);
		if (dev == -1) {
			dev = open(buf, O_RDONLY);
			if (dev == -1) {
				printf("Can't open \"%s\", set to access none!\n", buf);
				ptn->perm = ACCESS_NONE;
				continue;
			}
			ptn->perm = ACCESS_RD;
			readonly = 1;
		} else {
			ptn->perm = ACCESS_RDWR;
		}

		if (ioctl(dev, MEMGETINFO, &mtdinfo)) {
			printf("Can't get \"%s\" information.\n", buf);
			close(dev);
			continue;
		}

		if ((mtdinfo.type != MTD_NANDFLASH) && (mtdinfo.type != MTD_MLCNANDFLASH)) {
			close(dev);
			continue;
		}

		memset(devname, 0, sizeof(devname));
		(CVI_VOID)snprintf(devname, sizeof(devname), "mtd%d", ix);

		pstPartInfo = get_flash_partition_info(CVI_FLASH_TYPE_NAND_0, devname);

		if (pstPartInfo == NULL) {
			printf("Can't get \"%s\" partition information\n", buf);
			close(dev);
			continue;
		}

		strncpy(ptn->mtddev, buf, sizeof(ptn->mtddev));
		ptn->mtddev[sizeof(ptn->mtddev) - 1] = '\0';
		ptn->fd = dev;
		ptn->readonly = readonly;
#if 0
		ptn->start = nandraw_ctrl->size;
		nandraw_ctrl->size += mtdinfo.size;
		ptn->end   = nandraw_ctrl->size - 1;
#else
		ptn->start = pstPartInfo->StartAddr;
		ptn->end = pstPartInfo->StartAddr + mtdinfo.size - 1;
#endif
#ifdef FLASH_DEBUG
		printf("\n");
		printf("nandraw_ctrl->partition->mtddev = %s\n", ptn->mtddev);
		printf("nandraw_ctrl->partition->fd = %d\n", ptn->fd);
		printf("nandraw_ctrl->partition->readonly = %d\n", ptn->readonly);
		printf("nandraw_ctrl->partition->start = 0x%lx\n", ptn->start);
		printf("nandraw_ctrl->partition->end = 0x%lx\n", ptn->end);
#endif
		nandraw_ctrl->num_partition++;
		ptn++;

		if (nandraw_ctrl->num_partition == 1) {
			nandraw_ctrl->pagesize = mtdinfo.writesize;
			nandraw_ctrl->blocksize = mtdinfo.erasesize;
			nandraw_ctrl->pagemask = (mtdinfo.writesize - 1);
			nandraw_ctrl->blockmask = (mtdinfo.erasesize - 1);
			nandraw_ctrl->oobsize = mtdinfo.oobsize;
			nandraw_ctrl->oobused = HINFC610_OOBSIZE_FOR_YAFFS;

			nandraw_ctrl->pageshift = (CVI_U32)offshift(mtdinfo.writesize);
			nandraw_ctrl->blockshift = (CVI_U32)offshift(mtdinfo.erasesize);
		}
	}
#ifdef FLASH_DEBUG
	printf("nandraw_ctrl->num_partition = 0x%x\n", nandraw_ctrl->num_partition);
	printf("nandraw_ctrl->pagesize = 0x%x\n", nandraw_ctrl->pagesize);
	printf("nandraw_ctrl->blocksize = 0x%x\n", nandraw_ctrl->blocksize);
	printf("nandraw_ctrl->pagemask = 0x%x\n", nandraw_ctrl->pagemask);
	printf("nandraw_ctrl->blockmask = 0x%x\n", nandraw_ctrl->blockmask);
	printf("nandraw_ctrl->oobsize = 0x%x\n", nandraw_ctrl->oobsize);
	printf("nandraw_ctrl->oobused = 0x%x\n", nandraw_ctrl->oobused);
	printf("nandraw_ctrl->pageshift = 0x%x\n", nandraw_ctrl->pageshift);
	printf("nandraw_ctrl->blockshift = 0x%x\n", nandraw_ctrl->blockshift);
#endif
	if (!nandraw_ctrl->num_partition) {
		printf("Can't find nand type mtd device at /dev/mtdx\n");
		free(nandraw_ctrl);
		nandraw_ctrl = NULL;
		return 0;
	}

	nandraw_ctrl->size = get_flash_total_size(CVI_FLASH_TYPE_NAND_0);
	return 0;
}

CVI_VOID nand_raw_get_info(CVI_U64 *totalsize,
						  CVI_U32 *pagesize,
						  CVI_U32 *blocksize,
						  CVI_U32 *oobsize,
						  CVI_U32 *blockshift)
{
	if (!nandraw_ctrl) {
		printf("Nandraw Control is not initialized!\n");
		return;
	}

	*totalsize = nandraw_ctrl->size;
	*pagesize = nandraw_ctrl->pagesize;
	*blocksize = nandraw_ctrl->blocksize;
	*oobsize = nandraw_ctrl->oobsize;
	*blockshift = nandraw_ctrl->blockshift;
}

/*****************************************************************************/
/*
 * warning:
 *	  1. if open SPI/NOR FLASH, return 0
	  2. if dev_name cannot match nandraw_ctrl, return error_valid;
 */
CVI_U64 nand_raw_get_start_addr(const CVI_CHAR *dev_name, CVI_UL blocksize, CVI_S32 *value_valid)
{
	struct mtd_partition *ptn;
	CVI_S32 max_partition;
	CVI_S32 ix;

	UNUSED(blocksize);
	ptn = nandraw_ctrl->partition;

	max_partition = get_max_partition();
	if (max_partition < 0) {
		printf("Can't find mtd device at /dev/mtdx.\n");
		return 0;
	}

	/* as the partition start with 0, if we have x partition,the max_partition will be x-1 ,
	 * so in following code,we use  " ix <= max_partition" instead of " ix < max_partition"
	 * which be used in old SDK version. as well as , the later "if (max_partition < ix)" ,we use
	 * "<" instead of "=" which be used in old SDK version
	 */
	for (ix = 0; ix <= max_partition; ix++) {
		if (!strncmp(ptn->mtddev, dev_name,
					 strlen(ptn->mtddev) > strlen(dev_name) ?
					 strlen(ptn->mtddev):strlen(dev_name))) {
			break;
		}

		ptn++;
	}

	if (max_partition < ix) {
		*value_valid = 0;
		return 0;
	}
	*value_valid = 1;
	/*lint -e661*/
	return ptn->start;
	/*lint +e661*/
}
/*****************************************************************************/
/*
 * warning:
 *	  1. startaddr should be alignment with pagesize
 */

CVI_S32 nand_raw_read(CVI_S32 fd,
					 CVI_U64 *startaddr, /* this address maybe change when meet bad block */
					 CVI_U8 *buffer,
					 CVI_UL length, /* if CVI_FLASH_RW_FLAG_WITH_OOB, include oob */
					 CVI_U64 openaddr,
					 CVI_U64 limit_leng,
					 CVI_S32 read_oob,
					 CVI_S32 skip_badblock)
{
	CVI_S32 ix;
	CVI_S32 rel;
	CVI_S32 totalread = 0;
	CVI_UL num_read = 0;
	struct mtd_partition *ptn;
	CVI_U64 offset = *startaddr;

	UNUSED(fd);

	if (!nandraw_ctrl) {
		printf("Please initialize before use this function.\n");
		return -1;
	}
	if (buffer == NULL) {
		return -1;
	}

	if (offset >= nandraw_ctrl->size || !length) {
		return -1;
	}

	if ((CVI_UL)offset & nandraw_ctrl->pagemask) {
		printf("startaddr should be align with pagesize(0x%X)\n",
				 nandraw_ctrl->pagesize);
		return -1;
	}

	for (ix = 0; ix < nandraw_ctrl->num_partition && length; ix++) {
		ptn = &nandraw_ctrl->partition[ix];

		/*lint -save -e655*/
		while ((ptn->start <= offset) && (offset < ptn->end) && length && (ptn->perm & ACCESS_RD) &&
			   ptn->fd != INVALID_FD) {
			//printf("dev: \"%s\", from: 0x%llX, length: 0x%lX\n",
			// ptn->mtddev, (offset - ptn->start), length);
			if (skip_badblock) {
				CVI_S32 blockindex = (CVI_S32)((offset - ptn->start) / nandraw_ctrl->blocksize);

				rel = check_skip_badblock(ptn, &blockindex, (CVI_S32)nandraw_ctrl->blocksize);
				if (rel < 0) {
					return rel;
				}

				if (rel > 0) {
					/* move to start address of the block */
					offset += (CVI_UL)((CVI_U32)rel << nandraw_ctrl->blockshift);
					continue;
				}

				/* rel == 0, no bad block */
			}

			// printf(">ptn->fd=%d, length=%#lx, *startaddr=%#lx, offset=%#lx\n", ptn->fd, length,
			// 	*startaddr, offset);

			if (offset - openaddr >= limit_leng) {
				printf("bad block cause read end(beyond limit_leng =%#lx)!\n", limit_leng);
				return totalread;
			}

			/* read all pages in one block */
			do {
				/* read one page one by one */
				num_read = (length >= nandraw_ctrl->pagesize ? nandraw_ctrl->pagesize : length);
				if (lseek64(ptn->fd, (off64_t)(offset - ptn->start), SEEK_SET) != -1
					&& read(ptn->fd, buffer, (size_t)num_read) != (ssize_t)num_read) {
					printf("read \"%s\" fail. error(%d)\n",
							 ptn->mtddev, errno);
					return -1;
				}

				buffer += num_read;
				length -= num_read;
				totalread += num_read;

				rel = nand_read_refactoring(read_oob, length, buffer,
						 totalread, offset, ptn);
				if (rel < 0)
					return rel;
				offset += (CVI_UL)num_read;

			} while (length && (offset & nandraw_ctrl->blockmask));
		}
	}

	*startaddr = offset;

	return totalread;
}
/*****************************************************************************/
/*
 * warning:
 *	  1. offset should be alignment with blocksize
 *	  2. if there is a bad block, length should subtract.
 */
// int nand_raw_erase(unsigned long long offset, unsigned long long length, unsigned long long limit_leng)
CVI_S64 nand_raw_erase(CVI_S32 fd,
					  CVI_U64 startaddr,
					  CVI_U64 length,
					  CVI_U64 openaddr,
					  CVI_U64 limit_leng)
{
	CVI_S32 ix;
	CVI_S32 rel;
	CVI_S64 totalerase = 0;
	CVI_U64 offset = startaddr;
	CVI_S32 blockindex;
	struct mtd_partition *ptn;

	UNUSED(fd);
	if (!nandraw_ctrl) {
		printf("Please initialize before use this function.\n");
		return -1;
	}

	if (offset >= nandraw_ctrl->size || !length) {
		return -1;
	}

	if (((CVI_UL)offset & nandraw_ctrl->blockmask)
		|| ((CVI_UL)length & nandraw_ctrl->blockmask)) {
		printf("offset or length should be alignment with blocksize(0x%X)\n",
				 (CVI_U32)nandraw_ctrl->blocksize);
		return -1;
	}

	if (offset + length > nandraw_ctrl->size) {
		length = nandraw_ctrl->size - offset;
	}

	for (ix = 0; ix < nandraw_ctrl->num_partition && length; ix++) {
		ptn = &nandraw_ctrl->partition[ix];

		if (ptn->readonly) {
			printf("erase a read only partition \"%s\".\n", ptn->mtddev);
			return -1;
		}

		while ((ptn->start <= offset) && (offset < ptn->end) && length && (ptn->perm & ACCESS_WR) &&
			   ptn->fd != INVALID_FD) {
			//printf("dev: \"%s\", from: 0x%llX, length: 0x%llX\n",
			// ptn->mtddev, (offset - ptn->start), length);
			if (offset - openaddr >= limit_leng) {
				printf("1bad block cause erase end(beyond limit_leng =%#lx)!\n", limit_leng);
				return totalerase;
			}

			blockindex = (CVI_S32)((offset - ptn->start) >> nandraw_ctrl->blockshift);

			rel = check_skip_badblock(ptn, &blockindex, (CVI_S32)nandraw_ctrl->blocksize);

			/* no bad block */
			if (!rel) {
				struct erase_info_user64 eraseinfo;

				eraseinfo.start = (CVI_U64)(offset - ptn->start);
				eraseinfo.length = (CVI_U64)(nandraw_ctrl->blocksize);

				// printf(">ptn->fd=%d, length=%#lx, *startaddr=%#lx, offset=%#lx\n", ptn->fd, length,
				// 	startaddr, offset);

				/* the block will be marked bad when erase error, so don't deal with */
				if (ioctl(ptn->fd, MEMERASE64, &eraseinfo)) {
					printf("Erase 0x%lx failed!\n", offset);
					rel = nand_mark_badblock(offset, (CVI_U64)nandraw_ctrl->blocksize);
					if (rel != 0) {
						printf("\nMTD block_markbad at 0x%08lx failed: %d, aborting\n",
								 offset, rel);
						return CVI_FAILURE;
					}
				}

				rel = 1;
				length -= nandraw_ctrl->blocksize;
				totalerase += nandraw_ctrl->blocksize;
			}

			if (rel < 0) {
				return CVI_FAILURE;
			}

			/* rel > 0 */
			offset += (CVI_UL)((CVI_U32)rel << nandraw_ctrl->blockshift);
		}
	}

	return totalerase;
}

/*****************************************************************************/
/*
 * warning:
 *	  1. offset should be alignment with blocksize
 */
CVI_S32 nand_raw_force_erase(CVI_U64 offset)
{
	CVI_S32 ix;
	CVI_S32 rel;
	struct mtd_partition *ptn;

	if (!nandraw_ctrl) {
		printf("Please initialize before use this function.\n");
		return -1;
	}

	if (offset >= nandraw_ctrl->size) {
		return 0;
	}

	if ((CVI_UL)offset & nandraw_ctrl->blockmask) {
		printf("offset should be alignment with blocksize(0x%X)\n",
				 (CVI_U32)nandraw_ctrl->blocksize);
		return -1;
	}

	for (ix = 0; ix < nandraw_ctrl->num_partition; ix++) {
		ptn = &nandraw_ctrl->partition[ix];

		if (ptn->readonly) {
			printf("erase a read only partition \"%s\".\n", ptn->mtddev);
			return -1;
		}

		if ((ptn->start <= offset) && (offset < ptn->end) && (ptn->perm & ACCESS_WR) && ptn->fd != INVALID_FD) {
			printf("dev: \"%s\", from: 0x%lX\n",
					ptn->mtddev, (offset - ptn->start));

			offset = offset - ptn->start;

			rel = ioctl(ptn->fd, MEMFORCEERASEBLOCK, &offset);

			if (rel) {
				printf("Force Erase 0x%lx failed!\n", offset);
				return -1;
			}
		}
	}

	return 0;
}
/*****************************************************************************/
/*
 * warning:
 *	  1. startaddr should be alignment with pagesize
 */
CVI_S32 nand_raw_write(CVI_S32 fd,
					  CVI_U64 *startaddr,
					  CVI_U8 *buffer,
					  CVI_UL length,
					  CVI_U64 openaddr,
					  CVI_U64 limit_leng,
					  CVI_S32 write_oob)
{
	CVI_S32 ix;
	CVI_S32 rel;
	CVI_S32 totalwrite = 0;
	CVI_S32 num_write = 0;
	CVI_S32 blockindex;
	CVI_U8 *databuf = NULL;
	CVI_U8 *oobbuf = NULL;
	CVI_S32 ret = -1;

	UNUSED(fd);

	/* for ioctl cmd args */
	struct mtd_write_req write_req_buf;
	CVI_U64 start;
	CVI_U64 datalen;
	CVI_U64 ooblen;
	CVI_U8 *data_ptr;
	CVI_U8 *oob_ptr;

	struct mtd_partition *ptn;
	CVI_U64 offset = *startaddr;

	if (buffer == NULL) {
		return -1;
	}
	databuf = malloc(NAND_PAGE_BUF_SIZE);

	if (databuf == NULL) {
		ret = -1;
		goto fail;
	}

	memset(databuf, 0, NAND_PAGE_BUF_SIZE);

	oobbuf = malloc(NAND_OOB_BUF_SIZE);

	if (oobbuf == NULL) {
		ret = -1;
		goto fail;
	}
	memset(oobbuf, 0, NAND_OOB_BUF_SIZE);

	if (!nandraw_ctrl) {
		printf("Please initialize before use this function.\n");
		ret = -1;
		goto fail;
	}

	if (offset >= nandraw_ctrl->size || !length) {
		ret = -1;
		goto fail;
	}

	if ((CVI_UL)offset & nandraw_ctrl->pagemask) {
		printf("Startaddr should be alignment with pagemask(0x%X)\n",
				 nandraw_ctrl->pagemask);
		ret = -1;
		goto fail;
	}

	/* if write_oob is used, align with (oobsize + pagesize) */
	if (write_oob && (length % (nandraw_ctrl->pagesize + nandraw_ctrl->oobsize))) {
		printf("Length should be alignment with pagesize + oobsize when write oob.\n");
		ret = -1;
		goto fail;
	}

	if (!write_oob) {
		if (nandraw_ctrl->oobused > NAND_OOB_BUF_SIZE) {
			printf("%s: BUG program need enough oobbuf.\n",
					 __func__);
			ret = -1;
			goto fail;
		}
		memset(oobbuf, 0xFF, NAND_OOB_BUF_SIZE);
	}

	if (nandraw_ctrl->pagesize > NAND_PAGE_BUF_SIZE) {
		printf("%s: BUG program need enough databuf.\n",
				 __func__);
		ret = -1;
		goto fail;
	}

	for (ix = 0; ix < nandraw_ctrl->num_partition && length; ix++) {
		ptn = &nandraw_ctrl->partition[ix];
		if (ptn->readonly) {
			printf("Write a read only partition \"%s\".\n", ptn->mtddev);
			ret = -1;
			goto fail;
		}

		while ((ptn->start <= offset) && (offset < ptn->end) && length && (ptn->perm & ACCESS_WR) &&
			   ptn->fd != INVALID_FD) {
			/* must skip bad block when write */
			blockindex = (CVI_S32)((offset - ptn->start) >> nandraw_ctrl->blockshift);

			rel = check_skip_badblock(ptn, &blockindex, (CVI_S32)nandraw_ctrl->blocksize);
			if (rel < 0) {
				ret = rel;
				goto fail;
			}

			if (rel > 0) {
				/* move to start address of the block */
				offset += (CVI_UL)((CVI_U32)rel << nandraw_ctrl->blockshift);
				continue;
			}

			/* rel == 0, no bad block */

			// printf(">ptn->fd=%d, length=%#lx, *startaddr=%#lx, offset=%#lx\n", ptn->fd, length,
			// 		*startaddr, offset);

			if (offset - openaddr >= limit_leng) {
				printf("bad block cause write end(beyond limit_leng =%#lx)!\n", limit_leng);
				ret = CVI_FLASH_END_DUETO_BADBLOCK;
				goto fail;
			}

			/* write all pages in one block */
			do {
				num_write = (CVI_S32)(length >= nandraw_ctrl->pagesize ?
						 nandraw_ctrl->pagesize : length);

				start = offset - ptn->start;
				if ((CVI_U32)num_write < nandraw_ctrl->pagesize) {
					/* less than one pagesize */
					memset(databuf, 0xFF, NAND_PAGE_BUF_SIZE);
					memcpy(databuf, buffer, (size_t)num_write);
					data_ptr = databuf;
				} else {
					data_ptr = buffer;
				}

				datalen = nandraw_ctrl->pagesize;

				offset += (unsigned long)num_write;
				length -= (CVI_U32)num_write;
				buffer += num_write;
				totalwrite += num_write;

				if (write_oob) {	/* if write_oob */
					if (length < nandraw_ctrl->oobsize) {
						printf("%s(%d): buf not align error!\n", __FILE__, __LINE__);
						ret = -1;
						goto fail;
					}
					oob_ptr = buffer;

					buffer += nandraw_ctrl->oobsize;
					length -= nandraw_ctrl->oobsize;
					totalwrite += (int)nandraw_ctrl->oobsize;
				} else {
					oob_ptr = oobbuf;
				}

				ooblen = nandraw_ctrl->oobsize;

				/* avoid mark bad block unexpected. */
				if ((*(oob_ptr + 1) << 8) + *oob_ptr != 0xFFFF) {
					printf("Please check input data, it maybe mark bad block. value:0x%04X\n",
							 (*(oob_ptr + 1) << 8) + *oob_ptr);
					ret = -1;
					goto fail;
				}

				/* should reerase and write if write error when upgrade. */
				memset(&write_req_buf, 0xff, sizeof(write_req_buf));
				write_req_buf.start = start;
				write_req_buf.usr_data = (CVI_U64)(CVI_UL)data_ptr;
				write_req_buf.len = datalen;
				write_req_buf.usr_oob = (CVI_U64)(CVI_UL)oob_ptr;
				write_req_buf.ooblen = ooblen;
				write_req_buf.mode = MTD_OPS_PLACE_OOB;

				if (ioctl(ptn->fd, MEMWRITE, &write_req_buf) != 0) {
					printf("ioctl(%s MEMWRITE) fail. error(%d)\n",
							 ptn->mtddev, errno);
					/* union return value to CVI_FAILURE. */
					ret = CVI_FAILURE;
					goto fail;
				}
			} while (length && (offset & nandraw_ctrl->blockmask));
		}
	}

	*startaddr = offset;
	ret = totalwrite;

done:

	if (databuf) {
		free(databuf);
	}

	if (oobbuf) {
		free(oobbuf);
	}

	return ret;
fail:
	goto done;
}
/*****************************************************************************/
/*
 * warning:
 *	  1. offset and length should be alignment with blocksize
 */
CVI_S32 nand_mark_badblock(CVI_U64 offset, CVI_U64 length)
{
	CVI_S32 ix;
	CVI_U64 blockoffset;
	struct mtd_partition *ptn;

	if (!nandraw_ctrl) {
		printf("Please initialize before use this function.\n");
		return -1;
	}

	if (offset >= nandraw_ctrl->size || !length) {
		return 0;
	}

	if (((CVI_UL)offset & nandraw_ctrl->blockmask)
		|| ((CVI_UL)length & nandraw_ctrl->blockmask)) {
		printf("offset or length should be alignment with blocksize(0x%X)\n",
				 (CVI_U32)nandraw_ctrl->blocksize);
		return -1;
	}

	if (offset + length > nandraw_ctrl->size) {
		length = nandraw_ctrl->size - offset;
	}

	for (ix = 0; ix < nandraw_ctrl->num_partition && length; ix++) {
		ptn = &nandraw_ctrl->partition[ix];

		while ((ptn->start <= offset) && (offset < ptn->end) && length && ptn->fd != INVALID_FD) {
			if (ptn->readonly) {
				printf("mark bad block error, a read only partition \"%s\".\n", ptn->mtddev);
				return -1;
			}

		   printf("dev: \"%s\", from: 0x%lX, length: 0x%lX\n",
					ptn->mtddev, (offset - ptn->start), length);

			blockoffset = offset - ptn->start;

			if (ioctl(ptn->fd, MEMSETBADBLOCK, &blockoffset)) {
				printf("Mark bad block 0x%lX failed!\n", offset);
			}

			offset += (CVI_UL)(1UL << nandraw_ctrl->blockshift);
			length -= (CVI_UL)(1UL << nandraw_ctrl->blockshift);
		}
	}
	return 0;
}
/*****************************************************************************/
/*
 * warning:
 *	  1. offset and length should be alignment with blocksize
 */
CVI_S32 nand_show_badblock(CVI_U64 offset, CVI_U64 length)
{
	CVI_S32 ix;
	CVI_S32 badblock;
	CVI_U64 blockoffset;
	struct mtd_partition *ptn;

	if (!nandraw_ctrl) {
		printf("Please initialize before use this function.\n");
		return -1;
	}

	if (offset >= nandraw_ctrl->size || !length) {
		return 0;
	}

	if (((CVI_UL)offset & nandraw_ctrl->blockmask)
		|| ((CVI_UL)length & nandraw_ctrl->blockmask)) {
		printf("offset or length should be alignment with blocksize(0x%X)\n",
				 (CVI_U32)nandraw_ctrl->blocksize);
		return -1;
	}

	if (offset + length > nandraw_ctrl->size) {
		length = nandraw_ctrl->size - offset;
	}

	for (ix = 0; ix < nandraw_ctrl->num_partition && length; ix++) {
		ptn = &nandraw_ctrl->partition[ix];

		while ((ptn->start <= offset) && (offset < ptn->end) && length && ptn->fd != INVALID_FD) {
			printf("dev: \"%s\", from: 0x%lX, length: 0x%lX\n",
					ptn->mtddev, (offset - ptn->start), length);

			blockoffset = offset - ptn->start;

			badblock = ioctl(ptn->fd, MEMGETBADBLOCK, &blockoffset);
			if (badblock < 0) {
				printf("Get nand badblock fail. error(%d)\n", errno);
				return -1;
			}
			if (badblock == 1) {
				printf("Bad block at address: 0x%lX of \"%s\", absolute address: 0x%lX\n",
						 (CVI_UL)blockoffset, ptn->mtddev, (CVI_U64)offset);
			}

			offset += (CVI_UL)(1UL << nandraw_ctrl->blockshift);
			length -= (CVI_UL)(1UL << nandraw_ctrl->blockshift);
		}
	}
	return 0;
}
/*****************************************************************************/

CVI_S32 nand_raw_info(struct mtd_info_user *mtdinfo)
{
	CVI_S32 rel;

	if (!nandraw_ctrl) {
		printf("Please initialize before use this function.\n");
		return -1;
	}

	if (nandraw_ctrl->partition[0].fd == INVALID_FD) {
		return -1;
	}

	rel = ioctl(nandraw_ctrl->partition[0].fd, MEMGETINFO, mtdinfo);

	if (rel) {
		printf("ioctl \"%s\" fail. error(%d)\n",
				 nandraw_ctrl->partition[0].mtddev, errno);
		return rel;
	}

	if (nandraw_ctrl->size > 0xFFFFFFFF) {
		printf("nand flash size out of range of an CVI_UL.\n");
	}
	mtdinfo->size = (CVI_UL)nandraw_ctrl->size;

	return 0;
}
/*****************************************************************************/

CVI_S32 nand_raw_dump_partition(CVI_VOID)
{
	CVI_S32 ix;
	struct mtd_partition *ptn __attribute__((unused));

	if (!nandraw_ctrl) {
		printf("Please initialize before use this function.\n");
		return -1;
	}

	printf("-------------------------\n");
	printf("mtd device	 start length mode\n");
	for (ix = 0; ix < nandraw_ctrl->num_partition; ix++) {
		ptn = &nandraw_ctrl->partition[ix];

		printf("%-12s ", ptn->mtddev);
		printf("%5s ", int_to_size(ptn->start));
		printf("%6s ", int_to_size(ptn->end + 1 - ptn->start));
		printf("%2s ", ptn->readonly ? "r" : "rw");
		printf("\n");
	}

	return 0;
}
/*****************************************************************************/

CVI_S32 nand_raw_destroy(CVI_VOID)
{
	CVI_S32 ix;

	if (!nandraw_ctrl) {
		return 0;
	}

	for (ix = 0; ix < nandraw_ctrl->num_partition; ix++) {
		if (nandraw_ctrl->partition[ix].fd != INVALID_FD) {
			close(nandraw_ctrl->partition[ix].fd);
		}
	}

	if (nandraw_ctrl != NULL) {
		free(nandraw_ctrl);
	}

	nandraw_ctrl = NULL;
	//printf("\n");

	return 0;
}

/*****************************************************************************/
/*
 *	> 0 skip bad block num.
 *	= 0 no bad block.
 *	< 0 error.
 */
static CVI_S32 check_skip_badblock(struct mtd_partition *ptn, CVI_S32 *blockindex,
								  CVI_S32 blocksize)
{
	CVI_S32 rel = 0;
	CVI_S32 badblock;
	CVI_U64 size = (ptn->end - ptn->start) + 1;
	loff_t offset = ((loff_t)(*blockindex) * (loff_t)blocksize);

	if (ptn->fd == INVALID_FD) {
		return -1;
	}

	do {
		badblock = ioctl(ptn->fd, MEMGETBADBLOCK, &offset);
		if (badblock < 0) {
			printf("Get nand badblock fail. error(%d)\n", errno);
			return -1;
		}
		if (badblock == 1) {
			printf("Skip bad block at address: 0x%lX of \"%s\", absolute address: 0x%lX\n",
					 (CVI_U64)offset, ptn->mtddev, ((CVI_U64)offset + ptn->start));
			(*blockindex)++;
			rel++;
		}
		offset = (loff_t)(*blockindex) * (loff_t)blocksize;
	} while (badblock == 1 && offset < (loff_t)size);

	return rel;
}

// printf("\t> ptn->start=%#llx,end=%#llx,endaddr=%#llx,offset_addr=%#llx, offset=%#x, ptn->fd=%d\n",
// ptn->start, ptn->end, endaddr, offset_addr, (CVI_U32)offset, ptn->fd);
CVI_S32 nand_raw_get_physical_index(CVI_U64 startaddr, CVI_S32 *blockindex, CVI_S32 blocksize)
{
	struct mtd_partition *ptn;
	CVI_S32 badblock;
	CVI_S32 ix, i = 0;

	loff_t offset = (CVI_UL)0;
	CVI_U64 offset_addr = startaddr;
	CVI_S32 logcial_index = *blockindex;
	CVI_S32 physical_index = 0;

	for (ix = 0; i < logcial_index && ix < nandraw_ctrl->num_partition; ix++) {
		ptn = &nandraw_ctrl->partition[ix];

		if (ptn->end + 1 <= startaddr) {
			continue;
		}

		if (ptn->fd == INVALID_FD) {
			return -1;
		}

		while ((i < logcial_index) && (offset_addr < ptn->end)) {
			offset = (loff_t)(offset_addr - ptn->start);

			badblock = ioctl(ptn->fd, MEMGETBADBLOCK, &offset);
			if (badblock < 0) {
				printf("Get nand badblock fail. error(%d)\n", errno);
				return -1;
			}

			if (badblock == 0) {
				// printf("> Skip bad block at address: 0x%lX of \"%s\", absolute address: 0x%llX\n",
				// (unsigned long)offset, ptn->mtddev, (unsigned long long)(offset + ptn->start));
				i++;
			}
			physical_index++;
			offset_addr += (CVI_UL)blocksize;
		}
	}

	// printf("logcial_addr=%d, physical_addr=%d\n", logcial_index, physical_index);
	// (unsigned long long)physical_index << nandraw_ctrl->blockshift, physical_index);
	*blockindex = physical_index;

	return 0;
}

/*****************************************************************************/

