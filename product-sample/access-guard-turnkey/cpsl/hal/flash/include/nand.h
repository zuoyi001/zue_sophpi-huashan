#ifndef NANDH
#define NANDH
/******************************************************************************/
#include <stdint.h>
//#include "cvi_appcomm_log.h"
#include "cvi_flash.h"

#define MTD_ABSENT		 0
#define MTD_RAM			 1
#define MTD_ROM			 2
#define MTD_SPIFLASH	 3
#define MTD_NANDFLASH	 4 /* SLC NAND */
#define MTD_DATAFLASH	 6
#define MTD_UBIVOLUME	 7
#define MTD_MLCNANDFLASH 8 /* MLC NAND (including TLC) */

#define MAX_PARTS		  32 /* Flash max partition number */
#define MAX_MTD_PARTITION (MAX_PARTS)

#if defined(ANDROID)
#define DEV_MTDBASE "/dev/mtd/mtd"
#else
#define DEV_MTDBASE "/dev/mtd"
#endif

#define PROC_MTD_FILE "/proc/mtd"

/*****************************************************************************/
#ifdef CVI_MODULE
#undef CVI_MODULE
#endif
#define CVI_MODULE "cviFlash"

//#ifndef __kernel_loff_t
//typedef long long          __kernel_loff_t;
//#endifs

/*****************************************************************************/

struct mtd_info_user {
	CVI_U8 type;
	CVI_U32 flags;
	CVI_U32 size;
	CVI_U32 erasesize;
	CVI_U32 writesize;
	CVI_U32 oobsize;
	CVI_U32 ecctype;
	CVI_U32 eccsize;
};
#define MEMGETINFO _IOR('M', 1, struct mtd_info_user)

struct erase_info_user64 {
	CVI_U64 start;
	CVI_U64 length;
};
#define MEMERASE64 _IOW('M', 20, struct erase_info_user64)

struct mtd_oob_buf {
	CVI_U32 start;
	CVI_U32 length;
	CVI_U8 *ptr;
};
#define MEMREADOOB _IOWR('M', 4, struct mtd_oob_buf)

/**
 * struct mtd_write_req - data structure for requesting a write operation
 *
 * @start:		start address
 * @len:		length of data buffer
 * @ooblen:		length of OOB buffer
 * @usr_data:	user-provided data buffer
 * @usr_oob:	user-provided OOB buffer
 * @mode:		MTD mode (see "MTD operation modes")
 * @padding:	reserved, must be set to 0
 *
 * This structure supports ioctl(MEMWRITE) operations, allowing data and/or OOB
 * writes in various modes. To write to OOB-only, set @usr_data == NULL, and to
 * write data-only, set @usr_oob == NULL. However, setting both @usr_data and
 * @usr_oob to NULL is not allowed.
 */
struct mtd_write_req {
	CVI_U64 start;
	CVI_U64 len;
	CVI_U64 ooblen;
	CVI_U64 usr_data;
	CVI_U64 usr_oob;
	CVI_U8 mode;
	CVI_U8 padding[7];
};
#define MEMWRITE _IOWR('M', 24, struct mtd_write_req)

#define MEMGETBADBLOCK _IOW('M', 11, __kernel_loff_t)

#define MEMSETBADBLOCK _IOW('M', 12, __kernel_loff_t)

#define MEMFORCEERASEBLOCK _IOW('M', 128, __kernel_loff_t)

/*****************************************************************************/

struct mtd_partition {
	CVI_U64 start;
	CVI_U64 end;
	int readonly;
	enum CVI_FLASH_ACCESS_PERM_E perm;
#if defined(ANDROID)
	CVI_CHAR mtddev[25];
#else
	CVI_CHAR mtddev[12];
#endif
	CVI_S32 fd;
};

struct nand_raw_ctrl {
	CVI_S32 num_partition;
	CVI_U64 size;

	CVI_U32 pagesize;
	CVI_U32 blocksize;
	CVI_U32 oobsize;
	CVI_U32 oobused;

	CVI_U32 pageshift;
	CVI_U32 blockshift;

	CVI_U32 pagemask;
	CVI_U32 blockmask;

	struct mtd_partition partition[1];
};

CVI_CHAR *int_to_size(CVI_U64 size);

CVI_S32 get_max_partition(void);

CVI_S32 offshift(unsigned long n);

CVI_S32 flash_partition_info_init(void);

struct CVI_Flash_PartInfo_S *get_flash_partition_info(CVI_FLASH_TYPE_E FlashType, const CVI_CHAR *devname);

CVI_U64 get_flash_total_size(CVI_FLASH_TYPE_E FlashType);

/******************************************************************************/
#endif /* NANDH */

