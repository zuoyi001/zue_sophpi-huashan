
ifeq ($(CONFIG_SENSOR_GCORE_GC1054), y)
    KBUILD_DEFINES += -DSNS0_GCORE_GC1054
endif

ifeq ($(CONFIG_SENSOR_GCORE_GC2053), y)
    KBUILD_DEFINES += -DSNS0_GCORE_GC2053
endif

ifeq ($(CONFIG_SENSOR_OV_OV5647), y)
    KBUILD_DEFINES += -DSNS0_OV_OV5647
endif

ifeq ($(CONFIG_SENSOR_GCORE_GC2053_SLAVE), y)
    KBUILD_DEFINES += -DSNS1_GCORE_GC2053_SLAVE
endif

ifeq ($(CONFIG_SENSOR_GCORE_GC2093), y)
    KBUILD_DEFINES += -DSNS0_GCORE_GC2093
    KBUILD_DEFINES += -DSNS1_GCORE_GC2093
endif

ifeq ($(CONFIG_SENSOR_GCORE_GC2093_SLAVE), y)
    KBUILD_DEFINES += -DSNS1_GCORE_GC2093_SLAVE
endif

ifeq ($(CONFIG_SENSOR_GCORE_GC4023), y)
    KBUILD_DEFINES += -DSNS0_GCORE_GC4023
endif

ifeq ($(CONFIG_SENSOR_GCORE_GC4653), y)
    KBUILD_DEFINES += -DSNS0_GCORE_GC4653
endif

ifeq ($(CONFIG_SENSOR_GCORE_GC4653_SLAVE), y)
    KBUILD_DEFINES += -DSNS1_GCORE_GC4653_SLAVE
endif

ifeq ($(CONFIG_SENSOR_NEXTCHIP_N5), y)
    KBUILD_DEFINES += -DSNS0_NEXTCHIP_N5
endif

ifeq ($(CONFIG_SENSOR_NEXTCHIP_N6), y)
    KBUILD_DEFINES += -DSNS0_NEXTCHIP_N6
endif

ifeq ($(CONFIG_SENSOR_OV_OS02H10), y)
    KBUILD_DEFINES += -DSNS0_OV_OS02H10
endif

ifeq ($(CONFIG_SENSOR_OV_OS03B10), y)
    KBUILD_DEFINES += -DSNS0_OV_OS03B10
endif

ifeq ($(CONFIG_SENSOR_OV_OS04A10), y)
    KBUILD_DEFINES += -DSNS0_OV_OS04A10
endif

ifeq ($(CONFIG_SENSOR_OV_OS04C10), y)
    KBUILD_DEFINES += -DSNS0_OV_OS04C10
endif

ifeq ($(CONFIG_SENSOR_OV_OV4689), y)
    KBUILD_DEFINES += -DSNS0_OV_OV4689
endif

ifeq ($(CONFIG_SENSOR_OV_OS08A20), y)
    KBUILD_DEFINES += -DSNS0_OV_OS08A20
endif

ifeq ($(CONFIG_SENSOR_OV_OS08A20_SLAVE), y)
    KBUILD_DEFINES += -DSNS1_OV_OS08A20_SLAVE
endif

ifeq ($(CONFIG_SENSOR_PICO_384), y)
    KBUILD_DEFINES += -DSNS0_PICO_384
endif

ifeq ($(CONFIG_SENSOR_PICO_640), y)
    KBUILD_DEFINES += -DSNS0_PICO_640
endif

ifeq ($(CONFIG_SENSOR_PIXELPLUS_PR2020), y)
    KBUILD_DEFINES += -DSNS0_PIXELPLUS_PR2020
endif

ifeq ($(CONFIG_SENSOR_PIXELPLUS_PR2100), y)
    KBUILD_DEFINES += -DSNS0_PIXELPLUS_PR2100
endif

ifeq ($(CONFIG_SENSOR_SMS_SC1346_1L), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC1346_1L
endif

ifeq ($(CONFIG_SENSOR_SMS_SC200AI), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC200AI
endif

ifeq ($(CONFIG_SENSOR_SMS_SC2335), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC2335
endif

ifeq ($(CONFIG_SENSOR_SMS_SC500AI), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC500AI
endif

ifeq ($(CONFIG_SENSOR_SMS_SC501AI_2L), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC501AI_2L
endif

ifeq ($(CONFIG_SENSOR_SMS_SC531AI_2L), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC531AI_2L
endif

ifeq ($(CONFIG_SENSOR_SMS_SC3332), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC3332
endif

ifeq ($(CONFIG_SENSOR_SMS_SC3335), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC3335
endif

ifeq ($(CONFIG_SENSOR_SMS_SC3335_SLAVE), y)
    KBUILD_DEFINES += -DSNS1_SMS_SC3335_SLAVE
endif

ifeq ($(CONFIG_SENSOR_SMS_SC3336), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC3336
endif

ifeq ($(CONFIG_SENSOR_SMS_SC2335), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC2335
endif

ifeq ($(CONFIG_SENSOR_SMS_SC2336), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC2336
endif

ifeq ($(CONFIG_SENSOR_SMS_SC2336_1L), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC2336_1L
endif
ifeq ($(CONFIG_SENSOR_SMS_SC4336), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC4336
endif

ifeq ($(CONFIG_SENSOR_SMS_SC8238), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC8238
endif

ifeq ($(CONFIG_SENSOR_SMS_SC4210), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC4210
endif

ifeq ($(CONFIG_SENSOR_SMS_SC401AI), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC401AI
endif

ifeq ($(CONFIG_SENSOR_SOI_F23), y)
    KBUILD_DEFINES += -DSNS0_SOI_F23
endif

ifeq ($(CONFIG_SENSOR_SOI_F35), y)
    KBUILD_DEFINES += -DSNS0_SOI_F35
endif

ifeq ($(CONFIG_SENSOR_SOI_F35_SLAVE), y)
    KBUILD_DEFINES += -DSNS1_SOI_F35_SLAVE
endif

ifeq ($(CONFIG_SENSOR_SOI_F38P), y)
    KBUILD_DEFINES += -DSNS0_SOI_F38P
endif

ifeq ($(CONFIG_SENSOR_SOI_H65), y)
    KBUILD_DEFINES += -DSNS0_SOI_H65
endif

ifeq ($(CONFIG_SENSOR_SOI_K06), y)
    KBUILD_DEFINES += -DSNS0_SOI_K06
endif

ifeq ($(CONFIG_SENSOR_SOI_K04), y)
    KBUILD_DEFINES += -DSNS0_SOI_K04
endif

ifeq ($(CONFIG_SENSOR_SONY_IMX290_2L), y)
    KBUILD_DEFINES += -DSNS0_SONY_IMX290_2L
endif

ifeq ($(CONFIG_SENSOR_SONY_IMX307), y)
    KBUILD_DEFINES += -DSNS0_SONY_IMX307
endif

ifeq ($(CONFIG_SENSOR_SONY_IMX307_2L), y)
    KBUILD_DEFINES += -DSNS0_SONY_IMX307_2L
endif

ifeq ($(CONFIG_SENSOR_SONY_IMX307_SLAVE), y)
    KBUILD_DEFINES += -DSNS1_SONY_IMX307_SLAVE
endif

ifeq ($(CONFIG_SENSOR_SONY_IMX307_SUBLVDS), y)
    KBUILD_DEFINES += -DSNS0_SONY_IMX307_SUBLVDS
endif

ifeq ($(CONFIG_SENSOR_SONY_IMX327), y)
    KBUILD_DEFINES += -DSNS0_SONY_IMX327
endif

ifeq ($(CONFIG_SENSOR_SONY_IMX327_2L), y)
    KBUILD_DEFINES += -DSNS0_SONY_IMX327_2L
endif

ifeq ($(CONFIG_SENSOR_SONY_IMX327_SLAVE), y)
    KBUILD_DEFINES += -DSNS1_SONY_IMX327_SLAVE
endif

ifeq ($(CONFIG_SENSOR_SONY_IMX327_SUBLVDS), y)
    KBUILD_DEFINES += -DSNS0_SONY_IMX327_SUBLVDS
endif

ifeq ($(CONFIG_SENSOR_SONY_IMX327_10BIT), y)
    KBUILD_DEFINES += -DSNS0_SONY_IMX327_10BIT
endif

ifeq ($(CONFIG_SENSOR_SONY_IMX334), y)
    KBUILD_DEFINES += -DSNS0_SONY_IMX334
endif

ifeq ($(CONFIG_SENSOR_SONY_IMX335), y)
    KBUILD_DEFINES += -DSNS0_SONY_IMX335
endif

ifeq ($(CONFIG_SENSOR_SONY_IMX385), y)
    KBUILD_DEFINES += -DSNS0_SONY_IMX385
endif

ifeq ($(CONFIG_SENSOR_VIVO_MCS369Q), y)
    KBUILD_DEFINES += -DSNS0_VIVO_MCS369Q
endif

ifeq ($(CONFIG_SENSOR_VIVO_MCS369), y)
    KBUILD_DEFINES += -DSNS0_VIVO_MCS369
endif

ifeq ($(CONFIG_SENSOR_VIVO_MM308M2), y)
    KBUILD_DEFINES += -DSNS0_VIVO_MM308M2
endif

ifeq ($(CONFIG_SENSOR_SONY_IMX327_FPGA), y)
    KBUILD_DEFINES += -DSNS0_SONY_IMX327_FPGA
endif

ifeq ($(CONFIG_SENSOR_SONY_IMX347), y)
    KBUILD_DEFINES += -DSNS0_SONY_IMX347
endif

ifeq ($(CONFIG_SENSOR_GCORE_GC2053_1L), y)
    KBUILD_DEFINES += -DSNS0_GCORE_GC2053_1L
endif

ifeq ($(CONFIG_SENSOR_SMS_SC035HGS), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC035HGS
endif

ifeq ($(CONFIG_SENSOR_SMS_SC035GS), y)
    KBUILD_DEFINES += -DSNS0_SMS_SC035GS
endif
