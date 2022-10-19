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
#include "spi_raw.h"
#include "cvi_log.h"

static struct nand_raw_ctrl *spiraw_ctrl;

/*****************************************************************************/

CVI_S32 spi_raw_init(CVI_VOID)
{
	CVI_S32 ix;
	CVI_S32 dev;
	CVI_S32 readonly;
	CVI_S32 max_partition;
	// struct stat status;
	struct mtd_partition *ptn;
	struct mtd_info_user mtdinfo;

	// CVI_CHAR buf[sizeof(DEV_MTDBASE) + 5] = DEV_MTDBASE;
	// CVI_CHAR *ptr = buf + sizeof(DEV_MTDBASE) - 1;
	CVI_CHAR buf[PATH_MAX];

	if (spiraw_ctrl) {
		return 0;
	}

	max_partition = get_max_partition();
	if (max_partition == 0) {
#ifdef SPI_RAW_DBG
		printf("Can't find mtd device at /dev/mtdx.\n");
#endif
		return -1;
	}

	if (++max_partition >= MAX_MTD_PARTITION) {
		printf("partition maybe more than %d, please increase MAX_MTD_PARTITION.\n", MAX_MTD_PARTITION);
	}

	if ((flash_partition_info_init()) != 0) {
		printf("Initial partition information failure.\n");
		return -1;
	}

	spiraw_ctrl = (struct nand_raw_ctrl *)malloc(sizeof(struct nand_raw_ctrl)
			 + (CVI_U32) max_partition * sizeof(struct mtd_partition));

	if (!spiraw_ctrl) {
		printf("Not enough memory.\n");
		return -ENOMEM;
	}
	memset(spiraw_ctrl, 0, sizeof(struct nand_raw_ctrl)
		   + (CVI_U32) max_partition * sizeof(struct mtd_partition));

	spiraw_ctrl->num_partition = 0;
	spiraw_ctrl->size = 0;
	ptn = spiraw_ctrl->partition;

	for (ix = 0; ix < max_partition; ix++) {
		readonly = 0;
		ptn->fd = INVALID_FD;
		(CVI_VOID)snprintf(buf, PATH_MAX, DEV_MTDBASE "%d", ix);
		dev = open(buf, O_RDWR);
		if (-1 == dev) {
			dev = open(buf, O_RDONLY);
			if (-1 == dev) {
				// printf("Can't open \"%s\"\n", buf);
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

		if (mtdinfo.type != MTD_SPIFLASH) {
			close(dev);
			continue;
		}

		if ((mtdinfo.type == MTD_NANDFLASH) || (mtdinfo.type == MTD_MLCNANDFLASH)) {
			close(dev);
			break;
		}
		struct CVI_Flash_PartInfo_S *pstPartInfo = NULL;
		CVI_CHAR devname[32];

		memset(devname, 0, sizeof(devname));
		(CVI_VOID)snprintf(devname, sizeof(devname), "mtd%d", ix);
		pstPartInfo = get_flash_partition_info(CVI_FLASH_TYPE_SPI_0, devname);
		if (pstPartInfo == NULL) {
			printf("Can't get \"%s\" partition information.\n", devname);
			close(dev);
			continue;
		}

		strncpy(ptn->mtddev, buf, sizeof(ptn->mtddev));
		ptn->mtddev[sizeof(ptn->mtddev) - 1] = '\0';
		ptn->fd = dev;
		ptn->readonly = readonly;
#if 0
		ptn->start = spiraw_ctrl->size;
		spiraw_ctrl->size += mtdinfo.size;
		ptn->end   = spiraw_ctrl->size - 1;
#else
		ptn->start = pstPartInfo->StartAddr;
		ptn->end = pstPartInfo->StartAddr + mtdinfo.size - 1;
#endif
		spiraw_ctrl->num_partition++;

		ptn++;

		if (spiraw_ctrl->num_partition == 1) {
			spiraw_ctrl->pagesize = mtdinfo.writesize;
			spiraw_ctrl->blocksize = mtdinfo.erasesize;
			spiraw_ctrl->pagemask = (mtdinfo.writesize - 1);
			spiraw_ctrl->blockmask = (mtdinfo.erasesize - 1);
			spiraw_ctrl->oobsize = mtdinfo.oobsize;

			spiraw_ctrl->pageshift = (CVI_U32)offshift(mtdinfo.writesize);
			spiraw_ctrl->blockshift = (CVI_U32)offshift(mtdinfo.erasesize);
		}
	}

	if (!spiraw_ctrl->num_partition) {
		free(spiraw_ctrl);
		spiraw_ctrl = NULL;
		return 0;
	}
	spiraw_ctrl->size = get_flash_total_size(CVI_FLASH_TYPE_SPI_0);

	return 0;
}

CVI_VOID spi_raw_get_info(CVI_U64 *totalsize,
						 CVI_U32 *pagesize,
						 CVI_U32 *blocksize,
						 CVI_U32 *oobsize,
						 CVI_U32 *blockshift)
{
	*totalsize = spiraw_ctrl->size;
	*pagesize = spiraw_ctrl->pagesize;
	*blocksize = spiraw_ctrl->blocksize;
	*oobsize = spiraw_ctrl->oobsize;
	*blockshift = spiraw_ctrl->blockshift;
}

/*****************************************************************************/
/*
 * warning:
 *	  1. if open SPI/NOR FLASH, return 0
	  2. if dev_name cannot match spiraw_ctrl, return error_valid;
 */
CVI_U64 spi_raw_get_start_addr(const CVI_CHAR *dev_name, CVI_UL blocksize, CVI_S32 *value_valid)
{
	struct mtd_partition *ptn;
	CVI_S32 max_partition;
	CVI_S32 ix;

	ptn = spiraw_ctrl->partition;

	max_partition = get_max_partition();
	if (max_partition == 0) {
		// printf("Can't find mtd device at /dev/mtdx.\n");
		return 0;
	}

	if (spiraw_ctrl->blocksize != blocksize) {
		*value_valid = 1;
		return 0;
	}

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

CVI_S32 spi_raw_read(CVI_S32 fd,
					CVI_U64 *startaddr, /* this address maybe change when meet bad block */
					CVI_U8 *buffer,
					CVI_UL length, /* if CVI_FLASH_RW_FLAG_WITH_OOB, include oob */
					CVI_U64 openaddr,
					CVI_U64 limit_leng,
					CVI_S32 read_oob,
					CVI_S32 skip_badblock)
{
	CVI_S32 ix;
	CVI_S32 totalread = 0;
	CVI_UL num_read = 0;
	struct mtd_partition *ptn;
	CVI_U64 offset = *startaddr;

	UNUSED(fd);
	UNUSED(openaddr);
	UNUSED(limit_leng);
	UNUSED(read_oob);
	UNUSED(skip_badblock);

	if (!spiraw_ctrl) {
		printf("Please initialize before use this function.\n");
		return -1;
	}
	if (buffer == NULL) {
		return -1;
	}

	if (offset >= spiraw_ctrl->size || !length) {
		return -1;
	}

	for (ix = 0; ix < spiraw_ctrl->num_partition && length; ix++) {
		ptn = &spiraw_ctrl->partition[ix];
		/*lint -save -e655*/
		if ((ptn->start <= offset) && (offset < ptn->end) && length && (ptn->perm & ACCESS_RD) &&
			(ptn->fd != INVALID_FD)) {
			if (offset + length > ptn->end) {
				num_read = (CVI_UL)((ptn->end - offset) + 1);
			} else {
				num_read = length;
			}
			// printf(">ptn->fd=%d, length=%#lx, *startaddr=%#lx, offset=%#lx\n", ptn->fd, num_read,
			// 	*startaddr, offset);

			if (lseek(ptn->fd, (long)(offset - ptn->start), SEEK_SET) != -1
				&& read(ptn->fd, buffer, (size_t)num_read) != (ssize_t)num_read) {
				printf("read \"%s\" fail. error(%d)\n", ptn->mtddev, errno);
				return CVI_FAILURE;
			}

			buffer += num_read;
			length -= num_read;
			totalread += num_read;

			offset += (CVI_UL)num_read;
		}
	}
	*startaddr = offset;

	return totalread;
}
/*****************************************************************************/
/*
 * warning:
 *	  1. offset and length should be alignment with blocksize
 */
CVI_S64 spi_raw_erase(CVI_S32 fd,
					 CVI_U64 startaddr,
					 CVI_U64 length,
					 CVI_U64 openaddr,
					 CVI_U64 limit_leng)
{
	CVI_S32 ix;
	CVI_S64 totalerase = 0;
	CVI_U64 offset = startaddr;
	struct mtd_partition *ptn;

	UNUSED(fd);
	UNUSED(openaddr);
	UNUSED(limit_leng);

	if (!spiraw_ctrl) {
		printf("Please initialize before use this function.\n");
		return -1;
	}

	if (offset >= spiraw_ctrl->size || !length) {
		return -1;
	}

	if (((CVI_UL)offset & spiraw_ctrl->blockmask)
		|| ((CVI_UL)length & spiraw_ctrl->blockmask)) {
		printf("offset or length should be alignment with blocksize(0x%X)\n",
				 (CVI_U32)spiraw_ctrl->blocksize);
		return -1;
	}

	if (offset + length > spiraw_ctrl->size) {
		length = spiraw_ctrl->size - offset;
	}

	for (ix = 0; ix < spiraw_ctrl->num_partition && length; ix++) {
		ptn = &spiraw_ctrl->partition[ix];

		if (ptn->readonly) {
			printf("erase a read only partition \"%s\".\n", ptn->mtddev);
			return -1;
		}

		if ((ptn->start <= offset) && (offset < ptn->end) && length && (ptn->perm & ACCESS_WR)
			&& ptn->fd != INVALID_FD) {
			struct erase_info_user64 eraseinfo;

			eraseinfo.start = (CVI_U64)(offset - ptn->start);
			if (offset + length > ptn->end) {
				eraseinfo.length = (CVI_U64)((ptn->end - offset) + 1);
			} else {
				eraseinfo.length = (CVI_U64)length;
			}

			// printf(">ptn->fd=%d, length=%#lx, *startaddr=%#lx, offset=%#lx\n", ptn->fd,
			// 	(CVI_UL)eraseinfo.length, startaddr,
			// 		offset);

			/* don't deal with */
			if (ioctl(ptn->fd, MEMERASE64, &eraseinfo)) {
				printf("Erase 0x%lx failed!\n", offset);
			}

			length -= eraseinfo.length;
			offset += eraseinfo.length;
			totalerase += (CVI_S64)eraseinfo.length;
		}
	}
	return totalerase;
}

/*****************************************************************************/
/*
 * warning:
 *	  1. startaddr should be alignment with pagesize
 */
CVI_S32 spi_raw_write(CVI_S32 fd,
					 CVI_U64 *startaddr,
					 CVI_U8 *buffer,
					 CVI_UL length,
					 CVI_U64 openaddr,
					 CVI_U64 limit_leng,
					 CVI_S32 write_oob)
{
	CVI_S32 ix;
	CVI_S32 totalwrite = 0;
	CVI_S32 num_write = 0;
	struct mtd_partition *ptn;
	CVI_U64 offset = *startaddr;

	UNUSED(fd);
	UNUSED(openaddr);
	UNUSED(limit_leng);
	UNUSED(write_oob);

	if (!spiraw_ctrl) {
		printf("Please initialize before use this function.\n");
		return -1;
	}

	if (offset >= spiraw_ctrl->size || !length) {
		return -1;
	}

	for (ix = 0; ix < spiraw_ctrl->num_partition && length; ix++) {
		ptn = &spiraw_ctrl->partition[ix];
		if (ptn->readonly) {
			printf("Write a read only partition \"%s\".\n", ptn->mtddev);
			return -1;
		}

		if ((ptn->start <= offset) && (offset < ptn->end) && length && (ptn->perm &
				ACCESS_WR) && ptn->fd != INVALID_FD) {
			if (offset + length > ptn->end) {
				num_write = (CVI_S32)((ptn->end - offset) + 1);
			} else {
				num_write = (CVI_S32)length;
			}
			// printf(">ptn->fd=%d, length=%#x, *startaddr=%#lx, offset=%#lx\n", ptn->fd,
			// 	num_write, *startaddr, offset);

			if (lseek(ptn->fd, (off_t)(offset - ptn->start), SEEK_SET) != -1
				&& write(ptn->fd, buffer, (size_t)num_write) != (ssize_t)num_write) {
				printf("write \"%s\" fail. error(%d)\n", ptn->mtddev, errno);
				return CVI_FAILURE;
			}

			buffer += num_write;
			length -= (CVI_U32)num_write;
			totalwrite += num_write;

			offset += (CVI_UL)num_write;
		}
	}

	*startaddr = offset;

	return totalwrite;
}

/*****************************************************************************/

CVI_S32 spi_raw_dump_partition(CVI_VOID)
{
	CVI_S32 ix;
	struct mtd_partition *ptn;

	if (!spiraw_ctrl) {
		printf("Please initialize before use this function.\n");
		return -1;
	}

	printf("-------------------------\n");
	printf("mtd device	 start length mode\n");

	for (ix = 0; ix < spiraw_ctrl->num_partition; ix++) {
		ptn = &spiraw_ctrl->partition[ix];

		if ((ptn->perm & ACCESS_RD) == ACCESS_RD) {
			printf("%-12s ", ptn->mtddev);
			printf("%5s ", int_to_size(ptn->start));
			printf("%6s ", int_to_size(ptn->end + 1 - ptn->start));
			printf("%2s ", ptn->readonly ? "r" : "rw");
			printf("\n");
		}
	}

	return 0;
}
/*****************************************************************************/

CVI_S32 spi_raw_destroy(CVI_VOID)
{
	CVI_S32 ix;

	if (!spiraw_ctrl) {
		return 0;
	}

	for (ix = 0; ix < spiraw_ctrl->num_partition; ix++) {
		if (spiraw_ctrl->partition[ix].fd != INVALID_FD) {
			close(spiraw_ctrl->partition[ix].fd);
		}
	}

	if (spiraw_ctrl != NULL) {
		free(spiraw_ctrl);
	}

	spiraw_ctrl = NULL;
	//printf("\n");

	return 0;
}

