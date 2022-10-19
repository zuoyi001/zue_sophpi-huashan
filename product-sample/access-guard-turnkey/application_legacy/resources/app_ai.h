#ifndef _APP_AI_H_
#define _APP_AI_H_

enum {
	CMD_AI_NONE=0,
    CMD_AI_FD=1,
    CMD_AI_LIVENESS=2,
    CMD_AI_FR=4,
	CMD_AI_STOP=8,
};

typedef enum _AI_STATE_E {
    AI_INIT=0,    //
	AI_FD=1,
	AI_LIVENESS=2,
	AI_FR=4,
	AI_STOP=8,      //
	AI_UNKNOWN
}AI_STATE_E;

#define JUMP_FD_NUM 20

void APP_AI_ThreadInit();
void APP_AI_ThreadSendCmd(int cmd, void *args, int args_len, void *ext, int ext_len);
void* APP_AI_FaceThread(void *arg);
#endif