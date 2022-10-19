#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "lvgl/lvgl.h"

void keyboard_init();
lv_obj_t * create_keyboard(lv_obj_t **handle, lv_obj_t * text_area, lv_obj_t *parent, lv_coord_t offs_x, lv_coord_t offs_y);
lv_obj_t * create_pinyin_keyboard(lv_obj_t **handle, lv_obj_t * text_area, lv_obj_t *parent, lv_coord_t offs_x, lv_coord_t offs_y);

#endif