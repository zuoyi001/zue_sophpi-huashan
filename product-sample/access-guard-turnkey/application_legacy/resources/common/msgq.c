#include "msgq.h"

#define DEBUG(x)  //x

void msgq_init(msgq_t *msgq,char * msgqName)
{
    if (msgq == NULL)
        return;
    
    memset(msgq, 0, sizeof(msgq_t));
	memcpy(msgq->name,msgqName,strlen(msgqName)>= QUEUE_NAME_SIZE?QUEUE_NAME_SIZE:strlen(msgqName));
    pthread_mutex_init(&msgq->msgq_lock, NULL);
	pthread_condattr_init(&(msgq->cattr));
	pthread_condattr_setclock(&(msgq->cattr), CLOCK_MONOTONIC);
	pthread_cond_init(&msgq->msgq_cond,&(msgq->cattr));
}

bool msgq_send(msgq_t *msgq, msg_node_t *msg)
{
    if (msgq == NULL || msg == NULL)
    {
        return false;
    }

	DEBUG(printf("msgq_send lock name:%s\n",msgq->name));
    pthread_mutex_lock(&msgq->msgq_lock);
    if ((msgq->wr_idx + 1) % MSG_QUEUE_MAX_SIZE == msgq->rd_idx)
    {
        msgq->wr_idx = (msgq->wr_idx + 1) % MSG_QUEUE_MAX_SIZE;
        msgq->rd_idx = (msgq->rd_idx + 1) % MSG_QUEUE_MAX_SIZE;
        // pthread_mutex_unlock(&msgq->msgq_lock);
		//DEBUG(printf("msgq_send error unlock name:%s\n",msgq->name));
        //printf("msgq_send error unlock name:%s\n",msgq->name);
        //return false;
    }
    else
        msgq->wr_idx = (msgq->wr_idx + 1) % MSG_QUEUE_MAX_SIZE;

    memcpy(&msgq->buffer[msgq->wr_idx], msg, sizeof(msg_node_t));
    msgq->buffer[msgq->wr_idx].ext = NULL;
    msgq->buffer[msgq->wr_idx].ext_len = 0;
    if (msg->ext != NULL && msg->ext_len > 0) {
        msgq->buffer[msgq->wr_idx].ext = malloc(msg->ext_len);
        msgq->buffer[msgq->wr_idx].ext_len = msg->ext_len;
    }

	msgq->sem_count++;  //free resource +1

    //if(msgq->sem_count == 1 ||msgq->bWaiting)    //if this resource is 1, must be busy.
    if (msgq->bWaiting)
    {
        pthread_cond_signal(&msgq->msgq_cond);   //Send a signal
    }
	
    pthread_mutex_unlock(&msgq->msgq_lock);
	DEBUG(printf("msgq_send unlock name:%s\n",msgq->name));
    return true;
}

bool msgq_receive(msgq_t *msgq, msg_node_t *msg)
{
    if (msgq == NULL || msg == NULL)
    {
        return false;
    }

	DEBUG(printf("msgq_receive lock name:%s\n",msgq->name));
    pthread_mutex_lock(&msgq->msgq_lock);

	#if 0
	if(msgq->sem_count == 0)    //if resource == 0, there must a task waiting
    {
        if(pthread_cond_wait(&msgq->msgq_cond,&msgq->msgq_lock) != 0)
        {
            return -1;
        }
    }
	#endif

    if(msgq->sem_count > 0)
    {
        msgq->sem_count--;   // large than 0, resource -1
    }

	

    if (msgq->rd_idx == msgq->wr_idx) {
        pthread_mutex_unlock(&msgq->msgq_lock);
		DEBUG(printf("msgq_receive no unlock name:%s\n",msgq->name));
        return false;
    } else {
        msgq->rd_idx = (msgq->rd_idx + 1) % MSG_QUEUE_MAX_SIZE;
        memcpy(msg, &msgq->buffer[msgq->rd_idx], sizeof(msg_node_t));
        memset(&msgq->buffer[msgq->rd_idx], 0, sizeof(msg_node_t));
    }

    pthread_mutex_unlock(&msgq->msgq_lock);
	DEBUG(printf("msgq_receive unlock name:%s\n",msgq->name));

    return true;
}


bool msgq_receive_timeout(msgq_t *msgq, msg_node_t *msg,int ms)
{
	struct timespec outtime;
    if (msgq == NULL || msg == NULL)
    {
        return false;
    }

	DEBUG(printf("msgq_receive_timeout lock name:%s\n",msgq->name));
    pthread_mutex_lock(&msgq->msgq_lock);

	if(ms > 0 )
	{
		clock_gettime(CLOCK_MONOTONIC, &outtime);
		outtime.tv_sec += ms/1000;
		long long  us = outtime.tv_nsec/1000 + 1000 * (ms % 1000);
		outtime.tv_sec += us / 1000000; 
		us = us % 1000000;
	    outtime.tv_nsec = us * 1000;

		if(msgq->sem_count == 0)    //if resource == 0, there must a task waiting
	    {
	    	msgq->bWaiting = true;
	        if(pthread_cond_timedwait(&msgq->msgq_cond,&msgq->msgq_lock, &outtime) != 0)
	        {
	        	msgq->bWaiting = false;
				pthread_mutex_unlock(&msgq->msgq_lock);
				DEBUG(printf("msgq_receive_timeout error unlock name:%s\n",msgq->name));
	            return -1;
	        }
	    }
	}else if(msgq->sem_count == 0)	//if resource == 0, there must a task waiting
	{
		msgq->bWaiting = true;
		if(pthread_cond_wait(&msgq->msgq_cond,&msgq->msgq_lock) != 0)
		{
			msgq->bWaiting = false;
			pthread_mutex_unlock(&msgq->msgq_lock);
			DEBUG(printf("msgq_receive_timeout no error unlock name:%s\n",msgq->name));
			return -1;
		}
	}

	msgq->bWaiting = false;

    if(msgq->sem_count > 0)
    {
        msgq->sem_count--;   // large than 0, resource -1
    }

    if (msgq->rd_idx == msgq->wr_idx) {
        pthread_mutex_unlock(&msgq->msgq_lock);
        return false;
    } else {
        msgq->rd_idx = (msgq->rd_idx + 1) % MSG_QUEUE_MAX_SIZE;
        memcpy(msg, &msgq->buffer[msgq->rd_idx], sizeof(msg_node_t));
        memset(&msgq->buffer[msgq->rd_idx], 0, sizeof(msg_node_t));
    }

    pthread_mutex_unlock(&msgq->msgq_lock);
	DEBUG(printf("msgq_receive_timeout unlock name:%s\n",msgq->name));

    return true;
}


int msgq_get_len(msgq_t *msgq)
{
    if (msgq == NULL)
        return -1;

    pthread_mutex_lock(&msgq->msgq_lock);
    int len = 0;
    if (msgq->rd_idx > msgq->wr_idx) {
        len = msgq->wr_idx + MSG_QUEUE_MAX_SIZE - msgq->rd_idx;
    } else if (msgq->rd_idx < msgq->wr_idx) {
        len = msgq->wr_idx - msgq->rd_idx;
    }

    pthread_mutex_unlock(&msgq->msgq_lock);
    return len;
}

void msgq_clear(msgq_t *msgq)
{
    if (msgq->rd_idx == msgq->wr_idx)
        return;

    while (msgq->rd_idx != msgq->wr_idx) {
        msgq->rd_idx = (msgq->rd_idx + 1) % MSG_QUEUE_MAX_SIZE;
        if (msgq->buffer[msgq->rd_idx].ext) {
            free(msgq->buffer[msgq->rd_idx].ext);
            msgq->buffer[msgq->rd_idx].ext = NULL;
            msgq->buffer[msgq->rd_idx].ext_len = 0;
        }
    }

    msgq->rd_idx = msgq->wr_idx = 0;
}
int msg_send(msgq_t *msgq,int msg_type, void *args, int args_len, void *ext, int ext_len)
{
	msg_node_t msg;

	if(msgq == NULL) {
		printf("msg_send error msgq NULL.\n");
		return -1;
	}

	msg.msg_type = msg_type;
	if(args && args_len ) {
		args_len = args_len <= MSG_DATA_LEN?args_len:MSG_DATA_LEN;
		memcpy(msg.data,args,args_len);
	}
	msg.ext = ext;
	msg.ext_len = ext_len;

    if (!msgq_send(msgq, &msg))
    {
        printf("error send cmd\n");
		return -2;
    }

	return 0;
		
}
