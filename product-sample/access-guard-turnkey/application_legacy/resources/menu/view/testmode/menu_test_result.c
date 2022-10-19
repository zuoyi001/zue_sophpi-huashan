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


static lv_obj_t *menu_test_result_page_obj = NULL;
static lv_obj_t *bottom_panel_page_obj = NULL;
//static lv_obj_t *disp_btn_panel = NULL;

static lv_style_t disp_panel_style;
//#define LAUNCHER_MAIN_OPACITY (LV_OPA_TRANSP)
//#define ALTERNATE_UI_OPACITY (LV_OPA_50)
static void menu_test_result_create();
static void menu_test_result_destroy();
static void menu_test_result_show();
static void menu_test_result_hide();

static menu_page_t menu_test_result_page = {
    .onCreate = menu_test_result_create,
    .onDestroy = menu_test_result_destroy,
    .show = menu_test_result_show,
    .hide = menu_test_result_hide,
    .back = menuPageBack
};

pthread_t test_result_id = 0;
void * test_result_thread(void *arg)
{
	sleep(5);
	system("reboot");
	
	return NULL;
}


menu_page_t get_menu_test_result_page()
{
    return menu_test_result_page;
}

static void pass_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
		system("reboot");
    }
}

static void error_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	
			
    if (LV_EVENT_RELEASED == event) {
        //printf("[%s,%d]\n",__FUNCTION__,__LINE__);
		system("reboot");
    }
}


static bool styleInited = false;
static void _style_init()
{
    if (!styleInited)
    {
    	lv_style_init(&disp_panel_style);
		lv_style_set_text_font(&disp_panel_style, LV_STATE_DEFAULT, &stsong_36);
		lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
		lv_style_set_text_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);

        styleInited = true;
    }
}

/*页面内部layout*/
static void create_top_panel(lv_obj_t * par)
{
    lv_obj_t *top_panel = menu_page_create_test_mode_top_panel(par);

    lv_obj_t *page_title = lv_label_create(top_panel, NULL);
    lv_label_set_text(page_title, _("Test Result"));
    lv_obj_align(page_title, NULL, LV_ALIGN_CENTER, -20, 0);
    lv_obj_add_style(page_title, LV_LABEL_PART_MAIN, menu_page_title_style());
}

static void create_disp_panel(lv_obj_t * par)
{
	//char tempBuf[64]={0};
	//char card[] = "eth0";
	//char tmpBuf2[32] = {0};
	lv_obj_t *disp_panel = lv_cont_create(par, NULL);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_obj_set_pos(disp_panel, 0, TOP_TEST_MODE_PANEL_H);
	lv_obj_add_style(disp_panel, LV_CONT_PART_MAIN, &disp_panel_style);
    //lv_cont_set_layout(disp_panel, LV_LAYOUT_CENTER);
	//_lv_obj_set_style_local_color(lv_obj_t * obj, uint8_t part, lv_style_property_t prop, lv_color_t color)

	lv_obj_t * vs_label = lv_label_create(disp_panel, NULL);
	//lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
	lv_obj_add_style(vs_label, LV_STATE_DEFAULT, &disp_panel_style);
	lv_label_set_text(vs_label, _("Test End"));
	//lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(vs_label, NULL, LV_ALIGN_CENTER, 0, 0);	
	
}

/// 页面生命周期函数 start
static void menu_test_result_create()
{
	//static uint32_t user_data = 10;
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
    _style_init();
	
    menu_test_result_page_obj = lv_cont_create(get_test_mode_page_flag()==1 ? lv_scr_menu_test_mode():lv_scr_menu_factory_mode(), NULL);
    lv_obj_add_style(menu_test_result_page_obj, LV_CONT_PART_MAIN, menu_page_test_mode_title_style());
    lv_obj_set_size(menu_test_result_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	
	create_top_panel(menu_test_result_page_obj);
    create_disp_panel(menu_test_result_page_obj);
    
	bottom_panel_page_obj = create_bottom_2btn_panel(menu_test_result_page_obj, pass_btn_event_cb, error_btn_event_cb);
	lv_obj_set_hidden(bottom_panel_page_obj, false);

    lv_obj_set_hidden(menu_test_result_page_obj, true);

	pthread_create(&test_result_id,NULL,test_result_thread,NULL);
}

lv_obj_t * lv_scr_menu_result_mode()
{
    return menu_test_result_page_obj;
}

static void menu_test_result_destroy()
{
    if (menu_test_result_page_obj)
    {
        lv_obj_del(menu_test_result_page_obj);
        menu_test_result_page_obj = NULL;
    }
}

static void menu_test_result_show()
{
    if (menu_test_result_page_obj)
    {
        lv_obj_set_hidden(menu_test_result_page_obj, false);
    }
}

static void menu_test_result_hide()
{
    if (menu_test_result_page_obj)
    {
        lv_obj_set_hidden(menu_test_result_page_obj, true);
    }
}

