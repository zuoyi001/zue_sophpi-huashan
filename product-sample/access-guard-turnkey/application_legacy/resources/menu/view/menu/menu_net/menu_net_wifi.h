#ifndef MENU_NET_WIFI_H
#define MENU_NET_WIFI_H
#include "menu.h"
enum {
    MENU_ID_NET_WIFI_INVALID = 0,
    MENU_ID_NET_WIFI_MODE = 1,
    MENU_ID_NET_WIFI_IP,
    MENU_ID_NET_WIFI_NETMASK,
    MENU_ID_NET_WIFI_END
};

menu_page_t get_menu_net_wifi_page();

#endif
