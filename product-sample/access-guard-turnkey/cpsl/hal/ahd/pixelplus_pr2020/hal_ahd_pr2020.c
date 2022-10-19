#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "stdlib.h"
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdbool.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "cvi_hal_ahd.h"
#include "cvi_hal_gpio.h"
#include "cvi_sys.h"
#include "cvi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     AHD */
/** @{ */  /** <!-- [AHD] */

#ifndef UNUSED
#define UNUSED(x) ((x) = (x))
#endif

#define AHD_INIT_ADDR           0x03000098
#define AHD_INIT_MAGIC          0xEEA0D0FF
#define AHD_TYPE_PR2020         0x3
#define AHD_MODE_PR2020_720P25  0x3
#define AHD_MODE_PR2020_720P30  0x4
#define AHD_MODE_PR2020_1080P25 0x7
#define AHD_MODE_PR2020_1080P30 0x8
#define AHD_FILTER_CNT_MAX      0x2

static CVI_HAL_AHD_ATTR_S g_halAhdPR2020Ctx = {CVI_HAL_AHD_STATUS_BUIT, CVI_HAL_AHD_RES_BUIT};
static int hal_ahd_i2c_fd   = -1;
static int hal_ahd_i2c_dev  = 0x3;
static int hal_ahd_i2c_addr = 0x5C;
static int hal_ahd_filter_cnt = AHD_FILTER_CNT_MAX;

static int HAL_AHD_PR2020_I2c_Init(void)
{
    int ret;
    char acDevFile[16] = {0};

    if (hal_ahd_i2c_fd >= 0)
        return 0;

    snprintf(acDevFile, sizeof(acDevFile), "/dev/i2c-%u", hal_ahd_i2c_dev);
    printf("open %s\n", acDevFile);

    hal_ahd_i2c_fd = open(acDevFile, O_RDWR, 0600);

    if (hal_ahd_i2c_fd < 0) {
        printf("Open %s error!\n", acDevFile);
        return -1;
    }

    ret = ioctl(hal_ahd_i2c_fd, I2C_SLAVE_FORCE, hal_ahd_i2c_addr);
    if (ret < 0) {
        printf("I2C_SLAVE_FORCE error!\n");
        close(hal_ahd_i2c_fd);
        hal_ahd_i2c_fd = -1;
        return ret;
    }

    return 0;
}

static int HAL_AHD_PR2020_I2c_Exit(void)
{
    int ret = -1;

    if (hal_ahd_i2c_fd >= 0) {
        close(hal_ahd_i2c_fd);
        hal_ahd_i2c_fd = -1;
        ret = 0;
    }

    return ret;
}

static int pr2020_read_register(int addr)
{
    int ret, data;
    char buf[8];
    int idx = 0;

    if (hal_ahd_i2c_fd < 0)
        return 0;

    // add address byte 0
    buf[idx++] = addr & 0xff;

    ret = write(hal_ahd_i2c_fd, buf, 1);
    if (ret < 0) {
        printf("I2C_READ error!\n");
        return 0;
    }

    buf[0] = 0;
    buf[1] = 0;
    ret = read(hal_ahd_i2c_fd, buf, 1);
    if (ret < 0) {
        printf("I2C_READ error!\n");
        return 0;
    }

    // pack read back data
    data = buf[0];

    // printf("i2c r 0x%x = 0x%x\n", addr, data);
    return data;
}

static int pr2020_write_register(int addr, int data)
{
    int idx = 0;
    int ret;
    char buf[8];

    if (hal_ahd_i2c_fd < 0)
        return 0;

    // add address byte 0
    buf[idx++] = addr & 0xff;

    // add data byte 0
    buf[idx++] = data & 0xff;

    ret = write(hal_ahd_i2c_fd, buf, 2);
    if (ret < 0) {
        printf("I2C_WRITE error!\n");
        return -1;
    }

    // printf("i2c w 0x%x 0x%x\n", addr, data);
    return 0;
}

static int HAL_AHD_PR2020_Gpio_Init(void)
{
    //AHD_PWR_EN
    if (CVI_HAL_GpioSetValue(CVI_GPIOA_00, 1) != 0) {
        printf("set power down gpio error!\n");
        return -1;
    }

    //PR2K_RST
    if (CVI_HAL_GpioSetValue(CVI_GPIOC_31, 1) != 0) {
        printf("set reset gpio error!\n");
        return -1;
    }

    //BACK_DET
    if (CVI_HAL_GpioSetValue(CVI_GPIOD_01, 1) != 0) {
        printf("set back detect gpio error!\n");
        return -1;
    }

    return 0;
}

static void HAL_AHD_PR2020_GetEnvStatus(uint32_t ahd_env)
{
        uint8_t ahd_res = 0;
        ahd_res = 0xF & (ahd_env >> 8);
        printf("ahd_res=%d\n", ahd_res);
        if (ahd_res == AHD_MODE_PR2020_720P25) {
            g_halAhdPR2020Ctx.enStatus = CVI_HAL_AHD_STATUS_CONNECT;
            g_halAhdPR2020Ctx.enRes = CVI_HAL_AHD_RES_720P_25;
        } else if (ahd_res == AHD_MODE_PR2020_720P30) {
            g_halAhdPR2020Ctx.enStatus = CVI_HAL_AHD_STATUS_CONNECT;
            g_halAhdPR2020Ctx.enRes = CVI_HAL_AHD_RES_720P_30;
        } else if (ahd_res == AHD_MODE_PR2020_1080P25) {
            g_halAhdPR2020Ctx.enStatus = CVI_HAL_AHD_STATUS_CONNECT;
            g_halAhdPR2020Ctx.enRes = CVI_HAL_AHD_RES_1080P_25;
        } else if (ahd_res == AHD_MODE_PR2020_1080P30) {
            g_halAhdPR2020Ctx.enStatus = CVI_HAL_AHD_STATUS_CONNECT;
            g_halAhdPR2020Ctx.enRes = CVI_HAL_AHD_RES_1080P_30;
        }
}

static void HAL_AHD_PR2020_SetEnvStatus(void)
{
    uint32_t ahd_env = AHD_INIT_MAGIC;
    uint8_t ahd_type = AHD_TYPE_PR2020;
    uint8_t ahd_res = 0;
    char val[16] = {0};
    char shellcmd[32] = {0};

    if (g_halAhdPR2020Ctx.enRes == CVI_HAL_AHD_RES_720P_25) {
        ahd_res = AHD_MODE_PR2020_720P25;
    } else if (g_halAhdPR2020Ctx.enRes == CVI_HAL_AHD_RES_720P_30) {
        ahd_res = AHD_MODE_PR2020_720P30;
    } else if (g_halAhdPR2020Ctx.enRes == CVI_HAL_AHD_RES_1080P_25) {
        ahd_res = AHD_MODE_PR2020_1080P25;
    } else if (g_halAhdPR2020Ctx.enRes == CVI_HAL_AHD_RES_1080P_30) {
        ahd_res = AHD_MODE_PR2020_1080P30;
    }

    ahd_env |= (ahd_type << 16);
    ahd_env |= (ahd_res << 8);

    sprintf(val, "0x%X", ahd_env);
    snprintf(shellcmd, sizeof(shellcmd), "%s %s %s",
             "fw_setenv", "ahdstatus", val);
    printf("shellcmd=%s\n", shellcmd);
    system(shellcmd);
}

static bool HAL_AHD_PR2020_Check_PreInit(void)
{
    bool bInit = false;
    char *shellcmd = "fw_printenv ahdstatus";
    char *buf = NULL;
    char val[32] = {0};
    uint32_t ahd_env = 0;
    FILE* fp = NULL;

    fp = popen(shellcmd, "r");
    if (fp == NULL) {
        CVI_LOGI("HAL_AHD_PR2020_Check_PreInit popen failed\n");
        return bInit;
    }

    fgets(val, sizeof(val), fp);
    pclose(fp);

    buf = strtok(val, "=");
    buf = strtok(NULL, "=");
    CVI_LOGI("ahdstatus buf=%s\n", buf);

    if (buf != NULL) {
        ahd_env = strtol(buf, NULL, 16);
        printf("ahd_env=0x%X\n", ahd_env);
        if ((ahd_env & AHD_INIT_MAGIC) == AHD_INIT_MAGIC) {
            HAL_AHD_PR2020_GetEnvStatus(ahd_env);
            bInit = true;
        }
    }

    return bInit;
}

static bool HAL_AHD_PR2020_Check_ChipId(void)
{
    bool bRet = true;

    pr2020_write_register(0xff, 0x00);
    if (((pr2020_read_register(0xfc) << 8) | (pr2020_read_register(0xfd))) != 0x2000) {
        printf("read PR2020 chip id fail\n");
        bRet = false;
    }

    return bRet;
}

static void HAL_AHD_PR2020_FW_Init(void)
{
    pr2020_write_register(0xFF, 0x00);
    pr2020_write_register(0xD0, 0x30);
    pr2020_write_register(0xD1, 0x08);
    pr2020_write_register(0xD2, 0x21);
    pr2020_write_register(0xD3, 0x00);
    pr2020_write_register(0xD8, 0x37);
    pr2020_write_register(0xD9, 0x08);

    pr2020_write_register(0xFF, 0x01);
    pr2020_write_register(0x00, 0xE4);//no-video data, 0xe4: black, 0xe5: blue
    pr2020_write_register(0x01, 0x61);
    pr2020_write_register(0x02, 0x00);
    pr2020_write_register(0x03, 0x57);
    pr2020_write_register(0x04, 0x0C);
    pr2020_write_register(0x05, 0x88);
    pr2020_write_register(0x06, 0x04);
    pr2020_write_register(0x07, 0xB2);
    pr2020_write_register(0x08, 0x44);
    pr2020_write_register(0x09, 0x34);
    pr2020_write_register(0x0A, 0x02);
    pr2020_write_register(0x0B, 0x14);
    pr2020_write_register(0x0C, 0x04);
    pr2020_write_register(0x0D, 0x08);
    pr2020_write_register(0x0E, 0x5E);
    pr2020_write_register(0x0F, 0x5E);
    pr2020_write_register(0x10, 0x26);
}

static void HAL_AHD_PR2020_FW_Stop(void)
{
    // stop data
    pr2020_write_register(0xFF, 0x00);
    pr2020_write_register(0x7A, 0x88);
    pr2020_write_register(0x7B, 0x88);
}

static void HAL_AHD_PR2020_FW_Reset(void)
{
    // reset
    pr2020_write_register(0xFF, 0x00);
    pr2020_write_register(0x11, 0x00);
}

static void HAL_AHD_PR2020_SetRes_720P25(void)
{
    printf("HAL_AHD_PR2020_SetRes_720P25\n");
    //Page0 sys
    pr2020_write_register(0xff, 0x00);
    pr2020_write_register(0x10, 0x82);
    pr2020_write_register(0x11, 0x07);
    pr2020_write_register(0x12, 0x00);
    pr2020_write_register(0x13, 0x00);
    pr2020_write_register(0x14, 0x21);//b[1:0); => Select Camera Input. VinP(1), VinN(3), Differ(0).
    pr2020_write_register(0x15, 0x44);
    pr2020_write_register(0x16, 0x0d);
    pr2020_write_register(0x40, 0x00);
    pr2020_write_register(0x47, 0x3a);
    pr2020_write_register(0x4e, 0x3f);
    pr2020_write_register(0x80, 0x56);
    pr2020_write_register(0x81, 0x0e);
    pr2020_write_register(0x82, 0x0d);
    pr2020_write_register(0x84, 0x30);
    pr2020_write_register(0x86, 0x20);
    pr2020_write_register(0x87, 0x00);
    pr2020_write_register(0x8a, 0x00);
    pr2020_write_register(0x90, 0x00);
    pr2020_write_register(0x91, 0x00);
    pr2020_write_register(0x92, 0x00);
    pr2020_write_register(0x94, 0xff);
    pr2020_write_register(0x95, 0xff);
    pr2020_write_register(0x96, 0xff);
    pr2020_write_register(0xa0, 0x00);
    pr2020_write_register(0xa1, 0x20);
    pr2020_write_register(0xa4, 0x01);
    pr2020_write_register(0xa5, 0xe3);
    pr2020_write_register(0xa6, 0x00);
    pr2020_write_register(0xa7, 0x12);
    pr2020_write_register(0xa8, 0x00);
    pr2020_write_register(0xd0, 0x30);
    pr2020_write_register(0xd1, 0x08);
    pr2020_write_register(0xd2, 0x21);
    pr2020_write_register(0xd3, 0x00);
    pr2020_write_register(0xd8, 0x31);
    pr2020_write_register(0xd9, 0x08);
    pr2020_write_register(0xda, 0x21);
    pr2020_write_register(0xe0, 0x39);
    pr2020_write_register(0xe1, 0x90);
    pr2020_write_register(0xe2, 0x38);
    pr2020_write_register(0xe3, 0x19);
    pr2020_write_register(0xe4, 0x19);
    pr2020_write_register(0xea, 0x01);
    pr2020_write_register(0xeb, 0xff);
    pr2020_write_register(0xf1, 0x44);
    pr2020_write_register(0xf2, 0x01);

    //Page1 vdec
    pr2020_write_register(0xff, 0x01);
    pr2020_write_register(0x00, 0xe4);
    pr2020_write_register(0x01, 0x61);
    pr2020_write_register(0x02, 0x00);
    pr2020_write_register(0x03, 0x57);
    pr2020_write_register(0x04, 0x0c);
    pr2020_write_register(0x05, 0x88);
    pr2020_write_register(0x06, 0x04);
    pr2020_write_register(0x07, 0xb2);
    pr2020_write_register(0x08, 0x44);
    pr2020_write_register(0x09, 0x34);
    pr2020_write_register(0x0a, 0x02);
    pr2020_write_register(0x0b, 0x14);
    pr2020_write_register(0x0c, 0x04);
    pr2020_write_register(0x0d, 0x08);
    pr2020_write_register(0x0e, 0x5e);
    pr2020_write_register(0x0f, 0x5e);
    pr2020_write_register(0x10, 0x26);
    pr2020_write_register(0x11, 0x01);
    pr2020_write_register(0x12, 0x45);
    pr2020_write_register(0x13, 0x0c);
    pr2020_write_register(0x14, 0x00);
    pr2020_write_register(0x15, 0x1b);
    pr2020_write_register(0x16, 0xd0);
    pr2020_write_register(0x17, 0x00);
    pr2020_write_register(0x18, 0x41);
    pr2020_write_register(0x19, 0x46);
    pr2020_write_register(0x1a, 0x22);
    pr2020_write_register(0x1b, 0x05);
    pr2020_write_register(0x1c, 0xea);
    pr2020_write_register(0x1d, 0x45);
    pr2020_write_register(0x1e, 0x4c);
    pr2020_write_register(0x1f, 0x00);
    pr2020_write_register(0x20, 0x80);
    pr2020_write_register(0x21, 0x80);
    pr2020_write_register(0x22, 0x90);
    pr2020_write_register(0x23, 0x80);
    pr2020_write_register(0x24, 0x80);
    pr2020_write_register(0x25, 0x80);
    pr2020_write_register(0x26, 0x84);
    pr2020_write_register(0x27, 0x82);
    pr2020_write_register(0x28, 0x00);
    pr2020_write_register(0x29, 0x7d);
    pr2020_write_register(0x2a, 0x00);
    pr2020_write_register(0x2b, 0x00);
    pr2020_write_register(0x2c, 0x00);
    pr2020_write_register(0x2d, 0x00);
    pr2020_write_register(0x2e, 0x00);
    pr2020_write_register(0x2f, 0x00);
    pr2020_write_register(0x30, 0x00);
    pr2020_write_register(0x31, 0x00);
    pr2020_write_register(0x32, 0xc0);
    pr2020_write_register(0x33, 0x14);
    pr2020_write_register(0x34, 0x14);
    pr2020_write_register(0x35, 0x80);
    pr2020_write_register(0x36, 0x80);
    pr2020_write_register(0x37, 0xaa);
    pr2020_write_register(0x38, 0x48);
    pr2020_write_register(0x39, 0x08);
    pr2020_write_register(0x3a, 0x27);
    pr2020_write_register(0x3b, 0x02);
    pr2020_write_register(0x3c, 0x01);
    pr2020_write_register(0x3d, 0x23);
    pr2020_write_register(0x3e, 0x02);
    pr2020_write_register(0x3f, 0xc4);
    pr2020_write_register(0x40, 0x05);
    pr2020_write_register(0x41, 0x55);
    pr2020_write_register(0x42, 0x01);
    pr2020_write_register(0x43, 0x33);
    pr2020_write_register(0x44, 0x6a);
    pr2020_write_register(0x45, 0x00);
    pr2020_write_register(0x46, 0x09);
    pr2020_write_register(0x47, 0xe2);
    pr2020_write_register(0x48, 0x01);
    pr2020_write_register(0x49, 0x00);
    pr2020_write_register(0x4a, 0x7b);
    pr2020_write_register(0x4b, 0x60);
    pr2020_write_register(0x4c, 0x00);
    pr2020_write_register(0x4d, 0x4a);
    pr2020_write_register(0x4e, 0x00);
    pr2020_write_register(0x4f, 0x20);
    pr2020_write_register(0x50, 0x01);
    pr2020_write_register(0x51, 0x28);
    pr2020_write_register(0x52, 0x40);
    pr2020_write_register(0x53, 0x0c);
    pr2020_write_register(0x54, 0x0f);
    pr2020_write_register(0x55, 0x8d);
    pr2020_write_register(0x70, 0x06);
    pr2020_write_register(0x71, 0x08);
    pr2020_write_register(0x72, 0x0a);
    pr2020_write_register(0x73, 0x0c);
    pr2020_write_register(0x74, 0x0e);
    pr2020_write_register(0x75, 0x10);
    pr2020_write_register(0x76, 0x12);
    pr2020_write_register(0x77, 0x14);
    pr2020_write_register(0x78, 0x06);
    pr2020_write_register(0x79, 0x08);
    pr2020_write_register(0x7a, 0x0a);
    pr2020_write_register(0x7b, 0x0c);
    pr2020_write_register(0x7c, 0x0e);
    pr2020_write_register(0x7d, 0x10);
    pr2020_write_register(0x7e, 0x12);
    pr2020_write_register(0x7f, 0x14);
    pr2020_write_register(0x80, 0x00);
    pr2020_write_register(0x81, 0x09);
    pr2020_write_register(0x82, 0x00);
    pr2020_write_register(0x83, 0x07);
    pr2020_write_register(0x84, 0x00);
    pr2020_write_register(0x85, 0x17);
    pr2020_write_register(0x86, 0x03);
    pr2020_write_register(0x87, 0xe5);
    pr2020_write_register(0x88, 0x0a);
    pr2020_write_register(0x89, 0x48);
    pr2020_write_register(0x8a, 0x0a);
    pr2020_write_register(0x8b, 0x48);
    pr2020_write_register(0x8c, 0x0b);
    pr2020_write_register(0x8d, 0xe0);
    pr2020_write_register(0x8e, 0x05);
    pr2020_write_register(0x8f, 0x47);
    pr2020_write_register(0x90, 0x05);
    pr2020_write_register(0x91, 0x69);
    pr2020_write_register(0x92, 0x73);
    pr2020_write_register(0x93, 0xe8);
    pr2020_write_register(0x94, 0x0f);
    pr2020_write_register(0x95, 0x5e);
    pr2020_write_register(0x96, 0x07);
    pr2020_write_register(0x97, 0x90);
    pr2020_write_register(0x98, 0x17);
    pr2020_write_register(0x99, 0x34);
    pr2020_write_register(0x9a, 0x13);
    pr2020_write_register(0x9b, 0x56);
    pr2020_write_register(0x9c, 0x0b);
    pr2020_write_register(0x9d, 0x9a);
    pr2020_write_register(0x9e, 0x09);
    pr2020_write_register(0x9f, 0xab);
    pr2020_write_register(0xa0, 0x01);
    pr2020_write_register(0xa1, 0x74);
    pr2020_write_register(0xa2, 0x01);
    pr2020_write_register(0xa3, 0x6b);
    pr2020_write_register(0xa4, 0x00);
    pr2020_write_register(0xa5, 0xba);
    pr2020_write_register(0xa6, 0x00);
    pr2020_write_register(0xa7, 0xa3);
    pr2020_write_register(0xa8, 0x01);
    pr2020_write_register(0xa9, 0x39);
    pr2020_write_register(0xaa, 0x01);
    pr2020_write_register(0xab, 0x39);
    pr2020_write_register(0xac, 0x00);
    pr2020_write_register(0xad, 0xc1);
    pr2020_write_register(0xae, 0x00);
    pr2020_write_register(0xaf, 0xc1);
    pr2020_write_register(0xb0, 0x0b);
    pr2020_write_register(0xb1, 0x99);
    pr2020_write_register(0xb2, 0x12);
    pr2020_write_register(0xb3, 0xca);
    pr2020_write_register(0xb4, 0x00);
    pr2020_write_register(0xb5, 0x17);
    pr2020_write_register(0xb6, 0x08);
    pr2020_write_register(0xb7, 0xe8);
    pr2020_write_register(0xb8, 0xb0);
    pr2020_write_register(0xb9, 0xce);
    pr2020_write_register(0xba, 0x90);
    pr2020_write_register(0xbb, 0x00);
    pr2020_write_register(0xbc, 0x00);
    pr2020_write_register(0xbd, 0x04);
    pr2020_write_register(0xbe, 0x05);
    pr2020_write_register(0xbf, 0x00);
    pr2020_write_register(0xc0, 0x00);
    pr2020_write_register(0xc1, 0x12);
    pr2020_write_register(0xc2, 0x02);
    pr2020_write_register(0xc3, 0xd0);

    pr2020_write_register(0xff, 0x01);
    pr2020_write_register(0x54, 0x0e);
    pr2020_write_register(0xff, 0x01);
    pr2020_write_register(0x54, 0x0f);
}

static void HAL_AHD_PR2020_SetRes_720P30(void)
{
    printf("HAL_AHD_PR2020_SetRes_720P30\n");
    //Page0 sys
    pr2020_write_register(0xff, 0x00);
    pr2020_write_register(0x10, 0x92);
    pr2020_write_register(0x11, 0x07);
    pr2020_write_register(0x12, 0x00);
    pr2020_write_register(0x13, 0x00);
    pr2020_write_register(0x14, 0x21);//b[1:0); => Select Camera Input. VinP(1), VinN(3), Differ(0).
    pr2020_write_register(0x15, 0x44);
    pr2020_write_register(0x16, 0x0d);
    pr2020_write_register(0x40, 0x00);
    pr2020_write_register(0x47, 0x3a);
    pr2020_write_register(0x4e, 0x3f);
    pr2020_write_register(0x80, 0x56);
    pr2020_write_register(0x81, 0x0e);
    pr2020_write_register(0x82, 0x0d);
    pr2020_write_register(0x84, 0x30);
    pr2020_write_register(0x86, 0x20);
    pr2020_write_register(0x87, 0x00);
    pr2020_write_register(0x8a, 0x00);
    pr2020_write_register(0x90, 0x00);
    pr2020_write_register(0x91, 0x00);
    pr2020_write_register(0x92, 0x00);
    pr2020_write_register(0x94, 0xff);
    pr2020_write_register(0x95, 0xff);
    pr2020_write_register(0x96, 0xff);
    pr2020_write_register(0xa0, 0x00);
    pr2020_write_register(0xa1, 0x20);
    pr2020_write_register(0xa4, 0x01);
    pr2020_write_register(0xa5, 0xe3);
    pr2020_write_register(0xa6, 0x00);
    pr2020_write_register(0xa7, 0x12);
    pr2020_write_register(0xa8, 0x00);
    pr2020_write_register(0xd0, 0x30);
    pr2020_write_register(0xd1, 0x08);
    pr2020_write_register(0xd2, 0x21);
    pr2020_write_register(0xd3, 0x00);
    pr2020_write_register(0xd8, 0x31);
    pr2020_write_register(0xd9, 0x08);
    pr2020_write_register(0xda, 0x21);
    pr2020_write_register(0xe0, 0x39);
    pr2020_write_register(0xe1, 0x90);
    pr2020_write_register(0xe2, 0x38);
    pr2020_write_register(0xe3, 0x19);
    pr2020_write_register(0xe4, 0x19);
    pr2020_write_register(0xea, 0x01);
    pr2020_write_register(0xeb, 0xff);
    pr2020_write_register(0xf1, 0x44);
    pr2020_write_register(0xf2, 0x01);

    //Page1 vdec
    pr2020_write_register(0xff, 0x01);
    pr2020_write_register(0x00, 0xe4);
    pr2020_write_register(0x01, 0x61);
    pr2020_write_register(0x02, 0x00);
    pr2020_write_register(0x03, 0x57);
    pr2020_write_register(0x04, 0x0c);
    pr2020_write_register(0x05, 0x88);
    pr2020_write_register(0x06, 0x04);
    pr2020_write_register(0x07, 0xb2);
    pr2020_write_register(0x08, 0x44);
    pr2020_write_register(0x09, 0x34);
    pr2020_write_register(0x0a, 0x02);
    pr2020_write_register(0x0b, 0x14);
    pr2020_write_register(0x0c, 0x04);
    pr2020_write_register(0x0d, 0x08);
    pr2020_write_register(0x0e, 0x5e);
    pr2020_write_register(0x0f, 0x5e);
    pr2020_write_register(0x10, 0x26);
    pr2020_write_register(0x11, 0x00);
    pr2020_write_register(0x12, 0x45);
    pr2020_write_register(0x13, 0xfc);
    pr2020_write_register(0x14, 0x00);
    pr2020_write_register(0x15, 0x18);
    pr2020_write_register(0x16, 0xd0);
    pr2020_write_register(0x17, 0x00);
    pr2020_write_register(0x18, 0x41);
    pr2020_write_register(0x19, 0x46);
    pr2020_write_register(0x1a, 0x22);
    pr2020_write_register(0x1b, 0x05);
    pr2020_write_register(0x1c, 0xea);
    pr2020_write_register(0x1d, 0x45);
    pr2020_write_register(0x1e, 0x40);
    pr2020_write_register(0x1f, 0x00);
    pr2020_write_register(0x20, 0x80);
    pr2020_write_register(0x21, 0x80);
    pr2020_write_register(0x22, 0x90);
    pr2020_write_register(0x23, 0x80);
    pr2020_write_register(0x24, 0x80);
    pr2020_write_register(0x25, 0x80);
    pr2020_write_register(0x26, 0x84);
    pr2020_write_register(0x27, 0x82);
    pr2020_write_register(0x28, 0x00);
    pr2020_write_register(0x29, 0x7b);
    pr2020_write_register(0x2a, 0xa6);
    pr2020_write_register(0x2b, 0x00);
    pr2020_write_register(0x2c, 0x00);
    pr2020_write_register(0x2d, 0x00);
    pr2020_write_register(0x2e, 0x00);
    pr2020_write_register(0x2f, 0x00);
    pr2020_write_register(0x30, 0x00);
    pr2020_write_register(0x31, 0x00);
    pr2020_write_register(0x32, 0xc0);
    pr2020_write_register(0x33, 0x14);
    pr2020_write_register(0x34, 0x14);
    pr2020_write_register(0x35, 0x80);
    pr2020_write_register(0x36, 0x80);
    pr2020_write_register(0x37, 0xaa);
    pr2020_write_register(0x38, 0x48);
    pr2020_write_register(0x39, 0x08);
    pr2020_write_register(0x3a, 0x27);
    pr2020_write_register(0x3b, 0x02);
    pr2020_write_register(0x3c, 0x01);
    pr2020_write_register(0x3d, 0x23);
    pr2020_write_register(0x3e, 0x02);
    pr2020_write_register(0x3f, 0xc4);
    pr2020_write_register(0x40, 0x05);
    pr2020_write_register(0x41, 0x55);
    pr2020_write_register(0x42, 0x01);
    pr2020_write_register(0x43, 0x33);
    pr2020_write_register(0x44, 0x6a);
    pr2020_write_register(0x45, 0x00);
    pr2020_write_register(0x46, 0x09);
    pr2020_write_register(0x47, 0xdc);
    pr2020_write_register(0x48, 0xa0);
    pr2020_write_register(0x49, 0x00);
    pr2020_write_register(0x4a, 0x7b);
    pr2020_write_register(0x4b, 0x60);
    pr2020_write_register(0x4c, 0x00);
    pr2020_write_register(0x4d, 0x4a);
    pr2020_write_register(0x4e, 0x00);
    pr2020_write_register(0x4f, 0x20);
    pr2020_write_register(0x50, 0x01);
    pr2020_write_register(0x51, 0x28);
    pr2020_write_register(0x52, 0x40);
    pr2020_write_register(0x53, 0x0c);
    pr2020_write_register(0x54, 0x0f);
    pr2020_write_register(0x55, 0x8d);
    pr2020_write_register(0x70, 0x06);
    pr2020_write_register(0x71, 0x08);
    pr2020_write_register(0x72, 0x0a);
    pr2020_write_register(0x73, 0x0c);
    pr2020_write_register(0x74, 0x0e);
    pr2020_write_register(0x75, 0x10);
    pr2020_write_register(0x76, 0x12);
    pr2020_write_register(0x77, 0x14);
    pr2020_write_register(0x78, 0x06);
    pr2020_write_register(0x79, 0x08);
    pr2020_write_register(0x7a, 0x0a);
    pr2020_write_register(0x7b, 0x0c);
    pr2020_write_register(0x7c, 0x0e);
    pr2020_write_register(0x7d, 0x10);
    pr2020_write_register(0x7e, 0x12);
    pr2020_write_register(0x7f, 0x14);
    pr2020_write_register(0x80, 0x00);
    pr2020_write_register(0x81, 0x09);
    pr2020_write_register(0x82, 0x00);
    pr2020_write_register(0x83, 0x07);
    pr2020_write_register(0x84, 0x00);
    pr2020_write_register(0x85, 0x17);
    pr2020_write_register(0x86, 0x03);
    pr2020_write_register(0x87, 0xe5);
    pr2020_write_register(0x88, 0x08);
    pr2020_write_register(0x89, 0x91);
    pr2020_write_register(0x8a, 0x08);
    pr2020_write_register(0x8b, 0x91);
    pr2020_write_register(0x8c, 0x0b);
    pr2020_write_register(0x8d, 0xe0);
    pr2020_write_register(0x8e, 0x05);
    pr2020_write_register(0x8f, 0x47);
    pr2020_write_register(0x90, 0x05);
    pr2020_write_register(0x91, 0xa0);
    pr2020_write_register(0x92, 0x73);
    pr2020_write_register(0x93, 0xe8);
    pr2020_write_register(0x94, 0x0f);
    pr2020_write_register(0x95, 0x5e);
    pr2020_write_register(0x96, 0x07);
    pr2020_write_register(0x97, 0x90);
    pr2020_write_register(0x98, 0x17);
    pr2020_write_register(0x99, 0x34);
    pr2020_write_register(0x9a, 0x13);
    pr2020_write_register(0x9b, 0x56);
    pr2020_write_register(0x9c, 0x0b);
    pr2020_write_register(0x9d, 0x9a);
    pr2020_write_register(0x9e, 0x09);
    pr2020_write_register(0x9f, 0xab);
    pr2020_write_register(0xa0, 0x01);
    pr2020_write_register(0xa1, 0x74);
    pr2020_write_register(0xa2, 0x01);
    pr2020_write_register(0xa3, 0x6b);
    pr2020_write_register(0xa4, 0x00);
    pr2020_write_register(0xa5, 0xba);
    pr2020_write_register(0xa6, 0x00);
    pr2020_write_register(0xa7, 0xa3);
    pr2020_write_register(0xa8, 0x01);
    pr2020_write_register(0xa9, 0x39);
    pr2020_write_register(0xaa, 0x01);
    pr2020_write_register(0xab, 0x39);
    pr2020_write_register(0xac, 0x00);
    pr2020_write_register(0xad, 0xc1);
    pr2020_write_register(0xae, 0x00);
    pr2020_write_register(0xaf, 0xc1);
    pr2020_write_register(0xb0, 0x09);
    pr2020_write_register(0xb1, 0xaa);
    pr2020_write_register(0xb2, 0x0f);
    pr2020_write_register(0xb3, 0xae);
    pr2020_write_register(0xb4, 0x00);
    pr2020_write_register(0xb5, 0x17);
    pr2020_write_register(0xb6, 0x08);
    pr2020_write_register(0xb7, 0xe8);
    pr2020_write_register(0xb8, 0xb0);
    pr2020_write_register(0xb9, 0xce);
    pr2020_write_register(0xba, 0x90);
    pr2020_write_register(0xbb, 0x00);
    pr2020_write_register(0xbc, 0x00);
    pr2020_write_register(0xbd, 0x04);
    pr2020_write_register(0xbe, 0x05);
    pr2020_write_register(0xbf, 0x00);
    pr2020_write_register(0xc0, 0x00);
    pr2020_write_register(0xc1, 0x18);
    pr2020_write_register(0xc2, 0x02);
    pr2020_write_register(0xc3, 0xd0);

    pr2020_write_register(0xff, 0x01);
    pr2020_write_register(0x54, 0x0e);
    pr2020_write_register(0xff, 0x01);
    pr2020_write_register(0x54, 0x0f);
}

static void HAL_AHD_PR2020_SetRes_1080P25(void)
{
    printf("HAL_AHD_PR2020_SetRes_1080P25\n");
    //Page0 sys
    pr2020_write_register(0xff, 0x00);
    pr2020_write_register(0x10, 0x83);
    pr2020_write_register(0x11, 0x07);
    pr2020_write_register(0x12, 0x00);
    pr2020_write_register(0x13, 0x00);
    pr2020_write_register(0x14, 0x21);//b[1:0} => Select Camera Input. VinP(1), VinN(3), Differ(0).
    pr2020_write_register(0x15, 0x44);
    pr2020_write_register(0x16, 0x0d);
    pr2020_write_register(0x40, 0x00);
    pr2020_write_register(0x47, 0x3a);
    pr2020_write_register(0x4e, 0x3f);
    pr2020_write_register(0x80, 0x56);
    pr2020_write_register(0x81, 0x0e);
    pr2020_write_register(0x82, 0x0d);
    pr2020_write_register(0x84, 0x30);
    pr2020_write_register(0x86, 0x20);
    pr2020_write_register(0x87, 0x00);
    pr2020_write_register(0x8a, 0x00);
    pr2020_write_register(0x90, 0x00);
    pr2020_write_register(0x91, 0x00);
    pr2020_write_register(0x92, 0x00);
    pr2020_write_register(0x94, 0xff);
    pr2020_write_register(0x95, 0xff);
    pr2020_write_register(0x96, 0xff);
    pr2020_write_register(0xa0, 0x00);
    pr2020_write_register(0xa1, 0x20);
    pr2020_write_register(0xa4, 0x01);
    pr2020_write_register(0xa5, 0xe3);
    pr2020_write_register(0xa6, 0x00);
    pr2020_write_register(0xa7, 0x12);
    pr2020_write_register(0xa8, 0x00);
    pr2020_write_register(0xd0, 0x30);
    pr2020_write_register(0xd1, 0x08);
    pr2020_write_register(0xd2, 0x21);
    pr2020_write_register(0xd3, 0x00);
    pr2020_write_register(0xd8, 0x30);
    pr2020_write_register(0xd9, 0x08);
    pr2020_write_register(0xda, 0x21);
    pr2020_write_register(0xe0, 0x35);
    pr2020_write_register(0xe1, 0x80);//0xc0:ycbcr  0x80:cbycry
    pr2020_write_register(0xe2, 0x18);
    pr2020_write_register(0xe3, 0x00);
    pr2020_write_register(0xe4, 0x00);
    pr2020_write_register(0xea, 0x01);
    pr2020_write_register(0xeb, 0xff);
    pr2020_write_register(0xf1, 0x44);
    pr2020_write_register(0xf2, 0x01);

    //Page1 vdec
    pr2020_write_register(0xff, 0x01);
    pr2020_write_register(0x00, 0xe4);//no-video data, 0xe4: black, 0xe5: blue
    pr2020_write_register(0x01, 0x61);
    pr2020_write_register(0x02, 0x00);
    pr2020_write_register(0x03, 0x57);
    pr2020_write_register(0x04, 0x0c);
    pr2020_write_register(0x05, 0x88);
    pr2020_write_register(0x06, 0x04);
    pr2020_write_register(0x07, 0xb2);
    pr2020_write_register(0x08, 0x44);
    pr2020_write_register(0x09, 0x34);
    pr2020_write_register(0x0a, 0x02);
    pr2020_write_register(0x0b, 0x14);
    pr2020_write_register(0x0c, 0x04);
    pr2020_write_register(0x0d, 0x08);
    pr2020_write_register(0x0e, 0x5e);
    pr2020_write_register(0x0f, 0x5e);
    pr2020_write_register(0x10, 0x26);
    pr2020_write_register(0x11, 0x00);
    pr2020_write_register(0x12, 0x87);
    pr2020_write_register(0x13, 0x24);
    pr2020_write_register(0x14, 0x80);
    pr2020_write_register(0x15, 0x2a);
    pr2020_write_register(0x16, 0x38);
    pr2020_write_register(0x17, 0x00);
    pr2020_write_register(0x18, 0x80);
    pr2020_write_register(0x19, 0x48);
    pr2020_write_register(0x1a, 0x6c);
    pr2020_write_register(0x1b, 0x05);
    pr2020_write_register(0x1c, 0x61);
    pr2020_write_register(0x1d, 0x07);
    pr2020_write_register(0x1e, 0x7e);
    pr2020_write_register(0x1f, 0x80);
    pr2020_write_register(0x20, 0x80);
    pr2020_write_register(0x21, 0x80);
    pr2020_write_register(0x22, 0x90);
    pr2020_write_register(0x23, 0x80);
    pr2020_write_register(0x24, 0x80);
    pr2020_write_register(0x25, 0x80);
    pr2020_write_register(0x26, 0x84);
    pr2020_write_register(0x27, 0x82);
    pr2020_write_register(0x28, 0x00);
    pr2020_write_register(0x29, 0xff);
    pr2020_write_register(0x2a, 0xff);
    pr2020_write_register(0x2b, 0x00);
    pr2020_write_register(0x2c, 0x00);
    pr2020_write_register(0x2d, 0x00);
    pr2020_write_register(0x2e, 0x00);
    pr2020_write_register(0x2f, 0x00);
    pr2020_write_register(0x30, 0x00);
    pr2020_write_register(0x31, 0x00);
    pr2020_write_register(0x32, 0xc0);
    pr2020_write_register(0x33, 0x14);
    pr2020_write_register(0x34, 0x14);
    pr2020_write_register(0x35, 0x80);
    pr2020_write_register(0x36, 0x80);
    pr2020_write_register(0x37, 0xad);
    pr2020_write_register(0x38, 0x4b);
    pr2020_write_register(0x39, 0x08);
    pr2020_write_register(0x3a, 0x21);
    pr2020_write_register(0x3b, 0x02);
    pr2020_write_register(0x3c, 0x01);
    pr2020_write_register(0x3d, 0x23);
    pr2020_write_register(0x3e, 0x05);
    pr2020_write_register(0x3f, 0xc8);
    pr2020_write_register(0x40, 0x05);
    pr2020_write_register(0x41, 0x55);
    pr2020_write_register(0x42, 0x01);
    pr2020_write_register(0x43, 0x38);
    pr2020_write_register(0x44, 0x6a);
    pr2020_write_register(0x45, 0x00);
    pr2020_write_register(0x46, 0x14);
    pr2020_write_register(0x47, 0xb0);
    pr2020_write_register(0x48, 0xdf);
    pr2020_write_register(0x49, 0x00);
    pr2020_write_register(0x4a, 0x7b);
    pr2020_write_register(0x4b, 0x60);
    pr2020_write_register(0x4c, 0x00);
    pr2020_write_register(0x4d, 0x26);
    pr2020_write_register(0x4e, 0x00);
    pr2020_write_register(0x4f, 0x20);//RK:0x24
    pr2020_write_register(0x50, 0x01);
    pr2020_write_register(0x51, 0x28);
    pr2020_write_register(0x52, 0x40);
    pr2020_write_register(0x53, 0x0c);
    pr2020_write_register(0x54, 0x0f);
    pr2020_write_register(0x55, 0x8d);
    pr2020_write_register(0x70, 0x06);
    pr2020_write_register(0x71, 0x08);
    pr2020_write_register(0x72, 0x0a);
    pr2020_write_register(0x73, 0x0c);
    pr2020_write_register(0x74, 0x0e);
    pr2020_write_register(0x75, 0x10);
    pr2020_write_register(0x76, 0x12);
    pr2020_write_register(0x77, 0x14);
    pr2020_write_register(0x78, 0x06);
    pr2020_write_register(0x79, 0x08);
    pr2020_write_register(0x7a, 0x0a);
    pr2020_write_register(0x7b, 0x0c);
    pr2020_write_register(0x7c, 0x0e);
    pr2020_write_register(0x7d, 0x10);
    pr2020_write_register(0x7e, 0x12);
    pr2020_write_register(0x7f, 0x14);
    pr2020_write_register(0x80, 0x00);
    pr2020_write_register(0x81, 0x09);
    pr2020_write_register(0x82, 0x00);
    pr2020_write_register(0x83, 0x07);
    pr2020_write_register(0x84, 0x00);
    pr2020_write_register(0x85, 0x17);
    pr2020_write_register(0x86, 0x03);
    pr2020_write_register(0x87, 0xe5);
    pr2020_write_register(0x88, 0x05);
    pr2020_write_register(0x89, 0x24);
    pr2020_write_register(0x8a, 0x05);
    pr2020_write_register(0x8b, 0x24);
    pr2020_write_register(0x8c, 0x08);
    pr2020_write_register(0x8d, 0xe8);
    pr2020_write_register(0x8e, 0x05);
    pr2020_write_register(0x8f, 0x47);
    pr2020_write_register(0x90, 0x02);
    pr2020_write_register(0x91, 0xb4);
    pr2020_write_register(0x92, 0x73);
    pr2020_write_register(0x93, 0xe8);
    pr2020_write_register(0x94, 0x0f);
    pr2020_write_register(0x95, 0x5e);
    pr2020_write_register(0x96, 0x03);
    pr2020_write_register(0x97, 0xd0);
    pr2020_write_register(0x98, 0x17);
    pr2020_write_register(0x99, 0x34);
    pr2020_write_register(0x9a, 0x13);
    pr2020_write_register(0x9b, 0x56);
    pr2020_write_register(0x9c, 0x0b);
    pr2020_write_register(0x9d, 0x9a);
    pr2020_write_register(0x9e, 0x09);
    pr2020_write_register(0x9f, 0xab);
    pr2020_write_register(0xa0, 0x01);
    pr2020_write_register(0xa1, 0x74);
    pr2020_write_register(0xa2, 0x01);
    pr2020_write_register(0xa3, 0x6b);
    pr2020_write_register(0xa4, 0x00);
    pr2020_write_register(0xa5, 0xba);
    pr2020_write_register(0xa6, 0x00);
    pr2020_write_register(0xa7, 0xa3);
    pr2020_write_register(0xa8, 0x01);
    pr2020_write_register(0xa9, 0x39);
    pr2020_write_register(0xaa, 0x01);
    pr2020_write_register(0xab, 0x39);
    pr2020_write_register(0xac, 0x00);
    pr2020_write_register(0xad, 0xc1);
    pr2020_write_register(0xae, 0x00);
    pr2020_write_register(0xaf, 0xc1);
    pr2020_write_register(0xb0, 0x05);
    pr2020_write_register(0xb1, 0xcc);
    pr2020_write_register(0xb2, 0x09);
    pr2020_write_register(0xb3, 0x6d);
    pr2020_write_register(0xb4, 0x00);
    pr2020_write_register(0xb5, 0x17);
    pr2020_write_register(0xb6, 0x08);
    pr2020_write_register(0xb7, 0xe8);
    pr2020_write_register(0xb8, 0xb0);
    pr2020_write_register(0xb9, 0xce);
    pr2020_write_register(0xba, 0x90);
    pr2020_write_register(0xbb, 0x00);
    pr2020_write_register(0xbc, 0x00);
    pr2020_write_register(0xbd, 0x04);
    pr2020_write_register(0xbe, 0x07);
    pr2020_write_register(0xbf, 0x80);
    pr2020_write_register(0xc0, 0x00);
    pr2020_write_register(0xc1, 0x20);
    pr2020_write_register(0xc2, 0x04);
    pr2020_write_register(0xc3, 0x38);

    pr2020_write_register(0xff, 0x01);
    pr2020_write_register(0x54, 0x0e);
    pr2020_write_register(0xff, 0x01);
    pr2020_write_register(0x54, 0x0f);
}

static void HAL_AHD_PR2020_SetRes_1080P30(void)
{
    printf("HAL_AHD_PR2020_SetRes_1080P30\n");
    //Page0 sys
    pr2020_write_register(0xff, 0x00);
    pr2020_write_register(0x10, 0x93);
    pr2020_write_register(0x11, 0x07);
    pr2020_write_register(0x12, 0x00);
    pr2020_write_register(0x13, 0x00);
    pr2020_write_register(0x14, 0x21);//b[1:0} => Select Camera Input. VinP(1), VinN(3), Differ(0).
    pr2020_write_register(0x15, 0x44);
    pr2020_write_register(0x16, 0x0d);
    pr2020_write_register(0x40, 0x00);
    pr2020_write_register(0x47, 0x3a);
    pr2020_write_register(0x4e, 0x3f);
    pr2020_write_register(0x80, 0x56);
    pr2020_write_register(0x81, 0x0e);
    pr2020_write_register(0x82, 0x0d);
    pr2020_write_register(0x84, 0x30);
    pr2020_write_register(0x86, 0x20);
    pr2020_write_register(0x87, 0x00);
    pr2020_write_register(0x8a, 0x00);
    pr2020_write_register(0x90, 0x00);
    pr2020_write_register(0x91, 0x00);
    pr2020_write_register(0x92, 0x00);
    pr2020_write_register(0x94, 0xff);
    pr2020_write_register(0x95, 0xff);
    pr2020_write_register(0x96, 0xff);
    pr2020_write_register(0xa0, 0x00);
    pr2020_write_register(0xa1, 0x20);
    pr2020_write_register(0xa4, 0x01);
    pr2020_write_register(0xa5, 0xe3);
    pr2020_write_register(0xa6, 0x00);
    pr2020_write_register(0xa7, 0x12);
    pr2020_write_register(0xa8, 0x00);
    pr2020_write_register(0xd0, 0x30);
    pr2020_write_register(0xd1, 0x08);
    pr2020_write_register(0xd2, 0x21);
    pr2020_write_register(0xd3, 0x00);
    pr2020_write_register(0xd8, 0x30);
    pr2020_write_register(0xd9, 0x08);
    pr2020_write_register(0xda, 0x21);
    pr2020_write_register(0xe0, 0x35);
    pr2020_write_register(0xe1, 0x80);
    pr2020_write_register(0xe2, 0x18);
    pr2020_write_register(0xe3, 0x00);
    pr2020_write_register(0xe4, 0x00);
    pr2020_write_register(0xea, 0x01);
    pr2020_write_register(0xeb, 0xff);
    pr2020_write_register(0xf1, 0x44);
    pr2020_write_register(0xf2, 0x01);

    //Page1 vdec
    pr2020_write_register(0xff, 0x01);
    pr2020_write_register(0x00, 0xe4);//no-video data, 0xe4: black, 0xe5: blue
    pr2020_write_register(0x01, 0x61);
    pr2020_write_register(0x02, 0x00);
    pr2020_write_register(0x03, 0x57);
    pr2020_write_register(0x04, 0x0c);
    pr2020_write_register(0x05, 0x88);
    pr2020_write_register(0x06, 0x04);
    pr2020_write_register(0x07, 0xb2);
    pr2020_write_register(0x08, 0x44);
    pr2020_write_register(0x09, 0x34);
    pr2020_write_register(0x0a, 0x02);
    pr2020_write_register(0x0b, 0x14);
    pr2020_write_register(0x0c, 0x04);
    pr2020_write_register(0x0d, 0x08);
    pr2020_write_register(0x0e, 0x5e);
    pr2020_write_register(0x0f, 0x5e);
    pr2020_write_register(0x10, 0x26);
    pr2020_write_register(0x11, 0x00);
    pr2020_write_register(0x12, 0x87);
    pr2020_write_register(0x13, 0x2c);
    pr2020_write_register(0x14, 0x80);
    pr2020_write_register(0x15, 0x28);
    pr2020_write_register(0x16, 0x38);
    pr2020_write_register(0x17, 0x00);
    pr2020_write_register(0x18, 0x80);
    pr2020_write_register(0x19, 0x48);
    pr2020_write_register(0x1a, 0x6c);
    pr2020_write_register(0x1b, 0x05);
    pr2020_write_register(0x1c, 0x61);
    pr2020_write_register(0x1d, 0x07);
    pr2020_write_register(0x1e, 0x7e);
    pr2020_write_register(0x1f, 0x80);
    pr2020_write_register(0x20, 0x80);
    pr2020_write_register(0x21, 0x80);
    pr2020_write_register(0x22, 0x90);
    pr2020_write_register(0x23, 0x80);
    pr2020_write_register(0x24, 0x80);
    pr2020_write_register(0x25, 0x80);
    pr2020_write_register(0x26, 0x84);
    pr2020_write_register(0x27, 0x82);
    pr2020_write_register(0x28, 0x00);
    pr2020_write_register(0x29, 0xff);
    pr2020_write_register(0x2a, 0xff);
    pr2020_write_register(0x2b, 0x00);
    pr2020_write_register(0x2c, 0x00);
    pr2020_write_register(0x2d, 0x00);
    pr2020_write_register(0x2e, 0x00);
    pr2020_write_register(0x2f, 0x00);
    pr2020_write_register(0x30, 0x00);
    pr2020_write_register(0x31, 0x00);
    pr2020_write_register(0x32, 0xc0);
    pr2020_write_register(0x33, 0x14);
    pr2020_write_register(0x34, 0x14);
    pr2020_write_register(0x35, 0x80);
    pr2020_write_register(0x36, 0x80);
    pr2020_write_register(0x37, 0xad);
    pr2020_write_register(0x38, 0x4b);
    pr2020_write_register(0x39, 0x08);
    pr2020_write_register(0x3a, 0x21);
    pr2020_write_register(0x3b, 0x02);
    pr2020_write_register(0x3c, 0x01);
    pr2020_write_register(0x3d, 0x23);
    pr2020_write_register(0x3e, 0x05);
    pr2020_write_register(0x3f, 0xc8);
    pr2020_write_register(0x40, 0x05);
    pr2020_write_register(0x41, 0x55);
    pr2020_write_register(0x42, 0x01);
    pr2020_write_register(0x43, 0x38);
    pr2020_write_register(0x44, 0x6a);
    pr2020_write_register(0x45, 0x00);
    pr2020_write_register(0x46, 0x14);
    pr2020_write_register(0x47, 0xb2);
    pr2020_write_register(0x48, 0xbc);
    pr2020_write_register(0x49, 0x00);
    pr2020_write_register(0x4a, 0x7b);
    pr2020_write_register(0x4b, 0x60);
    pr2020_write_register(0x4c, 0x00);
    pr2020_write_register(0x4d, 0x26);
    pr2020_write_register(0x4e, 0x00);
    pr2020_write_register(0x4f, 0x20);//RK:0x24
    pr2020_write_register(0x50, 0x01);
    pr2020_write_register(0x51, 0x28);
    pr2020_write_register(0x52, 0x40);
    pr2020_write_register(0x53, 0x0c);
    pr2020_write_register(0x54, 0x0f);
    pr2020_write_register(0x55, 0x8d);
    pr2020_write_register(0x70, 0x06);
    pr2020_write_register(0x71, 0x08);
    pr2020_write_register(0x72, 0x0a);
    pr2020_write_register(0x73, 0x0c);
    pr2020_write_register(0x74, 0x0e);
    pr2020_write_register(0x75, 0x10);
    pr2020_write_register(0x76, 0x12);
    pr2020_write_register(0x77, 0x14);
    pr2020_write_register(0x78, 0x06);
    pr2020_write_register(0x79, 0x08);
    pr2020_write_register(0x7a, 0x0a);
    pr2020_write_register(0x7b, 0x0c);
    pr2020_write_register(0x7c, 0x0e);
    pr2020_write_register(0x7d, 0x10);
    pr2020_write_register(0x7e, 0x12);
    pr2020_write_register(0x7f, 0x14);
    pr2020_write_register(0x80, 0x00);
    pr2020_write_register(0x81, 0x09);
    pr2020_write_register(0x82, 0x00);
    pr2020_write_register(0x83, 0x07);
    pr2020_write_register(0x84, 0x00);
    pr2020_write_register(0x85, 0x17);
    pr2020_write_register(0x86, 0x03);
    pr2020_write_register(0x87, 0xe5);
    pr2020_write_register(0x88, 0x04);
    pr2020_write_register(0x89, 0x48);
    pr2020_write_register(0x8a, 0x04);
    pr2020_write_register(0x8b, 0x48);
    pr2020_write_register(0x8c, 0x08);
    pr2020_write_register(0x8d, 0xe8);
    pr2020_write_register(0x8e, 0x05);
    pr2020_write_register(0x8f, 0x47);
    pr2020_write_register(0x90, 0x03);
    pr2020_write_register(0x91, 0x13);
    pr2020_write_register(0x92, 0x73);
    pr2020_write_register(0x93, 0xe8);
    pr2020_write_register(0x94, 0x0f);
    pr2020_write_register(0x95, 0x5e);
    pr2020_write_register(0x96, 0x03);
    pr2020_write_register(0x97, 0xd0);
    pr2020_write_register(0x98, 0x17);
    pr2020_write_register(0x99, 0x34);
    pr2020_write_register(0x9a, 0x13);
    pr2020_write_register(0x9b, 0x56);
    pr2020_write_register(0x9c, 0x0b);
    pr2020_write_register(0x9d, 0x9a);
    pr2020_write_register(0x9e, 0x09);
    pr2020_write_register(0x9f, 0xab);
    pr2020_write_register(0xa0, 0x01);
    pr2020_write_register(0xa1, 0x74);
    pr2020_write_register(0xa2, 0x01);
    pr2020_write_register(0xa3, 0x6b);
    pr2020_write_register(0xa4, 0x00);
    pr2020_write_register(0xa5, 0xba);
    pr2020_write_register(0xa6, 0x00);
    pr2020_write_register(0xa7, 0xa3);
    pr2020_write_register(0xa8, 0x01);
    pr2020_write_register(0xa9, 0x39);
    pr2020_write_register(0xaa, 0x01);
    pr2020_write_register(0xab, 0x39);
    pr2020_write_register(0xac, 0x00);
    pr2020_write_register(0xad, 0xc1);
    pr2020_write_register(0xae, 0x00);
    pr2020_write_register(0xaf, 0xc1);
    pr2020_write_register(0xb0, 0x04);
    pr2020_write_register(0xb1, 0xd4);
    pr2020_write_register(0xb2, 0x07);
    pr2020_write_register(0xb3, 0xda);
    pr2020_write_register(0xb4, 0x00);
    pr2020_write_register(0xb5, 0x17);
    pr2020_write_register(0xb6, 0x08);
    pr2020_write_register(0xb7, 0xe8);
    pr2020_write_register(0xb8, 0xb0);
    pr2020_write_register(0xb9, 0xce);
    pr2020_write_register(0xba, 0x90);
    pr2020_write_register(0xbb, 0x00);
    pr2020_write_register(0xbc, 0x00);
    pr2020_write_register(0xbd, 0x04);
    pr2020_write_register(0xbe, 0x07);
    pr2020_write_register(0xbf, 0x80);
    pr2020_write_register(0xc0, 0x00);
    pr2020_write_register(0xc1, 0x20);
    pr2020_write_register(0xc2, 0x04);
    pr2020_write_register(0xc3, 0x38);

    pr2020_write_register(0xff, 0x01);
    pr2020_write_register(0x54, 0x0e);
    pr2020_write_register(0xff, 0x01);
    pr2020_write_register(0x54, 0x0f);
}

static void HAL_AHD_PR2020_DetectStatus(CVI_HAL_AHD_ATTR_S *pstPR2020Ctx)
{
    int lockstatus = 0;
    int detvideo = 0;

    pr2020_write_register(0xFF, 0x00);
    lockstatus = pr2020_read_register(0x01);
    detvideo = pr2020_read_register(0x00);
    // printf("lockstatus=0x%x, detvideo=0x%x\n", lockstatus, detvideo);
    if (((lockstatus & 0x18) == 0x18) && ((detvideo & 0x08) == 0x08)) { //camera plug in
        pstPR2020Ctx->enStatus = CVI_HAL_AHD_STATUS_CONNECT;
        pstPR2020Ctx->enRes = CVI_HAL_AHD_RES_BUIT;
        if ((detvideo & 0x03) == 0x00) { //NTSC
        } else if ((detvideo & 0x03) == 0x01) { //PAL
        } else if ((detvideo & 0x03) == 0x02) { //720p
            if ((detvideo & 0x30) == 0x00) { //25fps
                pstPR2020Ctx->enRes = CVI_HAL_AHD_RES_720P_25;
            } else if ((detvideo & 0x30) == 0x10) { //30fps
                pstPR2020Ctx->enRes = CVI_HAL_AHD_RES_720P_30;
            }
        } else if ((detvideo & 0x03) == 0x03) { //1080p
            if ((detvideo & 0x30) == 0x00) { //25fps
                pstPR2020Ctx->enRes = CVI_HAL_AHD_RES_1080P_25;
            } else if ((detvideo & 0x30) == 0x10) { //30fps
                pstPR2020Ctx->enRes = CVI_HAL_AHD_RES_1080P_30;
            }
        }
    } else { //camera plug out
        pstPR2020Ctx->enStatus = CVI_HAL_AHD_STATUS_DISCONNECT;
        pstPR2020Ctx->enRes = CVI_HAL_AHD_RES_BUIT;
    }
}

static void HAL_AHD_PR2020_SetResolution(CVI_HAL_AHD_RES_E enRes)
{
    HAL_AHD_PR2020_FW_Reset();
    switch (enRes)
    {
    case CVI_HAL_AHD_RES_720P_25:
        HAL_AHD_PR2020_SetRes_720P25();
        break;
    case CVI_HAL_AHD_RES_720P_30:
        HAL_AHD_PR2020_SetRes_720P30();
        break;
    case CVI_HAL_AHD_RES_1080P_25:
        HAL_AHD_PR2020_SetRes_1080P25();
        break;
    case CVI_HAL_AHD_RES_1080P_30:
        HAL_AHD_PR2020_SetRes_1080P30();
        break;
    case CVI_HAL_AHD_RES_BUIT:
    default:     
        HAL_AHD_PR2020_FW_Stop();
        break;
    }
}

static int32_t HAL_AHD_PR2020_Init(void)
{
    bool bPreInit = false;
    CVI_HAL_AHD_ATTR_S stPR2020Ctx = {CVI_HAL_AHD_STATUS_BUIT, CVI_HAL_AHD_RES_BUIT};

    bPreInit = HAL_AHD_PR2020_Check_PreInit();

    if (bPreInit == true) {
        if (HAL_AHD_PR2020_I2c_Init() != 0) {
            CVI_LOGI("HAL_AHD_PR2020_I2c_Init Error\n");
            return -1;
        }
        if (HAL_AHD_PR2020_Check_ChipId() != true) {
            CVI_LOGI("HAL_AHD_PR2020_Check_ChipId Error\n");
            return -1;
        }
    } else {
        if (HAL_AHD_PR2020_Gpio_Init() != 0) {
            CVI_LOGI("HAL_AHD_PR2020_Gpio_Init Error\n");
            return -1;
        }
        if (HAL_AHD_PR2020_I2c_Init() != 0) {
            CVI_LOGI("HAL_AHD_PR2020_I2c_Init Error\n");
            return -1;
        }
        if (HAL_AHD_PR2020_Check_ChipId() != true) {
            CVI_LOGI("HAL_AHD_PR2020_Check_ChipId Error\n");
            return -1;
        }
        HAL_AHD_PR2020_FW_Reset();
        HAL_AHD_PR2020_FW_Init();
    }

    HAL_AHD_PR2020_DetectStatus(&stPR2020Ctx);
    printf("stPR2020Ctx(%d, %d) g_halAhdPR2020Ctx(%d, %d)\n",
           stPR2020Ctx.enStatus, stPR2020Ctx.enRes,
           g_halAhdPR2020Ctx.enStatus, g_halAhdPR2020Ctx.enRes);
    if ((stPR2020Ctx.enStatus != g_halAhdPR2020Ctx.enStatus) ||
        (stPR2020Ctx.enRes != g_halAhdPR2020Ctx.enRes)) {
        if (stPR2020Ctx.enStatus == CVI_HAL_AHD_STATUS_CONNECT) {
            HAL_AHD_PR2020_SetResolution(stPR2020Ctx.enRes);
        } else {
            HAL_AHD_PR2020_FW_Reset();
            HAL_AHD_PR2020_FW_Stop();
        }
        g_halAhdPR2020Ctx.enStatus = stPR2020Ctx.enStatus;
        g_halAhdPR2020Ctx.enRes = stPR2020Ctx.enRes;
    }

    HAL_AHD_PR2020_SetEnvStatus();

    return 0;
}

static int32_t HAL_AHD_PR2020_Deinit(void)
{
    g_halAhdPR2020Ctx.enStatus = CVI_HAL_AHD_STATUS_BUIT;
    g_halAhdPR2020Ctx.enRes = CVI_HAL_AHD_RES_BUIT;

    HAL_AHD_PR2020_FW_Reset();
    HAL_AHD_PR2020_FW_Stop();
    HAL_AHD_PR2020_I2c_Exit();

    return 0;
}

static int32_t HAL_AHD_PR2020_StartChn(uint32_t u32Chn)
{
    UNUSED(u32Chn);

    CVI_HAL_AHD_ATTR_S stPR2020Ctx = {CVI_HAL_AHD_STATUS_BUIT, CVI_HAL_AHD_RES_BUIT};

    HAL_AHD_PR2020_DetectStatus(&stPR2020Ctx);
    if ((stPR2020Ctx.enStatus == CVI_HAL_AHD_STATUS_CONNECT) &&
        (stPR2020Ctx.enRes != CVI_HAL_AHD_RES_BUIT)) {
        HAL_AHD_PR2020_SetResolution(stPR2020Ctx.enRes);
    }

    g_halAhdPR2020Ctx.enStatus = stPR2020Ctx.enStatus;
    g_halAhdPR2020Ctx.enRes = stPR2020Ctx.enRes;

    return 0;
}

static int32_t HAL_AHD_PR2020_StopChn(uint32_t u32Chn)
{
    UNUSED(u32Chn);

    HAL_AHD_PR2020_FW_Reset();
    HAL_AHD_PR2020_FW_Stop();
    g_halAhdPR2020Ctx.enStatus = CVI_HAL_AHD_STATUS_BUIT;
    g_halAhdPR2020Ctx.enRes = CVI_HAL_AHD_RES_BUIT;

    return 0;
}

static int32_t HAL_AHD_PR2020_SetAttr(const CVI_HAL_AHD_ATTR_S *pstAttr)
{
    if (pstAttr == NULL) {
        return -1;
    }

    if (g_halAhdPR2020Ctx.enStatus != pstAttr->enStatus) {
        g_halAhdPR2020Ctx.enStatus = pstAttr->enStatus;
    }
    if (g_halAhdPR2020Ctx.enRes != pstAttr->enRes) {
        HAL_AHD_PR2020_SetResolution(pstAttr->enRes);
        g_halAhdPR2020Ctx.enRes = pstAttr->enRes;
    }

    return 0;
}

static int32_t HAL_AHD_PR2020_GetAttr(CVI_HAL_AHD_ATTR_S *pstAttr)
{
    if (pstAttr == NULL) {
        return -1;
    }

    pstAttr->enStatus = g_halAhdPR2020Ctx.enStatus;
    pstAttr->enRes = g_halAhdPR2020Ctx.enRes;

    return 0;
}

static int32_t HAL_AHD_PR2020_GetStatus(CVI_HAL_AHD_STATUS_E astAhdStatus[], uint32_t u32Cnt)
{
    UNUSED(u32Cnt);

    if (astAhdStatus == NULL) {
        return -1;
    }

    CVI_HAL_AHD_ATTR_S stPR2020Ctx = {CVI_HAL_AHD_STATUS_BUIT, CVI_HAL_AHD_RES_BUIT};
    HAL_AHD_PR2020_DetectStatus(&stPR2020Ctx);

    if (g_halAhdPR2020Ctx.enStatus != stPR2020Ctx.enStatus) {
        printf("hal_ahd_filter_cnt=%d\n", hal_ahd_filter_cnt);
        if (hal_ahd_filter_cnt-- == 0) {
            g_halAhdPR2020Ctx.enStatus = stPR2020Ctx.enStatus;
            g_halAhdPR2020Ctx.enRes = stPR2020Ctx.enRes;
            hal_ahd_filter_cnt = AHD_FILTER_CNT_MAX;
            if (g_halAhdPR2020Ctx.enStatus == CVI_HAL_AHD_STATUS_DISCONNECT) {
                HAL_AHD_PR2020_FW_Reset();
            } else {
               HAL_AHD_PR2020_SetResolution(g_halAhdPR2020Ctx.enRes);
            }
            HAL_AHD_PR2020_SetEnvStatus();
        }
    } else {
        hal_ahd_filter_cnt = AHD_FILTER_CNT_MAX;
    }

    astAhdStatus[0] = g_halAhdPR2020Ctx.enStatus;

    return 0;
}

CVI_HAL_AHD_OBJ_S g_halAhdPr2020Obj =
{
    .id = 0,
    .name = "HalAhdPr2020",
    .u32ChnMax = 1,
    .bUsedChn = {true, false},
    .pfnInit = HAL_AHD_PR2020_Init,
    .pfnDeinit = HAL_AHD_PR2020_Deinit,
    .pfnStartChn = HAL_AHD_PR2020_StartChn,
    .pfnStopChn = HAL_AHD_PR2020_StopChn,
    .pfnSetAttr = HAL_AHD_PR2020_SetAttr,
    .pfnGetAttr = HAL_AHD_PR2020_GetAttr,
    .pfnGetStatus = HAL_AHD_PR2020_GetStatus,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

