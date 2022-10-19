#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ctype.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <memory.h>

#include "hal_linux_i2c_userspace.h"

#define I2C_NAME  "/dev/atsh204a-enc"

#define DRV_ENC_CMD_BASE           0x04
#define DRV_ENC_CMD_WRITE          _IOW(DRV_ENC_CMD_BASE, 1, unsigned int)
#define DRV_ENC_CMD_READ           _IOR(DRV_ENC_CMD_BASE, 2, unsigned int)

unsigned char send_buf[128];
unsigned char recv_buf[128];

ATCA_STATUS hal_i2c_send(unsigned char *txdata, int txlength)
{
	int retval;
	int fd;
	fd = open(I2C_NAME, O_RDWR);
	if (fd < 0) 
	{
		printf("Open %s error!\n", I2C_NAME);
		return ATCA_COMM_FAIL;
	}
	
	send_buf[0] = txlength;
	memcpy(&send_buf[1],txdata,txlength);

	retval = ioctl(fd, DRV_ENC_CMD_WRITE, send_buf);
	if(retval != txlength)
	{
		retval = ATCA_TX_FAIL;
		close(fd);
		return retval;
	}
	else
	{
		retval = 0;
	}
	
	close(fd);
	return retval;
}

ATCA_STATUS hal_i2c_receive(unsigned char *rxdata, int rxlength)
{
	int retval;
    int fd;
	
	fd = open(I2C_NAME, O_RDWR);
	if (fd < 0) 
	{
		printf("Open %s error!\n", I2C_NAME);
		return ATCA_COMM_FAIL;
	}
	
	recv_buf[0]=rxlength;
	retval = ioctl(fd, DRV_ENC_CMD_READ, recv_buf);
	
	if(retval != 2)
	{
		retval = ATCA_RX_FAIL;
		close(fd);
		return retval;
	}
	else
	{
		retval = 0;
	}
	
	memset(rxdata,0x00,rxlength);
	
	memcpy(rxdata,recv_buf,rxlength);
	
	close(fd);
	return retval;
}

ATCA_STATUS hal_i2c_wake()
{
	unsigned char dummy_byte[2] = {0x00,0x0};
	hal_i2c_send(dummy_byte,2);
	return 0;
}

ATCA_STATUS hal_i2c_idle()
{
	unsigned char dummy_byte[2] = {0x02,0x0};
	int retval;
	retval = hal_i2c_send(dummy_byte,2);
	usleep(20*1000);
	return retval;
}
