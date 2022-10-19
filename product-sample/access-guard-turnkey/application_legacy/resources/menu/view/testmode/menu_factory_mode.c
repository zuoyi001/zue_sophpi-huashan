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
#include "menu_rs485_wg_test.h"
#include "menu_factory_mode.h"
#include "lv_i18n.h"
#include "menu.h"
#include "resource.h"
#include "menu_page.h"
#include "app_peripheral.h"
#include "app_utils.h"
#include "public.h"
#include "sys.h"


static void mainMenuPageCreate(void);
static void mainMenuPageDestroy(void);
static void mainMenu_page_show();
static void mainMenu_page_hide();

static lv_obj_t * scr_menu_factory_mode = NULL;
static lv_obj_t * mainMenuTestModePage_cont = NULL;

static menu_page_t mainMenuFactoryModePage = {
    .onCreate = mainMenuPageCreate,
    .onDestroy = mainMenuPageDestroy,
    .show = mainMenu_page_show,
    .hide = mainMenu_page_hide,
    .back = menu_factory_mode_PageBack
};

#if 0
static void mainMenu_back_event_cb(lv_obj_t * obj, lv_event_t event)
{
	switch (event)
	{
	case LV_EVENT_RELEASED:
        printf("mainMenu_back_event_cb.\n");
        menu_scr_hide();
        menuPageBack();
		break;
	default:
		break;
	}
}
#endif

void menu_factory_mode_scr_show()
{
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
    if (scr_menu_factory_mode) {
		printf("[%s,%d] \n",__FUNCTION__,__LINE__);
        lv_scr_load(scr_menu_factory_mode);
        menu_tree_push(mainMenuFactoryModePage);
        menu_tree_get_top().onCreate();
        menu_tree_get_top().show();
    }
}

void menu_factory_mode_scr_hide()
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
static lv_obj_t *vs_label;
static lv_obj_t *sn_label;
static lv_obj_t *mac_label;


static lv_obj_t *title;

static lv_style_t style_factory_mode_sb;
static lv_style_t btn_label_factory_mode_style;
static bool style_factory_mode_inited = false;
static void _style_init()
{
    if (!style_factory_mode_inited)
    {
        lv_style_init(&style_factory_mode_sb);
        lv_style_set_bg_color(&style_factory_mode_sb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_bg_grad_color(&style_factory_mode_sb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_border_color(&style_factory_mode_sb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_border_width(&style_factory_mode_sb, LV_STATE_DEFAULT, 1);
        lv_style_set_border_opa(&style_factory_mode_sb, LV_STATE_DEFAULT, LV_OPA_70);
        lv_style_set_bg_opa(&style_factory_mode_sb, LV_STATE_DEFAULT, LV_OPA_60);
        lv_style_set_pad_right(&style_factory_mode_sb, LV_STATE_DEFAULT, 3);
        lv_style_set_pad_bottom(&style_factory_mode_sb, LV_STATE_DEFAULT, 3);
        lv_style_set_pad_inner(&style_factory_mode_sb, LV_STATE_DEFAULT, 8);

       //lv_style_init(&btn_label_style);
        //lv_style_set_text_font(&btn_label_style, LV_STATE_DEFAULT, &stsong_22);
		lv_style_init(&btn_label_factory_mode_style);
		lv_style_set_text_color(&btn_label_factory_mode_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		lv_style_set_text_color(&btn_label_factory_mode_style, LV_STATE_PRESSED, LV_COLOR_WHITE);
		lv_style_set_text_font(&btn_label_factory_mode_style, LV_STATE_DEFAULT, &stsong_28);

		
        style_factory_mode_inited = true;
    }
}

static void pass_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {


		//menu_test_mode_PageBack();
		if (menu_tree_isEmpty())
                return;
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_lcd_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
	#if 0
        //lv_obj_t *face_info_window = createFaceAddWindow(NULL);
        //lv_obj_set_hidden(face_info_window, false);
        printf("[%s,%d]\n",__FUNCTION__,__LINE__);
		if (menu_tree_isEmpty())
                return;
            menu_tree_get_top().hide();
			/*
            menu_tree_push(get_menu_lcd_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
			*/
			get_menu_lcd_test_page().onCreate();
			get_menu_lcd_test_page().show();
			*/
			#endif
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
            menu_tree_get_top().hide();
			menu_tree_push(get_menu_lcd_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
			//get_menu_lcd_test_page().onCreate();
			//get_menu_lcd_test_page().show();
    }
}

static void create_top_panel(lv_obj_t *par)
{
    lv_obj_t * top_panel = menu_page_create_test_mode_top_panel(par);

    title = lv_label_create(top_panel, NULL);
    lv_obj_add_style(title, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
    lv_label_set_text(title, _("factory test"));
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(title, NULL, LV_ALIGN_CENTER, 0, 0);
}

static void create_content_panel(lv_obj_t *par)
{
	char tempBuf[64]={0};
	char card[] = "eth0";
	char tmpBuf2[32] = {0};
    lv_obj_t * vs_panel = menu_page_create_test_mode_disp_panel(par);

    vs_label = lv_label_create(vs_panel, NULL);
    //lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
    lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
	memset(tempBuf, 0x00, sizeof(tempBuf));
	sprintf(tempBuf,"%s:  %s",_("Version"),VERSION_NAME);
    lv_label_set_text(vs_label, tempBuf);
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(vs_label, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 100);	

    sn_label = lv_label_create(vs_panel, NULL);
    //lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
    lv_obj_add_style(sn_label, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
	memset(tempBuf, 0x00, sizeof(tempBuf));
	sprintf(tempBuf,"%s:  %s",_("Sn"),get_sn());
    lv_label_set_text(sn_label, tempBuf);
	//lv_label_set_offset_y(mac_label, 100);
    lv_label_set_align(sn_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(sn_label, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 150);

	mac_label = lv_label_create(vs_panel, NULL);
    //lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
    lv_obj_add_style(mac_label, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
	memset(tempBuf, 0x00, sizeof(tempBuf));
	get_mac(card, tmpBuf2);
	sprintf(tempBuf,"%s:  %s",_("Mac"),tmpBuf2);
    lv_label_set_text(mac_label, tempBuf);
	//lv_label_set_offset_y(mac_label, 100);
    lv_label_set_align(mac_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(mac_label, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 200);

	lv_obj_t * eth0_ip_label = lv_label_create(vs_panel, NULL);
	//lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
	lv_obj_add_style(eth0_ip_label, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
	memset(tempBuf, 0x00, sizeof(tempBuf));
	memset(tmpBuf2, 0x00, sizeof(tmpBuf2));
	get_network_ip_address("eth0", tmpBuf2);
	sprintf(tempBuf,"%s:  %s",_("Ip address"),tmpBuf2);
	lv_label_set_text(eth0_ip_label, tempBuf);
	//lv_label_set_offset_y(mac_label, 100);
	lv_label_set_align(eth0_ip_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(eth0_ip_label, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 250);

	lv_obj_t * wifi_mac_label = lv_label_create(vs_panel, NULL);
	//lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
	lv_obj_add_style(wifi_mac_label, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
	memset(tempBuf, 0x00, sizeof(tempBuf));
    get_mac("wlan0", tmpBuf2);
	sprintf(tempBuf,"%s:  %s",_("WIFI Mac"),tmpBuf2);
	lv_label_set_text(wifi_mac_label, tempBuf);
	//lv_label_set_offset_y(mac_label, 100);
	lv_label_set_align(wifi_mac_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(wifi_mac_label, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 300);
	
	lv_obj_t * wlan0_ip_label = lv_label_create(vs_panel, NULL);
	//lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
	lv_obj_add_style(wlan0_ip_label, LV_STATE_DEFAULT, menu_page_test_mode_title_style());
	memset(tempBuf, 0x00, sizeof(tempBuf));
	memset(tmpBuf2, 0x00, sizeof(tmpBuf2));
    get_network_ip_address("wlan0", tmpBuf2);
	sprintf(tempBuf,"%s:  %s",_("wifi ip"),tmpBuf2);
	lv_label_set_text(wlan0_ip_label, tempBuf);
	//lv_label_set_offset_y(mac_label, 100);
	lv_label_set_align(wlan0_ip_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(wlan0_ip_label, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 350);
	
}

#if 0
static void create_bottom_panel(lv_obj_t *par)
{
	lv_obj_t * bottom_panel = menu_page_create_test_mode_bottom_panel(par);


	lv_obj_t *next_label = lv_label_create(bottom_panel, NULL);
    lv_obj_set_click(next_label, true);
    lv_obj_align(next_label, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
    lv_obj_set_event_cb(next_label, pass_btn_event_cb);
    lv_obj_add_style(next_label, LV_LABEL_PART_MAIN, &btn_label_factory_mode_style);
    lv_label_set_text(next_label, _("pass"));

    lv_obj_t *prev_label = lv_label_create(bottom_panel, NULL);
    lv_obj_set_click(prev_label, true);
    lv_obj_align(prev_label, NULL, LV_ALIGN_IN_LEFT_MID, 100, 0);
    lv_obj_set_event_cb(prev_label, error_btn_event_cb);
    lv_obj_add_style(prev_label, LV_LABEL_PART_MAIN, &btn_label_factory_mode_style);
    lv_label_set_text(prev_label, _("error"));
}
#endif

static void mainMenuPageCreate(void)
{
    _style_init();
	lv_i18n_set_locale("zh-CN");
	IR_OnOFF(false);

    mainMenuTestModePage_cont = lv_cont_create(scr_menu_factory_mode, NULL);
    lv_obj_set_size(mainMenuTestModePage_cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_cont_set_fit(mainMenuTestModePage_cont, LV_FIT_NONE);
    lv_cont_set_layout(mainMenuTestModePage_cont, LV_LAYOUT_OFF);

    create_top_panel(mainMenuTestModePage_cont);
	create_content_panel(mainMenuTestModePage_cont);

	//create_bottom_panel(mainMenuTestModePage_cont);
	create_bottom_2btn_panel(mainMenuTestModePage_cont, pass_btn_event_cb, error_btn_event_cb);

	
    //mainMenu_test_mode_retranslateUI();
    
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

    lv_obj_set_hidden(mainMenuTestModePage_cont, false);
}

static void mainMenu_page_hide()
{
    if (mainMenuTestModePage_cont == NULL) {
        return;
    }

    lv_obj_set_hidden(mainMenuTestModePage_cont, true);
}

static bool menu_factory_mode_inited = false;
void menu_factory_mode_scr_init()
{
    if (menu_factory_mode_inited)
    {
        return;
    }

    scr_menu_factory_mode = lv_cont_create(NULL, NULL);
    static lv_style_t scr_menu_style;
    lv_style_init(&scr_menu_style);
    lv_style_set_bg_opa(&scr_menu_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_add_style(scr_menu_factory_mode, LV_CONT_PART_MAIN, &scr_menu_style);
    menu_factory_mode_inited = true;
}

lv_obj_t * lv_scr_menu_factory_mode()
{
    return scr_menu_factory_mode;
}

void menu_factory_mode_PageBack()
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
        //empty menu tree, got back to launcher ui
        launcher_show();
    }
}
