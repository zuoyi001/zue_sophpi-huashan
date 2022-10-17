#ifndef __WINDOW_COMMON_H__
#define __WINDOW_COMMON_H__
#include "awtk.h"

#define UI_KEY_BACK      TK_KEY_F1
#define UI_KEY_OK        TK_KEY_F2
#define UI_KEY_UP        TK_KEY_F3
#define UI_KEY_DOWN      TK_KEY_F4

#define EVT_LOW_BATTERY EVT_USER_START + 1

int uiapp_start(void);
int uiapp_stop(void);


ret_t application_init(void);
ret_t application_exit(void);
ret_t open_menu_window(void);
ret_t open_wrnmsg_window(uint32_t type);
ret_t open_format_window(void);
ret_t open_reset_window(void);
ret_t open_settime_window(void);

#endif