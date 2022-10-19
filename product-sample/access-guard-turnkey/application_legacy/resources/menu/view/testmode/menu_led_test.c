#include <stdio.h>
#include <stdint.h>

#include "menu.h"
#include "menu_common.h"
#include "menu_page.h"
#include "resource.h"
#include "lv_i18n.h"
#include "menu_test_mode.h"
#include "menu_led_test.h"
#include "menu_eth_test.h"
#include "factory.h"
#include "menu_factory_mode.h"
#include "app_peripheral.h"




/*个性化与通知反馈页面*/

static lv_obj_t *menu_led_test_page_obj = NULL;
static lv_obj_t *bottom_panel_page_obj = NULL;
static lv_obj_t *title;

static lv_obj_t * show_lcd_label = NULL;
static lv_style_t disp_panel_style;
//static int flag = 0;
static void menu_led_test_create();
static void menu_led_test_destroy();
static void menu_led_test_show();
static void menu_led_test_hide();

static menu_page_t menu_led_test_page = {
    .onCreate = menu_led_test_create,
    .onDestroy = menu_led_test_destroy,
    .show = menu_led_test_show,
    .hide = menu_led_test_hide,
    .back = menuPageBack
};

menu_page_t get_menu_led_test_page()
{
    return menu_led_test_page;
}

/*事件回调处理函数*/
#if 0
static void change_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	if (LV_EVENT_RELEASED == event)
    {

		if(flag++ != 0)
		{
			CVI_Test_Led(30);
			//LED_OnOFF(false);
			flag = 0;
		}
		else
		{
			//LED_OnOFF(true);
			CVI_Test_Led(100);
		}
		
        printf("back_btn_event_cb.\n");
    }
}
#endif

static void pass_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {


		//menu_test_mode_PageBack();
		if (menu_tree_isEmpty())
                return;
		if(get_test_mode_page_flag() == 1)
		{
			menu_btn_flag[MENU_ID_LED] = 0;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_eth_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
		}   
		CVI_Test_Led(0);
    }
}

static void error_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        //lv_obj_t *face_info_window = createFaceAddWindow(NULL);
        //lv_obj_set_hidden(face_info_window, false);
        printf("[%s,%d]\n",__FUNCTION__,__LINE__);

		//menu_test_mode_PageBack();

		if (menu_tree_isEmpty())
                return;
		if(get_test_mode_page_flag() == 1)
		{
			menu_btn_flag[MENU_ID_LED] = 1;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
			menu_tree_push(get_menu_eth_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
		}
		CVI_Test_Led(0);
    }
}

static void led_change_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_VALUE_CHANGED == event) {
        int value = lv_slider_get_value(obj);
		CVI_Test_Led(10*value);
    }
}

static void lcd_change_cb(lv_obj_t *obj, lv_event_t event)
{
	char tmpBuf[64] = {};
    if (LV_EVENT_VALUE_CHANGED == event) {
		int value = lv_slider_get_value(obj);
        cvi_set_device_config_int(CONF_KEY_DISPLAY_BRIGHTNESS, value);

		snprintf(tmpBuf,sizeof(tmpBuf),"%s:%d",_("Lcd brightness"),value);
		lv_label_set_text(show_lcd_label, tmpBuf);

		CVI_HAL_PwmDisable(0, 1);
		CVI_HAL_PwmUnExport(0, 1);
		CVI_HAL_PwmExport(0, 1);
		CVI_HAL_PwmSetParm(0, 1, PWM_LCD_PERIOD, PWM_LCD_DUTY_BASE * ((value == 0) ? 1 : value));
		CVI_HAL_PwmEnable(0, 1);
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
    lv_label_set_text(title, _("LED Test"));
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(title, NULL, LV_ALIGN_CENTER, 0, 0);
}

static void create_disp_panel(lv_obj_t * par)
{
	char tmpBuf[64] = {};
	int ret;
	lv_obj_t *disp_panel = lv_cont_create(par, NULL);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_obj_set_pos(disp_panel, 0, TOP_TEST_MODE_PANEL_H);
	lv_obj_add_style(disp_panel, LV_STATE_DEFAULT, &disp_panel_style);
    lv_cont_set_layout(disp_panel, LV_LAYOUT_OFF);

	lv_obj_t * show_label = lv_label_create(disp_panel, NULL);
    lv_obj_add_style(show_label, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
    lv_label_set_text(show_label, _("slipping"));
    lv_obj_align(show_label, NULL, LV_ALIGN_IN_RIGHT_MID, -300, -100);	

	lv_obj_t * slider = lv_slider_create(disp_panel, NULL);
    lv_slider_set_value(slider, 0, LV_ANIM_OFF);
    lv_obj_set_width(slider, 400);
	lv_obj_set_height(slider, 50);
	lv_slider_set_range(slider, 0, 10);
    lv_obj_align(slider, NULL, LV_ALIGN_IN_RIGHT_MID, -150, 0);
    lv_obj_set_event_cb(slider, led_change_cb);

	show_lcd_label = lv_label_create(disp_panel, NULL);
	lv_obj_add_style(show_lcd_label, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
	ret = cvi_get_device_config_int(CONF_KEY_DISPLAY_BRIGHTNESS, &device_cfg1.display_brightness);
	lv_obj_align(show_lcd_label, NULL, LV_ALIGN_IN_RIGHT_MID, -300, 150);

	lv_obj_t * slider2 = lv_slider_create(disp_panel, NULL);
    //lv_slider_set_value(slider2, 0, LV_ANIM_OFF);
    lv_obj_set_width(slider2, 400);
	lv_obj_set_height(slider2, 50);
	lv_slider_set_range(slider2, 0, 100);
	if(ret < 0) {
		lv_slider_set_value(slider2, 100, LV_ANIM_OFF);
		snprintf(tmpBuf,sizeof(tmpBuf),"%s:%d",_("Lcd brightness"),100);
	}
	else{
		lv_slider_set_value(slider2, device_cfg1.display_brightness, LV_ANIM_OFF);
		snprintf(tmpBuf,sizeof(tmpBuf),"%s:%d",_("Lcd brightness"),device_cfg1.display_brightness);
	}
	lv_label_set_text(show_lcd_label, tmpBuf);
    lv_obj_align(slider2, NULL, LV_ALIGN_IN_RIGHT_MID, -150, 250);
    lv_obj_set_event_cb(slider2, lcd_change_cb);

}
#if 0
static void create_bottom_panel(lv_obj_t *par, lv_event_cb_t cb)
{
	lv_obj_t * bottom_panel = menu_page_create_test_mode_bottom_panel(par);
/*
	lv_obj_t *back_img = lv_img_create(bottom_panel, NULL);
	lv_img_set_src(back_img, RES_IMG_BACK);
	lv_img_set_auto_size(back_img, true);
	lv_obj_set_click(back_img, true);
	lv_obj_align(back_img, NULL, LV_ALIGN_IN_TOP_LEFT, 15, (TOP_PANEL_H - 30 ) / 2);
	lv_obj_set_event_cb(back_img, add_btn_event_cb);

	lv_obj_t *add_btn = lv_img_create(bottom_panel, NULL);
	lv_img_set_src(add_btn, RES_IMG_ADD_BTN);
	lv_obj_align(add_btn, NULL, LV_ALIGN_CENTER, 300, 0);
	lv_img_set_auto_size(add_btn, true);
	lv_obj_set_click(add_btn, true);
	lv_obj_set_event_cb(add_btn, add_btn_event_cb);
*/

	lv_obj_t *next_label = lv_label_create(bottom_panel, NULL);
    lv_obj_set_click(next_label, true);
    lv_obj_align(next_label, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
    lv_obj_set_event_cb(next_label, pass_btn_event_cb);
    lv_obj_add_style(next_label, LV_LABEL_PART_MAIN, &btn_label_test_mode_style);
    lv_label_set_text(next_label, _("pass"));

    lv_obj_t *prev_label = lv_label_create(bottom_panel, NULL);
    lv_obj_set_click(prev_label, true);
    lv_obj_align(prev_label, NULL, LV_ALIGN_IN_LEFT_MID, 100, 0);
    lv_obj_set_event_cb(prev_label, error_btn_event_cb);
    lv_obj_add_style(prev_label, LV_LABEL_PART_MAIN, &btn_label_test_mode_style);
    lv_label_set_text(prev_label, _("error"));
}
#endif

/// 页面生命周期函数 start
static void menu_led_test_create()
{
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);

    _style_init();
	
    menu_led_test_page_obj = lv_cont_create(get_test_mode_page_flag()==1 ? lv_scr_menu_test_mode():lv_scr_menu_factory_mode(), NULL);
    lv_obj_add_style(menu_led_test_page_obj, LV_CONT_PART_MAIN, menu_page_test_mode_title_style());
    lv_obj_set_size(menu_led_test_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);

    
    create_disp_panel(menu_led_test_page_obj);
	create_top_panel(menu_led_test_page_obj);
	//create_bottom_panel(menu_led_test_page_obj);
	
	bottom_panel_page_obj = create_bottom_2btn_panel(menu_led_test_page_obj, pass_btn_event_cb, error_btn_event_cb);
	lv_obj_set_hidden(bottom_panel_page_obj, false);
	//lv_obj_set_hidden(bottom_panel_page_obj, true);
	//create_bottom_2btn_panel(menu_lcd_test_page_obj, pass_btn_event_cb, error_btn_event_cb);

    //lv_obj_set_hidden(menu_led_test_page_obj, true);
}

static void menu_led_test_destroy()
{
    if (menu_led_test_page_obj)
    {
        lv_obj_del(menu_led_test_page_obj);
        menu_led_test_page_obj = NULL;
    }
}

static void menu_led_test_show()
{
    if (menu_led_test_page_obj)
    {
        lv_obj_set_hidden(menu_led_test_page_obj, false);
    }
}

static void menu_led_test_hide()
{
    if (menu_led_test_page_obj)
    {
        lv_obj_set_hidden(menu_led_test_page_obj, true);
    }
}
/// 页面生命周期函数 end


