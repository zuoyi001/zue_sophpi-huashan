#ifndef __CYCLE_BUFFER_H__
#define __CYCLE_BUFFER_H__
#include <stdio.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CB_ARRAY (32)

typedef struct _cycleBuffer
{
	int size;
	int wroffset;
	int rdoffser;
    unsigned int realWR;
    unsigned int realRD;
	char * buf;
	int buflen;
    int isInit;
	pthread_mutex_t lock;
}cycleBuffer;

int CycleBufferInit(void ** ppstCb,int bufLen,int channel);
void CycleBufferDestory(void *pstCb);
//int CycleBufferRead(void *pstCb,char * outbuf,int readLen);
int CycleBufferWrite(void *pstCb,char * inbuf,int wrireLen);
int CycleBufferReadCB(char * outbuf,int bytes_len);
int CycleBufferCBMaxLen(void);
 int CycleBufferDataLen(void *pstCb);

 
 #ifdef __cplusplus
} /* extern "C" */
#endif

#endif //__CYCLE_BUFFER_H__

 
 
