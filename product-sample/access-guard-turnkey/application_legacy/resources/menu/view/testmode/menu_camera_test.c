#include <stdio.h>
#include <stdint.h>
#include "menu.h"
#include "menu_common.h"
#include "menu_page.h"
#include "resource.h"
#include "lv_i18n.h"
#include "menu_nfc_test.h"
#include "menu_test_mode.h"
#include "menu_camera_test.h"
#include "factory.h"
#include "app_comp.h"
#include "menu_factory_mode.h"
#include "app_peripheral.h"

/*个性化与通知反馈页面*/

static lv_obj_t *menu_camera_test_page_obj = NULL;
static lv_obj_t *bottom_panel_page_obj = NULL;
static lv_obj_t *bottom1_panel = NULL;
static lv_obj_t *bottom2_panel = NULL;

//static lv_obj_t *disp_btn_panel = NULL;

static lv_style_t disp_panel_style;
static lv_style_t disp_btn_panel_style;
static lv_style_t disp_camera_panel_style;
#define LAUNCHER_MAIN_OPACITY (LV_OPA_TRANSP)
#define ALTERNATE_UI_OPACITY (LV_OPA_50)


static int flag = 0;

static void menu_camera_test_create();
static void menu_camera_test_destroy();
static void menu_camera_test_show();
static void menu_camera_test_hide();

static menu_page_t menu_camera_test_page = {
    .onCreate = menu_camera_test_create,
    .onDestroy = menu_camera_test_destroy,
    .show = menu_camera_test_show,
    .hide = menu_camera_test_hide,
    .back = menuPageBack
};

menu_page_t get_menu_camera_test_page()
{
    return menu_camera_test_page;
}

/*事件回调处理函数*/
static void change_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	if (LV_EVENT_RELEASED == event)
    {
        printf("back_btn_event_cb.\n");
		if(0 == flag++)
		{
			lv_obj_set_hidden(bottom_panel_page_obj, true);
			lv_obj_set_hidden(bottom1_panel, true);
			lv_obj_set_hidden(bottom2_panel, true);
			IR_OnOFF(true);
			APP_CompSendCmd(CMD_APP_COMP_IR_START, NULL, 0, NULL, 0);
		}
		else
		{
			flag = 0;
			IR_OnOFF(false);
			APP_CompSendCmd(CMD_APP_COMP_GBR_START, NULL, 0, NULL, 0);
			lv_obj_set_hidden(bottom_panel_page_obj, false);
			lv_obj_set_hidden(bottom1_panel, false);
			lv_obj_set_hidden(bottom2_panel, false);
		}
        //menuPageBack();
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
			menu_btn_flag[MENU_ID_CAMERA] = 0;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_nfc_test_page());
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
			menu_btn_flag[MENU_ID_CAMERA] = 1;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
			menu_tree_push(get_menu_nfc_test_page());
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
		lv_style_set_text_font(&disp_panel_style, LV_STATE_DEFAULT, &stsong_22);
		lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
		lv_style_set_text_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);

		lv_style_init(&disp_btn_panel_style);
		lv_style_set_text_font(&disp_btn_panel_style, LV_STATE_DEFAULT, &stsong_22);
		lv_style_set_bg_color(&disp_btn_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLUE);
		lv_style_set_bg_color(&disp_btn_panel_style, LV_STATE_PRESSED, LV_COLOR_RED);
		lv_style_set_text_color(&disp_btn_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);

		lv_style_init(&disp_camera_panel_style);
		lv_style_set_bg_opa(&disp_camera_panel_style, LV_STATE_DEFAULT, LAUNCHER_MAIN_OPACITY);
		lv_style_set_opa_scale(&disp_camera_panel_style, LV_STATE_DEFAULT, ALTERNATE_UI_OPACITY);

		
        styleInited = true;
    }
}

/*页面内部layout*/
#if 0
static void create_top_panel(lv_obj_t * par)
{
    lv_obj_t *top_panel = menu_page_create_top_panel(par, NULL);

    lv_obj_t *page_title = lv_label_create(top_panel, NULL);
    lv_label_set_text(page_title, _("Profile & Notification"));
    lv_obj_align(page_title, NULL, LV_ALIGN_CENTER, -20, 0);
    lv_obj_add_style(page_title, LV_LABEL_PART_MAIN, menu_page_title_style());
}

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


static void create_disp_panel(lv_obj_t * par)
{
    //lv_obj_t *disp_panel = menu_page_create_disp_panel(par);
	//lv_obj_t *disp_panel = lv_cont_create(par, back_btn_event_cb);
	lv_obj_t *disp_panel = lv_cont_create(par, NULL);
	//lv_obj_set_event_cb(disp_panel, change_btn_event_cb);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_pos(disp_panel, 0, 0);
	lv_obj_add_style(disp_panel, LV_CONT_PART_MAIN, &disp_panel_style);
    lv_cont_set_layout(disp_panel, LV_LAYOUT_CENTER);
	//_lv_obj_set_style_local_color(lv_obj_t * obj, uint8_t part, lv_style_property_t prop, lv_color_t color)

	lv_obj_t * show_label = lv_label_create(disp_panel, NULL);
    //lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
    lv_obj_add_style(show_label, LV_STATE_DEFAULT, &disp_panel_style);
    lv_label_set_text(show_label, _("Touch blue button"));
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(show_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	
}
#endif

/// 页面生命周期函数 start
static void menu_camera_test_create()
{
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
    _style_init();
	APP_CompSendCmd(CMD_APP_COMP_GBR_START, NULL, 0, NULL, 0);
	
    menu_camera_test_page_obj = lv_cont_create(get_test_mode_page_flag()==1 ? lv_scr_menu_test_mode():lv_scr_menu_factory_mode(), NULL);
    lv_obj_add_style(menu_camera_test_page_obj, LV_CONT_PART_MAIN, &disp_camera_panel_style);
    lv_obj_set_size(menu_camera_test_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	lv_obj_set_size(menu_camera_test_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_cont_set_fit(menu_camera_test_page_obj, LV_FIT_NONE);
	lv_obj_set_event_cb(menu_camera_test_page_obj, change_btn_event_cb);
	
	
	bottom_panel_page_obj = create_bottom_2btn_panel2(menu_camera_test_page_obj,&bottom1_panel ,&bottom2_panel, pass_btn_event_cb, error_btn_event_cb);
	//lv_obj_set_hidden(bottom_panel_page_obj, false);
	lv_obj_set_hidden(bottom_panel_page_obj, true);
	lv_obj_set_hidden(bottom1_panel, true);
	lv_obj_set_hidden(bottom2_panel, true);
	//create_bottom_2btn_panel(menu_lcd_test_page_obj, pass_btn_event_cb, error_btn_event_cb);

    lv_obj_set_hidden(menu_camera_test_page_obj, true);
}

lv_obj_t * lv_scr_menu_camera_mode()
{
    return menu_camera_test_page_obj;
}

static void menu_camera_test_destroy()
{
    if (menu_camera_test_page_obj)
    {
    	APP_CompSendCmd(CMD_APP_COMP_VIDEOSTOP, NULL, 0, NULL, 0);
        lv_obj_del(menu_camera_test_page_obj);
        menu_camera_test_page_obj = NULL;
    }
}

static void menu_camera_test_show()
{
    if (menu_camera_test_page_obj)
    {
        lv_obj_set_hidden(menu_camera_test_page_obj, false);
    }
}

static void menu_camera_test_hide()
{
    if (menu_camera_test_page_obj)
    {
        lv_obj_set_hidden(menu_camera_test_page_obj, true);
    }
}
/// 页面生命周期函数 end


