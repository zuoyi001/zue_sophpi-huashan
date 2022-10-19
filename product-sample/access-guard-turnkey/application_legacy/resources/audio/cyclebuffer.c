#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cyclebuffer.h"
// #include "tcli.h"
#include "app_utils.h"


#define DEBUG(x) //x


struct cycleBufferArray {
	cycleBuffer *gapCb[MAX_CB_ARRAY];
	int count;
	int framelen_bytes;
	int channel_cnt;
	short *pTmp;
	int bInit;
	pthread_mutex_t lock;
};

static struct cycleBufferArray gsCbInfo;
static int CycleBufferCBIn(int framelen_bytes,int channel)
{
	if(gsCbInfo.bInit) {

		if(framelen_bytes > gsCbInfo.framelen_bytes) {
			gsCbInfo.framelen_bytes = framelen_bytes;
			gsCbInfo.pTmp = realloc(gsCbInfo.pTmp,framelen_bytes);
			if(!gsCbInfo.pTmp) {
				DBG_PRINTF(CVI_INFO, "error realloc for tmp buffer.\n");
			}
		}
		return 0;
	}
	memset(&gsCbInfo,0,sizeof(gsCbInfo));
	gsCbInfo.framelen_bytes = framelen_bytes;
	gsCbInfo.channel_cnt = channel;
	gsCbInfo.pTmp = malloc(framelen_bytes);
	gsCbInfo.bInit = 1;
	pthread_mutex_init(&(gsCbInfo.lock),0);
	return 0;
}


int CycleBufferInit(void ** ppstCb,int bufLen,int channel)
{
	cycleBuffer *pstCBTmp;
	int index = 0;
	pthread_mutex_lock(&gsCbInfo.lock);
    if(!ppstCb || bufLen == 0)
    {
    	DBG_PRINTF(CVI_INFO, "error param, CycleBufferInit ppstCb:%p ,bufLen:%d.\n",ppstCb,bufLen);
		pthread_mutex_unlock(&gsCbInfo.lock);
        return -1;
    }

	CycleBufferCBIn(bufLen/2,channel);

	
	while(index < MAX_CB_ARRAY)
	{
		if(gsCbInfo.gapCb[index] == NULL)
			break;
		index++;
	}

	if(index >= MAX_CB_ARRAY)
	{
		DBG_PRINTF(CVI_INFO, "Cycle buffer max count is %d.\n",MAX_CB_ARRAY);
		pthread_mutex_unlock(&gsCbInfo.lock);
		return -1;
	}
	
	*ppstCb = (cycleBuffer *)malloc(sizeof(cycleBuffer));
	if(*ppstCb == NULL) {
		DBG_PRINTF(CVI_INFO, "CycleBufferInit malloc failed.\n");
		pthread_mutex_unlock(&gsCbInfo.lock);
        return -1;
	}
	pstCBTmp = (cycleBuffer *)(*ppstCb);

	pthread_mutex_init(&(pstCBTmp->lock),0);

    memset(pstCBTmp,0,sizeof(cycleBuffer));
    pstCBTmp->isInit = 1;

    pstCBTmp->buflen=bufLen;
    pstCBTmp->buf=(char *)malloc(bufLen);
    memset(pstCBTmp->buf,0,bufLen);
	
	gsCbInfo.gapCb[index] = pstCBTmp;
	gsCbInfo.count++;
    DEBUG(DBG_PRINTF(CVI_INFO, "CycleBufferInit buflen:%d,cyclebuffer count=%d.\n",bufLen,gsCbInfo.count));
	pthread_mutex_unlock(&gsCbInfo.lock);
    return 0;
}

void CycleBufferDestory(void *pstCb)
{
	cycleBuffer *pstCBTmp = (cycleBuffer *)pstCb;
	int index = 0;
    if((pstCb == NULL) ||!pstCBTmp->isInit)
    {
    	DBG_PRINTF(CVI_INFO, "error param, CircleBufferDestory pstCb is NULL.\n");
        return ;
    }
	pthread_mutex_lock(&gsCbInfo.lock);

	while(index < MAX_CB_ARRAY) {
		if(gsCbInfo.gapCb[index] == pstCb) {
			gsCbInfo.gapCb[index] = NULL;
			gsCbInfo.count--;
			DBG_PRINTF(CVI_INFO, "detory cb suc[%d].\n",gsCbInfo.count);
		}
		index++;
	}

    if(pstCBTmp->buf)
    {
        free(pstCBTmp->buf);
    }
    memset(pstCBTmp,0,sizeof(cycleBuffer));
	pthread_mutex_unlock(&gsCbInfo.lock);
    DEBUG(DBG_PRINTF(CVI_INFO, "CycleBufferDestory .\n"));
    return;
}

static int CycleBufferRead(void *pstCb,char * outbuf,int readLen)
{
    int canReadLen=0;
	cycleBuffer *pstCBTmp = (cycleBuffer *)pstCb;

	if((pstCb == NULL) ||!(pstCBTmp->isInit))
    {
    	DBG_PRINTF(CVI_INFO, "error param, CycleBufferRead pstCb is NULL.\n");
        return 0;
    }
	
    pthread_mutex_lock(&pstCBTmp->lock);
    if(outbuf ==NULL) {
		pthread_mutex_unlock(&pstCBTmp->lock);
		return 0;
	}
        
	
    if(pstCBTmp->size < readLen) {
		pthread_mutex_unlock(&pstCBTmp->lock);
		return 0;
	}
    
    canReadLen=pstCBTmp->wroffset-pstCBTmp->rdoffser;
    if(canReadLen <= 0) 
        canReadLen += pstCBTmp->buflen;
    if(canReadLen < readLen)
    {
        readLen=canReadLen;
    }
    if(readLen < pstCBTmp->buflen - pstCBTmp->rdoffser)
    {
        memcpy(outbuf,&pstCBTmp->buf[pstCBTmp->rdoffser],readLen);
    }else
    {
        memcpy(outbuf,&pstCBTmp->buf[pstCBTmp->rdoffser],pstCBTmp->buflen-pstCBTmp->rdoffser);
        memcpy(&outbuf[pstCBTmp->buflen-pstCBTmp->rdoffser],pstCBTmp->buf,readLen - (pstCBTmp->buflen - pstCBTmp->rdoffser));
    }
    pstCBTmp->size -= readLen;
    pstCBTmp->rdoffser=(readLen+ pstCBTmp->rdoffser)% pstCBTmp->buflen;
    DEBUG(DBG_PRINTF(CVI_INFO, "CycleBufferRead r:%d,w:%d,size:%d\n",pstCBTmp->rdoffser,pstCBTmp->wroffset,pstCBTmp->size));

	pthread_mutex_unlock(&pstCBTmp->lock);
    return readLen;
}



int CycleBufferWrite(void *pstCb,char * inbuf,int wrireLen)
{
    int canWriteLen=0;
	cycleBuffer *pstCBTmp = (cycleBuffer *)pstCb;
	
	if((pstCb == NULL) ||!(pstCBTmp->isInit))
    {
    	DBG_PRINTF(CVI_INFO, "error param, CycleBufferWrite pstCb is NULL.\n");
        return 0;
    }
	
	pthread_mutex_lock(&pstCBTmp->lock);
    if((pstCBTmp->buflen== 0) ||(inbuf==NULL) || (wrireLen==0) )
    {
    	pthread_mutex_unlock(&pstCBTmp->lock);
        return 0;
    }
    if(pstCBTmp->size== pstCBTmp->buflen) 
    {
    	pthread_mutex_unlock(&pstCBTmp->lock);
        return 0;
    }

    canWriteLen=  pstCBTmp->rdoffser-  pstCBTmp->wroffset;
    if(canWriteLen<=0) 
        canWriteLen +=  pstCBTmp->buflen;
    
    if(wrireLen > canWriteLen) return -1;

    if(wrireLen < (pstCBTmp->buflen- pstCBTmp->wroffset))
    {
        memcpy(&pstCBTmp->buf[pstCBTmp->wroffset],inbuf,wrireLen);
    }else
    {
      
        memcpy(&pstCBTmp->buf[pstCBTmp->wroffset],inbuf,pstCBTmp->buflen-  pstCBTmp->wroffset);
        memcpy(pstCBTmp->buf,&inbuf[pstCBTmp->buflen- pstCBTmp->wroffset],wrireLen-(pstCBTmp->buflen-pstCBTmp->wroffset));
    }

    pstCBTmp->wroffset=(wrireLen+ pstCBTmp->wroffset)%  pstCBTmp->buflen;
    pstCBTmp->size+=wrireLen;
	pthread_mutex_unlock(&pstCBTmp->lock);
    DEBUG(DBG_PRINTF(CVI_INFO, "CycleBufferWrite r:%d,w:%d,size:%d\n",pstCBTmp->rdoffser,pstCBTmp->wroffset,pstCBTmp->size));
    return wrireLen;
}

 
int CycleBufferDataLen(void *pstCb)
{
	cycleBuffer *pstCBTmp = (cycleBuffer *)pstCb;
	
	if((pstCb == NULL) ||!(pstCBTmp->isInit))
    {
    	DBG_PRINTF(CVI_INFO, "error param, CycleBufferWrite pstCb is NULL.\n");
        return 0;
    }
	return pstCBTmp->size;
}


int CycleBufferCBMaxLen(void)
{
	int i,len,max_len;
	if(!gsCbInfo.count) {
		return 0;
	}

	max_len = 0;
	for(i = 0; i < MAX_CB_ARRAY; i++) {
		if(gsCbInfo.gapCb[i]) {
			len = CycleBufferDataLen(gsCbInfo.gapCb[i]);
			if(max_len < len)
				max_len = len;
		}
	}

	return max_len;
}



int CycleBufferReadCB(char * outbuf,int bytes_len)
{
	int ret,i,j;
	cycleBuffer *pCb;
	short *pBuffer = (short *)outbuf;

	ret = 0;
	pthread_mutex_lock(&gsCbInfo.lock);
	if(gsCbInfo.count <= 0) {
		pthread_mutex_unlock(&gsCbInfo.lock);
		return 0;
	}else if(bytes_len > gsCbInfo.framelen_bytes) {
		DBG_PRINTF(CVI_INFO, "error,read len is more than half of buffer size.\n");
		pthread_mutex_unlock(&gsCbInfo.lock);
		return 0;
	}
	memset(pBuffer,0,bytes_len);

	for(i = 0 ; i < MAX_CB_ARRAY; i++) {
		pCb = gsCbInfo.gapCb[i];
		if(pCb) {
			if(CycleBufferDataLen(pCb) >= bytes_len) {
					ret = CycleBufferRead(pCb,(char *)gsCbInfo.pTmp ,bytes_len);
				for(j = 0; j < bytes_len/2;j++)
					{
						pBuffer[j] += gsCbInfo.pTmp[j];
					}
			}
		}
	}
	pthread_mutex_unlock(&gsCbInfo.lock);
	return ret;
}

void CycleBufferInfo(void)
{
	int i;
	tcli_print("init:%d.\n",gsCbInfo.bInit);
	tcli_print("count:%d.\n",gsCbInfo.count);
	tcli_print("framelen_bytes:%d.\n",gsCbInfo.framelen_bytes);
	
	for(i = 0; i < MAX_CB_ARRAY; i++) {
		if(gsCbInfo.gapCb[i]) {
			cycleBuffer *p = gsCbInfo.gapCb[i];
			tcli_print("gapCb[%d],len:%d,r(%d),w(%d)\n",i,CycleBufferDataLen(p),p->rdoffser,p->wroffset);
		}
	}
}
 
 
