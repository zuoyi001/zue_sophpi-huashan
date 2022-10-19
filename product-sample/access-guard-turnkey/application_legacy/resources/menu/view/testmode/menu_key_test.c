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
#include "menu_test_result.h"
#include "factory.h"
#include "app_comp.h"
#include "net.h"
#include "cvi_hal_gpio.h"
#include "public.h"
#include "app_peripheral.h"
#include "menu_factory_mode.h"

/*个性化与通知反馈页面*/

static lv_obj_t *menu_key_test_page_obj = NULL;
static lv_obj_t *bottom_panel_page_obj = NULL;
//static lv_obj_t *disp_btn_panel = NULL;

static lv_obj_t * show_doorkey_label = NULL;
static lv_obj_t * show_doormag_label = NULL;
static lv_obj_t * show_rm_label = NULL;
static lv_obj_t * show_amarm_label = NULL;


static lv_task_t * task = NULL;

static lv_style_t disp_panel_style;
static lv_style_t disp_btn_panel_style;
static lv_style_t disp_key_panel_style;
#define LAUNCHER_MAIN_OPACITY (LV_OPA_TRANSP)
#define ALTERNATE_UI_OPACITY (LV_OPA_50)


int doorkeyflag = 0;
int doormagflag = 0;
int rmflag = 0;
int amarmflag = 0;

static void menu_key_test_create();
static void menu_key_test_destroy();
static void menu_key_test_show();
static void menu_key_test_hide();

static menu_page_t menu_key_test_page = {
    .onCreate = menu_key_test_create,
    .onDestroy = menu_key_test_destroy,
    .show = menu_key_test_show,
    .hide = menu_key_test_hide,
    .back = menuPageBack
};

menu_page_t get_menu_key_test_page()
{
    return menu_key_test_page;
}

void get_key_info_task(lv_task_t * task)
{
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	int ret = 1;
    CVI_HAL_GPIO_VALUE_E value;
	if(0 == doorkeyflag)
	{
	    ret = CVI_HAL_GpioGetValue(DOORKEY_GPIO, &value);
		if((0 == ret) && (CVI_HAL_GPIO_VALUE_H == value))
		{
			doorkeyflag = 1;
			lv_obj_add_style(show_doorkey_label, LV_STATE_DEFAULT, &disp_btn_panel_style);
			lv_label_set_text(show_doorkey_label, _("doorkey test pass"));
		}
		printf("[DOORKEY_GPIO value:%d %s,%d]\n",value,__FUNCTION__,__LINE__);
	}
	if(0 == doormagflag)
	{
		ret = CVI_HAL_GpioGetValue(DOORMAG_GPIO, &value);
		if((0 == ret) && (CVI_HAL_GPIO_VALUE_H == value))
		{
			doormagflag = 1;
			lv_obj_add_style(show_doormag_label, LV_STATE_DEFAULT, &disp_btn_panel_style);
			lv_label_set_text(show_doormag_label, _("doormag test pass"));
		}
		printf("[DOORMAG_GPIO value:%d %s,%d]\n",value,__FUNCTION__,__LINE__);
	}
	if(0 == rmflag)
	{
		ret = CVI_HAL_GpioGetValue(RM_GPIO, &value);
		if((0 == ret) && (CVI_HAL_GPIO_VALUE_L == value))
		{
			rmflag = 1;
			lv_obj_add_style(show_rm_label, LV_STATE_DEFAULT, &disp_btn_panel_style);
			lv_label_set_text(show_rm_label, _("rm test pass"));
		}
		printf("[RM_GPIO value:%d %s,%d]\n",value,__FUNCTION__,__LINE__);
	}
	if(0 == amarmflag)
	{
		ret = CVI_HAL_GpioGetValue(ALARM_GPIO, &value);
		printf("[ALARM_GPIO value:%d ret:%d %s,%d]\n",value,ret,__FUNCTION__,__LINE__);
		if((0 == ret) && (CVI_HAL_GPIO_VALUE_L == value))
		{
			amarmflag = 1;
			lv_obj_add_style(show_amarm_label, LV_STATE_DEFAULT, &disp_btn_panel_style);
			lv_label_set_text(show_amarm_label, _("Alarm test pass"));
		}
	}
}
static void pass_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
		if (menu_tree_isEmpty())
                return;
			if(get_test_mode_page_flag() == 1)
			{
				menu_btn_flag[MENU_ID_KEY] = 0;
				menu_test_mode_PageBack();
			}
			else
			{
				menu_tree_get_top().hide();
	            menu_tree_push(get_menu_test_result_page());
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
			menu_btn_flag[MENU_ID_KEY] = 1;
			menu_test_mode_PageBack();
		}else
		{
			menu_tree_get_top().hide();
            menu_tree_push(get_menu_test_result_page());
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
		lv_style_set_text_color(&disp_btn_panel_style, LV_STATE_DEFAULT, LV_COLOR_GREEN);

		lv_style_init(&disp_key_panel_style);
		lv_style_set_bg_opa(&disp_key_panel_style, LV_STATE_DEFAULT, LAUNCHER_MAIN_OPACITY);
		lv_style_set_opa_scale(&disp_key_panel_style, LV_STATE_DEFAULT, ALTERNATE_UI_OPACITY);

        styleInited = true;
    }
}

/*页面内部layout*/
static void create_top_panel(lv_obj_t * par)
{
    lv_obj_t *top_panel = menu_page_create_test_mode_top_panel(par);

    lv_obj_t *page_title = lv_label_create(top_panel, NULL);
    lv_label_set_text(page_title, _("Key test"));
    lv_obj_align(page_title, NULL, LV_ALIGN_CENTER, -20, 0);
    lv_obj_add_style(page_title, LV_LABEL_PART_MAIN, menu_page_title_style());
}

static void create_disp_panel(lv_obj_t * par)
{
	lv_obj_t *disp_panel = lv_cont_create(par, NULL);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_obj_set_pos(disp_panel, 0, TOP_TEST_MODE_PANEL_H);
	lv_obj_add_style(disp_panel, LV_CONT_PART_MAIN, &disp_panel_style);
    //lv_cont_set_layout(disp_panel, LV_LAYOUT_CENTER);
	//_lv_obj_set_style_local_color(lv_obj_t * obj, uint8_t part, lv_style_property_t prop, lv_color_t color)

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
	
}

/// 页面生命周期函数 start
static void menu_key_test_create()
{
	static uint32_t user_data = 10;
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
    _style_init();
	
	doorkeyflag = 0;
	doormagflag = 0;
	rmflag = 0;
	amarmflag = 0;

	task = lv_task_create(get_key_info_task, 500, LV_TASK_PRIO_MID, &user_data);
	
    menu_key_test_page_obj = lv_cont_create(get_test_mode_page_flag()==1 ? lv_scr_menu_test_mode():lv_scr_menu_factory_mode(), NULL);
    lv_obj_add_style(menu_key_test_page_obj, LV_CONT_PART_MAIN, menu_page_test_mode_title_style());
    lv_obj_set_size(menu_key_test_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	
	create_top_panel(menu_key_test_page_obj);
    create_disp_panel(menu_key_test_page_obj);
    
	bottom_panel_page_obj = create_bottom_2btn_panel(menu_key_test_page_obj, pass_btn_event_cb, error_btn_event_cb);
	//lv_obj_set_hidden(bottom_panel_page_obj, false);
	lv_obj_set_hidden(bottom_panel_page_obj, false);
	//create_bottom_2btn_panel(menu_lcd_test_page_obj, pass_btn_event_cb, error_btn_event_cb);

    lv_obj_set_hidden(menu_key_test_page_obj, true);
}

lv_obj_t * lv_scr_menu_key_mode()
{
    return menu_key_test_page_obj;
}

static void menu_key_test_destroy()
{
    if (menu_key_test_page_obj)
    {
        lv_obj_del(menu_key_test_page_obj);
        menu_key_test_page_obj = NULL;
    }
}

static void menu_key_test_show()
{
    if (menu_key_test_page_obj)
    {
        lv_obj_set_hidden(menu_key_test_page_obj, false);
    }
}

static void menu_key_test_hide()
{
    if (menu_key_test_page_obj)
    {
        lv_obj_set_hidden(menu_key_test_page_obj, true);
    }
	lv_task_del(task);
}
/// 页面生命周期函数 end


