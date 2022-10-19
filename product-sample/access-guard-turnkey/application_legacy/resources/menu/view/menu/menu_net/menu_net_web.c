#include "lvgl/lvgl.h"
#include "menu_setting.h"
#include "launcher.h"
#include "menu_common.h"
#include "menu_face_lib.h"
#include "menu_face_demo.h"
#include "menu_device_manage.h"
#include "menu_device_activate.h"
#include "menu_access_control.h"
#include "menu_notification.h"
#include "menu_record_manage.h"
#include "menu_device_info.h"
#include "menu_net.h"
#include "menu_net_wifi.h"
#include "menu_net_web.h"
#include "lv_i18n.h"
#include "menu.h"
#include "resource.h"
#include "menu_page.h"
#include "app_utils.h"
#include "cviconfig.h"
#include "net.h"
#include "cviconfigsystem.h"
#include "cviconfigwebserver.h"
#include "input_dialog.h"
#include "select_dialog.h"
#include "cvi_hal_face_api.h"

extern hal_facelib_handle_t facelib_handle;

static void netWebMenuPageCreate(void);
static void netWebMenuPageDestroy(void);
static void netWebMenu_page_show();
static void netWebMenu_page_hide();

static lv_obj_t * netWebMenuPage_cont = NULL;

static menu_page_t netWebMenuPage = {
    .onCreate = netWebMenuPageCreate,
    .onDestroy = netWebMenuPageDestroy,
    .show = netWebMenu_page_show,
    .hide = netWebMenu_page_hide,
    .back = menuPageBack
};
static void create_top_panel(lv_obj_t *par);
static void create_connect_web_section(lv_obj_t *parent);
static void create_disp_panel(lv_obj_t *parent);
static void create_web_list(lv_obj_t *par);
static lv_obj_t *wifi_mode_label;
static lv_obj_t *wifi_ip_label;
static lv_obj_t *wifi_netmask_label;
// static lv_obj_t *lang_label;
static lv_obj_t *title;
static lv_obj_t *web_on_offtitle;
static lv_style_t style_sb;
static lv_style_t btn_label_style;
static bool style_inited = false;

lv_obj_t *web_ip_label = NULL;
lv_obj_t *web_registerid_label = NULL;
lv_obj_t *web_in_out_label = NULL;

#if 0
static void refresh()
{
	if(netWebMenuPage_cont != NULL)
	{
		lv_obj_del(netWebMenuPage_cont);
	    netWebMenuPage_cont = NULL;
	}

	netWebMenuPage_cont = lv_cont_create(lv_scr_menu(), NULL);
    lv_obj_set_size(netWebMenuPage_cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_cont_set_fit(netWebMenuPage_cont, LV_FIT_NONE);
    lv_cont_set_layout(netWebMenuPage_cont, LV_LAYOUT_OFF);

    create_top_panel(netWebMenuPage_cont);
	create_disp_panel(netWebMenuPage_cont);
    create_web_list(netWebMenuPage_cont);
}
#endif

menu_page_t get_menu_net_web_page()
{
    return netWebMenuPage;
}

static void back_btn_event_cb(lv_obj_t *obj, lv_event_t event)
{
    switch (event) {
        case LV_EVENT_RELEASED:
            menuPageBack();
            break;
        default:
            break;
    }
}
static void web_onoff_cb(lv_obj_t *obj, lv_event_t event)
{
	if (LV_EVENT_CLICKED == event) {
        int value = lv_slider_get_value(obj);
		if(0 == value)
		{
			HAL_FACE_RepoRemoveAllIdentities(facelib_handle);
			CviLoadIdentify(facelib_handle);
			system("rm ./repo/*");

			device_cfg_system.web_onoff = 0;
			cvi_set_device_config_system_int(CONF_KEY_WEB_ONOFF, device_cfg_system.web_onoff);
			system("killall webserver &");
			lv_label_set_text(web_on_offtitle, _("CLOSE WEB"));
		}
		else
		{
			HAL_FACE_RepoRemoveAllIdentities(facelib_handle);
			CviLoadIdentify(facelib_handle);
			system("rm ./repo/*");

			device_cfg_system.web_onoff = 1;
			cvi_set_device_config_system_int(CONF_KEY_WEB_ONOFF, device_cfg_system.web_onoff);
			system("./webserver.sh &"); /* "/mnt/data/webserver.sh" */
			lv_label_set_text(web_on_offtitle, _("OPEN WEB"));

		}
    }
}

static void web_cb(lv_obj_t * obj, lv_event_t event)
{
    char* page_id = (char*) lv_obj_get_user_data(obj);
	printf("[event:%d %s,%d]\n",event,__FUNCTION__,__LINE__);

    if (LV_EVENT_CLICKED == event)
	{
        printf("[event:%d  ssid:%s %s,%d]\n",event,page_id,__FUNCTION__,__LINE__);
		input_data.id = INPUT_ID_WEB;
		snprintf(input_data.data,sizeof(input_data.data),"%s",page_id);
		create_input_dialog(NULL, page_id);
	}
}

static void registid_cb(lv_obj_t * obj, lv_event_t event)
{
    char* page_id = (char*) lv_obj_get_user_data(obj);
	printf("[event:%d %s,%d]\n",event,__FUNCTION__,__LINE__);

    if (LV_EVENT_CLICKED == event)
	{
        printf("[event:%d  ssid:%s %s,%d]\n",event,page_id,__FUNCTION__,__LINE__);
		input_data.id = INPUT_ID_REGISTID;
		snprintf(input_data.data,sizeof(input_data.data),"%s",page_id);
		create_input_dialog(NULL, page_id);
	}
}
static void inout_cb(lv_obj_t * obj, lv_event_t event)
{
	char bufTmp[128] = {0};
    char* page_id = (char*) lv_obj_get_user_data(obj);
	printf("[event:%d %s,%d]\n",event,__FUNCTION__,__LINE__);

    if (LV_EVENT_CLICKED == event)
	{
		create_select_dialog(NULL, page_id);
	}
	else if(LV_EVENT_DEFOCUSED == event)
	{
		memset(bufTmp,0x00,sizeof(bufTmp));
		if(1 == device_cfg_system.signType)
		{
			sprintf(bufTmp,"%s					%s",_("InOut"),_("Out"));
		}
		else if(2 == device_cfg_system.signType)
		{
			sprintf(bufTmp,"%s					%s",_("InOut"),_("In"));
		}
		else if(3 == device_cfg_system.signType)
		{
			sprintf(bufTmp,"%s					%s",_("InOut"),_("In and Out"));
		}
		lv_label_set_text(web_in_out_label, bufTmp);
		printf("[event:%d %s,%d]\n",event,__FUNCTION__,__LINE__);
	}
}

static void _style_init()
{
    if (!style_inited)
    {
        lv_style_init(&style_sb);
        lv_style_set_bg_color(&style_sb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_bg_grad_color(&style_sb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_border_color(&style_sb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_border_width(&style_sb, LV_STATE_DEFAULT, 1);
        lv_style_set_border_opa(&style_sb, LV_STATE_DEFAULT, LV_OPA_70);
        lv_style_set_bg_opa(&style_sb, LV_STATE_DEFAULT, LV_OPA_60);
        lv_style_set_pad_right(&style_sb, LV_STATE_DEFAULT, 3);
        lv_style_set_pad_bottom(&style_sb, LV_STATE_DEFAULT, 3);
        lv_style_set_pad_inner(&style_sb, LV_STATE_DEFAULT, 8);

        lv_style_init(&btn_label_style);
        lv_style_set_text_font(&btn_label_style, LV_STATE_DEFAULT, &stsong_36);
        style_inited = true;
    }
}
static void load_data()
{
	cvi_get_device_config_system_int(CONF_KEY_SIGN_TYPE, &device_cfg_system.signType);
	cvi_get_device_config_system_str(CONF_KEY_CLOUD_SERVER, device_cfg_system.cloud_server, sizeof(device_cfg_system.cloud_server));
	cvi_get_device_config_system_str(CONF_KEY_REGIST_ID, device_cfg_system.regist_id, sizeof(device_cfg_system.regist_id));
	cvi_get_device_config_system_int(CONF_KEY_WEB_ONOFF, &device_cfg_system.web_onoff);
}

static void create_top_panel(lv_obj_t *par)
{
    lv_obj_t * top_panel = menu_page_create_top_panel(par, back_btn_event_cb);

    title = lv_label_create(top_panel, NULL);
    lv_obj_add_style(title, LV_STATE_DEFAULT, menu_page_title_style());
    lv_label_set_text(title, _("Web Set"));
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(title, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(title, true);
}

static void create_connect_web_section(lv_obj_t *parent)
{
    //lv_obj_t *hardware_section = menu_section_create(parent, "guany");
	char bufTemp[64] = {0};

	lv_obj_t *hardware_section = lv_cont_create(parent, NULL);
    lv_obj_set_size(hardware_section, LV_HOR_RES_MAX, 2*TOP_PANEL_H);
    lv_cont_set_layout(hardware_section, LV_LAYOUT_OFF);
    lv_obj_set_drag(hardware_section, true);
    lv_page_glue_obj(hardware_section, true);
	//lv_obj_align(hardware_section, NULL, LV_ALIGN_CENTER, 0, 0);

	lv_obj_t * switch1 = lv_switch_create(hardware_section, NULL);
    lv_obj_set_width(switch1, 160);
	lv_obj_set_height(switch1, 80);
	lv_switch_set_anim_time(switch1, 1);
	if(1 == device_cfg_system.web_onoff)
	{
		lv_switch_on(switch1, LV_ANIM_ON);
		sprintf(bufTemp,"%s",_("OPEN WEB"));
	}
	else
	{
		lv_switch_off(switch1, LV_ANIM_OFF);
		sprintf(bufTemp,"%s",_("CLOSE WEB"));
	}
    lv_obj_align(switch1, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
    lv_obj_set_event_cb(switch1, web_onoff_cb);

	web_on_offtitle = lv_label_create(hardware_section, NULL);
    lv_obj_add_style(web_on_offtitle, LV_STATE_DEFAULT, &btn_label_style);
    lv_label_set_text(web_on_offtitle, bufTemp);
    lv_label_set_align(web_on_offtitle, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(web_on_offtitle, NULL, LV_ALIGN_IN_LEFT_MID, 100, 0);

}
static void create_disp_panel(lv_obj_t *parent)
{
    lv_obj_t *disp_panel = menu_page_create_disp_scrollable_panel(parent);

    create_connect_web_section(disp_panel);
}
static void create_web_list(lv_obj_t *par)
{
    lv_obj_t *web_ip;
	//lv_obj_t *web_ip_label;
	lv_obj_t *web_registerid;
	//lv_obj_t *web_registerid_label;
	lv_obj_t *web_in_out;
	//lv_obj_t *web_in_out_label;

	char wifiBuf[128] = {0};

    lv_obj_t * netEthMenuPage_obj = lv_list_create(par, NULL);
    lv_obj_add_style(netEthMenuPage_obj, LV_PAGE_PART_SCROLLABLE, &style_sb);
    lv_obj_set_pos(netEthMenuPage_obj, 0, 3*TOP_PANEL_H);
    lv_obj_set_size(netEthMenuPage_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_PANEL_H);
    lv_page_set_edge_flash(netEthMenuPage_obj, true);
    lv_page_set_scrollbar_mode(netEthMenuPage_obj, LV_SCROLLBAR_MODE_AUTO);
    lv_page_set_scrl_layout(netEthMenuPage_obj, LV_LAYOUT_COLUMN_MID);

	memset(wifiBuf,0x00,sizeof(wifiBuf));
	sprintf(wifiBuf,"%s				%s",_("web ip"),device_cfg_system.cloud_server);
    web_ip = lv_list_add_btn(netEthMenuPage_obj, RES_IMG_ICON_SBSZ1, wifiBuf);
    web_ip_label = lv_list_get_btn_label(web_ip);
    lv_obj_set_height(web_ip_label, 40);
    lv_obj_add_style(web_ip_label, LV_LABEL_PART_MAIN, &btn_label_style);
    lv_btn_set_fit(web_ip, LV_FIT_TIGHT);
    lv_obj_set_user_data(web_ip, (lv_obj_user_data_t)device_cfg_system.cloud_server);
    lv_obj_set_event_cb(web_ip, web_cb);

	memset(wifiBuf,0x00,sizeof(wifiBuf));
	sprintf(wifiBuf,"%s					%s",_("register id"),device_cfg_system.regist_id);
    web_registerid = lv_list_add_btn(netEthMenuPage_obj, RES_IMG_ICON_SBSZ1, wifiBuf);
    web_registerid_label = lv_list_get_btn_label(web_registerid);
    lv_obj_set_height(web_registerid_label, 40);
    lv_obj_add_style(web_registerid_label, LV_LABEL_PART_MAIN, &btn_label_style);
    lv_btn_set_fit(web_registerid, LV_FIT_TIGHT);
    lv_obj_set_user_data(web_registerid, (lv_obj_user_data_t)device_cfg_system.regist_id);
    lv_obj_set_event_cb(web_registerid, registid_cb);

	memset(wifiBuf,0x00,sizeof(wifiBuf));
	if(1 == device_cfg_system.signType)
	{
		sprintf(wifiBuf,"%s					%s",_("InOut"),_("Out"));
	}
	else if(2 == device_cfg_system.signType)
	{
		sprintf(wifiBuf,"%s					%s",_("InOut"),_("In"));
	}
	else if(3 == device_cfg_system.signType)
	{
		sprintf(wifiBuf,"%s					%s",_("InOut"),_("In and Out"));
	}
    web_in_out = lv_list_add_btn(netEthMenuPage_obj, RES_IMG_ICON_SBSZ1, wifiBuf);
    web_in_out_label = lv_list_get_btn_label(web_in_out);
    lv_obj_set_height(web_in_out_label, 40);
    lv_obj_add_style(web_in_out_label, LV_LABEL_PART_MAIN, &btn_label_style);
    lv_btn_set_fit(web_in_out, LV_FIT_TIGHT);
    //lv_obj_set_user_data(web_in_out, (lv_obj_user_data_t)"wifiBuf");
    lv_obj_set_event_cb(web_in_out, inout_cb);
}

static void netWebMenuPageCreate(void)
{
    _style_init();
	load_data();

    netWebMenuPage_cont = lv_cont_create(lv_scr_menu(), NULL);
    lv_obj_set_size(netWebMenuPage_cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_cont_set_fit(netWebMenuPage_cont, LV_FIT_NONE);
    lv_cont_set_layout(netWebMenuPage_cont, LV_LAYOUT_OFF);

    create_top_panel(netWebMenuPage_cont);
	create_disp_panel(netWebMenuPage_cont);
    create_web_list(netWebMenuPage_cont);
}

static void netWebMenuPageDestroy(void)
{
    lv_obj_del(netWebMenuPage_cont);
    netWebMenuPage_cont = NULL;
    wifi_mode_label = NULL;
    wifi_ip_label = NULL;
    wifi_netmask_label = NULL;
}

static void netWebMenu_page_show()
{
    if (netWebMenuPage_cont == NULL) {
        return;
    }

    lv_obj_set_hidden(netWebMenuPage_cont, false);
}

static void netWebMenu_page_hide()
{
    if (netWebMenuPage_cont == NULL) {
        return;
    }

    lv_obj_set_hidden(netWebMenuPage_cont, true);
}

