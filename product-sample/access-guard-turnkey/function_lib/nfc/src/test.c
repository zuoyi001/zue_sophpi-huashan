/*************************************************************/
//2014.07.10修改版
/*************************************************************/
#include <string.h>
#include <unistd.h>

#include "msg.h"
#include "spi.h"
#include "fm175xx.h"
#include "type_a.h"
#include "mifare_app.h"
void nfc_test()
{
	int ret = SPI_Init();
	printf("ret=%d \n",ret);
	SEND_MSG("FM17520,FM17522,FM17550 Mifare Card Reader\r\n");
	SEND_MSG("Version 3.0 2015.4.18\r\n");
	while (1) {
		FM175XX_HardReset();
		SEND_MSG("-> Reset OK\r\n");
		Set_Rf(3);   //选择TX1，TX2输出
		Pcd_ConfigISOType(0);//选择TYPE A模式
		while (1) {
			if (TypeA_CardActivate(PICC_ATQA, PICC_UID, PICC_SAK) == OK) {
				SEND_MSG("---------------------\r\n");
				SEND_MSG("-> Card Activate OK\r\n");
				SEND_MSG("<- ATQA = ");
				SEND_HEX(PICC_ATQA, 2);
				SEND_MSG("\r\n");
				SEND_MSG("<- UID = ");
				SEND_HEX(PICC_UID, 4);
				SEND_MSG("\r\n");
				SEND_MSG("<- SAK = ");
				SEND_HEX(PICC_SAK, 1);
				SEND_MSG("\r\n");

				MIFARE_APP();

				if (TypeA_Halt() == OK)
					SEND_MSG("-> HALT OK\r\n");
				else
					SEND_MSG("-> HALT ERROR\r\n");

			}
		}
	}
}
