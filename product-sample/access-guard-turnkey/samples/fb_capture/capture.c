/* File:    capture.c
 * Author:  alex chou <namedbison@gmail.com>
 * Brief:   capture the linux-embedded device screen.
 *
 * Copyright (c) 2013  alex chou <namedbison@gmail.com>
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * History:
 * ================================================================
 * 2013-07-03 alex chou <namedbison@gmail.com> created
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/fb.h>
#include <linux/kd.h>

#include <sys/socket.h>
#include <netinet/in.h>


#pragma pack(1)
typedef struct _fb_
{
	int fd;
	unsigned size;
	unsigned short *bits;
	struct fb_fix_screeninfo fi;
	struct fb_var_screeninfo vi;
} FB;

typedef struct _bmp_header_
{
	//14B
	unsigned short bfType;
	unsigned int bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int headersize;

	//40B
	unsigned int infoSize;
	int width;
	int height;
	unsigned short biPlanes;
	unsigned short bits;
	unsigned int biCompression;
	unsigned int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;

} BMPHEADER;

#define fb_width(fb) ((fb)->vi.xres)
#define fb_height(fb) ((fb)->vi.yres)
#define fb_size(fb) (fb->fi.smem_len)
#define fb_bpp(fb) (fb->vi.bits_per_pixel)
#define fb_device	"/dev/fb0"
#define file_name	"screenshot.bmp"
#define VERSION		"zkteco capture 1.0\n"
#define HELP		"Usage: capture [-v] [-t second] [-n count]\nOptions:\n\t-t second\tThe time interval for continuous capturing,unit is second\n\t-n count\tThe number of continuous capture,default time is 1 seconds\n\t-v\t\tVersion\n"
#define ERROR		"Bad option \n\n"

#ifndef NULL
#define NULL		((void *)0)
#endif

static int sCount = 65536;
static int sTime = 1;

static int fb_open(FB *fb)
{
	fb->fd = open(fb_device, O_RDWR);
	if(fb->fd < 0)
	{
		printf("can't open %s\n", fb_device);
		return -1;
	}

	if(ioctl(fb->fd, FBIOGET_FSCREENINFO, &fb->fi) < 0)
		goto fail;
	if(ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vi) < 0)
		goto fail;

	fb->bits = mmap(0, fb_size(fb), PROT_READ | PROT_WRITE, MAP_SHARED, fb->fd, 0);
	if(fb->bits == MAP_FAILED)
		goto fail;

	printf("%s, fb->size=%d\n",__FUNCTION__, fb_size(fb));
	return 0;
fail:
	printf("%s is not a framebuffer.\n", fb_device, fb_size(fb));
	close(fb->fd);
	fb->fd = -1;

	return -1;
}

static void fb_close(FB *fb)
{
	munmap(fb->bits, fb_size(fb));
	close(fb->fd);
	fb->fd = -1;
}

static void free_buff(char *buff)
{
	if(buff != NULL)
	{
		free(buff);
		buff = NULL;
	}
}

int capturebmp(FB *fb, char *buf)
{
	int w, h, depth;
	int i = 0;
	int bytes_per_pixel = 3;
	int bytesPerLine;
	BMPHEADER bh;
	uint8_t *rgb24;

	w = fb_width(fb);
	h = fb_height(fb);
	depth = fb_bpp(fb);
	//convert pixel data
	rgb24 = (uint8_t *) malloc(w * h * 3);
	printf("ddddddddddddd width=%d, height=%d, depth=%d...\n",w, h, depth);
	if(depth == 16)
	{
		for(i = 0; i < w * h; i++)
		{
			uint16_t pixel16 = ((uint16_t *) fb->bits)[i];
			rgb24[3 * i + 0] = (255 * (pixel16 & 0x001F)) / 32;  //Blue
			rgb24[3 * i + 1] = (255 * ((pixel16 & 0x03E0) >> 5)) / 32;  //Green
			rgb24[3 * i + 2] = (255 * ((pixel16 & 0x7C00) >> 10)) / 32; //Red

		}
	}
	else if(depth == 24)
	{
		memcpy(rgb24, fb->bits, w * h * 3);
	}
	else if(depth == 32)
	{
		//skip transparency channel
		for(i = 0; i < w * h; i++)
		{
			uint32_t pixel32 = ((uint32_t *) fb->bits)[i];
			// in rgb24 color max is 2^8 per channel
			rgb24[3 * i + 2] = (pixel32 & 0x00FF0000) >> 16; //Red
			rgb24[3 * i + 1] = (pixel32 & 0x0000FF00) >> 8; //Green
			rgb24[3 * i + 0] = pixel32 & 0x000000FF; //Blue
//			rgb24[3 * i + 2] = pixel32 & 0x000000FF; //Blue
//			rgb24[3 * i + 1] = (pixel32 & 0x0000FF00) >> 8; //Green
//			rgb24[3 * i + 0] = (pixel32 & 0x00FF0000) >> 16; //Red
		}
	}
	else
	{
		//free
		printf("don't support this kind of depth %d\n", depth);
		free_buff(rgb24);
		return -1;
	};

	//save RGB 24 Bitmap
	memset((void*) &bh, 0, sizeof(BMPHEADER)); // sets everything to 0
	//bh.filesize  =   calculated size of your file (see below)
	//bh.reserved  = two zero bytes
	bh.bfType = 0x4D42;
	bh.bfReserved1 = 0;
	bh.bfReserved2 = 0;
	bh.headersize = 54L;
	bh.infoSize = 0x28L; // for 24 bit images
	bh.width = w; // width of image in pixels
	bh.height = -h; // height of image in pixels
	bh.biPlanes = 1; // for 24 bit images
	bh.bits = 8 * bytes_per_pixel; // for 24 bit images
	bh.biCompression = 0L; // no compression
	bytesPerLine = w * bytes_per_pixel; // for 24 bit images
	//round up to a dword boundary
	if(bytesPerLine & 0x0003)
	{
		bytesPerLine |= 0x0003;
		++bytesPerLine;
	}
	bh.biSizeImage = (long) bytesPerLine * bh.height;
	bh.bfSize = bh.headersize + bh.biSizeImage;

	memcpy(buf, &bh,sizeof(bh));
	memcpy(buf+sizeof(bh), rgb24, w * h * 3);
	free_buff(rgb24);

	return (sizeof(bh)+w * h * 3);
}

#ifndef TTime
/** 时间结构体*/
typedef struct tm TTime;
#endif

TTime *DecodeTime(TTime *tm, time_t time)
{
	memcpy(tm, localtime(&time), sizeof(TTime));

	return tm;
}

int Captureimage(char *buf)
{
	char FileName[32]={0};
	time_t tt=time(NULL);
	TTime nowTime;
	int len = 0;
	FB fb = { 0 };
	int i = 0;

	memset(FileName, 0, sizeof(FileName));

	DecodeTime((char*)&nowTime, tt);
	snprintf(FileName, sizeof(FileName), "%04d%02d%02d%02d%02d%02d.bmp", nowTime.tm_year+1900, nowTime.tm_mon+1, nowTime.tm_mday,
			nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);
	printf("%s, %d,FileName=%s \n",__func__,__LINE__, FileName);
	while(1)
	{
		if(fb_open(&fb) == 0)
		{
				printf("%s, %d, \n",__func__,__LINE__);
				snprintf(FileName, sizeof(FileName), "/mnt/mtdblock/pic/%d.bmp", i++);
				len = capturebmp(&fb, buf);
				printf("%s, %d,len=%d \n",__func__,__LINE__, len);
				if (len>0)
				{
					FILE *fp=fopen(FileName, "wb+");
					fwrite(buf, 1, len, fp);
					sync();
					fclose(fp);
				}
			
		}
		fb_close(&fb);
	}

	return len;
}

