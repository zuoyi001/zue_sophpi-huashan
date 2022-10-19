#include "lvgl/lvgl.h"
#include "menu_setting.h"
#include "launcher.h"
#include "menu_common.h"
#include "menu_face_lib.h"
#include "menu_face_demo.h"
#include "menu_device_manage.h"
#include "menu_device_activate.h"
#include "menu_access_control.h"
#include "menu_notification.h"
#include "menu_record_manage.h"
#include "menu_device_info.h"
#include "menu_net.h"
#include "menu_net_eth.h"
#include "menu_net_wifi.h"
#include "menu_net_web.h"
#include "lv_i18n.h"
#include "menu.h"
#include "resource.h"
#include "menu_page.h"
#include "app_utils.h"

static void netMenuPageCreate(void);
static void netMenuPageDestroy(void);
static void netMenu_page_show();
static void netMenu_page_hide();

static lv_obj_t * netMenuPage_cont = NULL;

static menu_page_t netMenuPage = {
    .onCreate = netMenuPageCreate,
    .onDestroy = netMenuPageDestroy,
    .show = netMenu_page_show,
    .hide = netMenu_page_hide,
    .back = menuPageBack
};

menu_page_t get_net_menu_page()
{
    return netMenuPage;
}

static void back_btn_event_cb(lv_obj_t *obj, lv_event_t event)
{
    switch (event) {
        case LV_EVENT_RELEASED:
            menuPageBack();
            break;
        default:
            break;
    }
}

static void netMenu_page_navi_cb(lv_obj_t * obj, lv_event_t event)
{
    intptr_t page_id = (intptr_t) lv_obj_get_user_data(obj);

    switch (event)
	{
	case LV_EVENT_RELEASED:
    {
        if (page_id <= MENU_ID_NET_INVALID || page_id >= MENU_ID_NET_END)
            break;
        
        switch (page_id)
        {
        case MENU_ID_NET_ETH:
            printf("show DEV_ACT.\n");
            if (menu_tree_isEmpty())
                break;
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_net_eth_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
            break;
        case MENU_ID_NET_WLAN:
            printf("show access control page.\n");
            if (menu_tree_isEmpty())
                break;
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_net_wifi_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
            break;
        case MENU_ID_NET_WEB:
            printf("show device manage.\n");
            if (menu_tree_isEmpty())
                break;
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_net_web_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
            break;
        default:
            break;
        }
		break;
    }
	default:
		break;
	}
}

// static lv_obj_t *dev_act_label;
// static lv_obj_t *mode_switch_label;
static lv_obj_t *eth_set_label;
static lv_obj_t *wanl_set_label;
static lv_obj_t *web_set_label;


// static lv_obj_t *lang_label;
static lv_obj_t *title;

static lv_style_t style_sb;
static lv_style_t btn_label_style;
static bool style_inited = false;
static void _style_init()
{
    if (!style_inited)
    {
        lv_style_init(&style_sb);
        lv_style_set_bg_color(&style_sb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_bg_grad_color(&style_sb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_border_color(&style_sb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_border_width(&style_sb, LV_STATE_DEFAULT, 1);
        lv_style_set_border_opa(&style_sb, LV_STATE_DEFAULT, LV_OPA_70);
        lv_style_set_bg_opa(&style_sb, LV_STATE_DEFAULT, LV_OPA_60);
        lv_style_set_pad_right(&style_sb, LV_STATE_DEFAULT, 3);
        lv_style_set_pad_bottom(&style_sb, LV_STATE_DEFAULT, 3);
        lv_style_set_pad_inner(&style_sb, LV_STATE_DEFAULT, 8);

        lv_style_init(&btn_label_style);
        lv_style_set_text_font(&btn_label_style, LV_STATE_DEFAULT, &stsong_28);
        style_inited = true;
    }
}

static void create_top_panel(lv_obj_t *par)
{
    lv_obj_t * top_panel = menu_page_create_top_panel(par, back_btn_event_cb);

    title = lv_label_create(top_panel, NULL);
    lv_obj_add_style(title, LV_STATE_DEFAULT, menu_page_title_style());
    lv_label_set_text(title, _("Net Manage"));
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(title, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(title, true);
}

static void create_menu_list(lv_obj_t *par)
{
    //lv_obj_t * back_btn;
    lv_obj_t * eth_set;
    lv_obj_t * wanl_set;
    lv_obj_t * web_set;
 

    lv_obj_t * netMenuPage_obj = lv_list_create(par, NULL);
    lv_obj_add_style(netMenuPage_obj, LV_PAGE_PART_SCROLLABLE, &style_sb);
    lv_obj_set_pos(netMenuPage_obj, 0, TOP_PANEL_H);
    lv_obj_set_size(netMenuPage_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX - TOP_PANEL_H);
    lv_page_set_edge_flash(netMenuPage_obj, true);
    lv_page_set_scrollbar_mode(netMenuPage_obj, LV_SCROLLBAR_MODE_AUTO);
    lv_page_set_scrl_layout(netMenuPage_obj, LV_LAYOUT_COLUMN_MID);

    eth_set = lv_list_add_btn(netMenuPage_obj, RES_IMG_ICON_SBSZ1, _("Ethernet Set"));
    eth_set_label = lv_list_get_btn_label(eth_set);
    lv_obj_set_height(eth_set_label, 40);
    lv_obj_add_style(eth_set_label, LV_LABEL_PART_MAIN, &btn_label_style);
    lv_btn_set_fit(eth_set, LV_FIT_TIGHT);
    lv_obj_set_user_data(eth_set, (lv_obj_user_data_t)MENU_ID_NET_ETH); //it may has bugs? liang.wang 2021-03-15
    lv_obj_set_event_cb(eth_set, netMenu_page_navi_cb);

    wanl_set = lv_list_add_btn(netMenuPage_obj, RES_IMG_ICON_SBSZ1, _("WIFI Set"));
    wanl_set_label = lv_list_get_btn_label(wanl_set);
    lv_obj_set_height(wanl_set_label, 40);
    lv_obj_add_style(wanl_set_label, LV_LABEL_PART_MAIN, &btn_label_style);
    lv_btn_set_fit(wanl_set, LV_FIT_TIGHT);
    lv_obj_set_user_data(wanl_set, (lv_obj_user_data_t)MENU_ID_NET_WLAN); //it may has bugs? liang.wang 2021-03-15
    lv_obj_set_event_cb(wanl_set, netMenu_page_navi_cb);

	web_set = lv_list_add_btn(netMenuPage_obj, RES_IMG_ICON_SBSZ1, _("Web Set"));
    web_set_label = lv_list_get_btn_label(web_set);
    lv_obj_set_height(web_set_label, 40);
    lv_obj_add_style(web_set_label, LV_LABEL_PART_MAIN, &btn_label_style);
    lv_btn_set_fit(web_set, LV_FIT_TIGHT);
    lv_obj_set_user_data(web_set, (lv_obj_user_data_t)MENU_ID_NET_WEB); //it may has bugs? liang.wang 2021-03-15
    lv_obj_set_event_cb(web_set, netMenu_page_navi_cb);
}

static void netMenuPageCreate(void)
{
    _style_init();

    netMenuPage_cont = lv_cont_create(lv_scr_menu(), NULL);
    lv_obj_set_size(netMenuPage_cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_cont_set_fit(netMenuPage_cont, LV_FIT_NONE);
    lv_cont_set_layout(netMenuPage_cont, LV_LAYOUT_OFF);

    create_top_panel(netMenuPage_cont);
    create_menu_list(netMenuPage_cont);
}

static void netMenuPageDestroy(void)
{
    lv_obj_del(netMenuPage_cont);
    netMenuPage_cont = NULL;
    eth_set_label = NULL;
    wanl_set_label = NULL;
    web_set_label = NULL;
}

static void netMenu_page_show()
{
    if (netMenuPage_cont == NULL) {
        return;
    }

    lv_obj_set_hidden(netMenuPage_cont, false);
}

static void netMenu_page_hide()
{
    if (netMenuPage_cont == NULL) {
        return;
    }

    lv_obj_set_hidden(netMenuPage_cont, true);
}

