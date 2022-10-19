#ifndef SPI_H
#define SPI_H

#include "peripheral_gpio.h"

#define NFC_CS_GPIO CVI_GPIOB_09
#define NFC_RST_GPIO CVI_GPIOA_02

extern unsigned char SPIRead(unsigned char addr); //SPI读函数
extern void SPIWrite(unsigned char add,unsigned char wrdata); //SPI写函数
extern void SPIRead_Sequence(unsigned char sequence_length,unsigned char addr,unsigned char *reg_value);
extern void SPIWrite_Sequence(unsigned char sequence_length,unsigned char addr,unsigned char *reg_value);
extern int SPI_Init();
extern void SPI_Uninit();
#endif
