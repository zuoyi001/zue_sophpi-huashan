#include <string.h>
#include <unistd.h>

#include "nfc.h"
#include "spi.h"
#include "msg.h"
#include "fm175xx.h"
#include "type_a.h"
#include "mifare_card.h"

int CVI_NFC_Init(void)
{
    int ret = OK;

    ret = SPI_Init();
    if (ret < 0)
    {
        SEND_MSG("%s fail\n", __FUNCTION__);
        return ret;
    }

    FM175XX_HardReset();
    Set_Rf(3);
    Pcd_ConfigISOType(0);

    return ret;
}

int CVI_NFC_Uninit(void)
{
    int ret = OK;

    Set_Rf(0);
    SPI_Uninit();

    return ret;
}

int CVI_NFC_CardActivate(unsigned char * pUID)
{
    int ret = OK;

    ret = TypeA_CardActivate(PICC_ATQA, PICC_UID, PICC_SAK);

    if (ret == ERROR)
    {
        return ret;
    }

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

    for (int i = 0; i < 4; i++)
    {
        * (pUID + i) = PICC_UID[i];
    }

    return ret;
}

int CVI_NFC_CardHalt(void)
{
    int ret = OK;

    ret = TypeA_Halt();

    return ret;
}

int CVI_NFC_Mifare_Transfer(unsigned char block)
{
    int ret = OK;

    ret = Mifare_Transfer(block);
    if (ret != OK)
    {
        SEND_MSG("%s fail\n", __FUNCTION__);
        return ERROR;
    }

    return ret;
}

int CVI_NFC_Mifare_Restore(unsigned char block)
{
    int ret = OK;

    ret = Mifare_Restore(block);
    if (ret != OK)
    {
        SEND_MSG("%s fail\n", __FUNCTION__);
        return ERROR;
    }

    return ret;
}

int CVI_NFC_Mifare_Blockset(unsigned char block, unsigned char * buff)
{
    int ret = OK;

    ret = Mifare_Blockset(block, buff);
    if (ret != OK)
    {
        SEND_MSG("%s fail\n", __FUNCTION__);
        return ERROR;
    }

    return ret;
}

int CVI_NFC_Mifare_Blockinc(unsigned char block, unsigned char * buff)
{
    int ret = OK;

    ret = Mifare_Blockinc(block, buff);
    if (ret != OK)
    {
        SEND_MSG("%s fail\n", __FUNCTION__);
        return ERROR;
    }

    return ret;
}

int CVI_NFC_Mifare_Blockdec(unsigned char block, unsigned char * buff)
{
    int ret = OK;

    ret = Mifare_Blockdec(block, buff);
    if (ret != OK)
    {
        SEND_MSG("%s fail\n", __FUNCTION__);
        return ERROR;
    }

    return ret;
}

int CVI_NFC_Mifare_Blockwrite(unsigned char block, unsigned char * buff)
{
    int ret = OK;

    ret = Mifare_Blockwrite(block, buff);
    if (ret != OK)
    {
        SEND_MSG("%s fail\n", __FUNCTION__);
        return ERROR;
    }

    return ret;
}

int CVI_NFC_Mifare_Blockread(unsigned char block, unsigned char * buff)
{
    int ret = OK;

    ret = Mifare_Blockread(block, buff);
    if (ret != OK)
    {
        SEND_MSG("%s fail\n", __FUNCTION__);
        return ERROR;
    }

    return ret;
}

int CVI_NFC_Mifare_Auth(unsigned char mode, unsigned char sector, unsigned char * mifare_key,
                                    unsigned char * card_uid)
{
    int ret = OK;

    ret = Mifare_Auth(mode, sector, mifare_key, card_uid);
    if (ret != OK)
    {
        SEND_MSG("%s fail\n", __FUNCTION__);
        return ERROR;
    }

    return ret;
}
