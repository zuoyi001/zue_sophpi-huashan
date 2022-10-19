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
#include "lv_i18n.h"
#include "menu.h"
#include "resource.h"
#include "menu_page.h"
#include "app_utils.h"
#include "cviconfig.h"
#include "net.h"
#include "wifi_input_pwd_dialog.h"
#include "sys.h"



static void netWifiMenuPageCreate(void);
static void netWifiMenuPageDestroy(void);
static void netWifiMenu_page_show();
static void netWifiMenu_page_hide();

static lv_obj_t * netWifiMenuPage_cont = NULL;

lv_obj_t * netWifiList_obj = NULL;
lv_obj_t * netWifiConnectStatus_obj = NULL;

char wifiSSIDBuf[30][64];


static menu_page_t netWifiMenuPage = {
    .onCreate = netWifiMenuPageCreate,
    .onDestroy = netWifiMenuPageDestroy,
    .show = netWifiMenu_page_show,
    .hide = netWifiMenu_page_hide,
    .back = menuPageBack
};
static void create_top_panel(lv_obj_t *par);
static void create_connect_wifi_section(lv_obj_t *parent);
static void create_disp_panel(lv_obj_t *parent);
static void create_wifi_list(lv_obj_t *par);
static void create_wifi_button(lv_obj_t *par);

static void create_connect_wifi_status_item(lv_obj_t *par);
static void wifi_refresh();
static void load_data();
void update_wifi_status();

cvi_device_config_t device_cfg;
static WLAN_AP_INFO_S *wlan_apinfo = NULL;
static WLAN_AP_INFO_S *wlan_apinfo2 = NULL;


static lv_obj_t *wifi_mode_label;
static lv_obj_t *wifi_ip_label;
static lv_obj_t *wifi_netmask_label;
static lv_obj_t *wifi_on_off_label;
lv_obj_t *wifi_status_label;


// static lv_obj_t *lang_label;
static lv_obj_t *title;

lv_style_t style_sb_wifi;
lv_style_t btn_label_style_wifi;
bool style_inited_wifi = false;

int iCount = 0;

pthread_t check_wifi_id = 0;
pthread_t start_wifi_id = 0;
lv_task_t * wifi_task = NULL;

int check_wifi_thread_exit(void)
{
	if(check_wifi_id != 0)
	{
		pthread_cancel(check_wifi_id);
		check_wifi_id = 0;
	}

	return 0;
}

void * start_wifi_thread(void *arg)
{
	struct wlan_cfg cfg;

	//char bufTemp[128] = {0};
	//sprintf(bufTemp,"%s  %s",device_cfg.wifi_ssid,_("Connecting"));
	//lv_label_set_text(wifi_status_label, bufTemp);

	system("killall wpa_supplicant");
	system("killall udhcpc");
	
	CVI_NET_Wlan_Init(WLAN0_PORT);
	cvi_get_device_config_int(CONF_KEY_WIFI_ONOFF, &device_cfg.wifi_onoff);
	cvi_get_device_config_str(CONF_KEY_WIFI_SSID, device_cfg.wifi_ssid, sizeof(device_cfg.wifi_ssid));
	cvi_get_device_config_str(CONF_KEY_WIFI_PWD, device_cfg.wifi_pwd, sizeof(device_cfg.wifi_pwd));
	//sprintf(cfg.ssid,"%s",device_cfg.wifi_ssid);
	snprintf(cfg.ssid, sizeof(cfg.ssid), "%s", device_cfg.wifi_ssid);
	snprintf(cfg.passwd, sizeof(cfg.passwd), "%s", device_cfg.wifi_pwd);
	cvi_get_device_config_int(CONF_KEY_WIFI_ONOFF, &device_cfg.wifi_onoff);
	cvi_get_device_config_str(CONF_KEY_WIFI_SSID, device_cfg.wifi_ssid, sizeof(device_cfg.wifi_ssid));
	cvi_get_device_config_str(CONF_KEY_WIFI_PWD, device_cfg.wifi_pwd, sizeof(device_cfg.wifi_pwd));
	
	if((strlen(cfg.passwd) >= 8) && (1 == device_cfg.wifi_onoff))
	{
		CVI_NET_Wlan_Init(WLAN0_PORT);
		//system("killall udhcpc");
		sleep(1);
    	CVI_NET_Set_Wlan_Config(WLAN0_PORT, STATION, &cfg);
	}

	//sleep(1);
	//update_wifi_status();

	return NULL;
}


void * check_wifi_thread(void *arg)
{
	//int ret = 0;
	//char temBuf[1024] = {0};
	//char temBuf2[64] = {0};
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	sleep(3);

	while(true)
	{
		#if 0
		cvi_get_device_config_int(CONF_KEY_WIFI_ONOFF, &device_cfg.wifi_onoff);
		if(1 == device_cfg.wifi_onoff)
		{
			if(wlan_apinfo != NULL)
			{
				free(wlan_apinfo);
				wlan_apinfo = NULL;
			}
			//printf("[%s,%d]\n",__FUNCTION__,__LINE__);

			//if(iCount--<= 0)
			{
				iCount = 60;
			    wlan_apinfo = CVI_NET_Get_Ap_Info();
				
				if(wlan_apinfo != NULL)
				{
					printf("wlan_apinfo:%d \r\n",wlan_apinfo->count);
					/*
					for(int i = 0; i< wlan_apinfo->count;i++)
					{
						printf("signal_level:%d,%s,%d]\n",wlan_apinfo->ap_arr[i].signal_level,__FUNCTION__,__LINE__);
					}
					*/

					if(wlan_apinfo->count > 0)
					{
						if(wlan_apinfo2 != NULL)
						{
							free(wlan_apinfo2);
							wlan_apinfo2 = NULL;
						}
						wlan_apinfo2 = wlan_apinfo;
						wlan_apinfo = NULL;
						wifi_refresh();
						//break;
					}
				}
			}
			update_wifi_status();
		}
		#endif
		wifi_refresh();
		sleep(5);
	}
	printf("[exit check_wifi_thread%s,%d]\n",__FUNCTION__,__LINE__);
	return NULL;
}

void flash_wifi_info_task(lv_task_t * task)
{
	//int ret = 0;
	//char temBuf[1024] = {0};
	//char temBuf2[64] = {0};
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	//sleep(3);

	cvi_get_device_config_int(CONF_KEY_WIFI_ONOFF, &device_cfg.wifi_onoff);
	if(1 == device_cfg.wifi_onoff)
	{
		if(wlan_apinfo != NULL)
		{
			free(wlan_apinfo);
			wlan_apinfo = NULL;
		}
		//printf("[%s,%d]\n",__FUNCTION__,__LINE__);

		if(iCount--<= 0)
		{
			iCount = 30;
			system("wpa_cli -iwlan0 scan &");
			wlan_apinfo = CVI_NET_Flash_Ap_Info();
			
			if(wlan_apinfo != NULL)
			{
				printf("wlan_apinfo:%d \r\n",wlan_apinfo->count);
				/*
				for(int i = 0; i< wlan_apinfo->count;i++)
				{
					printf("signal_level:%d,%s,%d]\n",wlan_apinfo->ap_arr[i].signal_level,__FUNCTION__,__LINE__);
				}
				*/

				if(wlan_apinfo->count > 0)
				{
					if(wlan_apinfo2 != NULL)
					{
						free(wlan_apinfo2);
						wlan_apinfo2 = NULL;
					}
					wlan_apinfo2 = wlan_apinfo;
					wlan_apinfo = NULL;
					wifi_refresh();
					//break;
				}
			}
		    
		}
		
		update_wifi_status();
	}
	//wifi_refresh();
}

#if 0
void flash_wifi_info_task(lv_task_t * task)
{
	int ret = 0;
	char temBuf[1024] = {0};
	char temBuf2[64] = {0};
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	//sleep(3);

	cvi_get_device_config_int(CONF_KEY_WIFI_ONOFF, &device_cfg.wifi_onoff);
	if(1 == device_cfg.wifi_onoff)
	{
		if(wlan_apinfo != NULL)
		{
			free(wlan_apinfo);
			wlan_apinfo = NULL;
		}
		//printf("[%s,%d]\n",__FUNCTION__,__LINE__);

		if(iCount--<= 0)
		{
		    wlan_apinfo = CVI_NET_Get_Ap_Info();
			
			if(wlan_apinfo != NULL)
			{
			
				printf("wlan_apinfo:%d \r\n",wlan_apinfo->count);
				/*
				for(int i = 0; i< wlan_apinfo->count;i++)
				{
					printf("signal_level:%d,%s,%d]\n",wlan_apinfo->ap_arr[i].signal_level,__FUNCTION__,__LINE__);
				}
				*/

				if(wlan_apinfo->count > 0)
				{
					iCount = 60;
					if(wlan_apinfo2 != NULL)
					{
						free(wlan_apinfo2);
						wlan_apinfo2 = NULL;
					}
					wlan_apinfo2 = wlan_apinfo;
					wlan_apinfo = NULL;
					wifi_refresh();
					//break;
				}
			}
		}
		update_wifi_status();
	}
	//wifi_refresh();
}
#endif
static void wifi_refresh()
{
	load_data();
	if(netWifiList_obj != NULL)
	{
		printf("[%s,%d]\n",__FUNCTION__,__LINE__);
		lv_obj_clean(netWifiList_obj);
		printf("[%s,%d]\n",__FUNCTION__,__LINE__);
		//lv_obj_del(netWifiList_obj);
	    netWifiList_obj = NULL;
	}
	/*
	if(netWifiConnectStatus_obj != NULL)
	{
		lv_obj_del(netWifiConnectStatus_obj);
	    netWifiConnectStatus_obj = NULL;
	}
	*/
	//netWifiMenuPage_cont2 = lv_cont_create(lv_scr_menu(), NULL);
    //lv_obj_set_size(netWifiMenuPage_cont2, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    //lv_cont_set_fit(netWifiMenuPage_cont2, LV_FIT_NONE);
    //lv_cont_set_layout(netWifiMenuPage_cont2, LV_LAYOUT_OFF);

    //create_top_panel(netWifiMenuPage_cont2);
	//create_disp_panel(netWifiMenuPage_cont2);
	//create_connect_wifi_status_item(netWifiMenuPage_cont);
	update_wifi_status();
    create_wifi_list(netWifiMenuPage_cont);
/*
	if(netWifiMenuPage_cont != NULL)
	{
		lv_obj_del(netWifiMenuPage_cont);
	    netWifiMenuPage_cont = NULL;
		netWifiMenuPage_cont = netWifiMenuPage_cont2;
	}
	*/

	//netWifiMenu_page_show();
}


#if 0
static void wifi_refresh()
{
	load_data();
	if(netWifiList_obj != NULL)
	{
		lv_obj_del(netWifiList_obj);
	    netWifiList_obj = NULL;
	}
	if(netWifiConnectStatus_obj != NULL)
	{
		lv_obj_del(netWifiConnectStatus_obj);
	    netWifiConnectStatus_obj = NULL;
	}
	//netWifiMenuPage_cont2 = lv_cont_create(lv_scr_menu(), NULL);
    //lv_obj_set_size(netWifiMenuPage_cont2, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    //lv_cont_set_fit(netWifiMenuPage_cont2, LV_FIT_NONE);
    //lv_cont_set_layout(netWifiMenuPage_cont2, LV_LAYOUT_OFF);

    //create_top_panel(netWifiMenuPage_cont2);
	//create_disp_panel(netWifiMenuPage_cont2);
	create_connect_wifi_status_item(netWifiMenuPage_cont);
    create_wifi_list(netWifiMenuPage_cont);
/*
	if(netWifiMenuPage_cont != NULL)
	{
		lv_obj_del(netWifiMenuPage_cont);
	    netWifiMenuPage_cont = NULL;
		netWifiMenuPage_cont = netWifiMenuPage_cont2;
	}
	*/

	//netWifiMenu_page_show();
}
#endif
#if 0
static void wifi_refresh2()
{
	load_data();
	//netWifiMenuPage_cont2 = lv_cont_create(lv_scr_menu(), NULL);
	//lv_obj_set_size(netWifiMenuPage_cont2, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	//lv_cont_set_fit(netWifiMenuPage_cont2, LV_FIT_NONE);
	//lv_cont_set_layout(netWifiMenuPage_cont2, LV_LAYOUT_OFF);

	//create_top_panel(netWifiMenuPage_cont2);
	//create_disp_panel(netWifiMenuPage_cont2);
	//create_connect_wifi_status_item(netWifiMenuPage_cont2);
	//create_wifi_list(netWifiMenuPage_cont2);
	if(netWifiList_obj != NULL)
	{
		lv_obj_del(netWifiList_obj);
		netWifiList_obj = NULL;
	}
	
	if(netWifiConnectStatus_obj != NULL)
	{
		lv_obj_del(netWifiConnectStatus_obj);
		netWifiConnectStatus_obj = NULL;
	}
		
	create_connect_wifi_status_item(netWifiMenuPage_cont);
	create_wifi_list(netWifiMenuPage_cont);

	lv_obj_t *flash_label = lv_label_create(netWifiMenuPage_cont, NULL);
	lv_obj_add_style(flash_label, LV_STATE_DEFAULT, &btn_label_style);
	lv_label_set_text(flash_label, _("flashing"));
	lv_label_set_align(flash_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(flash_label, NULL, LV_ALIGN_CENTER, 0, 0);

	/*
	if(netWifiMenuPage_cont != NULL)
	{
		lv_obj_del(netWifiMenuPage_cont);
		netWifiMenuPage_cont = NULL;
		netWifiMenuPage_cont = netWifiMenuPage_cont2;
	}
	*/

	//netWifiMenu_page_show();
}
#else
static void wifi_refresh2()
{
	load_data();

	if(netWifiConnectStatus_obj != NULL)
	{
		lv_obj_clean(netWifiConnectStatus_obj);
	    netWifiConnectStatus_obj = NULL;
	}

	if(netWifiList_obj != NULL)
	{
		//lv_obj_del(netWifiList_obj);
		lv_obj_clean(netWifiList_obj);
	    netWifiList_obj = NULL;
	}
	if(netWifiMenuPage_cont != NULL)
	{
		lv_obj_del(netWifiMenuPage_cont);
	    netWifiMenuPage_cont = NULL;
	}
	
	netWifiMenuPage_cont = lv_cont_create(lv_scr_menu(), NULL);
    lv_obj_set_size(netWifiMenuPage_cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_cont_set_fit(netWifiMenuPage_cont, LV_FIT_NONE);
    lv_cont_set_layout(netWifiMenuPage_cont, LV_LAYOUT_OFF);

    create_top_panel(netWifiMenuPage_cont);
	create_disp_panel(netWifiMenuPage_cont);
	create_connect_wifi_status_item(netWifiMenuPage_cont);
    //create_wifi_list(netWifiMenuPage_cont2);
	create_wifi_button(netWifiMenuPage_cont);

	lv_obj_t *flash_label = lv_label_create(netWifiMenuPage_cont, NULL);
    lv_obj_add_style(flash_label, LV_STATE_DEFAULT, &btn_label_style_wifi);
    lv_label_set_text(flash_label, _("flashing"));
    lv_label_set_align(flash_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(flash_label, NULL, LV_ALIGN_CENTER, 0, 0);

	netWifiMenu_page_show();
}

#endif 
menu_page_t get_menu_net_wifi_page()
{
    return netWifiMenuPage;
}

static void back_btn_event_cb(lv_obj_t *obj, lv_event_t event)
{
    switch (event) {
        case LV_EVENT_RELEASED:
			printf("[event:%d %s,%d]\n",event,__FUNCTION__,__LINE__);
			if(wifi_task != NULL)
			{
				lv_task_del(wifi_task);
				wifi_task = NULL;
			}
			printf("[event:%d %s,%d]\n",event,__FUNCTION__,__LINE__);
			if(wlan_apinfo2 != NULL)
			{
				free(wlan_apinfo2);
				wlan_apinfo2 = NULL;
			}

			printf("[event:%d %s,%d]\n",event,__FUNCTION__,__LINE__);
            menuPageBack();
			printf("[event:%d %s,%d]\n",event,__FUNCTION__,__LINE__);
            break;
        default:
            break;
    }
}
static void wifi_mode_cb(lv_obj_t *obj, lv_event_t event)
{
	if (LV_EVENT_CLICKED == event) {
		int value = lv_slider_get_value(obj);
        if(0 == value)
		{
			system("killall wpa_supplicant &");
			system("killall udhcpc &");
			device_cfg.wifi_onoff = 0;
			cvi_set_device_config_int(CONF_KEY_WIFI_ONOFF, device_cfg.wifi_onoff);
			lv_label_set_text(wifi_on_off_label, _("CLOSE WLAN"));
		}
		else
		{
			//CVI_NET_Wlan_Init(WLAN0_PORT);
			device_cfg.wifi_onoff = 1;
			cvi_set_device_config_int(CONF_KEY_WIFI_ONOFF, device_cfg.wifi_onoff);
			lv_label_set_text(wifi_on_off_label, _("OPEN WLAN"));

			struct wlan_cfg cfg;
			//sprintf(cfg.ssid,"%s",device_cfg.wifi_ssid);
			snprintf(cfg.ssid, sizeof(cfg.ssid), "%s", device_cfg.wifi_ssid);
			snprintf(cfg.passwd, sizeof(cfg.passwd), "%s", device_cfg.wifi_pwd);
			if(strlen(cfg.passwd) >= 8)
			{
		    	//CVI_NET_Set_Wlan_Config(WLAN0_PORT, STATION, &cfg);
		    	pthread_create(&start_wifi_id,NULL,start_wifi_thread,NULL);
			}
		}
    }
}

static void netWifiMenu_page_navi_cb(lv_obj_t * obj, lv_event_t event)
{
	printf("[event:%d %s,%d]\n",event,__FUNCTION__,__LINE__);

    switch (event)
	{
		case LV_EVENT_CLICKED:
	    {
	       // printf("[event:%d  ssid:%s %s,%d]\n",event,page_id,__FUNCTION__,__LINE__);
			//check_wifi_thread_exit();
			char* page_id = (char*) lv_list_get_btn_text(obj);
			create_wifi_input_pwd_dialog(NULL, page_id);
			break;
	    }
		default:
			break;
	}
}

static void netWifiMenu_Flash_cb(lv_obj_t * obj, lv_event_t event)
{
    //char* page_id = (char*) lv_obj_get_user_data(obj);
	printf("[event:%d %s,%d]\n",event,__FUNCTION__,__LINE__);

    switch (event)
	{
		case LV_EVENT_CLICKED:
	    {
			wifi_refresh2();

			iCount = 0;
			if(0 == check_wifi_id)
			{
				//pthread_create(&check_wifi_id,NULL,check_wifi_thread,NULL);
			}
			break;
	    }
		default:
			break;
	}
}

void _style_init_wifi()
{
    if (!style_inited_wifi)
    {
        lv_style_init(&style_sb_wifi);
        lv_style_set_bg_color(&style_sb_wifi, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_bg_grad_color(&style_sb_wifi, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_border_color(&style_sb_wifi, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_border_width(&style_sb_wifi, LV_STATE_DEFAULT, 1);
        lv_style_set_border_opa(&style_sb_wifi, LV_STATE_DEFAULT, LV_OPA_70);
        lv_style_set_bg_opa(&style_sb_wifi, LV_STATE_DEFAULT, LV_OPA_60);
        lv_style_set_pad_right(&style_sb_wifi, LV_STATE_DEFAULT, 3);
        lv_style_set_pad_bottom(&style_sb_wifi, LV_STATE_DEFAULT, 3);
        lv_style_set_pad_inner(&style_sb_wifi, LV_STATE_DEFAULT, 8);

        lv_style_init(&btn_label_style_wifi);
        lv_style_set_text_font(&btn_label_style_wifi, LV_STATE_DEFAULT, &stsong_28);
        style_inited_wifi = true;
    }
}
void _style_reset_wifi()
{
	if(style_inited_wifi)
	{
		lv_style_reset(&style_sb_wifi);
		lv_style_reset(&btn_label_style_wifi);
		style_inited_wifi = false;
	}
}
static void load_data()
{
	//char card[] = "eth0";
    //get_mac(card, device_net_data.mac_address);
	//get_network_ip_address("eth0", device_net_data.ip_address);
    //get_network_netmask("eth0", device_net_data.netmask);
 	cvi_get_device_config_int(CONF_KEY_WIFI_ONOFF, &device_cfg.wifi_onoff);
	cvi_get_device_config_str(CONF_KEY_WIFI_SSID, device_cfg.wifi_ssid, sizeof(device_cfg.wifi_ssid));
	cvi_get_device_config_str(CONF_KEY_WIFI_PWD, device_cfg.wifi_pwd, sizeof(device_cfg.wifi_pwd));
}

static void create_top_panel(lv_obj_t *par)
{
    lv_obj_t * top_panel = menu_page_create_top_panel(par, back_btn_event_cb);

    title = lv_label_create(top_panel, NULL);
    lv_obj_add_style(title, LV_STATE_DEFAULT, menu_page_title_style());
    lv_label_set_text(title, _("WIFI Set"));
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(title, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(title, true);
}

static void create_connect_wifi_section(lv_obj_t *parent)
{
    //lv_obj_t *hardware_section = menu_section_create(parent, "guany");
	char bufTemp[64] = {0};

	lv_obj_t *hardware_section = lv_cont_create(parent, NULL);
    lv_obj_set_size(hardware_section, LV_HOR_RES_MAX, TOP_PANEL_H);
    lv_cont_set_layout(hardware_section, LV_LAYOUT_OFF);
    lv_obj_set_drag(hardware_section, true);
    lv_page_glue_obj(hardware_section, true);
	//lv_obj_align(hardware_section, NULL, LV_ALIGN_CENTER, 0, 0);

	lv_obj_t * switch1 = lv_switch_create(hardware_section, NULL);
    lv_obj_set_width(switch1, 160);
	lv_obj_set_height(switch1, 80);	
	lv_switch_set_anim_time(switch1, 1);
	if(1 == device_cfg.wifi_onoff)
	{
		lv_switch_on(switch1, LV_ANIM_ON);
		sprintf(bufTemp,"%s",_("OPEN WLAN"));
	}
	else
	{
		lv_switch_off(switch1, LV_ANIM_OFF);
		sprintf(bufTemp,"%s",_("CLOSE WLAN"));
	}
    lv_obj_align(switch1, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
    lv_obj_set_event_cb(switch1, wifi_mode_cb);

	wifi_on_off_label = lv_label_create(hardware_section, NULL);
    lv_obj_add_style(wifi_on_off_label, LV_STATE_DEFAULT, &btn_label_style_wifi);
    lv_label_set_text(wifi_on_off_label, bufTemp);
    lv_label_set_align(wifi_on_off_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(wifi_on_off_label, NULL, LV_ALIGN_IN_LEFT_MID, 100, 0);

	/*
	char net_mode_options[64];
	snprintf(net_mode_options, sizeof(net_mode_options), "%s\n%s", _("Off"), _("On"));
	if(1 == device_cfg.wifi_onoff)
	{
		sprintf(bufTemp,"%s",_("OPEN WLAN"));
	}
	else
	{
		sprintf(bufTemp,"%s",_("CLOSE WLAN"));
	}
	lv_obj_t *net_mode_dropdown_item = menu_section_add_dd_item(hardware_section, bufTemp,\
		net_mode_options, wifi_mode_cb);
	//const char *current_locale = lv_i18n_get_current_locale();
	printf("[device_cfg.wifi_onoff:%d %s,%d]\n",device_cfg.wifi_onoff,__FUNCTION__,__LINE__);
	if (0 == device_cfg.wifi_onoff) {
		lv_dropdown_set_selected(net_mode_dropdown_item, 0);
	} else {
		lv_dropdown_set_selected(net_mode_dropdown_item, 1);
	}
	*/
}
static void create_disp_panel(lv_obj_t *parent)
{
    lv_obj_t *disp_panel = menu_page_create_disp_scrollable_panel(parent);

    create_connect_wifi_section(disp_panel);
}

void update_wifi_status()
{
	char bufTemp[128] = {0};
 	char bufWifiIp[64] = {0};
	
	if((1 == device_cfg.wifi_onoff)&&(CONNECTED == getWlanState()))
	{
		get_network_ip_address(WLAN0_PORT, bufWifiIp);
		printf("[Ip:%s %s,%d]\n",bufWifiIp,__FUNCTION__,__LINE__);
		sprintf(bufTemp,"%s  %s  %s",device_cfg.wifi_ssid,_("Connected"),bufWifiIp);
	}
	else
	{
		sprintf(bufTemp,"%s  %s",device_cfg.wifi_ssid,_("Connecting"));
	}
	lv_label_set_text(wifi_status_label, bufTemp);
}

static void create_connect_wifi_status_item(lv_obj_t *par)
{
	char bufTemp[128] = {0};
	char bufWifiIp[64] = {0};
    //lv_obj_t * back_btn;
    lv_obj_t * eth_mode;
    //lv_obj_t * wanl_ip;
    //lv_obj_t * web_netmask;

	//lv_obj_t *wifi_status_label;
 
    netWifiConnectStatus_obj = lv_list_create(par, NULL);
    lv_obj_add_style(netWifiConnectStatus_obj, LV_PAGE_PART_SCROLLABLE, &style_sb_wifi);
    lv_obj_set_pos(netWifiConnectStatus_obj, 0, 3*TOP_PANEL_H);
    lv_obj_set_size(netWifiConnectStatus_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX - 5*TOP_PANEL_H);
    lv_page_set_edge_flash(netWifiConnectStatus_obj, true);
    lv_page_set_scrollbar_mode(netWifiConnectStatus_obj, LV_SCROLLBAR_MODE_AUTO);
    lv_page_set_scrl_layout(netWifiConnectStatus_obj, LV_LAYOUT_COLUMN_MID);

	if((1 == device_cfg.wifi_onoff)&&(CONNECTED == getWlanState()))
	{
		get_network_ip_address(WLAN0_PORT, bufWifiIp);
		printf("[Ip:%s %s,%d]\n",bufWifiIp,__FUNCTION__,__LINE__);
		sprintf(bufTemp,"%s  %s  %s",device_cfg.wifi_ssid,_("Connected"),bufWifiIp);
	}
	else
	{
		sprintf(bufTemp,"%s  %s",device_cfg.wifi_ssid,_("Connecting"));
	}

    eth_mode = lv_list_add_btn(netWifiConnectStatus_obj, RES_IMG_ICON_WLXZ, bufTemp);
    wifi_status_label = lv_list_get_btn_label(eth_mode);
    lv_obj_set_height(wifi_status_label, 40);
    lv_obj_add_style(wifi_status_label, LV_LABEL_PART_MAIN, &btn_label_style_wifi);
    lv_btn_set_fit(eth_mode, LV_FIT_TIGHT);
    //lv_obj_set_user_data(eth_mode, MENU_ID_NET_WIFI_MODE);
    //lv_obj_set_event_cb(eth_mode, netWifiMenu_page_navi_cb);

}

static void create_wifi_list(lv_obj_t *par)
{
    lv_obj_t * wifi_list;
	lv_obj_t *wifi_list_label[30];
	char wifiBuf[128] = {0};
	int wifi_num = 0;

 	if(NULL == netWifiList_obj)
 	{
	    netWifiList_obj = lv_list_create(par, NULL);
	    lv_obj_add_style(netWifiList_obj, LV_PAGE_PART_SCROLLABLE, &style_sb_wifi);
	    lv_obj_set_pos(netWifiList_obj, 0, 4*TOP_PANEL_H);
	    lv_obj_set_size(netWifiList_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX - 5*TOP_PANEL_H);
	    lv_page_set_edge_flash(netWifiList_obj, true);
	    lv_page_set_scrollbar_mode(netWifiList_obj, LV_SCROLLBAR_MODE_AUTO);
	    lv_page_set_scrl_layout(netWifiList_obj, LV_LAYOUT_COLUMN_MID);
 	}
	
	if(wlan_apinfo2 != NULL)
	{
		wifi_num = (wlan_apinfo2->count<=30)?wlan_apinfo2->count:30;
		//wifi_num = wlan_apinfo2->count;
		printf("[wifi_num:%d wlan_apinfo2->count:%d %s,%d]\n",wifi_num,wlan_apinfo2->count,__FUNCTION__,__LINE__);
	}
	else
	{
		printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	}
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	for(int i = 0; i< wifi_num; i++)
	{
		memset(wifiBuf,0x00,sizeof(wifiBuf));
		//printf("[%s,%d]\n",__FUNCTION__,__LINE__);
		sprintf(wifiBuf,"%s",wlan_apinfo2->ap_arr[i].ssid);
		printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	    wifi_list = lv_list_add_btn(netWifiList_obj, RES_IMG_ICON_WLXZ, wifiBuf);
		printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	    wifi_list_label[i] = lv_list_get_btn_label(wifi_list);
	    lv_obj_set_height(wifi_list_label[i], 40);
	    lv_obj_add_style(wifi_list_label[i], LV_LABEL_PART_MAIN, &btn_label_style_wifi);
	    lv_btn_set_fit(wifi_list, LV_FIT_TIGHT);
		//printf("[sizeof(wifiSSIDBuf[i]):%d %s,%d]\n",sizeof(wifiSSIDBuf[i]),__FUNCTION__,__LINE__);
		//memset(wifiSSIDBuf[i], 0x00, sizeof(wifiSSIDBuf[i]));
		//snprintf(wifiSSIDBuf[i],sizeof(wifiSSIDBuf[i]),"%s",wlan_apinfo2->ap_arr[i].ssid);
	    //lv_obj_set_user_data(wifi_list, (lv_obj_user_data_t)wifiSSIDBuf[i]);
		printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	    lv_obj_set_event_cb(wifi_list, netWifiMenu_page_navi_cb);
	}
	//printf("[%s,%d]\n",__FUNCTION__,__LINE__);
}

static void create_wifi_button(lv_obj_t *par)
{
	lv_obj_t * bottom_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom_panel, LV_HOR_RES_MAX, TOP_PANEL_H);
	lv_obj_set_pos(bottom_panel, 0, LV_VER_RES_MAX - TOP_PANEL_H);
	lv_obj_add_style(bottom_panel, LV_CONT_PART_MAIN, menu_page_btn_back_title_style());
	lv_obj_set_event_cb(bottom_panel, netWifiMenu_Flash_cb);

	lv_obj_t *flash_label = lv_label_create(bottom_panel, NULL);
    lv_obj_add_style(flash_label, LV_STATE_DEFAULT, &btn_label_style_wifi);
    lv_label_set_text(flash_label, _("flash"));
    lv_label_set_align(flash_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(flash_label, NULL, LV_ALIGN_CENTER, 0, 0);
  
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
}

static void netWifiMenuPageCreate(void)
{
	static uint32_t user_data = 10;
    _style_init_wifi();
	load_data();

    netWifiMenuPage_cont = lv_cont_create(lv_scr_menu(), NULL);
    lv_obj_set_size(netWifiMenuPage_cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_cont_set_fit(netWifiMenuPage_cont, LV_FIT_NONE);
    lv_cont_set_layout(netWifiMenuPage_cont, LV_LAYOUT_OFF);

	lv_obj_t *flash_label = lv_label_create(netWifiMenuPage_cont, NULL);
    lv_obj_add_style(flash_label, LV_STATE_DEFAULT, &btn_label_style_wifi);
    lv_label_set_text(flash_label, _("flashing"));
    lv_label_set_align(flash_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(flash_label, NULL, LV_ALIGN_CENTER, 0, 0);

    create_top_panel(netWifiMenuPage_cont);
	create_disp_panel(netWifiMenuPage_cont);
	create_connect_wifi_status_item(netWifiMenuPage_cont);
    create_wifi_list(netWifiMenuPage_cont);
	create_wifi_button(netWifiMenuPage_cont);

	//pthread_create(&check_wifi_id,NULL,check_wifi_thread,NULL);
	iCount = 0;
	wifi_task = lv_task_create(flash_wifi_info_task, 1000, LV_TASK_PRIO_MID, &user_data);
}

static void netWifiMenuPageDestroy(void)
{
	if(netWifiConnectStatus_obj != NULL)
	{
		lv_obj_clean(netWifiConnectStatus_obj);
		netWifiConnectStatus_obj = NULL;
	}

	if(netWifiList_obj != NULL)
	{
		//lv_obj_del(netWifiList_obj);
		lv_obj_clean(netWifiList_obj);
		netWifiList_obj = NULL;
	}

	if(netWifiMenuPage_cont != NULL)
	{
	    lv_obj_del(netWifiMenuPage_cont);
	    netWifiMenuPage_cont = NULL;
	}
    wifi_mode_label = NULL;
    wifi_ip_label = NULL;
    wifi_netmask_label = NULL;
	_style_reset_wifi();
}

static void netWifiMenu_page_show()
{
    if (netWifiMenuPage_cont == NULL) {
        return;
    }

    lv_obj_set_hidden(netWifiMenuPage_cont, false);
}

static void netWifiMenu_page_hide()
{
    if (netWifiMenuPage_cont == NULL) {
        return;
    }

    lv_obj_set_hidden(netWifiMenuPage_cont, true);
}

