#ifndef NAND_RAWH
#define NAND_RAWH

#include "nand.h"
#include "cvi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define CVI_FLASH_END_DUETO_BADBLOCK -10
#define HINFC610_OOBSIZE_FOR_YAFFS	32

/**
 * MTD operation modes
 *
 * @MTD_OPS_PLACE_OOB:	OOB data are placed at the given offset (default)
 * @MTD_OPS_AUTO_OOB:	OOB data are automatically placed at the free areas
 *						which are defined by the internal ecclayout
 * @MTD_OPS_RAW:		data are transferred as-is, with no error correction;
 *						this mode implies %MTD_OPS_PLACE_OOB
 *
 * These modes can be passed to ioctl(MEMWRITE) and are also used internally.
 * See notes on "MTD file modes" for discussion on %MTD_OPS_RAW vs.
 * %MTD_FILE_MODE_RAW.
 */
enum {
	MTD_OPS_PLACE_OOB = 0,
	MTD_OPS_AUTO_OOB = 1,
	MTD_OPS_RAW = 2,
};

CVI_S32 nand_read_refactoring(CVI_S32 read_oob, CVI_UL length, CVI_U8 *buffer, CVI_S32 totalread,
			CVI_U64 offset, struct mtd_partition *ptn);

CVI_S32 nand_raw_init(void);

CVI_S32 nand_raw_read(CVI_S32 fd, CVI_U64 *startaddr, CVI_U8 *buffer, CVI_UL length,
					 CVI_U64 openaddr, CVI_U64 limit_leng, CVI_S32 read_oob, CVI_S32 skip_badblock);

CVI_S32 nand_raw_write(CVI_S32 fd, CVI_U64 *startaddr, CVI_U8 *buffer, CVI_UL length,
					  CVI_U64 openaddr, CVI_U64 limit_leng, CVI_S32 write_oob);

CVI_S64 nand_raw_erase(CVI_S32 fd, CVI_U64 startaddr, CVI_U64 length, CVI_U64 openaddr, CVI_U64 limit_leng);

CVI_S32 nand_raw_force_erase(CVI_U64 offset);

CVI_S32 nand_mark_badblock(CVI_U64 offset, CVI_U64 length);

CVI_S32 nand_show_badblock(CVI_U64 offset, CVI_U64 length);

CVI_S32 nand_raw_info(struct mtd_info_user *mtdinfo);

CVI_S32 nand_raw_dump_partition(void);

CVI_S32 nand_raw_destroy(void);

CVI_U64 nand_raw_get_start_addr(const CVI_CHAR *dev_name, CVI_UL blocksize, CVI_S32 *value_valid);

void nand_raw_get_info(CVI_U64 *totalsize, CVI_U32 *pagesize, CVI_U32 *blocksize,
					   CVI_U32 *oobsize, CVI_U32 *blockshift);

int nand_raw_get_physical_index(CVI_U64 startaddr, CVI_S32 *blockindex, CVI_S32 blocksize);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* NAND_RAWH */

