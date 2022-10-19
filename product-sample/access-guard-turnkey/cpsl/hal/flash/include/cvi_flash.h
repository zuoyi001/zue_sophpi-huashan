#ifndef __CVI_FLASH_H__
#define __CVI_FLASH_H__
#include "cvi_type.h"
//#include "cvi_debug.h"
#include "emmc_raw.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*************************** Structure Definition ****************************/
/** \addtogroup	 FLASH */
/** @{ *//** <!-- [FLASH] */

#define FLASH_NAME_LEN 32 /** Flash Name max length */

//printf debug log or not
//#define FLASH_DEBUG

/** flash type */
typedef enum _CVI_FLASH_TYPE_E {
	CVI_FLASH_TYPE_SPI_0,  /**< SPI flash type */
	CVI_FLASH_TYPE_NAND_0, /**< NAND flash type */
	CVI_FLASH_TYPE_EMMC_0, /**< eMMC flash type */
	CVI_FLASH_TYPE_BUTT    /**< Invalid flash type */
} CVI_FLASH_TYPE_E;

#define INVALID_FD -1

#define CVI_FLASH_RW_FLAG_RAW		  0x0 /** read and write without OOB,for example: kernel/uboot/ubi/cramfs.. */
#define CVI_FLASH_RW_FLAG_WITH_OOB	  0x1 /** read and write with OOB, example: yaffs2 filesystem image */
#define CVI_FLASH_RW_FLAG_ERASE_FIRST 0x2 /** erase before write */

/** this macro for return value when nand flash have bad block or valid length less partition length */
/** CNcomment:When nand have bad block. Do write/read/erase action may let effective length smaller than */
/** partition size or open length. So after read/write/erase effective length, return the value */
#define CVI_FLASH_END_DUETO_BADBLOCK -10

extern CVI_S32 find_flash_part(CVI_CHAR *cmdline_string,
							  const CVI_CHAR *media_name, /* cvi_sfc, hinand */
							  CVI_CHAR *ptn_name,
							  CVI_U64 *start,
							  CVI_U64 *length);
extern CVI_S32 find_part_from_devname(CVI_CHAR *media_name, CVI_CHAR *bootargs,
						 CVI_CHAR *devname, CVI_U64 *start, CVI_U64 *size);
extern struct EMMC_FLASH_S g_stEmmcFlash;

/** Flash partition access permission type */
enum CVI_FLASH_ACCESS_PERM_E {
	ACCESS_NONE = 0,
	ACCESS_RD = (1 << 1),
	ACCESS_WR = (1 << 2),
	ACCESS_RDWR = (ACCESS_RD | ACCESS_WR),
	ACCESS_BUTT
};

/** Flash partition descriptions */
struct CVI_Flash_PartInfo_S {
	CVI_U64 StartAddr;				   /**< Partiton start address */
	CVI_U64 PartSize;				   /**< Partition size */
	CVI_U32 BlockSize;				   /**< The Block size of the flash where this partition at */
	CVI_FLASH_TYPE_E FlashType;		   /**< The flash type where this partition at */
	CVI_CHAR DevName[FLASH_NAME_LEN];  /**< The device node name where this partition relate to */
	CVI_CHAR PartName[FLASH_NAME_LEN]; /**< The partition name of this partition */
	enum CVI_FLASH_ACCESS_PERM_E perm;	   /**< The partition access permission type */
};

/** Flash operation descriptions */
struct FLASH_OPT_S {
	CVI_S32 (*raw_read)
	(CVI_S32 fd, CVI_U64 *startaddr, CVI_U8 *buffer, CVI_UL length,
	 CVI_U64 openaddr, CVI_U64 limit_leng, CVI_S32 read_oob, CVI_S32 skip_badblock);
	CVI_S32 (*raw_write)
	(CVI_S32 fd, CVI_U64 *startaddr, CVI_U8 *buffer, CVI_UL length,
	 CVI_U64 openaddr, CVI_U64 limit_leng, CVI_S32 write_oob);
	CVI_S64 (*raw_erase)(CVI_S32 fd, CVI_U64 startaddr, CVI_U64 length, CVI_U64 openaddr, CVI_U64 limit_leng);
};

/** Flash Information */
struct CVI_Flash_InterInfo_S {
	CVI_U64 TotalSize;				 /**< flash total size */
	CVI_U64 PartSize;				 /**< flash partition size */
	CVI_U32 BlockSize;				 /**< flash block size */
	CVI_U32 PageSize;				 /**< flash page size */
	CVI_U32 OobSize;				 /**< flash OOB size */
	CVI_S32 fd; /**< file handle */   /**<CNcomment:file handle, open by addr could not get true handle */
	CVI_U64 OpenAddr;				 /**< flash open address */
	CVI_U64 OpenLeng;				 /**< flash open length */
	CVI_FLASH_TYPE_E FlashType;		 /**< flash type */
	struct FLASH_OPT_S *pFlashopt;			/**< operation callbacks on this flash */
	struct CVI_Flash_PartInfo_S *pPartInfo; /**< parition descriptions on this flash */
};

/** @} *//** <!-- ==== Structure Definition end ==== */

/******************************* API declaration *****************************/
/** \addtogroup		 FLASH */
/** @{ *//** <!-- [FLASH] */

/**
 *\brief: open flash partiton
 *\attention \n
 *\param[in] enFlashType		Flash type
 *\param[in] pPartitionName	CNcomment:	  None EMMC device (such as SPI/NAND) can only use /dev/mtdx to
 *		be partition name. EMMC device can only use /dev/mmcblkopx to be partition name.
 *\param[in] u64Address		open address
 *\param[in] u64Len			  open length
 *\retval    fd					   Flash handle
 *\retval    INVALID_FD		  invaild fd
 *\see \n
 */
CVI_HANDLE CVI_Flash_Open(CVI_FLASH_TYPE_E enFlashType, CVI_CHAR *pPartitionName,
		CVI_U64 u64Address, CVI_U64 u64Len);
/**
 *\brief:  open flash patition by name
 *\attention \n
 *\param[in] pPartitionName	patition name CNcomment:None EMMC device (such as SPI/NAND) can only use /
 *		dev/mtdx to be partition name. Emmc device can only use the name set by blkdevparts=mmcblk0
 *		in bootargs, could not use /dev/mmcblkopx to be partition name
 *\retval    fd				Flash hande
 *\retval    INVALID_FD		invaild fd
 *\see \n
 */
CVI_HANDLE CVI_Flash_OpenByName(CVI_CHAR *pPartitionName);

/**
 *\brief: open flash patition by	type and name
 *\attention \n
 *\param[in] enFlashType		flash type
 *\param[in] pPartitionName	patition name	 None EMMC device (such as SPI/NAND) can only use /dev/mtdx
 *to be partition name. EMMC device can only use /dev/mmcblkopx to be partition name.
 *\retval    fd				Flash hande
 *\retval    INVALID_FD		invaild fd
 *\see \n
 */
CVI_HANDLE CVI_Flash_OpenByTypeAndName(CVI_FLASH_TYPE_E enFlashType, CVI_CHAR *pPartitionName);

/**
 *\brief: open flash patition by	address
 *\attention \n
 *\param[in] enFlashType		 flash type
 *\param[in] u64Address		 open address
 *\param[in] u64Len			 open length
 *\retval    fd				 Flash hande
 *\retval    INVALID_FD		 invaild fd
 *\see \n
 */
CVI_HANDLE CVI_Flash_OpenByTypeAndAddr(CVI_FLASH_TYPE_E enFlashType, CVI_U64 u64Address, CVI_U64 u64Len);

/**
 *\brief: close flash partition
 *\attention \n
 *\param[in] hFlash	 flash handle
 *\retval ::CVI_SUCCESS
 *\retval ::CVI_FAILURE
 *\see \n
 */
CVI_S32 CVI_Flash_Close(CVI_HANDLE hFlash);

/**
 *\brief: erase Flash partiton
 *\attention \n
 *\param[in] hFlash	   flash handle
 *\param[in] u64Address  erase start address, must align with blocksize
 *\param[in] u32Len	   data length, must align with blocksize
 *\retval ::TotalErase   erase total length
 *\retval ::CVI_FLASH_END_DUETO_BADBLOCK	   return value when nand flash have bad block or valid
 *length less partition length
 *\retval ::CVI_FAILURE	failure
 *\see \n
 */
CVI_S32 CVI_Flash_Erase(CVI_HANDLE hFlash, CVI_U64 u64Address, CVI_U64 u64Len);

/**
 *\brief:  read data from flash
 *\attention \n
 *\param[in] hFlash		flash handle
 *\param[in] u64Address	read start address, for nand, must align with pagesize.
 *\param[in] pBuf		destination buffer pointer
 *\param[in] u32Len		destination data length
 *\param[in] u32Flags	OOB flag  CNcomment:Can be valued to CVI_FLASH_RW_FLAG_WITH_OOB, means if
 *data has OOB CNend
 *\retval ::TotalRead	read flash valid length
 *\retval ::CVI_FLASH_END_DUETO_BADBLOCK	  CNcomment: Read Flash till meet bad block CNend
 *\retval ::CVI_FAILURE	 failure
 *\see \n
 */
CVI_S32 CVI_Flash_Read(CVI_HANDLE hFlash, CVI_U64 u64Address, CVI_U8 *pBuf,
					 CVI_U32 u32Len, CVI_U32 u32Flags);

/**
 *\brief: write data to flash
 *\attention \n
 *1) forbidden used the function when yaffs filesystem is using
 *2) can use CVI_FLASH_RW_FLAG_ERASE_FIRST manner write flah , can write over all partition one time
 *or write with block
 *
 *CNcomment:1) Could not call this interface to undate current using yaffs filesystem CNend
 *CNcomment:2) When call this interface to write flash, can use CVI_FLASH_RW_FLAG_ERASE_FIRS. And support
 *write down once or block by block.
 *But when write yaffs, must call CVI_Flash_Erase to erase partition to be write CNend
 *
 *\param[in] hFlash		flash handle
 *\param[in] u64Address	data start address, for nand, must align with pagesize
 *\param[in] pBuf		destination buffer pointer
 *\param[in] u32Len		destination data length, for nand, if write with oob, must align with
 *	(pagesize + oobsize)
 *\param[in] u32Flags	OOB flag CNcomment: Can be valued to CVI_FLASH_RW_FLAG_WITH_OOB, means if data
 *	has OOB CNend
 *\retval ::TotalWrite	write flash  length
 *\retval ::CVI_FLASH_END_DUETO_BADBLOCK	 have bad block CNcomment: Read Flash till meet bad block CNend
 *\retval ::CVI_FAILURE
 *\see \n
 */
CVI_S32 CVI_Flash_Write(CVI_HANDLE hFlash, CVI_U64 u64Address,
					  CVI_U8 *pBuf, CVI_U32 u32Len, CVI_U32 u32Flags);

/**
 *\brief: get flash partition info
 *\attention\n
 *info content: TotalSize,PartSize,BlockSize,PageSize,OobSize,fd
 *\param[in] hFlash		 flash handle
 *\param[in] pInterInfo	 info struct pointer
 *\retval ::CVI_SUCCESS
 *\retval ::CVI_FAILURE
 *\see \n
 */
CVI_S32 CVI_Flash_GetInfo(CVI_HANDLE hFlash, struct CVI_Flash_InterInfo_S *pFlashInfo);

/** @} *//** <!-- ==== API declaration end ==== */
CVI_S32 CVI_Flash_GetStartAddr(CVI_S32 ix, CVI_U64 *startaddr, CVI_U64 *endaddr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif	// __CVI_FLASH_H__

