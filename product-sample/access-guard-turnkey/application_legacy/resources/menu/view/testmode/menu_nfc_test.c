#include <stdio.h>
#include <stdint.h>
#include "menu.h"
#include "menu_common.h"
#include "menu_page.h"
#include "resource.h"
#include "lv_i18n.h"
#include "menu_mic_test.h"
#include "menu_test_mode.h"
#include "menu_nfc_test.h"
#include "factory.h"
#include "app_comp.h"
#include "app_peripheral.h"
#include "menu_factory_mode.h"


/*个性化与通知反馈页面*/

static lv_obj_t *menu_nfc_test_page_obj = NULL;
static lv_obj_t *bottom_panel_page_obj = NULL;
//static lv_obj_t *disp_btn_panel = NULL;
static lv_obj_t * show_label1 = NULL;

static lv_obj_t * show_label[10];


static lv_style_t disp_panel_style;
static lv_style_t disp_btn_panel_style;
static lv_style_t disp_nfc_panel_style;
#define LAUNCHER_MAIN_OPACITY (LV_OPA_TRANSP)
#define ALTERNATE_UI_OPACITY (LV_OPA_50)

static int index_txt = 0;
//static int flag = 0;
static lv_task_t * task = NULL;

static void menu_nfc_test_create();
static void menu_nfc_test_destroy();
static void menu_nfc_test_show();
static void menu_nfc_test_hide();

static menu_page_t menu_nfc_test_page = {
    .onCreate = menu_nfc_test_create,
    .onDestroy = menu_nfc_test_destroy,
    .show = menu_nfc_test_show,
    .hide = menu_nfc_test_hide,
    .back = menuPageBack
};

menu_page_t get_menu_nfc_test_page()
{
    return menu_nfc_test_page;
}
void get_nfc_info_task(lv_task_t * task)
{
	//printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	unsigned char card[4] = {0};
	char tmpBuf[16] = {0};
	
	get_card_no(card);
     
	if(strlen((char *)card) != 0)
	{
		sprintf(tmpBuf,"%s: %02X%02X%02X%02X",_("cardNo"),card[0],card[1],card[2],card[3]);
	    //lv_label_set_text(show_label, tmpBuf);
		if(0 == index_txt)
		{
			for(int i = 0;i< 10;i++)
			{
				lv_label_set_text(show_label[i], "");
			}
		}
	    
		lv_label_set_text(show_label[index_txt], tmpBuf);
		if(index_txt++ >= 9)
		{
			index_txt = 0;
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
			menu_btn_flag[MENU_ID_NFC] = 0;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_mic_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
		}
		if(task != NULL)
		{
			lv_task_del(task);
			task = NULL;
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
			menu_btn_flag[MENU_ID_NFC] = 1;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
			menu_tree_push(get_menu_mic_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
		}
		if(task != NULL)
		{
			lv_task_del(task);
			task = NULL;
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
    lv_label_set_text(page_title, _("NFC test"));
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
    lv_cont_set_layout(disp_panel, LV_LAYOUT_CENTER);
	//_lv_obj_set_style_local_color(lv_obj_t * obj, uint8_t part, lv_style_property_t prop, lv_color_t color)

	show_label1 = lv_label_create(disp_panel, NULL);
    //lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
    lv_obj_add_style(show_label1, LV_STATE_DEFAULT, &disp_panel_style);
    lv_label_set_text(show_label1, _("Swipe your card"));
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(show_label1, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

	index_txt = 0;
	for(int i = 0; i< 10;i++)
	{
		show_label[i] = lv_label_create(disp_panel, NULL);
	    lv_obj_add_style(show_label[i], LV_STATE_DEFAULT, &disp_panel_style);
		lv_label_set_text(show_label[i], "");
	    lv_obj_align(show_label[i], NULL, LV_ALIGN_IN_TOP_LEFT, 50, 50*i+20);
	}
	
}

/// 页面生命周期函数 start
static void menu_nfc_test_create()
{
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	static uint32_t user_data = 10;
    _style_init();

	task = lv_task_create(get_nfc_info_task, 50, LV_TASK_PRIO_MID, &user_data);
	
    menu_nfc_test_page_obj = lv_cont_create(get_test_mode_page_flag()==1 ? lv_scr_menu_test_mode():lv_scr_menu_factory_mode(), NULL);
    lv_obj_add_style(menu_nfc_test_page_obj, LV_CONT_PART_MAIN, menu_page_test_mode_title_style());
    lv_obj_set_size(menu_nfc_test_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	
	create_top_panel(menu_nfc_test_page_obj);
    create_disp_panel(menu_nfc_test_page_obj);
    
	bottom_panel_page_obj = create_bottom_2btn_panel(menu_nfc_test_page_obj, pass_btn_event_cb, error_btn_event_cb);
	//lv_obj_set_hidden(bottom_panel_page_obj, false);
	lv_obj_set_hidden(bottom_panel_page_obj, false);
	//create_bottom_2btn_panel(menu_lcd_test_page_obj, pass_btn_event_cb, error_btn_event_cb);

    lv_obj_set_hidden(menu_nfc_test_page_obj, true);
}

lv_obj_t * lv_scr_menu_nfc_mode()
{
    return menu_nfc_test_page_obj;
}

static void menu_nfc_test_destroy()
{
    if (menu_nfc_test_page_obj)
    {
        lv_obj_del(menu_nfc_test_page_obj);
        menu_nfc_test_page_obj = NULL;
		if(task != NULL)
		{
			lv_task_del(task);
			task = NULL;
		}
    }
}

static void menu_nfc_test_show()
{
    if (menu_nfc_test_page_obj)
    {
        lv_obj_set_hidden(menu_nfc_test_page_obj, false);
    }
}

static void menu_nfc_test_hide()
{
    if (menu_nfc_test_page_obj)
    {
        lv_obj_set_hidden(menu_nfc_test_page_obj, true);
		if(task != NULL)
		{
			lv_task_del(task);
			task = NULL;
		}
    }
}
/// 页面生命周期函数 end


