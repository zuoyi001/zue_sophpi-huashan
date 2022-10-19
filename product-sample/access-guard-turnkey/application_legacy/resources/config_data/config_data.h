#ifndef CONFIG_DATA_H_
#define CONFIG_DATA_H_

#include "cviconfig.h"

#define USE_CONFIG_DATA

/*门禁控制 config*/
#define CONFIG_AUTH_METHOD "authMethod"
#define CONFIG_RELAY_CTRL "relayCtrl"
#define CONFIG_WIEGAND_CTRL "wiegandCtrl"
#define CONFIG_COM_INDEX "comIndex"
#define CONFIG_COM_ENABLE "comEnable"
#define CONFIG_BAUDRATE "baudRate"
#define CONFIG_DATA_BITS_NUM "dataBitsNum"
#define CONFIG_CHECK_MODE "checkMode"
#define CONFIG_STOP_BIT "stopBit"
#define CONFIG_PROTOCOL "protocol"
#define CONFIG_MAG_DETECT "magDetect"
#define CONFIG_OPEN_DELAY "openDelay"
#define CONFIG_GATE_ACTION "gateAction"
#define CONFIG_ALARMER "alarmer"
#define CONFIG_GATE_BTN_CTRL "gateButtonCtrl"
#define CONFIG_GATE_OPEN_PERIOD "gateOpenPeriod"
#define CONFIG_FR_PERIOD "frPeriod"

/*个性化与通知 config*/
#define CONFIG_VOICE_MODE "voiceMode"
#define CONFIG_DISPLAY_MODE "displayMode"
#define CONFIG_COMPANY_NAME "companyName"

#define CONFIG_ADMIN_PASSWD "admin_passwd"
#define CONFIG_FACTORY_PASSWD "factory_passwd"

void config_data_init();
#endif