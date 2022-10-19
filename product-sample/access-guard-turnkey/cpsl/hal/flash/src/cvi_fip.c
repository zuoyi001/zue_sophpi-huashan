#include "cvi_fip.h"

#include "cvi_flash.h"
#include "nand.h"

#define PTR_INC(base, offset) (void *)((uint8_t *)(base) + (offset))
#define GET_PG_IDX_IN_BLK(x, y) ((x) % (y))
#define GET_BLK_IDX(x, y) ((x) / (y))
#define PLAT_BM_FIP_MAX_SIZE	0xA0000    // 640KB, Fixed, don't change unless you know it

uint8_t sys_vec_buf[MAX_PAGE_SIZE + MAX_SPARE_SIZE];
struct _spi_nand_base_vector_t *g_spi_nand_sys_vec = (struct _spi_nand_base_vector_t *)sys_vec_buf;
uint8_t pg_buf[MAX_PAGE_SIZE + MAX_SPARE_SIZE];
struct nand_raw_ctrl *nandraw_ctrl;


void spi_nand_dump_vec(void)
{
	u32 i = 0, j = 0;
	struct _spi_nand_base_vector_t *sv = g_spi_nand_sys_vec;

	printf("%s\n", __func__);
	printf("signature: 0x%x\n", sv->signature);
	printf("version 0x%x\n", sv->version);

	for (i = 0; i < SPI_NAND_BASE_DATA_BACKUP_COPY; i++)
		printf("spi_nand_vec_blks 0x%x\n", sv->spi_nand_vector_blks[i]);

	printf("fip_bin_blk_cnt 0x%x\n", sv->fip_bin_blk_cnt);

	for (i = 0; i < SPI_NAND_BASE_DATA_BACKUP_COPY; i++) {
		for (j = 0; j < SPI_NAND_FIP_DATA_BLOCK_COUNT; j++)
			printf("fip bin blks 0x%x\n", sv->fip_bin_blks[i][j]);
	}
#ifdef FLASH_DEBUG
	printf("spi nand info block cnt 0x%x\n", sv->spi_nand_info.block_cnt);
	printf("spi nand info id 0x%x\n", sv->spi_nand_info.id);
	printf("spi nand info pages per block 0x%x\n", sv->spi_nand_info.pages_per_block);
	printf("spi nand info pages per block shift 0x%x\n", sv->spi_nand_info.pages_per_block_shift);
	printf("spi nand info page size 0x%x\n", sv->spi_nand_info.page_size);
	printf("spi nand info spare size 0x%x\n", sv->spi_nand_info.spare_size);
#endif
}

void get_spi_nand_info(void)
{
	CVI_S32 s32Ret = CVI_FAILURE;

	//open fip mtd partition
	s32Ret = CVI_Flash_Open(CVI_FLASH_TYPE_NAND_0, "/dev/mtd0", 0, 0);
	if (s32Ret == CVI_FAILURE)
		printf("Open flash fail\n");
	else
		printf("Open flash success\n");

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

}

int spi_nand_scan_vector(void)
{
	struct _spi_nand_base_vector_t *sv = g_spi_nand_sys_vec;
	CVI_S32 s32Ret = CVI_FAILURE;

	memset(pg_buf, 0, MAX_PAGE_SIZE + MAX_SPARE_SIZE);
	for (int blk_idx = 0; blk_idx < SPI_NAND_BASE_DATA_BACKUP_COPY; blk_idx++) {
		// only check page 0 for each block
		s32Ret = CVI_Flash_Read(CVI_FLASH_TYPE_NAND_0, (CVI_U64)blk_idx * nandraw_ctrl->blocksize,
			pg_buf, nandraw_ctrl->pagesize, CVI_FLASH_RW_FLAG_RAW);

		if (s32Ret == CVI_FAILURE) {
			//printf("spi_nand_scan_vector read error\n");
			continue;
		} else {
			printf("spi_nand_scan_vector read success\n");
		}
		memcpy(sv, pg_buf, sizeof(struct _spi_nand_base_vector_t));

		if (sv->signature == SPI_NAND_VECTOR_SIGNATURE) {
			printf("sv found version 0x%x\n", sv->version);
			return 0;
		}
	}

	printf("Can't find correct system vector!\n");
	return -1;
}

void verify_fip_bin(void *buf)
{
	struct _spi_nand_base_vector_t *sv = g_spi_nand_sys_vec;
	int status = 0;
	void *buffer;
	uint8_t blk_buf[0x20000];

	buffer = (void *)buf;

	for (u32 i = 0; i < SPI_NAND_BASE_DATA_BACKUP_COPY; i++) {

		for (u32 j = 0; j < SPI_NAND_FIP_DATA_BLOCK_COUNT ; j++) {
			u32 blk_id = sv->fip_bin_blks[i][j];

			if (blk_id == 0) {
				continue;
			}
			status = CVI_Flash_Read(CVI_FLASH_TYPE_NAND_0, blk_id * nandraw_ctrl->blocksize, blk_buf,
				 nandraw_ctrl->blocksize, CVI_FLASH_RW_FLAG_RAW);
			if (status == CVI_FAILURE)
				printf("read flash fail\n");
			else
				printf("read flash success\n");

			if (memcmp(buffer, blk_buf, nandraw_ctrl->blocksize)) {
				printf("fip read back compare error!\n");
			}
			buffer += nandraw_ctrl->blocksize;
		}
	}
}

int spi_nand_flush_fip_bin(void *buf)
{
	int status = 0;
	void *buffer = (void *)buf;
	struct _spi_nand_base_vector_t *sv = g_spi_nand_sys_vec;

	for (u32 i = 0; i < SPI_NAND_BASE_DATA_BACKUP_COPY; i++) {
		for (u32 j = 0; j < SPI_NAND_FIP_DATA_BLOCK_COUNT ; j++) {
			u32 blk_id = sv->fip_bin_blks[i][j];

			if (blk_id == 0) {
				continue;
			}
			status = CVI_Flash_Erase(CVI_FLASH_TYPE_NAND_0, blk_id * nandraw_ctrl->blocksize,
					nandraw_ctrl->blocksize);

			if (status == CVI_FAILURE) {
				printf("erase error\n");
				return status;
			}

			status = CVI_Flash_Write(CVI_FLASH_TYPE_NAND_0, (CVI_U64)blk_id * nandraw_ctrl->blocksize,
					(void *)buffer, nandraw_ctrl->blocksize, CVI_FLASH_RW_FLAG_RAW);
			buffer += nandraw_ctrl->blocksize;
			if (status == CVI_FAILURE) {
				printf("Write fail\n");
				return status;
			}
		}
	}
	verify_fip_bin(buf);
	return 0;
}

int spi_nand_flush_vec(void)
{
	int status = 0;
	struct _spi_nand_base_vector_t *sv = g_spi_nand_sys_vec;

	spi_nand_dump_vec();
	for (u32 i = 0; i < SPI_NAND_BASE_DATA_BACKUP_COPY; i++) {
		u32 blk_id = sv->spi_nand_vector_blks[i];
		status = CVI_Flash_Write(CVI_FLASH_TYPE_NAND_0, (CVI_U64)blk_id * nandraw_ctrl->blocksize, (void *)sv,
								 nandraw_ctrl->pagesize, CVI_FLASH_RW_FLAG_RAW);

		if (status == CVI_FAILURE) {
			printf("prg error\n");
			return status;
		}
	}
	return 0;
}

int spi_nand_check_write_vector(void *buf)
{
	int status;
	get_spi_nand_info();
	status = spi_nand_scan_vector();

	if (status == 0) {
		struct _spi_nand_base_vector_t *sv = g_spi_nand_sys_vec;

		status = spi_nand_flush_fip_bin(buf);

		if (status) {
			printf("flush fip bin failed!, please check!\n");
			return -1;
		}
		sv->signature = SPI_NAND_VECTOR_SIGNATURE;

		sv->erase_count++;

		if (sv->version != SPI_NAND_VECTOR_VERSION) {
			printf("update sv & fip version to 0x%x\n", SPI_NAND_VECTOR_VERSION);
			sv->version = SPI_NAND_VECTOR_VERSION;
		}
		spi_nand_dump_vec();
	}

	// finish upgrade fip
	status = CVI_Flash_Close(CVI_FLASH_TYPE_NAND_0);
	if (status == CVI_FAILURE)
		printf("close flash fail\n");
	else
		printf("close flash success\n");
	return 0;
}

