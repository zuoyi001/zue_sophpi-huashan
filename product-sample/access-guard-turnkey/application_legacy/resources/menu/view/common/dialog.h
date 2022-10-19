#ifndef LAUNCHER_DIALOG_H
#define LAUNCHER_DIALOG_H
#include "lvgl/lvgl.h"

typedef enum {
    MSG_BOX_BTN_NONE = 0,
    MSG_BOX_BTN_OK = 1,
    MSG_BOX_BTN_OK_CANCEL = 2,
    MSG_BOX_BTN_END
}MSG_BOX_BTN_TYPE;

typedef void (*dialog_func_cb)(const void *arg);

void setPasswdClear();
lv_obj_t * createMsgBox(lv_obj_t *par, char *title, char *msg, MSG_BOX_BTN_TYPE type, dialog_func_cb ok, dialog_func_cb cancel);
lv_obj_t * createPasswdBox(lv_obj_t *par, char *title, dialog_func_cb ok);
lv_obj_t * createPopupMenu(lv_obj_t *par, void *face_img, char *name_str, const char *result_str, int width, int height, bool bPass);
lv_obj_t * createPopupMenuWMS(lv_obj_t *par);
#endif