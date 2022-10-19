#include <stdio.h>
#include <stdint.h>

#include "menu.h"
#include "menu_common.h"
#include "menu_page.h"
#include "resource.h"
#include "lv_i18n.h"
#include "menu_lcd_test.h"
#include "menu_test_mode.h"
#include "menu_factory_mode.h"
#include "menu_led_test.h"

/*个性化与通知反馈页面*/

static lv_obj_t *menu_lcd_test_page_obj = NULL;
static lv_obj_t *bottom_panel_page_obj = NULL;
static lv_obj_t *bottom1_panel = NULL;
static lv_obj_t *bottom2_panel = NULL;

static lv_obj_t *disp_panel = NULL;
lv_obj_t * img_gray_scale = NULL;
lv_obj_t * img_color_scale = NULL;


lv_obj_t * voice_notice_mode;
lv_obj_t * name_disp_mode;
lv_obj_t * company_name_item;
static lv_style_t disp_panel_style;
static lv_style_t lcd_test_border_style;

static int flag = 0;



static void menu_lcd_test_create();
static void menu_lcd_test_destroy();
static void menu_lcd_test_show();
static void menu_lcd_test_hide();

static menu_page_t menu_lcd_test_page = {
    .onCreate = menu_lcd_test_create,
    .onDestroy = menu_lcd_test_destroy,
    .show = menu_lcd_test_show,
    .hide = menu_lcd_test_hide,
    .back = menuPageBack
};

menu_page_t get_menu_lcd_test_page()
{
    return menu_lcd_test_page;
}

/*事件回调处理函数*/
static void change_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	if (LV_EVENT_RELEASED == event)
    {
    	flag++;
		if(flag >= 10)
		{
			flag = 0;
		}
        printf("back_btn_event_cb.\n");
		if(0 == flag)
		{
			lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, COLOR_BLUE);
		}else if(1 == flag)
		{
			lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, COLOR_RED);
		}else if(2 == flag)
		{
			lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, COLOR_GREEN);
		}else if(3 == flag)
		{
			lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
		}else if(4 == flag)
		{
			lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
		}
		else if(5 == flag)
		{
			lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
		}
		else if(6 == flag)
		{
			lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		}
		else if(7 == flag)
		{
			lv_obj_add_style(disp_panel, LV_CONT_PART_MAIN, &lcd_test_border_style);
			//lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		}else if(8 == flag)
		{
			//lv_obj_add_style(disp_panel, LV_CONT_PART_MAIN, &lcd_test_border_style);
			lv_obj_set_hidden(img_color_scale, true);
			lv_obj_set_hidden(img_gray_scale, false);
			//lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		}else if(9 == flag)
		{
			//lv_obj_add_style(disp_panel, LV_CONT_PART_MAIN, &lcd_test_border_style);
			lv_obj_set_hidden(img_gray_scale, true);
			lv_obj_set_hidden(img_color_scale, false);
			//lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		}
		
		
		if(0 == flag)
		{
			if(bottom_panel_page_obj != NULL)
			{
				lv_obj_set_hidden(bottom_panel_page_obj, false);
				lv_obj_set_hidden(bottom1_panel, false);
				lv_obj_set_hidden(bottom2_panel, false);
				lv_obj_add_style(disp_panel, LV_CONT_PART_MAIN, &disp_panel_style);
				lv_obj_set_hidden(img_gray_scale, true);
				lv_obj_set_hidden(img_color_scale, true);
			}
		}
		else
		{
			lv_obj_set_hidden(bottom_panel_page_obj, true);
			lv_obj_set_hidden(bottom1_panel, true);
			lv_obj_set_hidden(bottom2_panel, true);
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
			menu_btn_flag[MENU_ID_LCD] = 0;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_led_test_page());
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
			menu_btn_flag[MENU_ID_LCD] = 1;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
			menu_tree_push(get_menu_led_test_page());
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
		lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, COLOR_BLUE);
		lv_style_set_text_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);

		lv_style_init(&lcd_test_border_style);
		lv_style_set_text_font(&lcd_test_border_style, LV_STATE_DEFAULT, &stsong_22);
		lv_style_set_bg_color(&lcd_test_border_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		lv_style_set_text_color(&lcd_test_border_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
		lv_style_set_border_side(&lcd_test_border_style, LV_STATE_DEFAULT, LV_BORDER_SIDE_FULL);
		//lv_style_set_border_color(&lcd_test_border_style, LV_STATE_DEFAULT, LV_COLOR_BLUE);
		lv_style_set_border_color(&lcd_test_border_style, LV_STATE_FOCUSED, LV_COLOR_WHITE);
		lv_style_set_border_width(&lcd_test_border_style, LV_STATE_DEFAULT, 5);

		
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
#endif

static void create_disp_panel(lv_obj_t * par)
{
    //lv_obj_t *disp_panel = menu_page_create_disp_panel(par);
	//lv_obj_t *disp_panel = lv_cont_create(par, back_btn_event_cb);
	disp_panel = lv_cont_create(par, NULL);
	lv_obj_set_event_cb(disp_panel, change_btn_event_cb);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_pos(disp_panel, 0, 0);
	lv_obj_add_style(disp_panel, LV_CONT_PART_MAIN, &disp_panel_style);
    lv_cont_set_layout(disp_panel, LV_LAYOUT_CENTER);
	//_lv_obj_set_style_local_color(lv_obj_t * obj, uint8_t part, lv_style_property_t prop, lv_color_t color)

	lv_obj_t * show_label = lv_label_create(disp_panel, NULL);
    //lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
    lv_obj_add_style(show_label, LV_STATE_DEFAULT, &disp_panel_style);
    lv_label_set_text(show_label, _("Touch Screen"));
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(show_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	
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
static void menu_lcd_test_create()
{
    _style_init();
	
    menu_lcd_test_page_obj = lv_cont_create(get_test_mode_page_flag()==1 ? lv_scr_menu_test_mode():lv_scr_menu_factory_mode(), NULL);
    lv_obj_add_style(menu_lcd_test_page_obj, LV_CONT_PART_MAIN, menu_page_test_mode_title_style());
    lv_obj_set_size(menu_lcd_test_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    
    create_disp_panel(menu_lcd_test_page_obj);
	//create_top_panel(menu_lcd_test_page_obj);
	//create_bottom_panel(menu_lcd_test_page_obj);
	
	bottom_panel_page_obj = create_bottom_2btn_panel2(menu_lcd_test_page_obj,&bottom1_panel,&bottom2_panel, pass_btn_event_cb, error_btn_event_cb);
	//lv_obj_set_hidden(bottom_panel_page_obj, false);
	lv_obj_set_hidden(bottom_panel_page_obj, true);
	lv_obj_set_hidden(bottom1_panel, true);
	lv_obj_set_hidden(bottom2_panel, true);
	//create_bottom_2btn_panel(menu_lcd_test_page_obj, pass_btn_event_cb, error_btn_event_cb);

	img_gray_scale = lv_img_create(menu_lcd_test_page_obj, NULL);
    lv_img_set_src(img_gray_scale, RES_IMG_GRAY_SCALE);
    lv_img_set_auto_size(img_gray_scale, true);
    lv_obj_set_pos(img_gray_scale, 0, 0);
	lv_obj_set_hidden(img_gray_scale, true);

	img_color_scale = lv_img_create(menu_lcd_test_page_obj, NULL);
    lv_img_set_src(img_color_scale, RES_IMG_COLOR_SCALE);
    lv_img_set_auto_size(img_color_scale, true);
    lv_obj_set_pos(img_color_scale, 0, 0);
	lv_obj_set_hidden(img_color_scale, true);

    lv_obj_set_hidden(menu_lcd_test_page_obj, true);
}

lv_obj_t * lv_scr_menu_lcd_mode()
{
    return menu_lcd_test_page_obj;
}

static void menu_lcd_test_destroy()
{
    if (menu_lcd_test_page_obj)
    {
        lv_obj_del(menu_lcd_test_page_obj);
        menu_lcd_test_page_obj = NULL;
    }
}

static void menu_lcd_test_show()
{
    if (menu_lcd_test_page_obj)
    {
        lv_obj_set_hidden(menu_lcd_test_page_obj, false);
    }
}

static void menu_lcd_test_hide()
{
    if (menu_lcd_test_page_obj)
    {
        lv_obj_set_hidden(menu_lcd_test_page_obj, true);
    }
}
/// 页面生命周期函数 end


