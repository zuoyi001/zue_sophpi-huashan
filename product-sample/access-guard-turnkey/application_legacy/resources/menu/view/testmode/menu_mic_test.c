#include <stdio.h>
#include <stdint.h>
#include "menu.h"
#include "menu_common.h"
#include "menu_page.h"
#include "resource.h"
#include "lv_i18n.h"
#include "menu_wifi_test.h"
#include "menu_test_mode.h"
#include "menu_mic_test.h"
#include "factory.h"
#include "app_comp.h"
#include "cvi_audio.h"
#include "menu_factory_mode.h"
extern void audio_set_MicVolume(int left,int right);


/*个性化与通知反馈页面*/

static lv_obj_t *menu_mic_test_page_obj = NULL;
static lv_obj_t *bottom_panel_page_obj = NULL;
//static lv_obj_t *disp_btn_panel = NULL;

static lv_obj_t * show_label = NULL;
static lv_obj_t * mic_btn_label = NULL;
lv_obj_t *sound_label = NULL;
lv_obj_t *mic_label = NULL;
lv_obj_t *record_btn_label = NULL;
lv_obj_t *play_btn_label = NULL;


static lv_style_t disp_panel_style;
static lv_style_t disp_btn_panel_style;
static lv_style_t disp_nfc_panel_style;
#define LAUNCHER_MAIN_OPACITY (LV_OPA_TRANSP)
#define ALTERNATE_UI_OPACITY (LV_OPA_50)


int record_btn_flag = 0;
int play_btn_flag = 0;
int mic_btn_flag = 0;


static void menu_mic_test_create();
static void menu_mic_test_destroy();
static void menu_mic_test_show();
static void menu_mic_test_hide();

static menu_page_t menu_mic_test_page = {
    .onCreate = menu_mic_test_create,
    .onDestroy = menu_mic_test_destroy,
    .show = menu_mic_test_show,
    .hide = menu_mic_test_hide,
    .back = menuPageBack
};

menu_page_t get_menu_mic_test_page()
{
    return menu_mic_test_page;
}

static void mic_sound_change_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_VALUE_CHANGED == event) {
		char tmpBuf[64] = {0};
        int value = lv_slider_get_value(obj);
		snprintf(tmpBuf,sizeof(tmpBuf),"%s:%d",_("sound volume(0~15)"),value);
    	lv_label_set_text(sound_label, tmpBuf);
		//CVI_AO_SetVolume(1, value);
    }
	
	
}

static void mic_MicVolume_change_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_VALUE_CHANGED == event) {
		char tmpBuf[64] = {0};
        int value = lv_slider_get_value(obj);
		//CVI_AO_SetVolume(1, value);
		memset(tmpBuf, 0x00, sizeof(tmpBuf));
		snprintf(tmpBuf,sizeof(tmpBuf),"%s:%d",_("mic volume(0~7)"),value);
	    lv_label_set_text(mic_label, tmpBuf);
		//audio_set_MicVolume(value,value);
    }
}
static void record_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	if (LV_EVENT_RELEASED == event)
    {
        printf("back_btn_event_cb.\n");
		if(play_btn_flag!=0 || mic_btn_flag!=0)
		{
			return;
		}
		if(0 == record_btn_flag++)
		{
			CVI_Test_record_micdata(1);
			//CVI_Test_audio_local_loopback();
			lv_label_set_text(record_btn_label, _("stop"));
			lv_label_set_text(show_label, _("recording"));
			lv_obj_set_hidden(bottom_panel_page_obj, true);
		}
		else
		{
			CVI_Test_record_micdata(0);
			record_btn_flag = 0;
			lv_obj_set_hidden(bottom_panel_page_obj, false);
			
			//CVI_Test_audio_stop();
			lv_label_set_text(record_btn_label, _("start record"));
			lv_label_set_text(show_label, _("record off"));
		}
        //menuPageBack();
    }
}

static void play_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	if (LV_EVENT_RELEASED == event)
    {
        printf("back_btn_event_cb.\n");
		if(record_btn_flag!=0 || mic_btn_flag!=0)
		{
			return;
		}
		if(0 == play_btn_flag++)
		{
			CVI_Test_play_micdata();
			lv_label_set_text(play_btn_label, _("stop"));
			lv_label_set_text(show_label, _("playing"));
			lv_obj_set_hidden(bottom_panel_page_obj, true);
		}
		else
		{
			play_btn_flag = 0;
			lv_obj_set_hidden(bottom_panel_page_obj, false);

			CVI_Test_audio_stop();
			lv_label_set_text(play_btn_label, _("start"));
			lv_label_set_text(show_label, _("play off"));
		}
        //menuPageBack();
    }
}


static void mic_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	if (LV_EVENT_RELEASED == event)
    {
    	if(record_btn_flag!=0 || play_btn_flag!=0)
		{
			return;
		}
        printf("back_btn_event_cb.\n");
		if(0 == mic_btn_flag++)
		{
			CVI_Test_audio_local_loopback();
			lv_label_set_text(mic_btn_label, _("close mic"));
			lv_label_set_text(show_label, _("mic on"));
			lv_obj_set_hidden(bottom_panel_page_obj, true);
		}
		else
		{
			mic_btn_flag = 0;
			lv_obj_set_hidden(bottom_panel_page_obj, false);

			CVI_Test_audio_stop();
			lv_label_set_text(mic_btn_label, _("start mic"));
			lv_label_set_text(show_label, _("mic off"));
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
			menu_btn_flag[MENU_ID_MIC] = 0;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_wifi_test_page());
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
			menu_btn_flag[MENU_ID_MIC] = 1;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
			menu_tree_push(get_menu_wifi_test_page());
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

		lv_style_init(&disp_nfc_panel_style);
		lv_style_set_bg_opa(&disp_nfc_panel_style, LV_STATE_DEFAULT, LAUNCHER_MAIN_OPACITY);
		lv_style_set_opa_scale(&disp_nfc_panel_style, LV_STATE_DEFAULT, ALTERNATE_UI_OPACITY);

		
        styleInited = true;
    }
}

/*页面内部layout*/
static void create_top_panel(lv_obj_t * par)
{
    lv_obj_t *top_panel = menu_page_create_test_mode_top_panel(par);

    lv_obj_t *page_title = lv_label_create(top_panel, NULL);
    lv_label_set_text(page_title, _("MIC test"));
    lv_obj_align(page_title, NULL, LV_ALIGN_CENTER, -20, 0);
    lv_obj_add_style(page_title, LV_LABEL_PART_MAIN, menu_page_title_style());
}

static void create_disp_panel(lv_obj_t * par)
{
    //lv_obj_t *disp_panel = menu_page_create_disp_panel(par);
	//lv_obj_t *disp_panel = lv_cont_create(par, back_btn_event_cb);
	lv_obj_t *disp_panel = lv_cont_create(par, NULL);
	//lv_obj_set_event_cb(disp_panel, change_btn_event_cb);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_obj_set_pos(disp_panel, 0, TOP_TEST_MODE_PANEL_H);
	lv_obj_add_style(disp_panel, LV_CONT_PART_MAIN, &disp_panel_style);
    lv_cont_set_layout(disp_panel, LV_LAYOUT_OFF);
	//_lv_obj_set_style_local_color(lv_obj_t * obj, uint8_t part, lv_style_property_t prop, lv_color_t color)

	show_label = lv_label_create(disp_panel, NULL);
    //lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
    lv_obj_add_style(show_label, LV_STATE_DEFAULT, &disp_panel_style);
    lv_label_set_text(show_label, _("mic off"));
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(show_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 100);

	create_one_btn(disp_panel, NULL, &record_btn_label, 250, 180, _("start record"), record_btn_event_cb);
	
	create_one_btn(disp_panel, NULL, &play_btn_label, 250, 320, _("start"), play_btn_event_cb);
	
	create_one_btn(disp_panel, NULL, &mic_btn_label, 250, 460, _("start mic"), mic_btn_event_cb);

	
	
}

/// 页面生命周期函数 start
static void menu_mic_test_create()
{
	int i32volume = 0;
	char tmpBuf[64] = {0};
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
    _style_init();

    menu_mic_test_page_obj = lv_cont_create(get_test_mode_page_flag()==1 ? lv_scr_menu_test_mode():lv_scr_menu_factory_mode(), NULL);
    lv_obj_add_style(menu_mic_test_page_obj, LV_CONT_PART_MAIN, menu_page_test_mode_title_style());
    lv_obj_set_size(menu_mic_test_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	
	create_top_panel(menu_mic_test_page_obj);
    create_disp_panel(menu_mic_test_page_obj);


	cvi_get_device_config_int(CONF_KEY_VOICE_SOUND, &i32volume);
	
	sound_label = lv_label_create(menu_mic_test_page_obj, NULL);
    //lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
    lv_obj_add_style(sound_label, LV_STATE_DEFAULT, &disp_panel_style);
	snprintf(tmpBuf,sizeof(tmpBuf),"%s:%d",_("sound volume(0~15)"),i32volume);
    lv_label_set_text(sound_label, tmpBuf);
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(sound_label, NULL, LV_ALIGN_IN_RIGHT_MID, -250, 150);

	lv_obj_t * slider = lv_slider_create(menu_mic_test_page_obj, NULL);
    lv_slider_set_value(slider, i32volume, LV_ANIM_OFF);
    lv_obj_set_width(slider, 400);
	lv_obj_set_height(slider, 50);
	lv_slider_set_range(slider, 0, 15);
    lv_obj_align(slider, NULL, LV_ALIGN_IN_RIGHT_MID, -150, 200);
    lv_obj_set_event_cb(slider, mic_sound_change_cb);

	mic_label = lv_label_create(menu_mic_test_page_obj, NULL);
    //lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
    lv_obj_add_style(mic_label, LV_STATE_DEFAULT, &disp_panel_style);
	memset(tmpBuf, 0x00, sizeof(tmpBuf));
	snprintf(tmpBuf,sizeof(tmpBuf),"%s:%d",_("mic volume(0~7)"),1);
    lv_label_set_text(mic_label, tmpBuf);
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(mic_label, NULL, LV_ALIGN_IN_RIGHT_MID, -300, 300);

	lv_obj_t * slider1 = lv_slider_create(menu_mic_test_page_obj, NULL);
	//cvi_get_device_config_int(CONF_KEY_VOICE_SOUND, &i32volume);
    lv_slider_set_value(slider1, 7, LV_ANIM_OFF);
	//audio_set_MicVolume(7,7);
    lv_obj_set_width(slider1, 400);
	lv_obj_set_height(slider1, 50);
	lv_slider_set_range(slider1, 0, 7);
    lv_obj_align(slider1, NULL, LV_ALIGN_IN_RIGHT_MID, -150, 350);
    lv_obj_set_event_cb(slider1, mic_MicVolume_change_cb);
    
	bottom_panel_page_obj = create_bottom_2btn_panel(menu_mic_test_page_obj, pass_btn_event_cb, error_btn_event_cb);
	//lv_obj_set_hidden(bottom_panel_page_obj, false);
	lv_obj_set_hidden(bottom_panel_page_obj, false);
	//create_bottom_2btn_panel(menu_lcd_test_page_obj, pass_btn_event_cb, error_btn_event_cb);

    lv_obj_set_hidden(menu_mic_test_page_obj, true);
}

lv_obj_t * lv_scr_menu_mic_mode()
{
    return menu_mic_test_page_obj;
}

static void menu_mic_test_destroy()
{
    if (menu_mic_test_page_obj)
    {
        lv_obj_del(menu_mic_test_page_obj);
        menu_mic_test_page_obj = NULL;
    }
}

static void menu_mic_test_show()
{
    if (menu_mic_test_page_obj)
    {
        lv_obj_set_hidden(menu_mic_test_page_obj, false);
    }
}

static void menu_mic_test_hide()
{
    if (menu_mic_test_page_obj)
    {
        lv_obj_set_hidden(menu_mic_test_page_obj, true);
    }
}
/// 页面生命周期函数 end


