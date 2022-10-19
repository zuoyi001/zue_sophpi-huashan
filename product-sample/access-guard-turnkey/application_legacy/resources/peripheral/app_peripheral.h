#ifndef _PERI_H_
#define _PERI_H_

#include "stdio.h"
#include "cvi_hal_gpio.h"
#include "nfc.h"
#include "cvi_hal_uart.h"
#include "cvi_hal_wiegand.h"
#include "cvi_hal_watchdog.h"
#include "cvi_hal_pwm.h"
#include "app_utils.h"

#if 0
//IR LED gpio
#define IRLED_GPIO CVI_HAL_GPIOB_03
//relay
#define RELAY_GPIO CVI_HAL_GPIOB_16
#define DOORKEY_GPIO CVI_HAL_GPIOD_08
#define DOORMAG_GPIO CVI_HAL_GPIOD_06
#define RM_GPIO CVI_HAL_GPIOB_20
#define RADAR_GPIO CVI_HAL_GPIOA_17
#define ALARM_GPIO CVI_HAL_GPIOA_16
//audio mute
#define SPK_MUTE CVI_HAL_GPIOD_02
//rs485 gpio
#define RS485_GPIO CVI_HAL_GPIOA_19

#else
#define ALARM_GPIO CVI_HAL_GPIOA_16
//audio mute
#define SPK_MUTE        CVI_HAL_GPIOA_19
//IR LED gpio
#define IRLED_GPIO      CVI_HAL_GPIOB_04
//relay
#define RELAY_GPIO      CVI_HAL_GPIOC_16
//door key*mag
#define DOORKEY_GPIO    CVI_HAL_GPIOB_01
#define DOORMAG_GPIO    CVI_HAL_GPIOA_31
//button remove
#define RM_GPIO         CVI_HAL_GPIOB_24

#define SPI_RST         CVI_HAL_GPIOA_02
#define BT_RST          CVI_HAL_GPIOA_04
#define BL_EN           CVI_HAL_GPIOA_17

//RGB LED
#define LED_R           CVI_HAL_GPIOD_02
#define LED_G           CVI_HAL_GPIOC_28
#define LED_B           CVI_HAL_GPIOC_29

//LCD
#define LCD_PWR         CVI_HAL_GPIOC_09
#define LCD_RST         CVI_HAL_GPIOC_08

//rs485 gpio
#define RS485_GPIO      CVI_HAL_GPIOB_21

//not used
#define RADAR_GPIO      CVI_HAL_GPIOA_17

#endif

//camera0 power down gpio
#define CAM0_PWDN_GPIO CVI_HAL_GPIOA_06
//camera1 power down gpio
#define CAM1_PWDN_GPIO CVI_HAL_GPIOA_00

#define TURNONLEG_THRESHOLD 350
#define TURNONLEG_TIMER 10

#define PWM_LCD_PERIOD      200000
#define PWM_LCD_DUTY_BASE   2000
#define PWM_LCD_DUTY_RANGE  100

int peri_init(void);
int LED_OnOFF(bool onoff);
int IR_OnOFF(bool onoff);
int Get_CurrentLv();
int get_radar_status(void);
void set_card_no(unsigned char * pUID);
void get_card_no(unsigned char * pUID);

#endif
