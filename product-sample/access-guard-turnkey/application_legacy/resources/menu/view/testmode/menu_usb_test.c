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
#include "public.h"
#include "menu_factory_mode.h"

/*个性化与通知反馈页面*/

static lv_obj_t *menu_usb_test_page_obj = NULL;
static lv_obj_t *bottom_panel_page_obj = NULL;
//static lv_obj_t *disp_btn_panel = NULL;
static lv_obj_t * show_label = NULL;

static lv_task_t * task = NULL;

static lv_style_t disp_panel_style;
static lv_style_t disp_btn_panel_style;
static lv_style_t disp_usb_panel_style;
#define LAUNCHER_MAIN_OPACITY (LV_OPA_TRANSP)
#define ALTERNATE_UI_OPACITY (LV_OPA_50)


static int usbflag = 0;

static void menu_usb_test_create();
static void menu_usb_test_destroy();
static void menu_usb_test_show();
static void menu_usb_test_hide();

static menu_page_t menu_usb_test_page = {
    .onCreate = menu_usb_test_create,
    .onDestroy = menu_usb_test_destroy,
    .show = menu_usb_test_show,
    .hide = menu_usb_test_hide,
    .back = menuPageBack
};

menu_page_t get_menu_usb_test_page()
{
    return menu_usb_test_page;
}

void get_usb_info_task(lv_task_t * task)
{
	char dev[33] = {0};
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	//char mac[6] = {0x01,0x02,0x03,0x04,0x05,0x06};

	//set_sn("1234567891",10);
	//set_mac(mac, 6);
	//get_mac_show();

	if(CheckUsbExist(dev) == 1)
	{
		printf("[%s,%d]\n",__FUNCTION__,__LINE__);
		lv_label_set_text(show_label, _("check udisk"));
		usbflag = 1;
	}
	else
	{
		if(1 == usbflag)
		{
			lv_label_set_text(show_label, _("udisk out"));
			usbflag = 0;
		}
	}
}


/*事件回调处理函数*/
static void change_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	
	if (LV_EVENT_RELEASED == event)
    {
        printf("back_btn_event_cb.\n");
		
        //menuPageBack();
    }
}

static void pass_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {

		if (menu_tree_isEmpty())
                return;
		if(get_test_mode_page_flag() == 1)
		{
			menu_btn_flag[MENU_ID_USB] = 0;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_key_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
		}
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
			menu_btn_flag[MENU_ID_USB] = 1;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
			menu_tree_push(get_menu_key_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
		}
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
		lv_style_set_text_color(&disp_btn_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);

		lv_style_init(&disp_usb_panel_style);
		lv_style_set_bg_opa(&disp_usb_panel_style, LV_STATE_DEFAULT, LAUNCHER_MAIN_OPACITY);
		lv_style_set_opa_scale(&disp_usb_panel_style, LV_STATE_DEFAULT, ALTERNATE_UI_OPACITY);

		
        styleInited = true;
    }
}

/*页面内部layout*/
static void create_top_panel(lv_obj_t * par)
{
    lv_obj_t *top_panel = menu_page_create_test_mode_top_panel(par);

    lv_obj_t *page_title = lv_label_create(top_panel, NULL);
    lv_label_set_text(page_title, _("USB test"));
    lv_obj_align(page_title, NULL, LV_ALIGN_CENTER, -20, 0);
    lv_obj_add_style(page_title, LV_LABEL_PART_MAIN, menu_page_title_style());
}

#if 0
static lv_obj_t * create_little_btn_panel(lv_obj_t * par)
{
    lv_obj_t *disp_btn_panel = lv_cont_create(par, NULL);
	lv_obj_set_event_cb(disp_btn_panel, change_btn_event_cb);
    lv_obj_set_size(disp_btn_panel, 60, 60);
    lv_obj_set_pos(disp_btn_panel, 50, 50);
	lv_obj_add_style(disp_btn_panel, LV_CONT_PART_MAIN, &disp_btn_panel_style);
    lv_cont_set_layout(disp_btn_panel, LV_LAYOUT_CENTER);
	
	return disp_btn_panel;
}
#endif

static void create_disp_panel(lv_obj_t * par)
{
    //lv_obj_t *disp_panel = menu_page_create_disp_panel(par);
	//lv_obj_t *disp_panel = lv_cont_create(par, back_btn_event_cb);
	lv_obj_t *disp_panel = lv_cont_create(par, NULL);
	lv_obj_set_event_cb(disp_panel, change_btn_event_cb);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_obj_set_pos(disp_panel, 0, TOP_TEST_MODE_PANEL_H);
	lv_obj_add_style(disp_panel, LV_CONT_PART_MAIN, &disp_panel_style);
    lv_cont_set_layout(disp_panel, LV_LAYOUT_CENTER);
	//_lv_obj_set_style_local_color(lv_obj_t * obj, uint8_t part, lv_style_property_t prop, lv_color_t color)

	show_label = lv_label_create(disp_panel, NULL);
    //lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
    lv_obj_add_style(show_label, LV_STATE_DEFAULT, &disp_panel_style);
	//lv_obj_set_size(show_label, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_label_set_text(show_label, _("input udisk"));
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(show_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	
}

/// 页面生命周期函数 start
static void menu_usb_test_create()
{
	static uint32_t user_data = 10;
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
    _style_init();
	//66:C9:74:5F:FC:2F
	//char mac[6] = {0x66,0xC9,0x74,0x5F,0xF7,0x07};

	//set_sn("12345678901234567890",20);
	//set_mac(mac, 6);

	task = lv_task_create(get_usb_info_task, 5000, LV_TASK_PRIO_MID, &user_data);
	
    menu_usb_test_page_obj = lv_cont_create(get_test_mode_page_flag()==1 ? lv_scr_menu_test_mode():lv_scr_menu_factory_mode(), NULL);
    lv_obj_add_style(menu_usb_test_page_obj, LV_CONT_PART_MAIN, menu_page_test_mode_title_style());
    lv_obj_set_size(menu_usb_test_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	
	create_top_panel(menu_usb_test_page_obj);
    create_disp_panel(menu_usb_test_page_obj);
    
	bottom_panel_page_obj = create_bottom_2btn_panel(menu_usb_test_page_obj, pass_btn_event_cb, error_btn_event_cb);
	//lv_obj_set_hidden(bottom_panel_page_obj, false);
	lv_obj_set_hidden(bottom_panel_page_obj, false);
	//create_bottom_2btn_panel(menu_lcd_test_page_obj, pass_btn_event_cb, error_btn_event_cb);

    lv_obj_set_hidden(menu_usb_test_page_obj, true);
}

lv_obj_t * lv_scr_menu_usb_mode()
{
    return menu_usb_test_page_obj;
}

static void menu_usb_test_destroy()
{
    if (menu_usb_test_page_obj)
    {
        lv_obj_del(menu_usb_test_page_obj);
        menu_usb_test_page_obj = NULL;
    }
}

static void menu_usb_test_show()
{
    if (menu_usb_test_page_obj)
    {
        lv_obj_set_hidden(menu_usb_test_page_obj, false);
    }
}

static void menu_usb_test_hide()
{
    if (menu_usb_test_page_obj)
    {
        lv_obj_set_hidden(menu_usb_test_page_obj, true);
    }
	lv_task_del(task);
}
/// 页面生命周期函数 end


