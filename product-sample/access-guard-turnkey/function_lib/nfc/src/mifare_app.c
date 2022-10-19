#include <string.h>
#include "mifare_card.h"
#include "msg.h"
#include "type_a.h"
#include "fm175xx.h"

unsigned char MIFARE_APP(void)
{
	unsigned char MIFARE_KEY[6], BLOCK_BUF[16], VALUE_BUF[4], INC_BUF[4], DEC_BUF[4], result;

	memcpy(MIFARE_KEY, "\xFF\xFF\xFF\xFF\xFF\xFF", 6); //设置认证密钥数组
	
	result = Mifare_Auth(KEY_A, 0x01, MIFARE_KEY, PICC_UID); //扇区1进行KEY A 认证
	if (result != OK) {
		SEND_MSG("-> Sector [1] Auth ERROR\r\n");
		return ERROR;
	}
	SEND_MSG("-> Sector [1] Auth OK\r\n");

	memcpy(BLOCK_BUF, "\x11\x22\x33\x44\x55\x66\x77\x88\x11\x22\x33\x44\x55\x66\x77\x88",
		16); //设置待写入的16个字节数组

	result = Mifare_Blockwrite(0x04, BLOCK_BUF); //块地址0x04（扇区1，块0）写块操作
	if (result != OK) {
		SEND_MSG("-> Block [4] Write ERROR\r\n");
		return ERROR;
	}
	SEND_MSG("-> Block [4] Write OK \r\n");

	result = Mifare_Blockread(0x04, BLOCK_BUF); //块地址0x04（扇区1，块0）读块操作
	if (result != OK) {
		SEND_MSG("-> Block [4] Read ERROR\r\n");
		return ERROR;
	}
	SEND_MSG("-> Block [4] Read Data = ");
	SEND_HEX(BLOCK_BUF, 16);
	SEND_MSG("\r\n");

	memcpy(VALUE_BUF, "\x44\x44\x44\x44", 4); //数值块设置为44 44 44 44
	result = Mifare_Blockset(0x04, VALUE_BUF);
	if (result != OK) {
		SEND_MSG("-> Block [4] Set ERROR\r\n");
		return ERROR;
	}
	SEND_MSG("-> Block [4] Set OK\r\n");

	memcpy(VALUE_BUF, "\x44\x44\x44\x44", 4); //数值块设置为44 44 44 44
	result = Mifare_Blockset(0x05, VALUE_BUF);
	if (result != OK) {
		SEND_MSG("-> Block [5] Set ERROR\r\n");
		return ERROR;
	}
	SEND_MSG("-> Block [5] Set OK\r\n");

	result = Mifare_Blockread(0x04, BLOCK_BUF); //块地址0x04（扇区1，块0）读块操作
	if (result != OK) {
		SEND_MSG("-> Block [4] Read ERROR\r\n");
		return ERROR;
	}
	SEND_MSG("-> Block [4] Read Data = ");
	SEND_HEX(BLOCK_BUF, 16);
	SEND_MSG("\r\n");

	memcpy(INC_BUF, "\x01\x02\x03\x04", 4); //增值 01 02 03 04

	result = Mifare_Blockinc(0x04, INC_BUF); //块地址0x04（扇区1，块0）增值操作
	if (result != OK) {
		SEND_MSG("-> Block [4] Inc ERROR\r\n");
		return ERROR;
	}
	SEND_MSG("-> Block [4] Inc OK\r\n");

	result = Mifare_Transfer(0x04);//块地址0x04（扇区1，块0）传输操作
	if (result != OK) {
		SEND_MSG("->  Block [4] Transfer ERROR\r\n");
		return ERROR;
	}
	SEND_MSG("-> Block [4] Transfer OK\r\n");

	result = Mifare_Blockread(0x04, BLOCK_BUF); //块地址0x04（扇区1，块0）读块操作
	if (result != OK) {
		SEND_MSG("-> Block [4] Read ERROR\r\n");
		return ERROR;
	}
	SEND_MSG("-> Block [4] Read Data = ");
	SEND_HEX(BLOCK_BUF, 16);
	SEND_MSG("\r\n");

	memcpy(DEC_BUF, "\x00\x12\x34\x56", 4); //减值 00 12 34 56
	result = Mifare_Blockdec(0x04, DEC_BUF); //块地址0x04（扇区1，块0）增值操作
	if (result != OK) {
		SEND_MSG("->  Block [4] Dec ERROR\r\n");
		return ERROR;
	}
	SEND_MSG("-> Block [4] Dec OK\r\n");

	result = Mifare_Transfer(0x04);//块地址0x04（扇区1，块0）传输操作
	if (result != OK) {
		SEND_MSG("->  Block [4] Transfer ERROR\r\n");
		return ERROR;
	}
	SEND_MSG("-> Block [4] Transfer OK\r\n");

	result = Mifare_Blockread(0x04, BLOCK_BUF); //块地址0x04（扇区1，块0）读块操作
	if (result != OK) {
		SEND_MSG("-> Block [4] Read ERROR\r\n");
		return ERROR;
	}
	SEND_MSG("-> Block [4] Read Data = ");
	SEND_HEX(BLOCK_BUF, 16);
	SEND_MSG("\r\n");

	result = Mifare_Restore(0x05);//块地址0x05（扇区1，块0）恢复操作
	if (result != OK) {
		SEND_MSG("->  Block [5] Restore ERROR\r\n");
		return ERROR;
	}
	SEND_MSG("-> Block [5] Restore OK\r\n");

	result = Mifare_Transfer(0x04);//块地址0x04（扇区1，块0）传输操作
	if (result != OK) {
		SEND_MSG("->  Block [4] Transfer ERROR\r\n");
		return ERROR;
	}
	SEND_MSG("-> Block [4] Transfer OK\r\n");

	result = Mifare_Blockread(0x04, BLOCK_BUF); //块地址0x04（扇区1，块0）读块操作
	if (result != OK) {
		SEND_MSG("-> Block [4] Read ERROR\r\n");
		return ERROR;
	}
	SEND_MSG("-> Block [4] Read Data = ");
	SEND_HEX(BLOCK_BUF, 16);
	SEND_MSG("\r\n");

	memcpy(MIFARE_KEY, "\xFF\xFF\xFF\xFF\xFF\xFF", 6); //设置认证密钥数组
	result = Mifare_Auth(KEY_A, 0x02, MIFARE_KEY, PICC_UID); //扇区2进行KEY A 认证
	if (result != OK) {
		SEND_MSG("-> Sector [2] Auth ERROR\r\n");
		return ERROR;
	}
	SEND_MSG("-> Sector [2] Auth OK\r\n");

	return OK;
}
