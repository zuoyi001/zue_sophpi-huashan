#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/io.h>


/*at24c256, 256Kbit, 0x8000 Bytes*/
//char eeprom_rbuf[0x8000]
 
int main(int argc,char**argv)
{   
	int ret, fd,len,i;
	int write_data;
	int read_data;
	
	srand((int)time(NULL)); 

	
	fd=open("/sys/devices/platform/4020000.i2c/i2c-2/2-0050/eeprom",O_RDWR);
	
	if(fd<0)
	{
		printf("open device fail");
		return -1;
	} 

	len = 4;

	for(i=0; i<0x8000; i+=len)
	{
		lseek(fd,0+i,SEEK_SET);
		write_data = rand() ;
		printf("write data %x: [%x]\n",i, write_data);  
		if(ret=write(fd, &write_data, len)<0)  
		{
			printf("write  error\n");  
			return -1;    
		}
		//usleep(10000);
	}
	printf("write ok!\n");   
	 

	for(i=0; i<0x8000; i+=len)
	{
		lseek(fd,0+i,SEEK_SET);
		if(ret=read(fd, &read_data,len)<0)
		{
			printf("read error\n");
			return -1;
		}    
		printf("read data %x: [%x]\n",i, read_data);  
		//usleep(10000);
	}
	printf("read ok\n");



	close(fd);
	return 0;
  
}
