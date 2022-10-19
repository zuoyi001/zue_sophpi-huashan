#ifndef __SCREEN_MIPIDSI__H__
#define __SCREEN_MIPIDSI__H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

#include <stdint.h>
#include <stdbool.h>

#include "mipi_tx.h"
#include "cvi_type.h"
#include "cvi_comm_mipi_tx.h"
#include "cvi_common.h"

int32_t MIPIDSI_Init(void);
int32_t MIPIDSI_SetDeviceConfig(const struct combo_dev_cfg_s* devCfg);
int32_t MIPIDSI_EnableVideoDataTransport(void);
int32_t MIPIDSI_DisableVideoDataTransport(void);
int32_t MIPIDSI_WriteCmd(struct cmd_info_s *cmd_info);
int32_t MIPIDSI_ReadCmd(struct get_cmd_info_s *cmd_info);
void MIPIDSI_Deinit(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */

#endif /* SCREEN_MIPIDSI  */

