#ifndef _MSG_Q_H_
#define _MSG_Q_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#ifndef bool
#define bool int
#endif
#define true 1
#define false 0

#define MSG_DATA_LEN (1024)
#define MSG_QUEUE_MAX_SIZE (5)
#define QUEUE_NAME_SIZE (16)

typedef struct msgNode {
    int     msg_id;
    int     msg_type;
    char    data[MSG_DATA_LEN];
    void*   ext;
    int     ext_len;
} msg_node_t;

typedef struct {
	pthread_cond_t msgq_cond;
	pthread_condattr_t cattr;
    pthread_mutex_t msgq_lock;
	bool bWaiting;
	int sem_count;
    char    name[QUEUE_NAME_SIZE+1];
    int     rd_idx;
    int     wr_idx;
    msg_node_t  buffer[MSG_QUEUE_MAX_SIZE];
} msgq_t;

//void msgq_init(msgq_t *msgq);
void msgq_init(msgq_t *msgq,char * msgqName);

bool msgq_send(msgq_t *msgq, msg_node_t *msg);
bool msgq_receive(msgq_t *msgq, msg_node_t *msg);
int  msgq_get_len(msgq_t *msgq);
void msgq_clear(msgq_t *msgq);
bool msgq_receive_timeout(msgq_t *msgq, msg_node_t *msg,int ms);
int msg_send(msgq_t *msgq,int msg_type, void *args, int args_len, void *ext, int ext_len);

#endif
