# CVI_FLASH
Sample code is included in cvi_flash.c, This sample will demonstrate the basic oprations of flash.

### To use read/write/erase/getinfo function, You need call as follows:
CVI_Flash_Open -> CVI_Flash_ops(ops = read/write/erase/getinfo) ->CVI_Flash_Close

# CVI_Flash_Open
```c
*\brief: open flash partiton
*\attention
*\param[in] enFlashType Flash type(emmc:0, nand 1, spinor 2, other:3)
*\param[in] pPartitionName	CNcomment:	  None EMMC device (such as SPI/NAND) can only use /dev/mtdx to
*		be partition name. EMMC device can only use /dev/mmcblkopx to be partition name.
*\param[in] u64Address		open address
*\param[in] u64Len			  open length
*\retval    fd					   Flash handle
*\retval    INVALID_FD		  invaild fd
*\see \n
```
### pPartitionName and u64Address/u64Len is alternative, we only can use CVI_Flash_OpenByTypeAndAddr(eg.u64Address/u64Len) flow now.

# CVI_Flash_Read
```c
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
```
# CVI_Flash_Write
```c
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
```

# CVI_Flash_Erase
```c
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
```

# CVI_Flash_Close
```c
*\brief: close flash partition
*\attention \n
*\param[in] hFlash	 flash handle
*\retval ::CVI_SUCCESS
*\retval ::CVI_FAILURE
*\see \n
```
# CVI_Flash_GetInfo
```c
*\brief: get flash partition info
*\attention\n
*info content: TotalSize,PartSize,BlockSize,PageSize,OobSize,fd
*\param[in] hFlash		 flash handle
*\param[in] pInterInfo	 info struct pointer
*\retval ::CVI_SUCCESS
*\retval ::CVI_FAILURE
*\see \n
```
