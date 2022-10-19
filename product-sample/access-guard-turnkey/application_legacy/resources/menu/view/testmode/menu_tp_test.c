#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include "lv_drivers/indev/evdev.h"

#include "menu.h"
#include "menu_common.h"
#include "menu_page.h"
#include "resource.h"
#include "lv_i18n.h"
#include "menu_tp_test.h"
#include "menu_test_mode.h"
#include "menu_camera_test.h"
#include "menu_factory_mode.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <inttypes.h>
#include <errno.h>
#include "app_utils.h"

/*个性化与通知反馈页面*/

static lv_obj_t *menu_tp_test_page_obj = NULL;
static lv_obj_t *bottom_panel_page_obj = NULL;
static lv_obj_t *bottom1_panel = NULL;
static lv_obj_t *bottom2_panel = NULL;
//static lv_obj_t *disp_btn_panel = NULL;

static lv_style_t disp_panel_style;
static lv_style_t disp_btn_panel_style;
static lv_obj_t * show_label = 0;

static int flag = 0;
int pressding_flag = 0;
#define CANVAS_WIDTH  DISP_WIDTH
#define CANVAS_HEIGHT  DISP_HEIGHT
lv_obj_t * canvas = NULL;
static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(CANVAS_WIDTH, CANVAS_HEIGHT)];


static void menu_tp_test_create();
static void menu_tp_test_destroy();
static void menu_tp_test_show();
static void menu_tp_test_hide();

static menu_page_t menu_tp_test_page = {
    .onCreate = menu_tp_test_create,
    .onDestroy = menu_tp_test_destroy,
    .show = menu_tp_test_show,
    .hide = menu_tp_test_hide,
    .back = menuPageBack
};

menu_page_t get_menu_tp_test_page()
{
    return menu_tp_test_page;
}

static lv_point_t points[3];
		

static void change_canvas_event_cb(lv_obj_t * obj, lv_event_t event)
{
	printf("[event:%d %s,%d]\n",event,__FUNCTION__,__LINE__);
	uint32_t x;
    uint32_t y;
	lv_color_t c0;
	lv_draw_line_dsc_t line_dsc;
	lv_point_t pt;
	char tmpBuf[64] = {0};
	memset(&pt, 0x00, sizeof(pt));

	get_point(&pt);
	sprintf(tmpBuf,"X:%d Y:%d",pt.x,pt.y);
	lv_label_set_text(show_label, tmpBuf);

	//printf("[event:x:%d y:%d %s,%d]\n",pt.x,pt.y,__FUNCTION__,__LINE__);
	if (LV_EVENT_PRESSING == event)
    {
    	pressding_flag++;
		lv_draw_line_dsc_init(&line_dsc);
		
		line_dsc.color = LV_COLOR_BLUE;
		line_dsc.width = 12;

		if(0 == points[0].x && 0 == points[0].y && 0 == points[1].x && 0 == points[1].y)
		{
			points[0].x = pt.x;
			points[0].y = pt.y;
			
			points[1].x = pt.x;
			points[1].y = pt.y;
		}
		else
		{
			points[0].x = points[1].x;
			points[0].y = points[1].y;
			
			points[1].x = pt.x;
			points[1].y = pt.y;
			lv_canvas_draw_line(canvas, points, 2,&line_dsc);
		}
    }
	else if ((LV_EVENT_SHORT_CLICKED == event) &&(pressding_flag <= 1))
	{
		//c0.full = 1;
		c0 = LV_COLOR_BLUE;
		for( y = pt.y; y < 12+pt.y; y++) {
	        for( x = pt.x; x < 12+pt.x; x++) {
	            lv_canvas_set_px(canvas, x, y, c0);
	        }
	    }
	}
	else if (LV_EVENT_PRESSED == event)
	{
		points[0].x = 0;
		points[0].y = 0;
		
		points[1].x = 0;
		points[1].y = 0;
		lv_canvas_fill_bg(canvas, LV_COLOR_SILVER, LV_OPA_COVER);

		if(5 == flag++)
		{
			lv_obj_set_hidden(bottom1_panel, false);
			lv_obj_set_hidden(bottom2_panel, false);
			lv_obj_set_hidden(bottom_panel_page_obj, false);
		}
		else if(flag > 5)
		{
			flag = 0;
			lv_obj_set_hidden(bottom_panel_page_obj, true);
			lv_obj_set_hidden(bottom1_panel, true);
			lv_obj_set_hidden(bottom2_panel, true);
		}
	}
	else if (LV_EVENT_RELEASED == event)
	{
		pressding_flag = 0;
	}
}

static void pass_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {


		//menu_test_mode_PageBack();
		if (menu_tree_isEmpty())
                return;
		if(get_test_mode_page_flag() == 1)
		{
			menu_btn_flag[MENU_ID_TP] = 0;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_camera_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
		}
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
			menu_btn_flag[MENU_ID_TP] = 1;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
			menu_tree_push(get_menu_camera_test_page());
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
		lv_style_set_text_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_RED);

		lv_style_init(&disp_btn_panel_style);
		lv_style_set_text_font(&disp_btn_panel_style, LV_STATE_DEFAULT, &stsong_22);
		lv_style_set_bg_color(&disp_btn_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLUE);
		lv_style_set_bg_color(&disp_btn_panel_style, LV_STATE_PRESSED, LV_COLOR_RED);
		lv_style_set_text_color(&disp_btn_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);

		points[0].x = 0;
		points[0].y = 0;
		points[1].x = 0;
		points[1].y = 0;
		points[2].x = 0;
		points[2].y = 0;
        styleInited = true;
    }
}
static void create_canvas_panel(lv_obj_t * par)
{
    /*Create a canvas and initialize its the palette*/
    canvas = lv_canvas_create(par, NULL);

    lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    lv_obj_align(canvas, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_canvas_fill_bg(canvas, LV_COLOR_SILVER, LV_OPA_COVER);
	lv_obj_set_click(canvas, true);
	lv_obj_set_event_cb(canvas, change_canvas_event_cb);
	
}

/// 页面生命周期函数 start
static void menu_tp_test_create()
{
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
    _style_init();
	
    menu_tp_test_page_obj = lv_cont_create(get_test_mode_page_flag()==1 ? lv_scr_menu_test_mode():lv_scr_menu_factory_mode(), NULL);
    lv_obj_add_style(menu_tp_test_page_obj, LV_CONT_PART_MAIN, menu_page_test_mode_title_style());
    lv_obj_set_size(menu_tp_test_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    
    create_canvas_panel(menu_tp_test_page_obj);
	
	show_label = lv_label_create(canvas, NULL);
    //lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
    lv_obj_add_style(show_label, LV_STATE_DEFAULT, &disp_panel_style);
    lv_label_set_text(show_label, "");
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(show_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	
	bottom_panel_page_obj = create_bottom_2btn_panel2(canvas, &bottom1_panel, &bottom2_panel, pass_btn_event_cb, error_btn_event_cb);
	//lv_obj_set_hidden(bottom_panel_page_obj, false);
	lv_obj_set_hidden(bottom_panel_page_obj, true);
	lv_obj_set_hidden(bottom1_panel, true);
	lv_obj_set_hidden(bottom2_panel, true);

	//pthread_create(&tp_test_id,NULL,tp_test_thread,NULL);
	//create_bottom_2btn_panel(menu_lcd_test_page_obj, pass_btn_event_cb, error_btn_event_cb);

    //lv_obj_set_hidden(menu_tp_test_page_obj, true);
}

lv_obj_t * lv_scr_menu_tp_mode()
{
    return menu_tp_test_page_obj;
}

static void menu_tp_test_destroy()
{
    if (menu_tp_test_page_obj)
    {
        lv_obj_del(menu_tp_test_page_obj);
        menu_tp_test_page_obj = NULL;
    }
}

static void menu_tp_test_show()
{
    if (menu_tp_test_page_obj)
    {
        lv_obj_set_hidden(menu_tp_test_page_obj, false);
    }
}

static void menu_tp_test_hide()
{
    if (menu_tp_test_page_obj)
    {
        lv_obj_set_hidden(menu_tp_test_page_obj, true);
    }
}
/// 页面生命周期函数 end


