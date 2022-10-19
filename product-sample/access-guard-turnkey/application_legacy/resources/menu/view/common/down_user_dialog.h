#ifndef DOWN_USER_DIALOG_H
#define DOWN_USER_DIALOG_H

#include "lvgl/lvgl.h"

lv_obj_t * createPopupMenuDownUser(lv_obj_t *par);
void hide_down_user_popmenu();
void show_down_user_popmenu();
void set_down_user_time(time_t times);

#endif