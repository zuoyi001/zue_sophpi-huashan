#include "lvgl/lvgl.h"
#include "menu_setting.h"
#include "launcher.h"
#include "menu_common.h"
#include "menu_face_lib.h"
#include "menu_face_demo.h"
#include "menu_device_manage.h"
#include "menu_device_activate.h"
#include "menu_access_control.h"
#include "menu_lcd_test.h"
#include "menu_led_test.h"
#include "menu_audio_test.h"
#include "menu_relay_test.h"
#include "menu_tp_test.h"
#include "menu_camera_test.h"
#include "menu_nfc_test.h"
#include "menu_mic_test.h"
#include "menu_wifi_test.h"
#include "menu_rs485_wg_test.h"
#include "menu_usb_test.h"
#include "menu_key_test.h"
#include "menu_radar_test.h"
#include "menu_eth_test.h"
#include "menu_about_test.h"
#include "menu_test_mode.h"
#include "lv_i18n.h"
#include "menu.h"
#include "resource.h"
#include "menu_page.h"
#include "app_peripheral.h"
#include "app_utils.h"

int test_mode_page_flag = 0;

static void mainMenuPageCreate(void);
static void mainMenuPageDestroy(void);
static void mainMenu_page_show();
static void mainMenu_page_hide();

static lv_obj_t * scr_menu_test_mode = NULL;
static lv_obj_t * mainMenuTestModePage_cont = NULL;

static menu_page_t mainMenuTestModePage = {
    .onCreate = mainMenuPageCreate,
    .onDestroy = mainMenuPageDestroy,
    .show = mainMenu_page_show,
    .hide = mainMenu_page_hide,
    .back = menu_test_mode_PageBack
};

void set_test_mode_page_flag(int flag)
{
	
	test_mode_page_flag = flag;
}
int get_test_mode_page_flag(void)
{
	return test_mode_page_flag;
}

static void mainMenu_back_event_cb(lv_obj_t * obj, lv_event_t event)
{
	switch (event)
	{
	case LV_EVENT_RELEASED:
        printf("mainMenu_back_event_cb.\n");
        menu_scr_hide();
        menu_test_mode_PageBack();
		break;
	default:
		break;
	}
}

static void testMenu_page_navi_cb(lv_obj_t * obj, lv_event_t event)
{
	intptr_t page_id = (intptr_t) lv_obj_get_user_data(obj);

	printf("event:%d \n",event);

	switch (event)
	{
	case LV_EVENT_RELEASED:
	{
		if (page_id <= MENU_ID_TEST_INVALID || page_id >= MENU_ID_TEST_END)
			break;
		
		switch (page_id)
		{
		case MENU_ID_ABOUT:
			printf("show DEV_ACT.\n");
			if (menu_tree_isEmpty())
				break;
			menu_tree_get_top().hide();
			menu_tree_push(get_menu_about_test_page());
			menu_tree_get_top().onCreate();
			menu_tree_get_top().show();
			break;
		case MENU_ID_LCD:
			if (menu_tree_isEmpty())
				break;
			menu_tree_get_top().hide();
			menu_tree_push(get_menu_lcd_test_page());
			menu_tree_get_top().onCreate();
			menu_tree_get_top().show();
			break;
		case MENU_ID_LED:
			printf("show device manage.\n");
			if (menu_tree_isEmpty())
				break;
			menu_tree_get_top().hide();
			menu_tree_push(get_menu_led_test_page());
			menu_tree_get_top().onCreate();
			menu_tree_get_top().show();
			break;
		case MENU_ID_AUDIO:
			printf("show Face lib ui.\n");
			if (menu_tree_isEmpty())
				break;
			menu_tree_get_top().hide();
			menu_tree_push(get_menu_audio_test_page());
			menu_tree_get_top().onCreate();
			menu_tree_get_top().show();
			break;
		case MENU_ID_RELAY:
			printf("show profile notify ui.\n");
			if (menu_tree_isEmpty())
				break;
			menu_tree_get_top().hide();
			menu_tree_push(get_menu_relay_test_page());
			menu_tree_get_top().onCreate();
			menu_tree_get_top().show();
			break;
		case MENU_ID_TP:
			printf("show profile notify ui.\n");
			if (menu_tree_isEmpty())
				break;
			menu_tree_get_top().hide();
			menu_tree_push(get_menu_tp_test_page());
			menu_tree_get_top().onCreate();
			menu_tree_get_top().show();
			break;
		case MENU_ID_CAMERA:
			printf("show profile notify ui.\n");
			if (menu_tree_isEmpty())
				break;
			menu_tree_get_top().hide();
			menu_tree_push(get_menu_camera_test_page());
			menu_tree_get_top().onCreate();
			menu_tree_get_top().show();
			break;
		case MENU_ID_NFC:
			printf("show profile notify ui.\n");
			if (menu_tree_isEmpty())
				break;
			menu_tree_get_top().hide();
			menu_tree_push(get_menu_nfc_test_page());
			menu_tree_get_top().onCreate();
			menu_tree_get_top().show();
			break;
		case MENU_ID_MIC:
			printf("show profile notify ui.\n");
			if (menu_tree_isEmpty())
				break;
			menu_tree_get_top().hide();
			menu_tree_push(get_menu_mic_test_page());
			menu_tree_get_top().onCreate();
			menu_tree_get_top().show();
			break;
		case MENU_ID_WIFI:
			printf("show profile notify ui.\n");
			if (menu_tree_isEmpty())
				break;
			menu_tree_get_top().hide();
			menu_tree_push(get_menu_wifi_test_page());
			menu_tree_get_top().onCreate();
			menu_tree_get_top().show();
			break;
		case MENU_ID_RS485_WG:
			printf("show profile notify ui.\n");
			if (menu_tree_isEmpty())
				break;
			menu_tree_get_top().hide();
			menu_tree_push(get_menu_rs485_wg_test_page());
			menu_tree_get_top().onCreate();
			menu_tree_get_top().show();
			break;
		case MENU_ID_USB:
			printf("show profile notify ui.\n");
			if (menu_tree_isEmpty())
				break;
			menu_tree_get_top().hide();
			menu_tree_push(get_menu_usb_test_page());
			menu_tree_get_top().onCreate();
			menu_tree_get_top().show();
			break;

		case MENU_ID_KEY:
			printf("show profile notify ui.\n");
			if (menu_tree_isEmpty())
				break;
			menu_tree_get_top().hide();
			menu_tree_push(get_menu_key_test_page());
			menu_tree_get_top().onCreate();
			menu_tree_get_top().show();
			break;
		case MENU_ID_RADAR:
			printf("show profile notify ui.\n");
			if (menu_tree_isEmpty())
				break;
			menu_tree_get_top().hide();
			menu_tree_push(get_menu_radar_test_page());
			menu_tree_get_top().onCreate();
			menu_tree_get_top().show();
			break;
		case MENU_ID_ETH:
			printf("show profile notify ui.\n");
			if (menu_tree_isEmpty())
				break;
			menu_tree_get_top().hide();
			menu_tree_push(get_menu_eth_test_page());
			menu_tree_get_top().onCreate();
			menu_tree_get_top().show();
			break;
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}



void menu_test_mode_scr_show()
{
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
    if (scr_menu_test_mode) {
		printf("[%s,%d] \n",__FUNCTION__,__LINE__);
        lv_scr_load(scr_menu_test_mode);
        menu_tree_push(mainMenuTestModePage);
        menu_tree_get_top().onCreate();
        menu_tree_get_top().show();
    }
}

void menu_test_mode_scr_hide()
{

}

// static lv_obj_t *dev_act_label;
/*
static lv_obj_t *mode_switch_label;
static lv_obj_t *dev_manage_label;
static lv_obj_t *dev_info_label;
static lv_obj_t *face_lib_label;
static lv_obj_t *access_ctrl_label;
static lv_obj_t *notification_label;
static lv_obj_t *lang_label;
*/
//static lv_obj_t *vs_label;
//static lv_obj_t *sn_label;
//static lv_obj_t *mac_label;


static lv_obj_t * bottom1_panel = NULL;
static lv_obj_t * bottom2_panel = NULL;
static lv_obj_t * bottom3_panel = NULL;
static lv_obj_t * bottom21_panel = NULL;
static lv_obj_t * bottom22_panel = NULL;
static lv_obj_t * bottom23_panel = NULL;
static lv_obj_t * bottom31_panel = NULL;
static lv_obj_t * bottom32_panel = NULL;
static lv_obj_t * bottom33_panel = NULL;
static lv_obj_t * bottom41_panel = NULL;
static lv_obj_t * bottom42_panel = NULL;
static lv_obj_t * bottom43_panel = NULL;
static lv_obj_t * bottom51_panel = NULL;
static lv_obj_t * bottom52_panel = NULL;
static lv_obj_t * bottom53_panel = NULL;




static lv_obj_t *title;

static lv_style_t style_test_mode_sb;
static lv_style_t btn_label_test_mode_style;
static lv_style_t btn_menu_style;
static lv_style_t fail_btn_menu_style;


int menu_btn_flag[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static bool style_test_mode_inited = false;
static void _style_init()
{
    if (!style_test_mode_inited)
    {
        lv_style_init(&style_test_mode_sb);
        lv_style_set_bg_color(&style_test_mode_sb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_bg_grad_color(&style_test_mode_sb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_border_color(&style_test_mode_sb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_border_width(&style_test_mode_sb, LV_STATE_DEFAULT, 1);
        lv_style_set_border_opa(&style_test_mode_sb, LV_STATE_DEFAULT, LV_OPA_70);
        lv_style_set_bg_opa(&style_test_mode_sb, LV_STATE_DEFAULT, LV_OPA_60);
        lv_style_set_pad_right(&style_test_mode_sb, LV_STATE_DEFAULT, 3);
        lv_style_set_pad_bottom(&style_test_mode_sb, LV_STATE_DEFAULT, 3);
        lv_style_set_pad_inner(&style_test_mode_sb, LV_STATE_DEFAULT, 8);

       //lv_style_init(&btn_label_style);
        //lv_style_set_text_font(&btn_label_style, LV_STATE_DEFAULT, &stsong_22);
		lv_style_init(&btn_label_test_mode_style);
		lv_style_set_text_color(&btn_label_test_mode_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		lv_style_set_text_color(&btn_label_test_mode_style, LV_STATE_PRESSED, LV_COLOR_WHITE);
		lv_style_set_text_font(&btn_label_test_mode_style, LV_STATE_DEFAULT, &stsong_28);

		lv_style_init(&btn_menu_style);
		lv_style_set_text_color(&btn_menu_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		lv_style_set_text_color(&btn_menu_style, LV_STATE_PRESSED, LV_COLOR_WHITE);
		lv_style_set_text_font(&btn_menu_style, LV_STATE_DEFAULT, &stsong_28);
		lv_style_set_bg_color(&btn_menu_style, LV_STATE_DEFAULT, COLOR_GREEN);
		lv_style_set_bg_color(&btn_menu_style, LV_STATE_PRESSED, COLOR_RED);

		lv_style_init(&fail_btn_menu_style);
		lv_style_set_text_color(&fail_btn_menu_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		lv_style_set_text_color(&fail_btn_menu_style, LV_STATE_PRESSED, LV_COLOR_WHITE);
		lv_style_set_text_font(&fail_btn_menu_style, LV_STATE_DEFAULT, &stsong_28);
		lv_style_set_bg_color(&fail_btn_menu_style, LV_STATE_DEFAULT, COLOR_RED);
		lv_style_set_bg_color(&fail_btn_menu_style, LV_STATE_PRESSED, COLOR_RED);
		
        style_test_mode_inited = true;
    }
}
static void create_top_panel(lv_obj_t *par)
{
    lv_obj_t * top_panel = menu_page_create_test2_mode_top_panel(par,mainMenu_back_event_cb);

    title = lv_label_create(top_panel, NULL);
    lv_obj_add_style(title, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
    lv_label_set_text(title, _("Test mode"));
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(title, NULL, LV_ALIGN_CENTER, 0, 0);
}

static void create_menu_btn_panel(lv_obj_t *par)
{
	int row = 0;
	//1:1
	bottom1_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom1_panel, LV_MENU_WIDTH, LV_MENU_HEIGHT);
	lv_obj_set_pos(bottom1_panel, 11, (TOP_TEST_MODE_PANEL_H+10+row*(LV_MENU_HEIGHT+10)));
	if(0 == menu_btn_flag[MENU_ID_ABOUT])
	{
		lv_obj_add_style(bottom1_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom1_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	lv_obj_set_user_data(bottom1_panel, (lv_obj_user_data_t)MENU_ID_ABOUT);
	lv_obj_set_event_cb(bottom1_panel, testMenu_page_navi_cb);
	lv_obj_t *next_label = lv_label_create(bottom1_panel, NULL);
	//lv_obj_set_click(next_label, true);
	lv_obj_set_size(next_label, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_label, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	//lv_obj_set_event_cb(next_label, cb0);
	//lv_obj_add_style(next_label, LV_LABEL_PART_MAIN, &btn_label_bottom_style);
	lv_label_set_text(next_label, _("About Device"));
	//1:2
	bottom2_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom2_panel, LV_MENU_WIDTH, LV_MENU_HEIGHT);
	lv_obj_set_pos(bottom2_panel, 2*11+LV_MENU_WIDTH, (TOP_TEST_MODE_PANEL_H+10+row*(LV_MENU_HEIGHT+10)));
	if(0 == menu_btn_flag[MENU_ID_LCD])
	{
		lv_obj_add_style(bottom2_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom2_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	lv_obj_set_user_data(bottom2_panel, (lv_obj_user_data_t)MENU_ID_LCD);
	lv_obj_set_event_cb(bottom2_panel, testMenu_page_navi_cb);
	lv_obj_t *next_labe2 = lv_label_create(bottom2_panel, NULL);
	//lv_obj_set_click(next_labe2, true);
	lv_obj_set_size(next_labe2, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_labe2, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	lv_label_set_text(next_labe2, _("LCD Test"));
	//1:3
	bottom3_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom3_panel, LV_MENU_WIDTH, LV_MENU_HEIGHT);
	lv_obj_set_pos(bottom3_panel, 11+2*(10+LV_MENU_WIDTH), (TOP_TEST_MODE_PANEL_H+10+row*(LV_MENU_HEIGHT+10)));
	if(0 == menu_btn_flag[MENU_ID_LED])
	{
		lv_obj_add_style(bottom3_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom3_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	lv_obj_set_user_data(bottom3_panel, (lv_obj_user_data_t)MENU_ID_LED);
	lv_obj_set_event_cb(bottom3_panel, testMenu_page_navi_cb);
	lv_obj_t *next_labe3 = lv_label_create(bottom3_panel, NULL);
	//lv_obj_set_click(next_labe3, true);
	lv_obj_set_size(next_labe3, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_labe3, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	lv_label_set_text(next_labe3, _("LED Test"));

	//2:1
	row =1;
	bottom21_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom21_panel, LV_MENU_WIDTH, LV_MENU_HEIGHT);
	lv_obj_set_pos(bottom21_panel, 11, (TOP_TEST_MODE_PANEL_H+10+row*(LV_MENU_HEIGHT+10)));
	if(0 == menu_btn_flag[MENU_ID_AUDIO])
	{
		lv_obj_add_style(bottom21_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom21_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	lv_obj_set_user_data(bottom21_panel, (lv_obj_user_data_t)MENU_ID_AUDIO);
	lv_obj_set_event_cb(bottom21_panel, testMenu_page_navi_cb);
	lv_obj_t *next_labe2l = lv_label_create(bottom21_panel, NULL);
	//lv_obj_set_click(next_labe2l, true);
	lv_obj_set_size(next_labe2l, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_labe2l, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	//lv_obj_set_event_cb(next_label, cb0);
	//lv_obj_add_style(next_label, LV_LABEL_PART_MAIN, &btn_label_bottom_style);
	lv_label_set_text(next_labe2l, _("Sound Test"));
	//2:2
	bottom22_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom22_panel, LV_MENU_WIDTH, LV_MENU_HEIGHT);
	lv_obj_set_pos(bottom22_panel, 2*11+LV_MENU_WIDTH, (TOP_TEST_MODE_PANEL_H+10+row*(LV_MENU_HEIGHT+10)));
	if(0 == menu_btn_flag[MENU_ID_RELAY])
	{
		lv_obj_add_style(bottom22_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom22_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	lv_obj_set_user_data(bottom22_panel, (lv_obj_user_data_t)MENU_ID_RELAY);
	lv_obj_set_event_cb(bottom22_panel, testMenu_page_navi_cb);
	lv_obj_t *next_labe22 = lv_label_create(bottom22_panel, NULL);
	//lv_obj_set_click(next_labe22, true);
	lv_obj_set_size(next_labe22, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_labe22, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	lv_label_set_text(next_labe22, _("Relay Test"));
	//2:3
	bottom23_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom23_panel, LV_MENU_WIDTH, LV_MENU_HEIGHT);
	lv_obj_set_pos(bottom23_panel, 11+2*(10+LV_MENU_WIDTH), (TOP_TEST_MODE_PANEL_H+10+row*(LV_MENU_HEIGHT+10)));
	if(0 == menu_btn_flag[MENU_ID_TP])
	{
		lv_obj_add_style(bottom23_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom23_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	lv_obj_set_user_data(bottom23_panel, (lv_obj_user_data_t)MENU_ID_TP);
	lv_obj_set_event_cb(bottom23_panel, testMenu_page_navi_cb);
	lv_obj_t *next_labe23 = lv_label_create(bottom23_panel, NULL);
	//lv_obj_set_click(next_labe23, true);
	lv_obj_set_size(next_labe23, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_labe23, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	lv_label_set_text(next_labe23, _("TP Test"));

	//3:1
	row =2;
	bottom31_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom31_panel, LV_MENU_WIDTH, LV_MENU_HEIGHT);
	lv_obj_set_pos(bottom31_panel, 11, (TOP_TEST_MODE_PANEL_H+10+row*(LV_MENU_HEIGHT+10)));
	if(0 == menu_btn_flag[MENU_ID_CAMERA])
	{
		lv_obj_add_style(bottom31_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom31_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	lv_obj_set_user_data(bottom31_panel, (lv_obj_user_data_t)MENU_ID_CAMERA);
	lv_obj_set_event_cb(bottom31_panel, testMenu_page_navi_cb);
	lv_obj_t *next_labe3l = lv_label_create(bottom31_panel, NULL);
	//lv_obj_set_click(next_labe3l, true);
	lv_obj_set_size(next_labe3l, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_labe3l, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	//lv_obj_set_event_cb(next_label, cb0);
	//lv_obj_add_style(next_label, LV_LABEL_PART_MAIN, &btn_label_bottom_style);
	lv_label_set_text(next_labe3l, _("Camera Test"));
	//3:2
	bottom32_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom32_panel, LV_MENU_WIDTH, LV_MENU_HEIGHT);
	lv_obj_set_pos(bottom32_panel, 2*11+LV_MENU_WIDTH, (TOP_TEST_MODE_PANEL_H+10+row*(LV_MENU_HEIGHT+10)));
	if(0 == menu_btn_flag[MENU_ID_NFC])
	{
		lv_obj_add_style(bottom32_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom32_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	lv_obj_set_user_data(bottom32_panel, (lv_obj_user_data_t)MENU_ID_NFC);
	lv_obj_set_event_cb(bottom32_panel, testMenu_page_navi_cb);
	lv_obj_t *next_labe32 = lv_label_create(bottom32_panel, NULL);
	//lv_obj_set_click(next_labe32, true);
	lv_obj_set_size(next_labe32, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_labe32, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	lv_label_set_text(next_labe32, _("NFC test"));
	//2:3
	bottom33_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom33_panel, LV_MENU_WIDTH, LV_MENU_HEIGHT);
	lv_obj_set_pos(bottom33_panel, 11+2*(10+LV_MENU_WIDTH), (TOP_TEST_MODE_PANEL_H+10+row*(LV_MENU_HEIGHT+10)));
	if(0 == menu_btn_flag[MENU_ID_MIC])
	{
		lv_obj_add_style(bottom33_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom33_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	lv_obj_set_user_data(bottom33_panel, (lv_obj_user_data_t)MENU_ID_MIC);
	lv_obj_set_event_cb(bottom33_panel, testMenu_page_navi_cb);
	lv_obj_t *next_labe33 = lv_label_create(bottom33_panel, NULL);
	//lv_obj_set_click(next_labe33, true);
	lv_obj_set_size(next_labe33, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_labe33, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	lv_label_set_text(next_labe33, _("MIC test"));

	//4:1
	row =3;
	bottom41_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom41_panel, LV_MENU_WIDTH, LV_MENU_HEIGHT);
	lv_obj_set_pos(bottom41_panel, 11, (TOP_TEST_MODE_PANEL_H+10+row*(LV_MENU_HEIGHT+10)));
	if(0 == menu_btn_flag[MENU_ID_WIFI])
	{
		lv_obj_add_style(bottom41_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom41_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	lv_obj_set_user_data(bottom41_panel, (lv_obj_user_data_t)MENU_ID_WIFI);
	lv_obj_set_event_cb(bottom41_panel, testMenu_page_navi_cb);
	lv_obj_t *next_labe4l = lv_label_create(bottom41_panel, NULL);
	//lv_obj_set_click(next_labe4l, true);
	lv_obj_set_size(next_labe4l, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_labe4l, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	//lv_obj_set_event_cb(next_label, cb0);
	//lv_obj_add_style(next_label, LV_LABEL_PART_MAIN, &btn_label_bottom_style);
	lv_label_set_text(next_labe4l, _("WIFI test"));
	//4:2
	bottom42_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom42_panel, LV_MENU_WIDTH, LV_MENU_HEIGHT);
	lv_obj_set_pos(bottom42_panel, 2*11+LV_MENU_WIDTH, (TOP_TEST_MODE_PANEL_H+10+row*(LV_MENU_HEIGHT+10)));
	if(0 == menu_btn_flag[MENU_ID_RS485_WG])
	{
		lv_obj_add_style(bottom42_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom42_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	lv_obj_set_user_data(bottom42_panel, (lv_obj_user_data_t)MENU_ID_RS485_WG);
	lv_obj_set_event_cb(bottom42_panel, testMenu_page_navi_cb);
	lv_obj_t *next_labe42 = lv_label_create(bottom42_panel, NULL);
	//lv_obj_set_click(next_labe42, true);
	lv_obj_set_size(next_labe42, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_labe42, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	lv_label_set_text(next_labe42, _("RS485/wg test"));
	//4:3
	bottom43_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom43_panel, LV_MENU_WIDTH, LV_MENU_HEIGHT);
	lv_obj_set_pos(bottom43_panel, 11+2*(10+LV_MENU_WIDTH), (TOP_TEST_MODE_PANEL_H+10+row*(LV_MENU_HEIGHT+10)));
	if(0 == menu_btn_flag[MENU_ID_USB])
	{
		lv_obj_add_style(bottom43_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom43_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	lv_obj_set_user_data(bottom43_panel, (lv_obj_user_data_t)MENU_ID_USB);
	lv_obj_set_event_cb(bottom43_panel, testMenu_page_navi_cb);
	lv_obj_t *next_labe43 = lv_label_create(bottom43_panel, NULL);
	//lv_obj_set_click(next_labe43, true);
	lv_obj_set_size(next_labe43, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_labe43, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	lv_label_set_text(next_labe43, _("USB test"));

	//5:1
	row =4;
	bottom51_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom51_panel, LV_MENU_WIDTH, LV_MENU_HEIGHT);
	lv_obj_set_pos(bottom51_panel, 11, (TOP_TEST_MODE_PANEL_H+10+row*(LV_MENU_HEIGHT+10)));
	if(0 == menu_btn_flag[MENU_ID_KEY])
	{
		lv_obj_add_style(bottom51_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom51_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	lv_obj_set_user_data(bottom51_panel, (lv_obj_user_data_t)MENU_ID_KEY);
	lv_obj_set_event_cb(bottom51_panel, testMenu_page_navi_cb);
	lv_obj_t *next_labe5l = lv_label_create(bottom51_panel, NULL);
	//lv_obj_set_click(next_labe5l, true);
	lv_obj_set_size(next_labe5l, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_labe5l, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	//lv_obj_set_event_cb(next_label, cb0);
	//lv_obj_add_style(next_label, LV_LABEL_PART_MAIN, &btn_label_bottom_style);
	lv_label_set_text(next_labe5l, _("Key test"));
	//5:2
	bottom52_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom52_panel, LV_MENU_WIDTH, LV_MENU_HEIGHT);
	lv_obj_set_pos(bottom52_panel, 2*11+LV_MENU_WIDTH, (TOP_TEST_MODE_PANEL_H+10+row*(LV_MENU_HEIGHT+10)));
	if(0 == menu_btn_flag[MENU_ID_RADAR])
	{
		lv_obj_add_style(bottom52_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom52_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	lv_obj_set_user_data(bottom52_panel, (lv_obj_user_data_t)MENU_ID_RADAR);
	lv_obj_set_event_cb(bottom52_panel, testMenu_page_navi_cb);
	lv_obj_t *next_labe52 = lv_label_create(bottom52_panel, NULL);
	//lv_obj_set_click(next_labe52, true);
	lv_obj_set_size(next_labe52, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_labe52, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	lv_label_set_text(next_labe52, _("Radar test"));
	//5:3
	bottom53_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom53_panel, LV_MENU_WIDTH, LV_MENU_HEIGHT);
	lv_obj_set_pos(bottom53_panel, 11+2*(10+LV_MENU_WIDTH), (TOP_TEST_MODE_PANEL_H+10+row*(LV_MENU_HEIGHT+10)));
	if(0 == menu_btn_flag[MENU_ID_ETH])
	{
		lv_obj_add_style(bottom53_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom53_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	lv_obj_set_user_data(bottom53_panel, (lv_obj_user_data_t)MENU_ID_ETH);
	lv_obj_set_event_cb(bottom53_panel, testMenu_page_navi_cb);
	lv_obj_t *next_labe53 = lv_label_create(bottom53_panel, NULL);
	//lv_obj_set_click(next_labe53, true);
	lv_obj_set_size(next_labe53, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_labe53, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	lv_label_set_text(next_labe53, _("eth test"));

/*

	lv_obj_t * bottom1_panel = menu_page_create_bottom_panel(par,0, LV_VER_RES_MAX-TOP_TEST_MODE_PANEL_H);
	lv_obj_set_event_cb(bottom1_panel, cb2);

	lv_obj_t *next_label = lv_label_create(bottom1_panel, NULL);
	lv_obj_set_click(next_label, true);
	lv_obj_set_size(next_label, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_label, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	//lv_obj_set_event_cb(next_label, cb0);
	lv_obj_add_style(next_label, LV_LABEL_PART_MAIN, &btn_label_bottom_style);
	lv_label_set_text(next_label, _("pass"));

		
*/

/*
	lv_obj_t * bottom2_panel = menu_page_create_bottom_panel(par,2*(LV_HOR_RES_MAX/3), LV_VER_RES_MAX-TOP_TEST_MODE_PANEL_H);
	lv_obj_set_event_cb(bottom2_panel, cb0);

	lv_obj_t *next_label = lv_label_create(bottom2_panel, NULL);
	lv_obj_set_click(next_label, true);
	lv_obj_set_size(next_label, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(next_label, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
	//lv_obj_set_event_cb(next_label, cb0);
	lv_obj_add_style(next_label, LV_LABEL_PART_MAIN, &btn_label_bottom_style);
	lv_label_set_text(next_label, _("pass"));

	lv_obj_t *prev_label = lv_label_create(bottom1_panel, NULL);
	lv_obj_set_click(prev_label, true);
	lv_obj_set_size(prev_label, 200, TOP_TEST_MODE_PANEL_H);
	lv_obj_align(prev_label, NULL, LV_ALIGN_IN_LEFT_MID, 100, 0);
	//lv_obj_set_event_cb(prev_label, cb2);
	lv_obj_add_style(prev_label, LV_LABEL_PART_MAIN, &btn_label_bottom_style);
	lv_label_set_text(prev_label, _("fail"));
*/
}



static void mainMenuPageCreate(void)
{
    _style_init();
	lv_i18n_set_locale("zh-CN");
	IR_OnOFF(false);
	set_test_mode_page_flag(1);

	printf("menu_btn_flag:%d \n",menu_btn_flag[1]);

    mainMenuTestModePage_cont = lv_cont_create(scr_menu_test_mode, NULL);
    lv_obj_set_size(mainMenuTestModePage_cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_cont_set_fit(mainMenuTestModePage_cont, LV_FIT_NONE);
    lv_cont_set_layout(mainMenuTestModePage_cont, LV_LAYOUT_OFF);

    create_top_panel(mainMenuTestModePage_cont);

	create_menu_btn_panel(mainMenuTestModePage_cont);
    //APP_CompSendCmd(CMD_APP_COMP_VIDEOSTOP, NULL, 0, NULL, 0);
}

static void mainMenuPageDestroy(void)
{
    lv_obj_del(mainMenuTestModePage_cont);
    mainMenuTestModePage_cont = NULL;
	
	set_enter_menu_flag(OUT_MENU);

    APP_CompSendCmd(CMD_APP_COMP_RESTART, NULL, 0, NULL, 0);
}

static void mainMenu_page_show()
{
    if (mainMenuTestModePage_cont == NULL) {
        return;
    }
	
	if(0 == menu_btn_flag[MENU_ID_ABOUT])
	{
		lv_obj_add_style(bottom1_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom1_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	if(0 == menu_btn_flag[MENU_ID_LCD])
	{
		lv_obj_add_style(bottom2_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom2_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	if(0 == menu_btn_flag[MENU_ID_LED])
	{
		lv_obj_add_style(bottom3_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom3_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	if(0 == menu_btn_flag[MENU_ID_AUDIO])
	{
		lv_obj_add_style(bottom21_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom21_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	if(0 == menu_btn_flag[MENU_ID_RELAY])
	{
		lv_obj_add_style(bottom22_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom22_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	if(0 == menu_btn_flag[MENU_ID_TP])
	{
		lv_obj_add_style(bottom23_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom23_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	if(0 == menu_btn_flag[MENU_ID_CAMERA])
	{
		lv_obj_add_style(bottom31_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom31_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	if(0 == menu_btn_flag[MENU_ID_NFC])
	{
		lv_obj_add_style(bottom32_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom32_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	if(0 == menu_btn_flag[MENU_ID_MIC])
	{
		lv_obj_add_style(bottom33_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom33_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	if(0 == menu_btn_flag[MENU_ID_WIFI])
	{
		lv_obj_add_style(bottom41_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom41_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	if(0 == menu_btn_flag[MENU_ID_RS485_WG])
	{
		lv_obj_add_style(bottom42_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom42_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	if(0 == menu_btn_flag[MENU_ID_USB])
	{
		lv_obj_add_style(bottom43_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom43_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	if(0 == menu_btn_flag[MENU_ID_KEY])
	{
		lv_obj_add_style(bottom51_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom51_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	if(0 == menu_btn_flag[MENU_ID_RADAR])
	{
		lv_obj_add_style(bottom52_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom52_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}
	if(0 == menu_btn_flag[MENU_ID_ETH])
	{
		lv_obj_add_style(bottom53_panel, LV_CONT_PART_MAIN, &btn_menu_style);
	}
	else
	{
		lv_obj_add_style(bottom53_panel, LV_CONT_PART_MAIN, &fail_btn_menu_style);
	}

    lv_obj_set_hidden(mainMenuTestModePage_cont, false);
}

static void mainMenu_page_hide()
{
    if (mainMenuTestModePage_cont == NULL) {
        return;
    }

    lv_obj_set_hidden(mainMenuTestModePage_cont, true);
}

static bool menu_test_mode_inited = false;
void menu_test_mode_scr_init()
{
    if (menu_test_mode_inited)
    {
        return;
    }

    scr_menu_test_mode = lv_cont_create(NULL, NULL);
    static lv_style_t scr_menu_style;
    lv_style_init(&scr_menu_style);
    lv_style_set_bg_opa(&scr_menu_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_add_style(scr_menu_test_mode, LV_CONT_PART_MAIN, &scr_menu_style);
    menu_test_mode_inited = true;
}

lv_obj_t * lv_scr_menu_test_mode()
{
    return scr_menu_test_mode;
}

void menu_test_mode_PageBack()
{
    if (menu_tree_isEmpty())
    {
        return;
    }

    menu_tree_get_top().hide();
    menu_tree_get_top().onDestroy();
    menu_tree_pop();
    if (!menu_tree_isEmpty()) {
        menu_tree_get_top().show();
        printf("show prev page\n");
    }
    else {
		set_test_mode_page_flag(0);
        //empty menu tree, got back to launcher ui
        launcher_show();
    }
}
