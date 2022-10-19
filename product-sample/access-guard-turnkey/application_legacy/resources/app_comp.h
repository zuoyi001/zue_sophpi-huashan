#ifndef _APP_COMP_H_
#define _APP_COMP_H_

enum {
    CMD_APP_COMP_UNKNOWN,
    CMD_APP_COMP_VIDEOSTOP ,
    CMD_APP_COMP_REGISTER,
    CMD_APP_COMP_RESTART,
    CMD_APP_COMP_VIDEORESIZE,
    CMD_APP_COMP_VIDEOFREEZE,
    CMD_APP_COMP_FACE_REGISTER,
    CMD_APP_COMP_FACE_REGISTER_MULTI,
    CMD_APP_COMP_GBR_START,
    CMD_APP_COMP_IR_START,
};

void APP_CompInit();
void APP_CompSendCmd(int cmd, void *args, int args_len, void *ext, int ext_len);
void APP_CompTaskHandler();
#endif