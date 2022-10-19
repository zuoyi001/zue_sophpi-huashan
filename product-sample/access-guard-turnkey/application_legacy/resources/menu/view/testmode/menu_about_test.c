#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

#include "menu.h"
#include "menu_common.h"
#include "menu_page.h"
#include "resource.h"
#include "lv_i18n.h"
#include "menu_key_test.h"
#include "menu_test_mode.h"
#include "menu_usb_test.h"
#include "factory.h"
#include "app_comp.h"
#include "net.h"
#include "cvi_hal_gpio.h"
#include "public.h"
#include "app_peripheral.h"
#include "menu_factory_mode.h"
#include "sys.h"


/*个性化与通知反馈页面*/

static lv_obj_t *menu_about_test_page_obj = NULL;
static lv_obj_t *bottom_panel_page_obj = NULL;
//static lv_obj_t *disp_btn_panel = NULL;
//static lv_obj_t * show_doorkey_label = NULL;
//static lv_obj_t * show_doormag_label = NULL;
//static lv_obj_t * show_rm_label = NULL;
//static lv_obj_t * show_amarm_label = NULL;

static lv_style_t disp_panel_style;
static lv_style_t disp_btn_panel_style;
static lv_style_t disp_about_panel_style;
#define LAUNCHER_MAIN_OPACITY (LV_OPA_TRANSP)
#define ALTERNATE_UI_OPACITY (LV_OPA_50)
static void menu_about_test_create();
static void menu_about_test_destroy();
static void menu_about_test_show();
static void menu_about_test_hide();

static menu_page_t menu_about_test_page = {
    .onCreate = menu_about_test_create,
    .onDestroy = menu_about_test_destroy,
    .show = menu_about_test_show,
    .hide = menu_about_test_hide,
    .back = menuPageBack
};

menu_page_t get_menu_about_test_page()
{
    return menu_about_test_page;
}

static void pass_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
		if (menu_tree_isEmpty())
                return;
			if(get_test_mode_page_flag() == 1)
			{
				menu_btn_flag[MENU_ID_ABOUT] = 0;
				menu_test_mode_PageBack();
			}
			/*
            menu_tree_get_top().hide();
            //menu_tree_push(get_menu_rs485_wg_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
			*/
    }
}

static void error_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	
			
    if (LV_EVENT_RELEASED == event) {
        printf("[%s,%d]\n",__FUNCTION__,__LINE__);

		if (menu_tree_isEmpty())
                return;
		if(get_test_mode_page_flag() == 1)
		{
			menu_btn_flag[MENU_ID_ABOUT] = 1;
			menu_test_mode_PageBack();
		}
		/*
            menu_tree_get_top().hide();
			//menu_tree_push(get_menu_rs485_wg_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
			*/
    }
}


/*内部样式*/
static bool styleInited = false;
static void _style_init()
{
    if (!styleInited)
    {
    	lv_style_init(&disp_panel_style);
		lv_style_set_text_font(&disp_panel_style, LV_STATE_DEFAULT, &stsong_28);
		lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
		lv_style_set_text_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);

		lv_style_init(&disp_btn_panel_style);
		lv_style_set_text_font(&disp_btn_panel_style, LV_STATE_DEFAULT, &stsong_28);
		lv_style_set_bg_color(&disp_btn_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLUE);
		lv_style_set_bg_color(&disp_btn_panel_style, LV_STATE_PRESSED, LV_COLOR_RED);
		lv_style_set_text_color(&disp_btn_panel_style, LV_STATE_DEFAULT, LV_COLOR_GREEN);

		lv_style_init(&disp_about_panel_style);
		lv_style_set_bg_opa(&disp_about_panel_style, LV_STATE_DEFAULT, LAUNCHER_MAIN_OPACITY);
		lv_style_set_opa_scale(&disp_about_panel_style, LV_STATE_DEFAULT, ALTERNATE_UI_OPACITY);

        styleInited = true;
    }
}

/*页面内部layout*/
static void create_top_panel(lv_obj_t * par)
{
    lv_obj_t *top_panel = menu_page_create_test_mode_top_panel(par);

    lv_obj_t *page_title = lv_label_create(top_panel, NULL);
    lv_label_set_text(page_title, _("About Device"));
    lv_obj_align(page_title, NULL, LV_ALIGN_CENTER, -20, 0);
    lv_obj_add_style(page_title, LV_LABEL_PART_MAIN, menu_page_title_style());
}

static void create_disp_panel(lv_obj_t * par)
{
	char tempBuf[64]={0};
	char card[] = "eth0";
	char tmpBuf2[32] = {0};
	lv_obj_t *disp_panel = lv_cont_create(par, NULL);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_obj_set_pos(disp_panel, 0, TOP_TEST_MODE_PANEL_H);
	lv_obj_add_style(disp_panel, LV_CONT_PART_MAIN, &disp_panel_style);
    //lv_cont_set_layout(disp_panel, LV_LAYOUT_CENTER);
	//_lv_obj_set_style_local_color(lv_obj_t * obj, uint8_t part, lv_style_property_t prop, lv_color_t color)

	lv_obj_t * vs_label = lv_label_create(disp_panel, NULL);
	//lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
	lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
	memset(tempBuf, 0x00, sizeof(tempBuf));
	sprintf(tempBuf,"%s:  %s",_("Version"),VERSION_NAME);
	lv_label_set_text(vs_label, tempBuf);
	//lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(vs_label, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 100);	

	lv_obj_t * sn_label = lv_label_create(disp_panel, NULL);
	//lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
	lv_obj_add_style(sn_label, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
	memset(tempBuf, 0x00, sizeof(tempBuf));
	sprintf(tempBuf,"%s:  %s",_("Sn"),get_sn());
	lv_label_set_text(sn_label, tempBuf);
	//lv_label_set_offset_y(mac_label, 100);
	lv_label_set_align(sn_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(sn_label, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 150);

	lv_obj_t * mac_label = lv_label_create(disp_panel, NULL);
	//lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
	lv_obj_add_style(mac_label, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
	memset(tempBuf, 0x00, sizeof(tempBuf));
    get_mac(card, tmpBuf2);
	sprintf(tempBuf,"%s:  %s",_("Mac"),tmpBuf2);
	lv_label_set_text(mac_label, tempBuf);
	//lv_label_set_offset_y(mac_label, 100);
	lv_label_set_align(mac_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(mac_label, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 200);

	lv_obj_t * eth0_ip_label = lv_label_create(disp_panel, NULL);
	//lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
	lv_obj_add_style(eth0_ip_label, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
	memset(tempBuf, 0x00, sizeof(tempBuf));
	memset(tmpBuf2, 0x00, sizeof(tmpBuf2));
	get_network_ip_address("eth0", tmpBuf2);
	sprintf(tempBuf,"%s:  %s",_("Ip address"),tmpBuf2);
	lv_label_set_text(eth0_ip_label, tempBuf);
	//lv_label_set_offset_y(mac_label, 100);
	lv_label_set_align(eth0_ip_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(eth0_ip_label, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 250);


	lv_obj_t * wifi_mac_label = lv_label_create(disp_panel, NULL);
	//lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
	lv_obj_add_style(wifi_mac_label, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
	memset(tempBuf, 0x00, sizeof(tempBuf));
    get_mac("wlan0", tmpBuf2);
	sprintf(tempBuf,"%s:  %s",_("WIFI Mac"),tmpBuf2);
	lv_label_set_text(wifi_mac_label, tempBuf);
	//lv_label_set_offset_y(mac_label, 100);
	lv_label_set_align(wifi_mac_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(wifi_mac_label, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 300);

	lv_obj_t * wlan0_ip_label = lv_label_create(disp_panel, NULL);
	//lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
	lv_obj_add_style(wlan0_ip_label, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
	memset(tempBuf, 0x00, sizeof(tempBuf));
	memset(tmpBuf2, 0x00, sizeof(tmpBuf2));
    get_network_ip_address("wlan0", tmpBuf2);
	sprintf(tempBuf,"%s:  %s",_("wifi ip"),tmpBuf2);
	lv_label_set_text(wlan0_ip_label, tempBuf);
	//lv_label_set_offset_y(mac_label, 100);
	lv_label_set_align(wlan0_ip_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(wlan0_ip_label, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 350);

/*
	show_doorkey_label = lv_label_create(disp_panel, NULL);
    lv_obj_add_style(show_doorkey_label, LV_STATE_DEFAULT, &disp_panel_style);
	//lv_obj_set_size(show_doorkey_label, 500, 100);
	//lv_obj_set_size(show_label, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_label_set_text(show_doorkey_label, _("doorkey testing"));
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(show_doorkey_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 400);

	show_doormag_label = lv_label_create(disp_panel, NULL);
    lv_obj_add_style(show_doormag_label, LV_STATE_DEFAULT, &disp_panel_style);
	//lv_obj_set_pos(show_doormag_label, 300, 650);
	//lv_obj_set_size(show_label, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_label_set_text(show_doormag_label, _("doormag testing"));
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(show_doormag_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 500);

	show_rm_label = lv_label_create(disp_panel, NULL);
    lv_obj_add_style(show_rm_label, LV_STATE_DEFAULT, &disp_panel_style);
	//lv_obj_set_size(show_label, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_label_set_text(show_rm_label, _("rm testing"));
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(show_rm_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 600);

	show_amarm_label = lv_label_create(disp_panel, NULL);
    lv_obj_add_style(show_amarm_label, LV_STATE_DEFAULT, &disp_panel_style);
	//lv_obj_set_size(show_label, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_label_set_text(show_amarm_label, _("Alarm testing"));
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(show_amarm_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 700);
	*/
	
}

/// 页面生命周期函数 start
static void menu_about_test_create()
{
	//static uint32_t user_data = 10;
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
    _style_init();
	
    menu_about_test_page_obj = lv_cont_create(get_test_mode_page_flag()==1 ? lv_scr_menu_test_mode():lv_scr_menu_factory_mode(), NULL);
    lv_obj_add_style(menu_about_test_page_obj, LV_CONT_PART_MAIN, menu_page_test_mode_title_style());
    lv_obj_set_size(menu_about_test_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	
	create_top_panel(menu_about_test_page_obj);
    create_disp_panel(menu_about_test_page_obj);
    
	bottom_panel_page_obj = create_bottom_2btn_panel(menu_about_test_page_obj, pass_btn_event_cb, error_btn_event_cb);
	//lv_obj_set_hidden(bottom_panel_page_obj, false);
	lv_obj_set_hidden(bottom_panel_page_obj, false);
	//create_bottom_2btn_panel(menu_lcd_test_page_obj, pass_btn_event_cb, error_btn_event_cb);

    lv_obj_set_hidden(menu_about_test_page_obj, true);
}

lv_obj_t * lv_scr_menu_about_mode()
{
    return menu_about_test_page_obj;
}

static void menu_about_test_destroy()
{
    if (menu_about_test_page_obj)
    {
        lv_obj_del(menu_about_test_page_obj);
        menu_about_test_page_obj = NULL;
    }
}

static void menu_about_test_show()
{
    if (menu_about_test_page_obj)
    {
        lv_obj_set_hidden(menu_about_test_page_obj, false);
    }
}

static void menu_about_test_hide()
{
    if (menu_about_test_page_obj)
    {
        lv_obj_set_hidden(menu_about_test_page_obj, true);
    }
}
/// 页面生命周期函数 end


