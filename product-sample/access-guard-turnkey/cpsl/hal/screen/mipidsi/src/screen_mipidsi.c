#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "screen_mipidsi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

//rtos api
#if 0//!defined(AMP_LINUX)
extern int  mipi_tx_module_init(void);
extern void  mipi_tx_module_exit(void);
#endif

#define MIPIDSI_DEV       ("/dev/cvi-mipi-tx")
static int32_t g_MipiDsiFd = -1;

int32_t MIPIDSI_Init(void)
{
    // int32_t ret = 0;
#if 0//!defined(AMP_LINUX)
    ret = mipi_tx_module_init();
    if (ret != 0) {
        printf("init screen failed\n");
        return -1;
    };
#else
	//loader cvi_mipi_tx driver 
    // ret = system("insmod /mnt/system/ko/cvi_mipi_tx.ko smooth=0");
	// if(ret != 0){
	// 	perror("insmod failed!.\n");
	// }
#endif
    g_MipiDsiFd = open(MIPIDSI_DEV, O_RDWR);
    if (g_MipiDsiFd == -1) {
        return -1;
    }
	
    return 0;
}

void MIPIDSI_Deinit(void)
{
	int32_t ret = 0;
	
    if (g_MipiDsiFd != -1) {
        close(g_MipiDsiFd);
    }
#if 0//!defined(AMP_LINUX)
    mipi_tx_module_exit();
#else
    ret = system("rmmod cvi_mipi_tx.ko");
	if(ret != 0){
		perror("rmmod failed!.\n");
	}

#endif
}


int32_t MIPIDSI_SetDeviceConfig(const struct combo_dev_cfg_s *devCfg)
{
    int32_t ret = 0;
    struct combo_dev_cfg_s comboDevCfg = {0};
    memcpy(&comboDevCfg, devCfg, sizeof(struct combo_dev_cfg_s));
	
	ret = mipi_tx_cfg(g_MipiDsiFd, &comboDevCfg);
    if (ret != 0) {
        perror("mipi_tx_cfg failed.\n");
        close(g_MipiDsiFd);
        return -1 ;
    }
    return ret;
}

int32_t MIPIDSI_EnableVideoDataTransport(void)
{
    int32_t ret = 0;
	
	ret = mipi_tx_enable(g_MipiDsiFd);
    if (ret != 0) {
        perror("mipi_tx_enable failed\n");
        close(g_MipiDsiFd);
        return -1;
    }
	
    return ret;
}

int32_t MIPIDSI_DisableVideoDataTransport(void)
{
    int32_t ret = 0;

	ret = mipi_tx_disable(g_MipiDsiFd);
    if (ret != 0) {
        perror("mipi_tx_disable failed\n");
        close(g_MipiDsiFd);
        return -1;
    }
    return ret;
}

int32_t MIPIDSI_WriteCmd(struct cmd_info_s *cmd_info)
{
	int32_t ret = 0;
	
	if (cmd_info->cmd_size == 0)
		return -1;

	ret = mipi_tx_send_cmd(g_MipiDsiFd, cmd_info);
	if (-1 == ret) {
		perror("mipi_tx_send_cmd");
		return -1;
	}
	
	return 0;
}

int32_t MIPIDSI_ReadCmd(struct get_cmd_info_s *cmd_info)
{
	int32_t ret = 0;

	ret = mipi_tx_recv_cmd(g_MipiDsiFd, cmd_info);
	if (-1 == ret) {
		perror("mipi_tx_recv_cmd");
		return -1;
	}
	
	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

