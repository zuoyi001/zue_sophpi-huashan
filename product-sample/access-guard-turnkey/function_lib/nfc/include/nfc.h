#ifndef __NFC_H__
#define __NFC_H__

int CVI_NFC_Init(void);
int CVI_NFC_Uninit(void);
int CVI_NFC_CardActivate(unsigned char * pUID);
int CVI_NFC_CardHalt(void);
int CVI_NFC_Mifare_Transfer(unsigned char block);
int CVI_NFC_Mifare_Restore(unsigned char block);
int CVI_NFC_Mifare_Blockset(unsigned char block, unsigned char * buff);
int CVI_NFC_Mifare_Blockinc(unsigned char block, unsigned char * buff);
int CVI_NFC_Mifare_Blockdec(unsigned char block, unsigned char * buff);
int CVI_NFC_Mifare_Blockwrite(unsigned char block, unsigned char * buff);
int CVI_NFC_Mifare_Blockread(unsigned char block, unsigned char * buff);
int CVI_NFC_Mifare_Auth(unsigned char mode, unsigned char sector, unsigned char * mifare_key,
                                    unsigned char * card_uid);

#endif /* __NFC_H__ */
