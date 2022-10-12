#include <cpu.h>
#include <mmio.h>
#include <debug.h>
#include <assert.h>
#include <errno.h>
#include <bl_common.h>
#include <platform.h>
#include <delay_timer.h>
#include <console.h>
#include <string.h>
#include <rom_api.h>

#include "ddr_pkg_info.h"
#include "rtc.h"

void panic_handler(void)
{
	void *ra;

	ATF_ERR = ATF_ERR_PLAT_PANIC;

	ra = __builtin_return_address(0);
	mmio_write_32(ATF_ERR_INFO0, ((uint64_t)ra) & 0xFFFFFFFFUL);

	ERROR("ra=0x%lx\n", (uint64_t)ra);

	__system_reset("panic", -1);
	__builtin_unreachable();
}

void __system_reset(const char *file, unsigned int line)
{
	ATF_ERR = ATF_ERR_PLAT_SYSTEM_RESET;
	ERROR("RESET:%s:%d\n", file, line);

	console_flush();

	ATF_STATE = ATF_STATE_RESET_WAIT;
	mdelay(5000);

	// enable rtc wdt reset
	mmio_write_32(0x050260E0, 0x0001); //enable rtc_core wathdog reset enable
	mmio_write_32(0x050260C8, 0x0001); //enable rtc_core power cycle   enable

	// sw delay 100us
	ATF_STATE = ATF_STATE_RESET_RTC_WAIT;
	udelay(100);

	// mmio_write_32(0x05025018,0x00FFFFFF); //Mercury rtcsys_rstn_src_sel
	mmio_write_32(0x050250AC, 0x00000000); //Mars rtcsys_rstn_src_sel
	mmio_write_32(0x05025004, 0x0000AB18);
	mmio_write_32(0x05025008, 0x00400040); //enable rtc_ctrl wathdog reset enable

	// printf("Enable TOP_WDT\n");
	// mmio_write_32(0x03010004,0x00000000); //config watch dog 2.6ms
	// mmio_write_32(0x03010004,0x00000022); //config watch dog 166ms
	mmio_write_32(0x03010004, 0x00000066); //config watch dog 166ms
	mmio_write_32(0x0301001c, 0x00000020);
	mmio_write_32(0x0301000c, 0x00000076);
	mmio_write_32(0x03010000, 0x00000011);

	// ROM_PWR_CYC
	if (get_sw_info()->reset_type == 1) {
		ATF_ERR = ATF_ERR_PLAT_SYSTEM_PWR_CYC;
		// printf("Issue RTCSYS_PWR_CYC\n");

		// wait pmu state to ON
		while (mmio_read_32(0x050260D4) != 0x00000003) {
			;
		}

		mmio_write_32(0x05025008, 0x00080008);
	}

	while (1)
		;

	__builtin_unreachable();
}

void reset_c906l(uintptr_t reset_address)
{
	NOTICE("RSC.\n");

	mmio_clrbits_32(0x3003024, 1 << 6);

	mmio_setbits_32(SEC_SYS_BASE + 0x04, 1 << 13);
	mmio_write_32(SEC_SYS_BASE + 0x20, reset_address);
	mmio_write_32(SEC_SYS_BASE + 0x24, reset_address >> 32);

	mmio_setbits_32(0x3003024, 1 << 6);
}

void setup_dl_flag(void)
{
	uint32_t v = p_rom_api_get_boot_src();

	switch (v) {
	case BOOT_SRC_SD:
		mmio_write_32(BOOT_SOURCE_FLAG_ADDR, MAGIC_NUM_SD_DL);
		break;
	case BOOT_SRC_USB:
		mmio_write_32(BOOT_SOURCE_FLAG_ADDR, MAGIC_NUM_USB_DL);
		break;
	default:
		mmio_write_32(BOOT_SOURCE_FLAG_ADDR, v);
		break;
	}
}

void sys_switch_all_to_pll(void)
{
	// TBD
}

void sys_pll_init(void)
{
	// TBD
}

void switch_rtc_mode_1st_stage(void)
{
	// TBD
}

void switch_rtc_mode_2nd_stage(void)
{
	// TBD
}

void set_rtc_en_registers(void)
{
	// TBD
}

void init_comm_info(void)
{
#ifdef RTOS_ENABLE_FREERTOS
	struct transfer_config_t *transfer_config = (struct transfer_config_t *)MAILBOX_FIELD;
	struct transfer_config_t transfer_config_s;
	unsigned char *ptr = (unsigned char *)&transfer_config_s;
	unsigned short checksum = 0;
	/* mailbox field is 4 byte write access, and can not access byte by byte.
	 * so init parameters and copy all to mailbox field together.
	 */
	transfer_config_s.conf_magic = RTOS_MAGIC_HEADER;
	transfer_config_s.conf_size = ((uint64_t)&transfer_config_s.checksum - (uint64_t)&transfer_config_s.conf_magic);
	transfer_config_s.isp_buffer_addr = CVIMMAP_ISP_MEM_BASE_ADDR;
	transfer_config_s.isp_buffer_size = CVIMMAP_ISP_MEM_BASE_SIZE;
	transfer_config_s.encode_img_addr = CVIMMAP_H26X_BITSTREAM_ADDR;
	transfer_config_s.encode_img_size = CVIMMAP_H26X_BITSTREAM_SIZE;
	transfer_config_s.encode_buf_addr = CVIMMAP_H26X_ENC_BUFF_ADDR;
	transfer_config_s.encode_buf_size = CVIMMAP_H26X_ENC_BUFF_SIZE;
	transfer_config_s.dump_print_enable = RTOS_DUMP_PRINT_ENABLE;
	transfer_config_s.dump_print_size_idx = RTOS_DUMP_PRINT_SZ_IDX;
	transfer_config_s.image_type = RTOS_FAST_IMAGE_TYPE;
	transfer_config_s.mcu_status = MCU_STATUS_NONOS_DONE;
	for (int i = 0; i < transfer_config_s.conf_size; i++) {
		checksum += ptr[i];
	}

	transfer_config_s.checksum = checksum;
	memcpy(transfer_config, &transfer_config_s, sizeof(struct transfer_config_t));
#endif
}

void apply_analog_trimming_data(void)
{
}
