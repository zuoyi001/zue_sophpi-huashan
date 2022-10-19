#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ctype.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <memory.h>

#include "hal_linux_i2c_userspace.h"

static int atcrc(int length, const unsigned char *data, unsigned char *crc_le)
{
	if(length < 0)
	{
		printf("line = %d file = %s\n",__LINE__,__FILE__);
		return ATCA_PARAM_FAIL;
	}
		
	if(NULL == data)
	{
		printf("line = %d file = %s\n",__LINE__,__FILE__);
		return ATCA_PARAM_FAIL;
	}

	if(NULL == crc_le)
	{
		printf("line = %d file = %s\n",__LINE__,__FILE__);
		return ATCA_PARAM_FAIL;
	}

	int counter;
	unsigned short crc_register = 0;
    unsigned short polynom = 0x8005;
    unsigned char shift_register;
    unsigned char  data_bit, crc_bit;
	
	for (counter = 0; counter < length; counter++)
	{
		for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1)
		{
			data_bit = (data[counter] & shift_register) ? 1 : 0;
			crc_bit = crc_register >> 15;
			crc_register <<= 1;
			if (data_bit != crc_bit)
			{
				crc_register ^= polynom;        
			}        
		}
	}
	
	crc_le[0] = (unsigned char)(crc_register & 0x00FF);
    crc_le[1] = (unsigned char)(crc_register >> 8);
	
	return 0;
}


static ATCA_STATUS atmel204a_idle()
{
	int ret;
	ret = hal_i2c_idle();
	return ret;
}


int atmel_wake()
{
	int ret;
	char recv_buf[32];
	unsigned char expected[4] = {0x04,0x11,0x33,0x43};
	
	memset(recv_buf,0x00,sizeof(recv_buf));

	ret = hal_i2c_wake();
	if(0 != ret)
	{
		printf("ERROR line = %d file = %s \n",__LINE__,__FILE__);
	}
	usleep(10*1000);
	ret = hal_i2c_receive(recv_buf,4);
	if(0 != ret)
	{
		printf("ERROR line = %d file = %s \n",__LINE__,__FILE__);
	}
	else
	{
		if(0 == memcmp(expected,recv_buf,4))
		{
			printf("wake up\n");
		}
		else
		{
			printf("0x%02x	0x%02x	0x%02x	0x%02x \n",recv_buf[0],recv_buf[1],recv_buf[2],recv_buf[3]);
			printf("ERROR not wake up\n");
			return -1;
		}
	}
	
	return 0;
}

int atmel204a_get_sn()
{	
	int retval;
	unsigned char buff[8] = {0x03,0x07,0x02,0x80,0,0,0,0};
	unsigned char recv_buff[32];
	memset(recv_buff,0x0,sizeof(recv_buff));
	
	atcrc(buff[1]-2, &buff[1], buff+(buff[1]-2)+1);
	retval = hal_i2c_send(buff,sizeof(buff));
	if(0 != retval)
	{
		printf("line = %d file = %s\n",__LINE__,__FILE__);
		return retval;
	}
	
	usleep(30*1000);
	
	retval = hal_i2c_receive(recv_buff,sizeof(recv_buff));
	if(0 != retval)
	{
		printf("line = %d file = %s\n",__LINE__,__FILE__);
		return retval;
	}

	int k;
	printf("read sn:\n");
	for(k=1;k<31;k++)
	{
		printf("0x%02x  ",recv_buff[k]);
		if(0 == k%4)
			printf("\n");
	}
	printf("\n");
	
	return 0;
}

int atmel204a_get_other()
{
	int retval;
	unsigned char buff[8] = {0x03,0x07,0x1B,0x00,0,0,0,0};
	unsigned char recv_buff[35];
	
	atcrc(buff[1]-2, &buff[1], buff+(buff[1]-2)+1);

	retval = hal_i2c_send(buff,sizeof(buff));
	if(0 != retval)
	{
		printf("line = %d file = %s\n",__LINE__,__FILE__);
		return retval;
	}
	
	usleep(60*1000);
	memset(recv_buff,0x00,sizeof(recv_buff));
	retval = hal_i2c_receive(recv_buff,35);
	if(0 != retval)
	{
		printf("line = %d file = %s\n",__LINE__,__FILE__);
		return retval;
	}

	int k;
	printf("read other:\n");
	for(k=1;k<33;k++)
	{
		printf("0x%02x  ",recv_buff[k]);
		if(0 == k%4)
			printf("\n");
	}
	printf("\n");
	
	return 0;
}


int main()
{
	printf("date = %s  time = %s\n",__DATE__,__TIME__);

	int ret;
	ret = atmel_wake();
	if(ret != 0)
	{
		printf("ERROR atmel_wake\n");
		return -1;
	}

	//get sn
	ret = atmel204a_get_sn();
	if(ret != 0)
	{
		printf("ERROR atmel_get_sn\n");
		return -1;
	}
	
	ret = atmel204a_idle();
	if(ATCA_SUCCESS != ret)
	{
		printf("ERROR atmel204a_idle\n");
		return ret;
	}
	ret = atmel_wake();
	if(ATCA_SUCCESS != ret)
	{
		printf("ERROR atmel_wake\n");
		return ret;
	}

	//get 
	ret = atmel204a_get_other();
	if(ATCA_SUCCESS != ret)
	{
		printf("ERROR atmel_get_other\n");
		return ret;
	}

	ret = atmel204a_idle();
	if(ATCA_SUCCESS != ret)
	{
		printf("ERROR atmel_idle\n");
		return ret;
	}
	
	printf("test done\n");
	return 0;
}
