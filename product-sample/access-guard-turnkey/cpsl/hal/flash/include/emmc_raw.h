#ifndef __EMMC_RAW_H__
#define __EMMC_RAW_H__

#include "cvi_type.h"
#include <linux/types.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define EMMC_EXT_PART_ID (5)
#define EMMC_SECTOR_TAIL (0xAA55)
#define EMMC_SECTOR_SIZE (512)
#define MMC_BLOCK_MAJOR  (179)
#define MMC_ERASE_CMD	 _IOW(MMC_BLOCK_MAJOR, 1, struct mmc_erase_cmd)

extern CVI_S32 cmdline_parts_init(CVI_CHAR *bootargs);
extern CVI_S32 get_part_info(CVI_U8 partindex, CVI_U64 *start, CVI_U64 *size);

enum EMMC_PART_TYPE_E {
	EMMC_PART_TYPE_RAW,
	EMMC_PART_TYPE_LOGIC,
	EMMC_PART_TYPE_BUTT
};

struct EMMC_FLASH_S {
	CVI_U64 u64RawAreaStart; /**< absolutely offset from emmc flash start*/
		/** none ext area start address */
	CVI_U64 u64RawAreaSize;  /**< none ext area size, in Byte */
	CVI_U32 u32EraseSize;	 /**< Block size. Default is 512B */
};

struct EMMC_CB_S {
	CVI_S32 fd;
	CVI_U64 u64Address;
	CVI_U64 u64PartSize;
	CVI_U32 u32EraseSize;
	enum EMMC_PART_TYPE_E enPartType;
};

struct mmc_erase_cmd {
	CVI_U32 from; /**< first sector to erase */
	CVI_U32 nr;   /**< number of sectors to erase */
	CVI_U32 arg;  /**< erase command argument (SD supports only %MMC_ERASE_ARG) */
};

CVI_S32 emmc_raw_init(char *bootargs);
struct EMMC_CB_S *emmc_raw_open(CVI_U64 u64Addr, CVI_U64 u64Length);
struct EMMC_CB_S *emmc_node_open(const CVI_U8 *pu8Node);

CVI_S32 emmc_block_read(CVI_S32 fd, CVI_U64 u32Start, CVI_U32 u32Len, CVI_VOID *buff);

CVI_S32 emmc_block_write(CVI_S32 fd, CVI_U64 u32Start, CVI_U32 u32Len, const CVI_VOID *buff);

CVI_S32 emmc_raw_read(const struct EMMC_CB_S *pstEmmcCB, CVI_U64 u64Offset, CVI_U32 u32Length, CVI_U8 *buf);

CVI_S32 emmc_raw_write(const struct EMMC_CB_S *pstEmmcCB, CVI_U64 u64Offset, CVI_U32 u32Length, const CVI_U8 *buf);

CVI_S32 emmc_raw_close(struct EMMC_CB_S *pstEmmcCB);

#if defined(CVI_EMMC_ERASE_SUPPORT)
CVI_S32 emmc_raw_erase(struct EMMC_CB_S *pstEmmcCB, CVI_U64 u64Offset, CVI_U64 u64Length);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __EMMC_RAW_H__ */

