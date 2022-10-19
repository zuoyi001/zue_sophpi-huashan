#ifndef MENU_NET_H
#define MENU_NET_H
//#include "lvgl/lvgl.h"
#include "menu.h"
enum {
    MENU_ID_NET_INVALID = 0,
    MENU_ID_NET_ETH = 1,
    MENU_ID_NET_WLAN,
    MENU_ID_NET_WEB,
    MENU_ID_NET_END
};

menu_page_t get_net_menu_page();

#endif
