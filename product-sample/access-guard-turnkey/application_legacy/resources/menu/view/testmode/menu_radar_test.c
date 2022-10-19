#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <pthread.h>
#include "menu.h"
#include "menu_common.h"
#include "menu_page.h"
#include "resource.h"
#include "lv_i18n.h"
#include "menu_test_mode.h"
#include "menu_rs485_wg_test.h"
#include "menu_radar_test.h"
#include "menu_usb_test.h"
#include "factory.h"
#include "app_comp.h"
#include "menu_factory_mode.h"


/*个性化与通知反馈页面*/

static lv_obj_t *menu_radar_test_page_obj = NULL;
static lv_obj_t *bottom_panel_page_obj = NULL;
//static lv_obj_t *disp_btn_panel = NULL;
static lv_obj_t *radar_disp_panel = NULL;
static lv_obj_t * show_label = NULL;
static lv_style_t disp_panel_style;
static lv_style_t come_disp_panel_style;
static lv_style_t leave_disp_panel_style;
static lv_style_t disp_btn_panel_style;
static lv_style_t disp_radar_panel_style;
#define LAUNCHER_MAIN_OPACITY (LV_OPA_TRANSP)
#define ALTERNATE_UI_OPACITY (LV_OPA_50)


static int flag = 0;

static void menu_radar_test_create();
static void menu_radar_test_destroy();
static void menu_radar_test_show();
static void menu_radar_test_hide();

static menu_page_t menu_radar_test_page = {
    .onCreate = menu_radar_test_create,
    .onDestroy = menu_radar_test_destroy,
    .show = menu_radar_test_show,
    .hide = menu_radar_test_hide,
    .back = menuPageBack
};

menu_page_t get_menu_radar_test_page()
{
    return menu_radar_test_page;
}
static pthread_t radar_id = 0;

void * radar_check_thread(void *arg)
{
	int fd;
	int ret = -1;
	struct input_event t;
	fd_set readfds;
	struct timeval tv;

	fd = open("/dev/input/event0", O_RDWR);
	if (fd < 0) {
		printf("open radar dev failed.\n");
		return NULL;
	}

	
	while(0 == flag)
	{
		//printf("[flag:%d %s,%d]\n",flag,__FUNCTION__,__LINE__);

		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
		tv.tv_sec=0;
		tv.tv_usec=20*1000;//tv.tv_usec=20*1000;
		ret = select(fd+1, &readfds, NULL, NULL, &tv);

		if(ret < 0)
		{
			printf("select radar fd error\n");
			continue;
		}

		if(FD_ISSET(fd, &readfds))
		{
			if (read(fd, &t, sizeof (t)) == sizeof (t)) {
				if (t.type == EV_KEY) {
					if (t.value == 0 || t.value == 1) {
						printf ("key 0x%03X %s\n", t.code, (t.value) ? "person come" : "person leave");
						if(0 == flag)
						{
							if(t.value)
							{
								lv_obj_add_style(radar_disp_panel, LV_CONT_PART_MAIN, &come_disp_panel_style);
								lv_label_set_text(show_label, _("person come"));
							}
							else
							{
								lv_obj_add_style(radar_disp_panel, LV_CONT_PART_MAIN, &leave_disp_panel_style);
								lv_label_set_text(show_label, _("person leave"));
							}
						}
						//if (t.code == KEY_ESC)
						//	return;
					}
				}
			}
		}
		//printf("[flag:%d %s,%d]\n",flag,__FUNCTION__,__LINE__);
	};
	
	close(fd);
	return NULL;
}
static void pass_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
		printf("[%s,%d]\n",__FUNCTION__,__LINE__);

		//menu_test_mode_PageBack();
		if (menu_tree_isEmpty())
                return;
		if(get_test_mode_page_flag() == 1)
		{
			menu_btn_flag[MENU_ID_RADAR] = 0;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_usb_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
		}
			flag = 1;
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
			menu_btn_flag[MENU_ID_RADAR] = 1;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
			menu_tree_push(get_menu_usb_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
		}
			flag = 1;
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

		lv_style_init(&come_disp_panel_style);
		lv_style_set_text_font(&come_disp_panel_style, LV_STATE_DEFAULT, &stsong_28);
		lv_style_set_bg_color(&come_disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
		lv_style_set_text_color(&come_disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_RED);

		lv_style_init(&leave_disp_panel_style);
		lv_style_set_text_font(&leave_disp_panel_style, LV_STATE_DEFAULT, &stsong_28);
		lv_style_set_bg_color(&leave_disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
		lv_style_set_text_color(&leave_disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLUE);

		lv_style_init(&disp_btn_panel_style);
		lv_style_set_text_font(&disp_btn_panel_style, LV_STATE_DEFAULT, &stsong_28);
		lv_style_set_bg_color(&disp_btn_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLUE);
		lv_style_set_bg_color(&disp_btn_panel_style, LV_STATE_PRESSED, LV_COLOR_RED);
		lv_style_set_text_color(&disp_btn_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);

		lv_style_init(&disp_radar_panel_style);
		lv_style_set_bg_opa(&disp_radar_panel_style, LV_STATE_DEFAULT, LAUNCHER_MAIN_OPACITY);
		lv_style_set_opa_scale(&disp_radar_panel_style, LV_STATE_DEFAULT, ALTERNATE_UI_OPACITY);
		
        styleInited = true;
    }
}

/*页面内部layout*/
static void create_top_panel(lv_obj_t * par)
{
    lv_obj_t *top_panel = menu_page_create_test_mode_top_panel(par);

    lv_obj_t *page_title = lv_label_create(top_panel, NULL);
    lv_label_set_text(page_title, _("Radar test"));
    lv_obj_align(page_title, NULL, LV_ALIGN_CENTER, -20, 0);
    lv_obj_add_style(page_title, LV_LABEL_PART_MAIN, menu_page_title_style());
}

static void create_disp_panel(lv_obj_t * par)
{
    //lv_obj_t *disp_panel = menu_page_create_disp_panel(par);
	//lv_obj_t *disp_panel = lv_cont_create(par, back_btn_event_cb);
	radar_disp_panel = lv_cont_create(par, NULL);
	//lv_obj_set_event_cb(disp_panel, change_btn_event_cb);
    lv_obj_set_size(radar_disp_panel, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_obj_set_pos(radar_disp_panel, 0, TOP_TEST_MODE_PANEL_H);
	lv_obj_add_style(radar_disp_panel, LV_CONT_PART_MAIN, &disp_panel_style);
    lv_cont_set_layout(radar_disp_panel, LV_LAYOUT_CENTER);
	//_lv_obj_set_style_local_color(lv_obj_t * obj, uint8_t part, lv_style_property_t prop, lv_color_t color)

	show_label = lv_label_create(radar_disp_panel, NULL);
    //lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
    lv_obj_add_style(show_label, LV_STATE_DEFAULT, &disp_panel_style);
    //lv_label_set_text(show_label, _("Swipe your card"));
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(show_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	
}

/// 页面生命周期函数 start
static void menu_radar_test_create()
{
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
    _style_init();
	flag = 0;

	pthread_create(&radar_id,NULL,radar_check_thread,NULL);
	
    menu_radar_test_page_obj = lv_cont_create(get_test_mode_page_flag()==1 ? lv_scr_menu_test_mode():lv_scr_menu_factory_mode(), NULL);
    lv_obj_add_style(menu_radar_test_page_obj, LV_CONT_PART_MAIN, menu_page_test_mode_title_style());
    lv_obj_set_size(menu_radar_test_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	//lv_obj_set_event_cb(menu_radar_test_page_obj, change_btn_event_cb);
	create_top_panel(menu_radar_test_page_obj);
    create_disp_panel(menu_radar_test_page_obj);
    
	bottom_panel_page_obj = create_bottom_2btn_panel(menu_radar_test_page_obj, pass_btn_event_cb, error_btn_event_cb);
	//lv_obj_set_hidden(bottom_panel_page_obj, false);
	lv_obj_set_hidden(bottom_panel_page_obj, false);
	//create_bottom_2btn_panel(menu_lcd_test_page_obj, pass_btn_event_cb, error_btn_event_cb);

    lv_obj_set_hidden(menu_radar_test_page_obj, true);
}

lv_obj_t * lv_scr_menu_radar_mode()
{
    return menu_radar_test_page_obj;
}

static void menu_radar_test_destroy()
{
    if (menu_radar_test_page_obj)
    {
        lv_obj_del(menu_radar_test_page_obj);
        menu_radar_test_page_obj = NULL;
		flag = 1;
    }
}

static void menu_radar_test_show()
{
    if (menu_radar_test_page_obj)
    {
        lv_obj_set_hidden(menu_radar_test_page_obj, false);
    }
}

static void menu_radar_test_hide()
{
    if (menu_radar_test_page_obj)
    {
        lv_obj_set_hidden(menu_radar_test_page_obj, true);
    }
}
/// 页面生命周期函数 end


