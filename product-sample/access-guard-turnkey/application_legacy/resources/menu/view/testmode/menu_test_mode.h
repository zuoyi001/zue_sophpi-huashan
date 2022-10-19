#ifndef MENU_TEST_MODE_SETTING_H
#define MENU_TEST_MODE_SETTING_H
#include "lvgl/lvgl.h"

#define MENU_SETTING_MAIN_OPACITY (LV_OPA_100)

#define LV_MENU_WIDTH          (226)
#define LV_MENU_HEIGHT         (220)

enum {
	MENU_ID_TEST_INVALID = 0,
    MENU_ID_ABOUT = 1,
    MENU_ID_LCD = 2,
    MENU_ID_LED,
    MENU_ID_AUDIO,
    MENU_ID_RELAY,
    MENU_ID_TP,
    MENU_ID_CAMERA,
    MENU_ID_NFC,
    MENU_ID_MIC,
    MENU_ID_WIFI,
    MENU_ID_RS485_WG,
    MENU_ID_USB,
    MENU_ID_KEY,
    MENU_ID_RADAR,
    MENU_ID_ETH,
    MENU_ID_TEST_END
};

extern int menu_btn_flag[17];

void menu_test_mode_scr_init();
void menu_test_mode_scr_show();
void menu_test_mode_scr_hide();
lv_obj_t * lv_scr_menu_test_mode();
void menu_test_mode_PageBack();
void mainMenu_test_mode_retranslateUI();
int get_test_mode_page_flag(void);

typedef void(* page_return_cb)(void);
#endif
