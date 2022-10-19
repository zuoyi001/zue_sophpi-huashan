#ifndef MENU_FACTORY_MODE_SETTING_H
#define MENU_FACTORY_MODE_SETTING_H
#include "lvgl/lvgl.h"

#define MENU_SETTING_MAIN_OPACITY (LV_OPA_100)

void menu_factory_mode_scr_init();
void menu_factory_mode_scr_show();
void menu_factory_mode_scr_hide();
lv_obj_t * lv_scr_menu_factory_mode();
void menu_factory_mode_PageBack();
void mainMenu_factory_mode_retranslateUI();

typedef void(* page_return_cb)(void);
#endif