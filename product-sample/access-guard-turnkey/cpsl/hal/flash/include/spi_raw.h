#ifndef SPI_RAWH
#define SPI_RAWH

#include "nand.h"
#include "cvi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

CVI_S32 spi_raw_init(void);

CVI_S32 spi_raw_destroy(void);

CVI_S32 spi_raw_dump_partition(void);

CVI_S32 spi_raw_read(CVI_S32 fd, CVI_U64 *startaddr, CVI_U8 *buffer, CVI_UL length,
					CVI_U64 openaddr, CVI_U64 limit_leng, CVI_S32 read_oob, CVI_S32 skip_badblock);

CVI_S32 spi_raw_write(CVI_S32 fd, CVI_U64 *startaddr, CVI_U8 *buffer, CVI_UL length,
					 CVI_U64 openaddr, CVI_U64 limit_leng, CVI_S32 write_oob);

CVI_S64 spi_raw_erase(CVI_S32 fd, CVI_U64 startaddr, CVI_U64 length, CVI_U64 openaddr, CVI_U64 limit_leng);

CVI_U64 spi_raw_get_start_addr(const CVI_CHAR *dev_name, CVI_UL blocksize, CVI_S32 *value_valid);

CVI_VOID spi_raw_get_info(CVI_U64 *totalsize, CVI_U32 *pagesize, CVI_U32 *blocksize,
						 CVI_U32 *oobsize, CVI_U32 *blockshift);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* SPI_RAWH */

