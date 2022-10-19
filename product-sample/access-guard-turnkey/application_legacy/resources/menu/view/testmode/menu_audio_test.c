#include <stdio.h>
#include <stdint.h>

#include "menu.h"
#include "menu_common.h"
#include "menu_page.h"
#include "resource.h"
#include "lv_i18n.h"
#include "menu_test_mode.h"
#include "menu_audio_test.h"
#include "menu_relay_test.h"
#include "factory.h"
#include "app_peripheral.h"
#include "../audio/cvi_audio_main.h"
#include "menu_factory_mode.h"
#include "cvi_audio.h"



/*个性化与通知反馈页面*/

static lv_obj_t *menu_audio_test_page_obj = NULL;
static lv_obj_t *bottom_panel_page_obj = NULL;
static lv_obj_t *title;
lv_obj_t * btn = NULL;
static lv_obj_t * show_label = NULL;
static lv_obj_t * btn_label = NULL;


static int flag = 0;

static lv_style_t disp_panel_style;

static void menu_audio_test_create();
static void menu_audio_test_destroy();
static void menu_audio_test_show();
static void menu_audio_test_hide();

static menu_page_t menu_audio_test_page = {
    .onCreate = menu_audio_test_create,
    .onDestroy = menu_audio_test_destroy,
    .show = menu_audio_test_show,
    .hide = menu_audio_test_hide,
    .back = menuPageBack
};

menu_page_t get_menu_audio_test_page()
{
    return menu_audio_test_page;
}

/*事件回调处理函数*/
static void change_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	if (LV_EVENT_RELEASED == event)
    {
		//audio_play_single_tone();
		if(0 == flag++)
		{
			CVI_Test_audio_stop();
			lv_label_set_text(show_label, _("audio stop"));
			lv_label_set_text(btn_label, _("start"));
		}else
		{
			flag = 0;
			CVI_Test_speaker_loopaudio();
			lv_label_set_text(show_label, _("audio testing"));
			lv_label_set_text(btn_label, _("stop"));
		}
        printf("[%s,%d]\n",__FUNCTION__,__LINE__);		
    }
}
static void sound_change_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_VALUE_CHANGED == event) {
        int value = lv_slider_get_value(obj);
        printf("value = %d.\n", value);
		//CVI_AO_SetVolume(1, value);
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
			menu_btn_flag[MENU_ID_AUDIO] = 0;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_relay_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
		}
		CVI_Test_audio_stop();
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
			menu_btn_flag[MENU_ID_AUDIO] = 1;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
			menu_tree_push(get_menu_relay_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
		}
		CVI_Test_audio_stop();
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
    lv_label_set_text(title, _("Sound Test"));
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(title, NULL, LV_ALIGN_CENTER, 0, 0);
}

static void create_disp_panel(lv_obj_t * par)
{
	int voice_sound = 0;
    //lv_obj_t *disp_panel = menu_page_create_disp_panel(par);
	//lv_obj_t *disp_panel = lv_cont_create(par, back_btn_event_cb);
	lv_obj_t *disp_panel = lv_cont_create(par, NULL);
	//lv_obj_set_event_cb(disp_panel, change_btn_event_cb);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_obj_set_pos(disp_panel, 0, TOP_TEST_MODE_PANEL_H);
	lv_obj_add_style(disp_panel, LV_STATE_DEFAULT, &disp_panel_style);
    lv_cont_set_layout(disp_panel, LV_LAYOUT_OFF);
	//_lv_obj_set_style_local_color(lv_obj_t * obj, uint8_t part, lv_style_property_t prop, lv_color_t color)

	//btn = menu_page_create_bottom_panel(disp_panel,100, 100);
	//lv_obj_set_event_cb(btn, change_btn_event_cb);

	show_label = lv_label_create(disp_panel, NULL);
    //lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
    lv_obj_add_style(show_label, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
    lv_label_set_text(show_label, _("audio testing"));
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(show_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 200);	

	lv_obj_t * slider = lv_slider_create(disp_panel, NULL);
	cvi_get_device_config_int(CONF_KEY_VOICE_SOUND, &voice_sound);
    lv_slider_set_value(slider, voice_sound, LV_ANIM_OFF);
    lv_obj_set_width(slider, 400);
	lv_obj_set_height(slider, 50);
	lv_slider_set_range(slider, 0, 15);
    lv_obj_align(slider, NULL, LV_ALIGN_IN_RIGHT_MID, -150, 100);
    lv_obj_set_event_cb(slider, sound_change_cb);

	create_one_btn(disp_panel, NULL, &btn_label, 250, 300, _("stop"), change_btn_event_cb);
	
}

/// 页面生命周期函数 start
static void menu_audio_test_create()
{
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);

    _style_init();

	CVI_Test_speaker_loopaudio();
	
    menu_audio_test_page_obj = lv_cont_create(get_test_mode_page_flag()==1 ? lv_scr_menu_test_mode():lv_scr_menu_factory_mode(), NULL);
    lv_obj_add_style(menu_audio_test_page_obj, LV_CONT_PART_MAIN, menu_page_test_mode_title_style());
    lv_obj_set_size(menu_audio_test_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);

    
    create_disp_panel(menu_audio_test_page_obj);
	create_top_panel(menu_audio_test_page_obj);
	//create_bottom_panel(menu_led_test_page_obj);
	
	bottom_panel_page_obj = create_bottom_2btn_panel(menu_audio_test_page_obj, pass_btn_event_cb, error_btn_event_cb);
	lv_obj_set_hidden(bottom_panel_page_obj, false);
	//lv_obj_set_hidden(bottom_panel_page_obj, true);
	//create_bottom_2btn_panel(menu_lcd_test_page_obj, pass_btn_event_cb, error_btn_event_cb);

    //lv_obj_set_hidden(menu_led_test_page_obj, true);
}

static void menu_audio_test_destroy()
{
    if (menu_audio_test_page_obj)
    {
        lv_obj_del(menu_audio_test_page_obj);
        menu_audio_test_page_obj = NULL;
    }
}

static void menu_audio_test_show()
{
    if (menu_audio_test_page_obj)
    {
        lv_obj_set_hidden(menu_audio_test_page_obj, false);
    }
}

static void menu_audio_test_hide()
{
    if (menu_audio_test_page_obj)
    {
        lv_obj_set_hidden(menu_audio_test_page_obj, true);
    }
}
/// 页面生命周期函数 end


