#ifndef MENU_COMMON_H
#define MENU_COMMON_H
#include "menu_setting.h"
#include "config_data.h"

enum {
    MENU_ID_INVALID = 0,
    MENU_ID_DEV_ACTIVATE = 1,
    MENU_ID_MODE_SWITCH,
    MENU_ID_ACCESS_CTRL,
    MENU_ID_DEV_MANAGE,
    MENU_ID_DEV_SETTING,
    MENU_ID_DEV_INFO,
    MENU_ID_FACE_LIB,
    MENU_ID_RECORD_MANAGE,
    MENU_ID_NET_MANAGE,
    MENU_ID_PROFILE_NOTIFY,
    MENU_ID_END
};

LV_FONT_DECLARE(stsong_36);
LV_FONT_DECLARE(stsong_34);
LV_FONT_DECLARE(stsong_32);
LV_FONT_DECLARE(stsong_30);
LV_FONT_DECLARE(stsong_28);
LV_FONT_DECLARE(stsong_26);
LV_FONT_DECLARE(stsong_24);
LV_FONT_DECLARE(stsong_22);
LV_FONT_DECLARE(stsong_18);
LV_FONT_DECLARE(stsong_16);

#define COLOR_TOP_PANEL (lv_color_hex(0x48D1CC))
#define COLOR_BLUE (lv_color_hex(0x00008B))
#define COLOR_GREEN (lv_color_hex(0x008000))
#define COLOR_RED (lv_color_hex(0xFF0000))

#define TOP_PANEL_H (100)
#define TOP_TEST_MODE_PANEL_H (120)
#define MENU_ITEM_H (80)
#define MENU_SEC_HEADER_H (50)
#define ITEM_DROPDOWN_WIDTH (150)
#define ITEM_VALUE_WIDTH (150)
#define ITEM_PAD_LEN (20)
#define ITEM_TILE_PAD (20)
#define SCROLL_BAR_WIDTH (50)

#define tr(text) lv_i18n_get_text(text)

#endif
