#ifndef _GUI_CTRL_H_
#define _GUI_CTRL_H_

enum {
    CMD_UI_UNKNOWN = 0,
    CMD_UI_DRAW_FACE_RECT = 1,
    CMD_UI_CLEAR_FACE_RECT,
    CMD_UI_SHOW_POPMENU,
    CMD_UI_HIDE_POPMENU,
    CMD_UI_MAC_SN_POPMENU,
    CMD_UI_UPDATE_POPMENU,
    CMD_UI_DOWN_USER_POPMENU
};
void gui_comp_init();

void gui_task_handler();
void gui_comp_sendcmd(int cmd, void *args, int args_len, void *ext, int ext_len);
#endif