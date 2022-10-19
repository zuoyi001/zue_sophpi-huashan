#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "menu.h"
#include "menu_common.h"
#include "menu_page.h"
#include "resource.h"
#include "lv_i18n.h"
#include "menu_test_mode.h"
#include "menu_tp_test.h"
#include "menu_relay_test.h"
#include "factory.h"
#include "app_peripheral.h"
#include "menu_factory_mode.h"



/*个性化与通知反馈页面*/

static lv_obj_t *menu_relay_test_page_obj = NULL;
static lv_obj_t *bottom_panel_page_obj = NULL;
static lv_obj_t *title;

static lv_obj_t * show_label = NULL;


static lv_style_t disp_panel_style;

static void menu_relay_test_create();
static void menu_relay_test_destroy();
static void menu_relay_test_show();
static void menu_relay_test_hide();

static menu_page_t menu_relay_test_page = {
    .onCreate = menu_relay_test_create,
    .onDestroy = menu_relay_test_destroy,
    .show = menu_relay_test_show,
    .hide = menu_relay_test_hide,
    .back = menuPageBack
};

menu_page_t get_menu_relay_test_page()
{
    return menu_relay_test_page;
}

static void Relay_change_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        int value = lv_slider_get_value(obj);
		if(0 == value)
		{
			CVI_Test_Relay(false);
			lv_label_set_text(show_label, _("Relay Off"));
		}
		else
		{
			CVI_Test_Relay(true);
			lv_label_set_text(show_label, _("Relay On"));
		}
    }
}

static void pass_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {


		//menu_test_mode_PageBack();
		if (menu_tree_isEmpty())
                return;
		if(get_test_mode_page_flag() == 1)
		{
			menu_btn_flag[MENU_ID_RELAY] = 0;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_tp_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
		}
    }
}

static void error_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
        //lv_obj_t *face_info_window = createFaceAddWindow(NULL);
        //lv_obj_set_hidden(face_info_window, false);
        printf("[%s,%d]\n",__FUNCTION__,__LINE__);

		//menu_test_mode_PageBack();

		if (menu_tree_isEmpty())
                return;
		if(get_test_mode_page_flag() == 1)
		{
			menu_btn_flag[MENU_ID_RELAY] = 1;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
			menu_tree_push(get_menu_tp_test_page());
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
		lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        styleInited = true;
    }
}

/*页面内部layout*/
static void create_top_panel(lv_obj_t * par)
{
	lv_obj_t * top_panel = menu_page_create_test_mode_top_panel(par);

    title = lv_label_create(top_panel, NULL);
    lv_obj_add_style(title, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
    lv_label_set_text(title, _("Relay Test"));
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(title, NULL, LV_ALIGN_CENTER, 0, 0);
}

static void create_disp_panel(lv_obj_t * par)
{
    //lv_obj_t *disp_panel = menu_page_create_disp_panel(par);
	//lv_obj_t *disp_panel = lv_cont_create(par, back_btn_event_cb);
	lv_obj_t *disp_panel = lv_cont_create(par, NULL);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_obj_set_pos(disp_panel, 0, TOP_TEST_MODE_PANEL_H);
	lv_obj_add_style(disp_panel, LV_STATE_DEFAULT, &disp_panel_style);
    lv_cont_set_layout(disp_panel, LV_LAYOUT_OFF);
	//_lv_obj_set_style_local_color(lv_obj_t * obj, uint8_t part, lv_style_property_t prop, lv_color_t color)

	show_label = lv_label_create(disp_panel, NULL);
    //lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
    lv_obj_add_style(show_label, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
    lv_label_set_text(show_label, _("Relay Off"));
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(show_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 200);	

	lv_obj_t * switch1 = lv_switch_create(disp_panel, NULL);
    lv_obj_set_width(switch1, 160);
	lv_obj_set_height(switch1, 80);
	lv_switch_set_anim_time(switch1, 1);
    lv_obj_align(switch1, NULL, LV_ALIGN_IN_TOP_MID, 0, 500);
    lv_obj_set_event_cb(switch1, Relay_change_cb);
	
}

/// 页面生命周期函数 start
static void menu_relay_test_create()
{
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);

    _style_init();
		
    menu_relay_test_page_obj = lv_cont_create(get_test_mode_page_flag()==1 ? lv_scr_menu_test_mode():lv_scr_menu_factory_mode(), NULL);
    lv_obj_add_style(menu_relay_test_page_obj, LV_CONT_PART_MAIN, menu_page_test_mode_title_style());
    lv_obj_set_size(menu_relay_test_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);

    
    create_disp_panel(menu_relay_test_page_obj);
	create_top_panel(menu_relay_test_page_obj);
	//create_bottom_panel(menu_led_test_page_obj);
	
	bottom_panel_page_obj = create_bottom_2btn_panel(menu_relay_test_page_obj, pass_btn_event_cb, error_btn_event_cb);
	lv_obj_set_hidden(bottom_panel_page_obj, false);
	//lv_obj_set_hidden(bottom_panel_page_obj, true);
	//create_bottom_2btn_panel(menu_lcd_test_page_obj, pass_btn_event_cb, error_btn_event_cb);

    //lv_obj_set_hidden(menu_led_test_page_obj, true);
}

static void menu_relay_test_destroy()
{
    if (menu_relay_test_page_obj)
    {
        lv_obj_del(menu_relay_test_page_obj);
        menu_relay_test_page_obj = NULL;
    }
}

static void menu_relay_test_show()
{
    if (menu_relay_test_page_obj)
    {
        lv_obj_set_hidden(menu_relay_test_page_obj, false);
    }
}

static void menu_relay_test_hide()
{
    if (menu_relay_test_page_obj)
    {
        lv_obj_set_hidden(menu_relay_test_page_obj, true);
    }
}
/// 页面生命周期函数 end


