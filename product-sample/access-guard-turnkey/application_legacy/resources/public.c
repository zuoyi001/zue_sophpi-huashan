#include<sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <inttypes.h>

#include <sys/mount.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "cviconfig.h"
#include "public.h"
#include "core.h"
#include "md5.h"

char snBuf[64] = {0};
char macBuf[64] = {0};
#define  SNLEN (20)

cvi_device_config_t gDevice_Cfg;


/* �Ƿ����U�� */
int gbUsbExit = 0;  //0-������  1-���� 
int giUpdateResult = 0;
int getUpdateResult()
{
	return giUpdateResult;
}
int setUpdateResult(int iUpdateResult)
{
	return giUpdateResult = iUpdateResult;
}
#if 0

#endif

///////////////////////////////////MD5 BEGIN///////////////////////////////////////////////
/* Constants for MD5Transform routine.
 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

void MD5Transform (UINT4 [4], unsigned char [64]);
void Encode (unsigned char *, UINT4 *, unsigned int);
void Decode (UINT4 *, unsigned char *, unsigned int);
void MD5_memcpy (POINTER, POINTER, unsigned int);
void MD5_memset (POINTER, int, unsigned int);

unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 *	Note:	The following MD5 macros can be implemented as functions
 *			for code compactness, (at the expense of execution speed).
 */

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

/* MD5 initialization. Begins an MD5 operation, writing a new context.
 */
void MD5Init (MD5_CONTEXT *context)/* context */
{
  context->count[0] = context->count[1] = 0;
  /* Load magic initialization constants.
*/
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
  operation, processing another message block, and updating the
  context.
 */
void MD5Update (MD5_CONTEXT *context, unsigned char *input, unsigned int inputLen)
//MD5_CONTEXT *context;                      /* context */
//unsigned char *input;                      /* input block */
//unsigned int inputLen;                     /* length of input block */
{
  unsigned int i, index, partLen;

  /* Compute number of bytes mod 64 */
  index = (unsigned int)((context->count[0] >> 3) & 0x3F);

  /* Update number of bits */
  if ((context->count[0] += ((UINT4)inputLen << 3))
   < ((UINT4)inputLen << 3))
 context->count[1]++;
  context->count[1] += ((UINT4)inputLen >> 29);

  partLen = 64 - index;

  /* Transform as many times as possible.
*/
  if (inputLen >= partLen) {
 MD5_memcpy
   ((POINTER)&context->buffer[index], (POINTER)input, partLen);
 MD5Transform (context->state, context->buffer);

 for (i = partLen; i + 63 < inputLen; i += 64)
   MD5Transform (context->state, &input[i]);

 index = 0;
  }
  else
 i = 0;

  /* Buffer remaining input */
  MD5_memcpy
 ((POINTER)&context->buffer[index], (POINTER)&input[i],
  inputLen-i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
  the message digest and zeroizing the context.
 */
void MD5Final (unsigned char digest[16], MD5_CONTEXT *context)
//unsigned char digest[16];                         /* message digest */
//MD5_CONTEXT *context;                                       /* context */
{
  unsigned char bits[8];
  unsigned int index, padLen;

  /* Save number of bits */
  Encode (bits, context->count, 8);

  /* Pad out to 56 mod 64.
*/
  index = (unsigned int)((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  MD5Update (context, PADDING, padLen);

  /* Append length (before padding) */
  MD5Update (context, bits, 8);
  /* Store state in digest */
  Encode (digest, context->state, 16);

  /* Zeroize sensitive information.
*/
  MD5_memset ((POINTER)context, 0, sizeof (*context));
}

/* MD5 basic transformation. Transforms state based on block.
 */
void MD5Transform (UINT4 state[4], unsigned char block[64])
{
  UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

  Decode (x, block, 64);

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

 /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  /* Zeroize sensitive information.
*/
  MD5_memset ((POINTER)x, 0, sizeof (x));
}

/* Encodes input (UINT4) into output (unsigned char). Assumes len is
  a multiple of 4.
 */
void Encode (unsigned char *output, UINT4 *input, unsigned int len)
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) {
 output[j] = (unsigned char)(input[i] & 0xff);
 output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
 output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
 output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
  }
}

/* Decodes input (unsigned char) into output (UINT4). Assumes len is
  a multiple of 4.
 */
void Decode (UINT4 *output, unsigned char *input, unsigned int len)
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
 output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
   (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}

/* Note: Replace "for loop" with standard memcpy if possible.
 */

void MD5_memcpy (POINTER output, POINTER input, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		output[i] = input[i];
}

/* Note: Replace "for loop" with standard memset if possible.
 */
void MD5_memset (POINTER output, int value, unsigned int len)
{
  unsigned int i;

  for (i = 0; i < len; i++)
 ((char *)output)[i] = (char)value;
}
///////////////////////////////////MD5 END///////////////////////////////////////////////


/*************************************************************
 * �������ƣ� CheckUsbExist
 * ���ܣ� check usb�豸�Ƿ����
 * ������ pDev: ����豸�� ��ΪNULL
 * ���أ� ����Ϊ1 ������Ϊ0
 ***************************************************************/
int CheckUsbExist(char *pDev)
{
	int bRet = 0;
	struct stat st;
	char path[256] = {0};
	int i = 0;
	/*
	for(i = 0; i < 5; i ++)
	{
		sprintf(path, "/sys/block/sd%c", 'a'+i);
		if(stat(path, &st)==0)
		{
			break;
		}
	}
	*/

	if(i < 5)
	{
		sprintf(path, "/dev/sd%c", 'a'+i);
		if(stat(path, &st)==0)
		{
			if(pDev)
			{
				strcpy(pDev, path);
				//printf("CheckUsbExist -> pDev = %s, %s %d\r\n", pDev, __FUNCTION__, __LINE__);
			}			
			bRet = 1;
		}
	}
	return bRet;
}


#define SN_PART "/dev/mmcblk0p8"
#define SN_LEN 16

void get_mac_show()
{
	FILE *pFile = NULL;
	char magic[5] = { '\0' };
	char sn[SN_LEN + 1] = { '\0' };
	unsigned char mac[7] = { '\0' };
	pFile = fopen(SN_PART, "r");
	fread(magic, 4, 1, pFile);
	fread(mac, 6, 1, pFile);
	fread(sn, SN_LEN, 1, pFile);
	printf("magic:%s mac:%02x:%02x:%02x:%02x:%02x:%02x SN:%s\n", magic,
		   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], sn);

}
int set_mac(char* pmac)
{
	int iRet = -1;
	FILE *pFile = NULL;
	char tmp[3] = { 0 };
	char* p = NULL;
	unsigned char nMac[6]={0};
	printf("[strlen(pmac):%zu %s,%d]\n",strlen(pmac),__FUNCTION__,__LINE__);
	printf("[strlen(pmac):%s %s,%d]\n",pmac,__FUNCTION__,__LINE__);

	if(strlen(pmac) == 17)
	{
		p = pmac;
		for(int i = 0; i < 6; i++)
		{
			tmp[0] = *p++;
			tmp[1] = *p++;
			tmp[2] = '\0';
			nMac[i] = (unsigned char)strtoul(tmp, NULL, 16);
			p++;
		}
		
		pFile = fopen(SN_PART, "w");

		fseek(pFile, 4, SEEK_SET);
		
		fwrite(nMac, 6, 1, pFile);

		printf("write mac:%02x:%02x:%02x:%02x:%02x:%02x \n",
		   nMac[0], nMac[1], nMac[2], nMac[3], nMac[4], nMac[5]);
		
		fclose(pFile);
		iRet = 0;
	}

	return iRet;
}

int set_sn(char* psn, int size)
{
	int ret = -1;
	char tmpBuf[64] = {0};
	FILE *pFile = NULL;

	if(SNLEN >= size)
	{
		pFile = fopen(SN_PART, "w");
		printf("[SN_PART:%s %s,%d]\n",SN_PART,__FUNCTION__,__LINE__);
		if(pFile > 0)
		{
			memset(tmpBuf, 0x00, sizeof(tmpBuf));
			memcpy(tmpBuf, psn, size);
			fseek(pFile, 10, SEEK_SET);
			ret = fwrite(tmpBuf, SNLEN, 1, pFile);
			printf("[ret:%d %s,%d]\n",ret,__FUNCTION__,__LINE__);
			fclose(pFile);
			memset(snBuf, 0x00, sizeof(snBuf));
			get_sn();
		}
	}
	return ret;
}
char* get_sn()
{
	//int ret = -1;
	FILE *pFile = NULL;
	
	if(strlen(snBuf) != SNLEN)
	{
		pFile = fopen(SN_PART, "r");
		if(pFile > 0)
		{
			fseek(pFile, 10, SEEK_SET);
			memset(snBuf, 0x00, sizeof(snBuf));
			fread(snBuf, SNLEN, 1, pFile);
			fclose(pFile);
		}
	}
	printf("snlen:%zu\n",strlen(snBuf));
	return snBuf;
	//printf("magic:%s mac:%02x:%02x:%02x:%02x:%02x:%02x SN:%s\n", magic,
	//	   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], sn);

}

char* get_mac1()
{
	//int ret = -1;
	unsigned char mac[7] = { '\0' };
	FILE *pFile = NULL;
	
	if(strlen(macBuf) != SNLEN)
	{
		pFile = fopen(SN_PART, "r");
		if(pFile > 0)
		{
			fseek(pFile, 4, SEEK_SET);
			memset(macBuf, 0x00, sizeof(macBuf));
			fread(mac, 6, 1, pFile);
			fclose(pFile);
			sprintf(macBuf,"%02x:%02x:%02x:%02x:%02x:%02x",mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
	}
	return macBuf;
	//printf("magic:%s mac:%02x:%02x:%02x:%02x:%02x:%02x SN:%s\n", magic,
	//	   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], sn);

}

void GetDateFromTime(time_t t, char* buf)
{
	struct tm* pTm = NULL;

	if (t)
	{
		pTm = gmtime(&t);
	}
	
	if (pTm == NULL)
	{
		buf[0] = '\0';
	}
	else
	{
		sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d",
				pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday,
				pTm->tm_hour+8, pTm->tm_min, pTm->tm_sec);
	}
}


#if 1

/******************************************************************************
 * �������ƣ� MountDevice
 * ���ܣ� �����豸
 * ������ pDev: nType��Ϊ0ʱ usb���ص㣬NULL ʱ���ص�/mnt/usb
 * ���أ� 0 ���سɹ� ��0 ���ز��ɹ�
 ******************************************************************************/
int MountUsb(char *pDev)
{
	int ret = 0;
	int errno = 0;
	umount(STORAGE_USB_PATH);
	
	if(pDev && *pDev != '\0')
	{
		struct stat st;
		char path[256] = {0};
		
		for(int i = 0; i < 10; i ++)
		{
			if(i == 0)
			{
				strcpy(path, pDev);
			}
			else
			{
				sprintf(path, "%s%d", pDev, i);
			}

			if(stat(path, &st) == -1)
			{
				printf("%s not exist! %s %d\r\n", path, __FUNCTION__, __LINE__);
			}
			else 
			{
				ret = mount(path, STORAGE_USB_PATH, "vfat", 0, "iocharset=utf8");
                if(ret == 0) 
				{
					printf("mount %s ok! %s %d\r\n", path,__FUNCTION__, __LINE__);
					break;
				}
				else
				{
					printf("mount %s Failed! err: %s. %s %d\r\n", path, strerror(errno), __FUNCTION__, __LINE__);
				}
			}
		}
	}
	else
	{
		ret = mount("/dev/sda", STORAGE_USB_PATH, "vfat", 0, "iocharset=utf8");
	}

	if(ret==0)
	{
		printf("Mount %s OK !\r\n", STORAGE_USB_PATH);
	}
	else 
	{
		int err = errno;
		printf("mount %s Failed: %s(%d)\r\n", STORAGE_USB_PATH, strerror(err), err);		
	}
	
	return ret;
}

/**************************************************************\
** �������ƣ� CheckFile
** ���ܣ� ����ļ��Ƿ����
** ������ pcFileName: �����ļ�·��
          piFileType: �����ļ�����
          piFileLen:  �����ļ���С
** ���أ� 0�ɹ���-1ʧ�� 
\**************************************************************/
int CheckFile(char *pcFileName, int *piFileType, int *piFileLen)
{	
    int ret = -1;
    int iFileType = 2;
    int iFileLen = 0;
    struct stat st;

    if(pcFileName == NULL || *pcFileName == '\0') 
    {
        ret = -1;
    }
    else 
    {
        if(stat(pcFileName, &st) == 0) 
        {
            iFileLen = st.st_size;
        }

        if(iFileLen)
        {
            ret = 0;
            if(strstr(pcFileName, ".jpg"))
            {
                iFileType = 1;
            }
            else if(strstr(pcFileName, ".bmp")) 
            {
                iFileType = 0;
            }
        }
    }
	
    if(ret == 0)
    {
        if(piFileType)
        {
            *piFileType = iFileType;
        }
        if(piFileLen)
        {
            *piFileLen = iFileLen;
        }
    }	

    return ret;
    
}

int FoundAndLoadUsb()
{
	char dev[33] = {0};
	int i = 0;
	int iRet = -1;

	/*
		�Ƿ�֧������������0-�� 1-�� 
		ע:��Ҫ�ǻ�������ǿ���ʱһֱ����U�̣�
		�Ͳ�������������ֹ�����ϵ�һֱ����U��һֱ��������
	*/
	//static int isUpDataSoftWare = 0;
	
	if(CheckUsbExist(dev) == 1)
	{
		if(gbUsbExit == 0) 
		{
			for(i = 0; i < 4; i ++)
			{
				if(MountUsb(dev) == 0)
				{
					gbUsbExit = 1;
					//if (isUpDataSoftWare == 1)
					{
					
						//���MAC��SN�ļ�
						if (CheckFile("/mnt/usb/x3b_sn.txt", NULL, NULL) == 0)
						{
							//g_AutoDeviceTestMode = 1;
							printf("[%s,%d]\n",__FUNCTION__,__LINE__);

							gui_comp_sendcmd(CMD_UI_MAC_SN_POPMENU, NULL, 0, NULL, 0);
						}
						else if (CheckFile("/mnt/usb/x3b_mac.txt", NULL, NULL) == 0)
						{
							printf("[%s,%d]\n",__FUNCTION__,__LINE__);
							gui_comp_sendcmd(CMD_UI_MAC_SN_POPMENU, NULL, 0, NULL, 0);
							//g_AutoDeviceTestMode = 1;
						}
						
						if (CheckFile("/mnt/usb/update.ini", NULL, NULL) == 0)
						{
							setUpdateResult(0);
							gui_comp_sendcmd(CMD_UI_UPDATE_POPMENU, NULL, 0, NULL, 0);
							iRet = Update();

							setUpdateResult(iRet);
							/*
							//����
							DATEOP dateOp;
							dateOp.op = 3;
							dateOp.nFormat = 0;
							SetCurUSBTransNum(0);
							MessageSend(MSG_DATE_BACKUP, (char*)&dateOp, sizeof(DATEOP));
							*/
						}
						
					}
					break;
				}
				sleep(1);
			}

			if(i < 4) 
			{                
                return 1;
			}
		}
	}
	else 
	{
		//isUpDataSoftWare = 1;
		
        if (gbUsbExit)
        {
			umount(STORAGE_USB_PATH);
            gbUsbExit = 0;
			printf("umount[%s,%d]\n",__FUNCTION__,__LINE__);
        }
	}	
	return 0;
}
#endif

int getonelinesn(char* sn , int snlen)
{
     FILE * fp;
     char * line = NULL;
	 int count = 0;
	 int iRet = -1;
     size_t len = 0;
     ssize_t read;

	 if(NULL == sn)
	 {
	 	return iRet;
	 }
 
     fp = fopen("/mnt/usb/x3b_sn.txt", "r");
     if (fp == NULL)
     {
     	return iRet;
     }
 
     while ((read = getline(&line, &len, fp)) != -1)
	 {
	 	if(0 == count++)
	 	{
			//printf("[command:imei:%d  %s,%d]\n",strcmp("command:imei", "command:imei"),__FUNCTION__,__LINE__);
			//printf("[command:imei:%d  %s,%d]\n",strstr(line, "command:imei"),__FUNCTION__,__LINE__);
			//printf("[11command:imei:%d  %s,%d]\n",strcmp(line, "11command:imei"),__FUNCTION__,__LINE__);
			
			if (strstr(line, "command:sn") == 0)
			{
				break;
			}
	 	}
		else if ('#' == line[0])
		{
			continue;
		}
		else
		{
			if(strlen(line) < snlen)
			{
				snlen = strlen(line);
			}
			//-2ȥ�����з�
			memcpy(sn,line,snlen-2);
			printf("snlen:%d  sn:%s\n",snlen-2, sn);
			iRet = 0;
			break;
		}
     }
     
     if(line)
        free(line);
	 if(fp != NULL)
	 {
	 	fclose(fp);
	 }
	return iRet;
}

int setsnbeenuse()
{
     FILE * fp;
     char * line = NULL;
	 int count = 0;
	 int iRet = -1;
	 int iSeek = 0;
	 size_t len = 0;
     ssize_t read;
 
     fp = fopen("/mnt/usb/x3b_sn.txt", "r+");
     if (fp == NULL)
     {
     	//printf("[%s,%d]\n",__FUNCTION__,__LINE__);
     	return iRet;
     }
 
     while ((read = getline(&line, &len, fp)) != -1)
	 {
	 	if(0 == count++)
	 	{
			//printf("[command:imei:%d  %s,%d]\n",strcmp("command:imei", "command:imei"),__FUNCTION__,__LINE__);
			//printf("[command:imei:%d  %s,%d]\n",strstr(line, "command:imei"),__FUNCTION__,__LINE__);
			//printf("[11command:imei:%d  %s,%d]\n",strcmp(line, "11command:imei"),__FUNCTION__,__LINE__);
			
			if (strstr(line, "command:sn") == 0)
			{
				break;
			}
	 	}
		else if ('#' == line[0])
		{
			continue;
		}
		else
		{			
			len = strlen(line);
			iSeek = 0 - len;
			//printf("[iSeek:%d %s,%d]\n",iSeek,__FUNCTION__,__LINE__);
			fseek(fp,iSeek,SEEK_CUR);
			//fwrite("#", 1, 1, fp);
			fputs("#",fp);
			//system("sync");
			iRet = 0;
			break;
		}
     }
     
     if(line)
        free(line);
	 if(fp != NULL)
	 {
	 	fclose(fp);
		sync();
	 }
	 //printf("[iRet:%d %s,%d]\n",iRet,__FUNCTION__,__LINE__);
	return iRet;
}


int getonelinemac(char* mac , int maclen)
{
	FILE * fp;
	char * line = NULL;
	int count = 0;
	int iRet = -1;
	size_t len = 0;
	ssize_t read;

	if(NULL == mac)
	{
	return iRet;
	}

	fp = fopen("/mnt/usb/x3b_mac.txt", "r");
	if (fp == NULL)
	{
	return iRet;
	}

	while ((read = getline(&line, &len, fp)) != -1)
	{
		printf("%s", line);
		if(0 == count++)
		{
			//printf("[command:imei:%d  %s,%d]\n",strcmp("command:imei", "command:imei"),__FUNCTION__,__LINE__);
			//printf("[command:imei:%d  %s,%d]\n",strstr(line, "command:imei"),__FUNCTION__,__LINE__);
			//printf("[11command:imei:%d  %s,%d]\n",strcmp(line, "11command:imei"),__FUNCTION__,__LINE__);
			
			if (strstr(line, "command:eth") == 0)
			{
				break;
			}
		}
		else if ('#' == line[0])
		{
			continue;
		}
		else
		{
			if(strlen(line) < maclen)
			{
				maclen = strlen(line);
			}
			//-2ȥ�����з�
			memcpy(mac,line,maclen-2);
			printf("maclen:%d  sn:%s\n",maclen-2, mac);
			iRet = 0;
			break;
		}
	}

	if(line)
		free(line);

	if(fp != NULL)
	{
		fclose(fp);
	}
	return iRet;
}

int setmacbeenuse()
{
	FILE * fp;
	char * line = NULL;
	int count = 0;
	int iRet = -1;
	int iSeek = 0;
	size_t len = 0;
	ssize_t read;

	fp = fopen("/mnt/usb/x3b_mac.txt", "r+");
	if (fp == NULL)
	{
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	return iRet;
	}

	while ((read = getline(&line, &len, fp)) != -1)
	{
		//printf("%s", line);
		if(0 == count++)
		{
			//printf("[command:imei:%d  %s,%d]\n",strcmp("command:imei", "command:imei"),__FUNCTION__,__LINE__);
			//printf("[command:imei:%d  %s,%d]\n",strstr(line, "command:imei"),__FUNCTION__,__LINE__);
			//printf("[11command:imei:%d  %s,%d]\n",strcmp(line, "11command:imei"),__FUNCTION__,__LINE__);
			
			if (strstr(line, "command:eth") == 0)
			{
				break;
			}
		}
		else if ('#' == line[0])
		{
			continue;
		}
		else
		{			
			len = strlen(line);
			iSeek = 0 - len;
			//printf("[iSeek:%d %s,%d]\n",iSeek,__FUNCTION__,__LINE__);
			fseek(fp,iSeek,SEEK_CUR);
			//fwrite("#", 1, 1, fp);
			fputs("#",fp);
			//system("sync");
			iRet = 0;
			break;
		}
	}
	
	if(line)
		free(line);

	if(fp != NULL)
	{
		fclose(fp);
		sync();
	}
	//printf("[iRet:%d %s,%d]\n",iRet,__FUNCTION__,__LINE__);
return iRet;
}

int UpdateSoftware(char * pData, int nLen)
{
	int ret = -1;
	printf("Updating software, please wait ... ");
	if(nLen>0)
	{
		FILE* f = fopen(UPDATE_FOFT_FILE_NAME, "wb");
		if (f==NULL)
		{
			printf("Create file %s fail.\r\n", UPDATE_FOFT_FILE_NAME);
		}
		else 
		{
			int	nwrite = fwrite(pData, nLen, 1, f);
			fclose(f);
			if (nwrite == 1) 
			{
				ret = 1;
				printf("OK.\r\n");
			} 
			else
			{
				printf("Write file %s  FAIL !\r\n", UPDATE_FOFT_FILE_NAME);
			}
		}
	} 
	else 
	{
		printf("File size invalid !\r\n");
	}
	return ret;
}

int Update()
{
	int ret = 0;
	struct stat st;
	char filename[261] = {0};
	//int i = 0;
	
    memset(filename, 0x00, sizeof(filename));
    if(stat("/mnt/usb/update.ini", &st) == 0 && st.st_size > 0) 
	{
		FILE *in = NULL;
		char line[512] = {0};
	    char *s1 = NULL;
        //char *s2 = NULL;
		in = fopen("/mnt/usb/update.ini", "r");
		if(in) 
		{
			printf("[%s,%d]\n",__FUNCTION__,__LINE__);
			BOOL flag = FALSE;
			while(fgets(line, 512, in)) 
			{
				if(flag==FALSE)
				{
					char *pMatch = NULL;
					
					pMatch = strstr(line, "uploadfiles");
					if(pMatch==NULL)
					{
						continue;
					}
					flag = TRUE;
					printf("[%s,%d]\n",__FUNCTION__,__LINE__);
				}
				else 
				{
					//char *p = NULL;
			        s1 = strstr(line, "=");
					printf("[line:%s %s,%d]\n",line,__FUNCTION__,__LINE__);

					char filePath[261] = {0};
					char filePath_2[261] = {0};
					int len = strlen(s1)-1;
					if(len > 260)
					{
						len = 260;
					}
					memcpy(filePath_2, s1+1, len);
					filePath_2[len]='\0';
					snprintf(filePath, 260, "%s%s", "/mnt/usb/", filePath_2);
					printf("[filePath:%s %s,%d]\n",filePath,__FUNCTION__,__LINE__);
					if(stat(filePath, &st)==0 && st.st_size > 0)
					{
						memcpy(filename, filePath, 260);
						break;
					}
					else
					{
						printf("No Exist: %s \r\n", filePath);
					}
					
				}
			}
			fclose(in);
		}
	}
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);

	//for(i = 0; i < 10; i++) 
	if(strlen(filename) > 0)
	{
		int h = 0;
		ret = 0;
	
		if(stat(filename, &st) == 0)
		{
			h = open(filename, O_RDONLY);
			if(h == -1)
			{
				ret = -1;
			}
			else 
			{
				int filesize=0;
				//int filetype = -1;
				char *pBuf = NULL;
				pBuf = (char *)malloc(st.st_size);
				if(pBuf)
				{
					printf("[%s,%d]\n",__FUNCTION__,__LINE__);
					filesize = read(h, pBuf, st.st_size);
				}
				else
				{
					printf("[%s,%d]\n",__FUNCTION__,__LINE__);
					filesize = -1;
				}
				close(h);
				
				if(filesize != st.st_size )
				{
					ret = -1;
				}
				else
				{
					printf("filesize = %d \r\n", filesize);
					
					MD5_CONTEXT mdc;
					unsigned char key[16];
					MD5Init(&mdc);
					MD5Update(&mdc, (unsigned char *)pBuf, filesize-16);
					MD5Final(key, &mdc);
					
					if(memcmp(pBuf+filesize-16, key, 16)==0)
					{
						filesize-=16;
					}
					else 
					{
						printf("key[] = ");
						int KK=0;
						for (KK=0; KK<16; KK++)
						{
							printf("%02X ", key[KK]);
						}
						printf("\r\n");
						
						/* MD5 У��ʧ�� */
						printf("Verify MD5 digits Failed.\r\n");
						filesize = -1;
						ret = -2;
						if(pBuf != NULL)
						{
							free(pBuf);
						}
						return -1;
					}

					if(filesize > 0)
					{
						ret = UpdateSoftware(pBuf, filesize);
					}
				}
				
				if(pBuf != NULL)
				{
					free(pBuf);
				}
			}
		}
	}
	return ret;
}

