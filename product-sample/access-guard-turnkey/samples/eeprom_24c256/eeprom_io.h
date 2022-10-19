/* **************************************************************************
 * File name:   eeprom_io.h
 * Function:    eeprom读写操作
 * Description: 利用系统提供的I2C总线驱动实现eeprom设备的读写方法,
 *              对外开放4个函数接口, 分别为:
 *              单字节写入函数: int eeprom_byte_write(u8 pos, u8 data)
 *              单字节读取函数: u8 eeprom_byte_read(u8 pos)
 *              多字节写入函数: int eeprom_page_write(u8 pos, u8 *data, int size)
 *              多字节读取函数: int eeprom_page_read(u8 pos, u8 *data, int size)
 * Author & Date: Joshua Chan, 2011/12/18
 * **************************************************************************/
#ifndef _EEPROM_IO_H
#define _EEPROM_IO_H

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

/* I2C总线ioctl方法所使用的结构体 */
typedef struct i2c_rdwr_ioctl_data ioctl_st;

typedef unsigned char u8;
typedef unsigned int u32;

/* eeprom的slave_address, 对于AT24C08, 容量为1024 bytes, 由于byte_address
   仅有8位, 只能表示0 ~ 255 bytes, 故此需在slave_address中分出2位来配合
   byte_address一起寻址, 而slave_address可以取值0x50, 0x51, 0x52, 0x53,
   配合byte_address刚好可以实现0 ~ 1023 bytes的寻址, 另外byte_address
   取值范围从0 ~ 255, 若大于255则按其值对256取余计算 */
enum eeprom_address {
    eeprom_addr0 = 0x50,
    eeprom_addr1 = 0x51,
    eeprom_addr2 = 0x52,
    eeprom_addr3 = 0x53,
};

#define DEV_PATH "/dev/i2c-2"  //设备文件路径
#define I2C_M_WR          0    //定义写标志
#define MAX_MSG_NR        2    //根据AT24C08手册, 最大消息数为2
#define EEPROM_BLOCK_SIZE 256  //每个block容量256 bytes
#define EEPROM_PAGE_SIZE  16   //AT24C08页大小为16字节
#define I2C_MSG_SIZE (sizeof(struct i2c_msg))

/* 自定义eeprom参数结构体 */
typedef struct eeprom_data {
    u8 slave_addr;
    u8 byte_addr;
    u8 len;
    u8 *buf;
} eeprom_st;


/* 从eeprom的pos位置处读取1个字节
   @pos: eeprom的byte_address, 取值范围为0 ~ 255
   返回值为读取的字节数据 */
extern u8 eeprom_byte_read(u8 pos);

/* 将1个字节数据写入eeprom的pos位置
   @pos: eeprom的byte_address, 取值范围为0 ~ 255
   @data: 待写入的字节数据 */
extern int eeprom_byte_write(u8 pos, u8 data);

/* 从eeprom的pos位置开始读取size长度数据
   @pos: eeprom的byte_address, 取值范围为0 ~ 255
   @data: 接收数据的缓冲区
   @size: 待读取的数据长度, 取值范围为1 ~ 16 */
extern int eeprom_page_read(u8 pos, u8 *data, int size);

/* 自eeprom的pos位置开始写入数据
   @pos: eeprom的byte_address, 取值范围为0 ~ 255
   @data: 待写入的数据
   @size: 待写入数据的长度, 取值范围为1 ~ 16 */
extern int eeprom_page_write(u8 pos, u8 *data, int size);

#endif