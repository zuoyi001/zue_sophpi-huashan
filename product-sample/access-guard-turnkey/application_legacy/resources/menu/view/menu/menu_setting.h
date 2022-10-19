#ifndef MENU_SETTING_H
#define MENU_SETTING_H
#include "lvgl/lvgl.h"

#define MENU_SETTING_MAIN_OPACITY (LV_OPA_100)

void menu_scr_init();
void menu_scr_show();
void menu_scr_hide();
lv_obj_t * lv_scr_menu();
void menuPageBack();
void mainMenu_retranslateUI();

typedef void(* page_return_cb)(void);
#endif