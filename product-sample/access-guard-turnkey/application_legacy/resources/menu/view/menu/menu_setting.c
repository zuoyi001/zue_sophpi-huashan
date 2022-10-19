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
#include "lv_i18n.h"
#include "menu.h"
#include "resource.h"
#include "menu_page.h"
#include "app_utils.h"

static void mainMenuPageCreate(void);
static void mainMenuPageDestroy(void);
static void mainMenu_page_show();
static void mainMenu_page_hide();

static lv_obj_t * scr_menu;
static lv_obj_t * mainMenuPage_cont = NULL;

static menu_page_t mainMenuPage = {
    .onCreate = mainMenuPageCreate,
    .onDestroy = mainMenuPageDestroy,
    .show = mainMenu_page_show,
    .hide = mainMenu_page_hide,
    .back = menuPageBack
};

static void mainMenu_back_event_cb(lv_obj_t * obj, lv_event_t event)
{
	switch (event)
	{
	case LV_EVENT_RELEASED:
        printf("mainMenu_back_event_cb.\n");
        menu_scr_hide();
        menuPageBack();
		break;
	default:
		break;
	}
}

static void mainMenu_page_navi_cb(lv_obj_t * obj, lv_event_t event)
{
    intptr_t page_id = (intptr_t) lv_obj_get_user_data(obj);

    switch (event)
	{
	case LV_EVENT_RELEASED:
    {
        if (page_id <= MENU_ID_INVALID || page_id >= MENU_ID_END)
            break;
        
        switch (page_id)
        {
		case MENU_ID_FACE_LIB:
            printf("show Face lib ui.\n");
            if (menu_tree_isEmpty())
                break;
            menu_tree_get_top().hide();
            menu_tree_push(getFaceLibPageSt());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
            break;
		case MENU_ID_RECORD_MANAGE:
            printf("show profile notify ui.\n");
            if (menu_tree_isEmpty())
                break;
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_record_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
            break;
        case MENU_ID_ACCESS_CTRL:
            printf("show access control page.\n");
            if (menu_tree_isEmpty())
                break;
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_access_control_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
            break;
        case MENU_ID_DEV_MANAGE:
            printf("show device manage.\n");
            if (menu_tree_isEmpty())
                break;
            menu_tree_get_top().hide();
            menu_tree_push(get_dev_manage_menu_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
            break;
		case MENU_ID_NET_MANAGE:
            printf("show profile notify ui.\n");
            if (menu_tree_isEmpty())
                break;
            menu_tree_get_top().hide();
            menu_tree_push(get_net_menu_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
            break;
		case MENU_ID_DEV_INFO:
	        printf("show profile notify ui.\n");
	        if (menu_tree_isEmpty())
	            break;
	        menu_tree_get_top().hide();
	        menu_tree_push(get_dev_info_menu_page());
	        menu_tree_get_top().onCreate();
	        menu_tree_get_top().show();
	        break;
	/*
        case MENU_ID_DEV_ACTIVATE:
            printf("show DEV_ACT.\n");
            if (menu_tree_isEmpty())
                break;
            menu_tree_get_top().hide();
            menu_tree_push(getDevActMenuPage());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
            break;
			*/
			/*
		case MENU_ID_PROFILE_NOTIFY:
            printf("show profile notify ui.\n");
            if (menu_tree_isEmpty())
                break;
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_notification_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
            break;
			*/
        default:
            break;
        }
		break;
    }
	default:
		break;
	}
}

#if 0
static void mainMenu_lang_switch_cb(lv_obj_t * obj, lv_event_t event)
{
    static int lang_toggle = 0;
    if (LV_EVENT_RELEASED == event) {
        if (lang_toggle == 0) {
            lang_toggle = 1;
            lv_i18n_set_locale("en-US");
            mainMenu_retranslateUI();
        } else if (lang_toggle == 1) {
            lang_toggle = 0;
            lv_i18n_set_locale("zh-CN");
            mainMenu_retranslateUI();
        }
    }
}
#endif

void menu_scr_show()
{
    if (scr_menu) {
        lv_scr_load(scr_menu);
        menu_tree_push(mainMenuPage);
        menu_tree_get_top().onCreate();
        menu_tree_get_top().show();
    }
}

void menu_scr_hide()
{

}

// static lv_obj_t *dev_act_label;
// static lv_obj_t *mode_switch_label;
static lv_obj_t *dev_manage_label;
static lv_obj_t *dev_info_label;
static lv_obj_t *face_lib_label;
static lv_obj_t *record_manage_label;
static lv_obj_t *net_manage_label;


static lv_obj_t *access_ctrl_label;
static lv_obj_t *notification_label;
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
    lv_obj_t * top_panel = menu_page_create_top_panel(par, mainMenu_back_event_cb);

    title = lv_label_create(top_panel, NULL);
    lv_obj_add_style(title, LV_STATE_DEFAULT, menu_page_title_style());
    lv_label_set_text(title, _("Setting"));
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(title, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(title, true);
}

static void create_menu_list(lv_obj_t *par)
{
    //lv_obj_t * back_btn;
    // lv_obj_t * device_activate;
    // lv_obj_t * mode_switch;
    lv_obj_t * device_manage;
    //lv_obj_t * device_set;
    lv_obj_t * device_info;
    lv_obj_t * face_lib_item;
	lv_obj_t * record_manage_item;
	lv_obj_t * net_manage_item;
    //lv_obj_t * notification_item;
    // lv_obj_t * lang;
    lv_obj_t * access_control;

    lv_obj_t * mainMenuPage_obj = lv_list_create(par, NULL);
    lv_obj_add_style(mainMenuPage_obj, LV_PAGE_PART_SCROLLABLE, &style_sb);
    lv_obj_set_pos(mainMenuPage_obj, 0, TOP_PANEL_H);
    lv_obj_set_size(mainMenuPage_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX - TOP_PANEL_H);
    lv_page_set_edge_flash(mainMenuPage_obj, true);
    lv_page_set_scrollbar_mode(mainMenuPage_obj, LV_SCROLLBAR_MODE_AUTO);
    lv_page_set_scrl_layout(mainMenuPage_obj, LV_LAYOUT_COLUMN_MID);

	face_lib_item = lv_list_add_btn(mainMenuPage_obj, RES_IMG_ICON_RLK, _("Face Lib"));
    face_lib_label = lv_list_get_btn_label(face_lib_item);
    lv_obj_set_height(face_lib_label, 40);
    lv_obj_add_style(face_lib_label, LV_LABEL_PART_MAIN, &btn_label_style);
    lv_btn_set_fit(face_lib_item, LV_FIT_TIGHT);
    lv_obj_set_user_data(face_lib_item, (lv_obj_user_data_t)MENU_ID_FACE_LIB);
    lv_obj_set_event_cb(face_lib_item, mainMenu_page_navi_cb);

	record_manage_item = lv_list_add_btn(mainMenuPage_obj, RES_IMG_ICON_RLK, _("Record Manage"));
    record_manage_label = lv_list_get_btn_label(record_manage_item);
    lv_obj_set_height(record_manage_label, 40);
    lv_obj_add_style(record_manage_label, LV_LABEL_PART_MAIN, &btn_label_style);
    lv_btn_set_fit(record_manage_item, LV_FIT_TIGHT);
    lv_obj_set_user_data(record_manage_item, (lv_obj_user_data_t)MENU_ID_RECORD_MANAGE);
    lv_obj_set_event_cb(record_manage_item, mainMenu_page_navi_cb);

    // device_activate = lv_list_add_btn(mainMenuPage_obj, RES_IMG_ICON_SBJH, _("Device Activate"));
    // dev_act_label = lv_list_get_btn_label(device_activate);
    // lv_obj_set_height(dev_act_label, 30);
    // lv_btn_set_fit(device_activate, LV_FIT_TIGHT);
    // lv_obj_set_user_data(device_activate, MENU_ID_DEV_ACTIVATE);
    // lv_obj_set_event_cb(device_activate, mainMenu_page_navi_cb);
    // lv_obj_add_style(dev_act_label, LV_LABEL_PART_MAIN, &btn_label_style);

    // mode_switch = lv_list_add_btn(mainMenuPage_obj, RES_IMG_ICON_MSQH, _("Mode Switch"));
    // mode_switch_label = lv_list_get_btn_label(mode_switch);
    // lv_obj_set_height(mode_switch_label, 30);
    // lv_obj_add_style(mode_switch_label, LV_LABEL_PART_MAIN, &btn_label_style);
    // lv_btn_set_fit(mode_switch, LV_FIT_TIGHT);
    // lv_obj_set_user_data(mode_switch, MENU_ID_MODE_SWITCH);
    // lv_obj_set_event_cb(mode_switch, mainMenu_page_navi_cb);

    access_control = lv_list_add_btn(mainMenuPage_obj, RES_IMG_ICON_SBSZ1, _("Access Control"));
    access_ctrl_label = lv_list_get_btn_label(access_control);
    lv_obj_set_height(access_ctrl_label, 40);
    lv_obj_add_style(access_ctrl_label, LV_LABEL_PART_MAIN, &btn_label_style);
    lv_btn_set_fit(access_control, LV_FIT_TIGHT);
    lv_obj_set_user_data(access_control, (lv_obj_user_data_t)MENU_ID_ACCESS_CTRL);
    lv_obj_set_event_cb(access_control, mainMenu_page_navi_cb);

    device_manage = lv_list_add_btn(mainMenuPage_obj, RES_IMG_ICON_SBSZ1, _("Device Manage"));
    dev_manage_label = lv_list_get_btn_label(device_manage);
    lv_obj_set_height(dev_manage_label, 40);
    lv_obj_add_style(dev_manage_label, LV_LABEL_PART_MAIN, &btn_label_style);
    lv_btn_set_fit(device_manage, LV_FIT_TIGHT);
    lv_obj_set_user_data(device_manage, (lv_obj_user_data_t)MENU_ID_DEV_MANAGE);
    lv_obj_set_event_cb(device_manage, mainMenu_page_navi_cb);

	net_manage_item = lv_list_add_btn(mainMenuPage_obj, RES_IMG_ICON_RLK, _("Net Manage"));
    net_manage_label = lv_list_get_btn_label(net_manage_item);
    lv_obj_set_height(net_manage_label, 40);
    lv_obj_add_style(net_manage_label, LV_LABEL_PART_MAIN, &btn_label_style);
    lv_btn_set_fit(net_manage_item, LV_FIT_TIGHT);
    lv_obj_set_user_data(net_manage_item, (lv_obj_user_data_t)MENU_ID_NET_MANAGE);
    lv_obj_set_event_cb(net_manage_item, mainMenu_page_navi_cb);

    device_info = lv_list_add_btn(mainMenuPage_obj, RES_IMG_ICON_SBXX, _("Device Information"));
    dev_info_label = lv_list_get_btn_label(device_info);
    lv_obj_set_height(dev_info_label, 40);
    lv_obj_add_style(dev_info_label, LV_LABEL_PART_MAIN, &btn_label_style);
    lv_btn_set_fit(device_info, LV_FIT_TIGHT);
    lv_obj_set_user_data(device_info, (lv_obj_user_data_t)MENU_ID_DEV_INFO);
    lv_obj_set_event_cb(device_info, mainMenu_page_navi_cb);

/*
    notification_item = lv_list_add_btn(mainMenuPage_obj, RES_IMG_ICON_GXH, _("Profile & Notification"));
    notification_label = lv_list_get_btn_label(notification_item);
    lv_obj_set_height(notification_label, 40);
    lv_obj_add_style(notification_label, LV_LABEL_PART_MAIN, &btn_label_style);
    lv_btn_set_fit(notification_item, LV_FIT_TIGHT);
    lv_obj_set_user_data(notification_item, MENU_ID_PROFILE_NOTIFY);
    lv_obj_set_event_cb(notification_item, mainMenu_page_navi_cb);
	*/

    // lang = lv_list_add_btn(mainMenuPage_obj, RES_IMG_ICON_SBSZ, _("Lang Switch"));
    // lang_label = lv_list_get_btn_label(lang);
    // lv_obj_set_height(lang_label, 30);
    // lv_obj_add_style(lang_label, LV_LABEL_PART_MAIN, &btn_label_style);
    // lv_btn_set_fit(lang, LV_FIT_TIGHT);
    // lv_obj_set_event_cb(lang, mainMenu_lang_switch_cb);
}

static void mainMenuPageCreate(void)
{
    _style_init();

    mainMenuPage_cont = lv_cont_create(scr_menu, NULL);
    lv_obj_set_size(mainMenuPage_cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_cont_set_fit(mainMenuPage_cont, LV_FIT_NONE);
    lv_cont_set_layout(mainMenuPage_cont, LV_LAYOUT_OFF);

    create_top_panel(mainMenuPage_cont);
    create_menu_list(mainMenuPage_cont);
    
    //APP_CompSendCmd(CMD_APP_COMP_VIDEOSTOP, NULL, 0, NULL, 0);
}

static void mainMenuPageDestroy(void)
{
    lv_obj_del(mainMenuPage_cont);
    mainMenuPage_cont = NULL;
    // dev_act_label = NULL;
    // mode_switch_label = NULL;
    dev_manage_label = NULL;
    dev_info_label = NULL;
    face_lib_label = NULL;
    // lang_label = NULL;
    access_ctrl_label = NULL;
    notification_label = NULL;
	
	set_enter_menu_flag(OUT_MENU);

    APP_CompSendCmd(CMD_APP_COMP_RESTART, NULL, 0, NULL, 0);
}

static void mainMenu_page_show()
{
    if (mainMenuPage_cont == NULL) {
        return;
    }

    lv_obj_set_hidden(mainMenuPage_cont, false);
}

static void mainMenu_page_hide()
{
    if (mainMenuPage_cont == NULL) {
        return;
    }

    lv_obj_set_hidden(mainMenuPage_cont, true);
}

static bool menu_inited = false;
void menu_scr_init()
{
    if (menu_inited)
    {
        return;
    }

    scr_menu = lv_cont_create(NULL, NULL);
    static lv_style_t scr_menu_style;
    lv_style_init(&scr_menu_style);
    lv_style_set_bg_opa(&scr_menu_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_add_style(scr_menu, LV_CONT_PART_MAIN, &scr_menu_style);
    menu_inited = true;
}

void mainMenu_retranslateUI()
{
    lv_label_set_text(title, _("Setting"));
    lv_obj_realign(title);
    // lv_label_set_text(dev_act_label, _("Device Activate"));
    // lv_obj_realign(dev_act_label);
    // lv_label_set_text(mode_switch_label, _("Mode Switch"));
    // lv_obj_realign(mode_switch_label);

	lv_label_set_text(face_lib_label, _("Face Lib"));
    lv_obj_realign(face_lib_label);

	lv_label_set_text(record_manage_label, _("Record Manage"));
	lv_obj_realign(record_manage_label);

    lv_label_set_text(access_ctrl_label, _("Access Control"));
    lv_obj_realign(access_ctrl_label);
	
    lv_label_set_text(dev_manage_label, _("Device Manage"));
    lv_obj_realign(dev_manage_label);

	lv_label_set_text(net_manage_label, _("Net Manage"));
	lv_obj_realign(net_manage_label);

    lv_label_set_text(dev_info_label, _("Device Information"));
    lv_obj_realign(dev_info_label);
    
    //lv_label_set_text(notification_label, _("Profile & Notification"));
    //lv_obj_realign(notification_label);
    // lv_label_set_text(lang_label, _("Lang Switch"));
    // lv_obj_realign(lang_label);
}

lv_obj_t * lv_scr_menu()
{
    return scr_menu;
}

void menuPageBack()
{
    if (menu_tree_isEmpty())
    {
        return;
    }

    menu_tree_get_top().hide();
    menu_tree_get_top().onDestroy();
    menu_tree_pop();
    if (!menu_tree_isEmpty()) {
        menu_tree_get_top().show();
        printf("show prev page\n");
    }
    else {
        //empty menu tree, got back to launcher ui
        launcher_show();
    }
}
