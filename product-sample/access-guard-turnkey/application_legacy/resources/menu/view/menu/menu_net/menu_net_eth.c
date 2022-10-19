#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "menu_device_manage.h"
#include "menu_net_eth.h"
#include "cvi_audio.h"
#include "lv_i18n.h"
#include "resource.h"
#include "menu_common.h"
#include "menu_page.h"
#include "cviconfig.h"
#include "keyboard.h"
#include "sys.h"
#include "cvi_hal_face_api.h"

static void net_eth_menu_page_create();
static void net_eth_menu_page_destroy();
static void net_eth_menu_page_show();
static void net_eth_menu_page_hide();

static lv_obj_t *net_eth_menu_obj = NULL;
static menu_page_t net_eth_menu_page = {
    .onCreate = net_eth_menu_page_create,
    .onDestroy = net_eth_menu_page_destroy,
    .show = net_eth_menu_page_show,
    .hide = net_eth_menu_page_hide,
    .back = menuPageBack
};

typedef struct {
    char mac_address[32];
    char ip_address[32];
    char netmask[32];
    char broadcast_address[32];
	char gateway[32];
	char dns[32];
	int net_mode;
} device_net_t;

static device_net_t device_net_data;
static lv_obj_t *kb;

static lv_obj_t *eth_on_off_label;
static lv_obj_t *ip_ta = NULL;
static lv_obj_t *nm_ta = NULL;
static lv_obj_t *gw_ta = NULL;
static lv_obj_t *dns_ta = NULL;

// const int SCROLL_BAR_WIDTH = 50;

//static char* wifi_password;
//static char* wifi_ssid;
static int change_flag = 0;
static bool load_data_flag = false;

/*****************************
    PRIVATE STATIC FUNCTIONS
 *****************************/

static lv_obj_t * create_menu_container(lv_obj_t *parent);
static void load_data();
  
 static void create_top_panel(lv_obj_t *parent);
static void back_btn_event_cb(lv_obj_t * obj, lv_event_t event);
static void create_disp_panel(lv_obj_t *parent);
static void create_hardware_section(lv_obj_t *parent);
//static void refresh();
static void keyboard_cb(lv_obj_t *obj, lv_event_t event);
static void net_mode_cb(lv_obj_t *obj, lv_event_t event);
static void ip_address_cb(lv_obj_t *obj, lv_event_t event);
static void gateway_cb(lv_obj_t *obj, lv_event_t event);
static void dns_cb(lv_obj_t *obj, lv_event_t event);
static void netmask_cb(lv_obj_t *obj, lv_event_t event);

menu_page_t get_menu_net_eth_page()
{
    return net_eth_menu_page;
}


void set_net_config()
{
	char command[128] = {0};
	printf("device_net_data.ip_address:%s \r\n",device_net_data.ip_address);
    snprintf(command, sizeof(command), "ifconfig eth0 %s netmask %s", device_net_data.ip_address, device_net_data.netmask);
    system(command);

	memset(command, 0x00, sizeof(command));
    snprintf(command, sizeof(command), "route add default gw %s", device_net_data.gateway);
    system(command);
}

static void net_eth_menu_page_create()
{
    load_data();

    lv_obj_t *parent = lv_scr_menu();
    lv_obj_t *menu_container = create_menu_container(parent);
    create_top_panel(menu_container);
    create_disp_panel(menu_container);
}

static void net_eth_menu_page_destroy()
{
	if (kb)
    {
        lv_obj_del(kb);
        kb = NULL;
    }
    lv_obj_del(net_eth_menu_obj);
    net_eth_menu_obj = NULL;
}

static void net_eth_menu_page_show()
{
    if (net_eth_menu_obj == NULL) {
        return;
    }

    lv_obj_set_hidden(net_eth_menu_obj, false);
}

static void net_eth_menu_page_hide()
{
    if (net_eth_menu_obj == NULL) {
        return;
    }

    lv_obj_set_hidden(net_eth_menu_obj, true);
}

static lv_obj_t * create_menu_container(lv_obj_t *parent)
{
    net_eth_menu_obj = lv_cont_create(parent, NULL);
    lv_obj_add_style(net_eth_menu_obj, LV_CONT_PART_MAIN, menu_page_style());
    lv_obj_set_size(net_eth_menu_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_hidden(net_eth_menu_obj, true);

    return net_eth_menu_obj;
}

static void create_top_panel(lv_obj_t * parent)
{
    lv_obj_t *top_panel = menu_page_create_top_panel(parent, back_btn_event_cb);

    lv_obj_t *page_title = lv_label_create(top_panel, NULL);
    lv_label_set_text(page_title, _("Ethernet Set"));
    lv_label_set_align(page_title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(page_title, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(page_title, true);
    lv_obj_add_style(page_title, LV_LABEL_PART_MAIN, menu_page_title_style());
}

static void create_disp_panel(lv_obj_t *parent)
{
    lv_obj_t *disp_panel = menu_page_create_disp_scrollable_panel(parent);

    create_hardware_section(disp_panel);
}

static void load_data()
{
	char card[] = "eth0";
    get_mac(card, device_net_data.mac_address);
	get_network_ip_address("eth0", device_net_data.ip_address);
    get_network_netmask("eth0", device_net_data.netmask);
 	cvi_get_device_config_int(CONF_KEY_NET_MODE, &device_net_data.net_mode);
	cvi_get_device_config_str(CONF_KEY_GATEWAY, device_net_data.gateway, sizeof(device_net_data.gateway));
	cvi_get_device_config_str(CONF_KEY_DNS, device_net_data.dns, sizeof(device_net_data.dns));
}

 static void back_btn_event_cb(lv_obj_t *obj, lv_event_t event)
{
    switch (event) {
        case LV_EVENT_RELEASED:
			
			printf("change_flag:%d \r\n",change_flag);
			if(1 == change_flag)
			{
				set_net_config();
			}
            menuPageBack();
            break;
        default:
            break;
    }
}

static void create_hardware_section(lv_obj_t *parent)
{
	char bufTemp[64] = {0};
	lv_obj_t * cont = lv_cont_create(parent, NULL);
    lv_obj_set_size(cont, LV_HOR_RES_MAX, 3*MENU_SEC_HEADER_H);
    lv_cont_set_layout(cont, LV_LAYOUT_OFF);
    lv_obj_set_drag(cont, true);
    lv_page_glue_obj(cont, true);

	lv_obj_t * switch1 = lv_switch_create(cont, NULL);
    lv_obj_set_width(switch1, 160);
	lv_obj_set_height(switch1, 80);	
	lv_switch_set_anim_time(switch1, 1);
	if(0 == device_net_data.net_mode)
	{
		lv_switch_on(switch1, LV_ANIM_ON);
		sprintf(bufTemp,"%s",_("hand mode"));
	}
	else
	{
		lv_switch_off(switch1, LV_ANIM_ON);
		sprintf(bufTemp,"%s",_("auto mode"));
        load_data_flag = true;
	}
    lv_obj_align(switch1, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
    lv_obj_set_event_cb(switch1, net_mode_cb);

	eth_on_off_label = lv_label_create(cont, NULL);
    lv_obj_add_style(eth_on_off_label, LV_STATE_DEFAULT, menu_page_btn_label_style());
    lv_label_set_text(eth_on_off_label, bufTemp);
    lv_label_set_align(eth_on_off_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(eth_on_off_label, NULL, LV_ALIGN_IN_LEFT_MID, 100, 0);
	
	
    lv_obj_t *hardware_section = menu_section_create(parent, "");
    lv_obj_set_width(hardware_section, lv_obj_get_width(hardware_section) - SCROLL_BAR_WIDTH);
    ip_ta = menu_section_add_text_item(hardware_section, _("Ip address"), device_net_data.ip_address, ip_address_cb);
    nm_ta = menu_section_add_text_item(hardware_section, _("Netmask"), device_net_data.netmask, netmask_cb);
	gw_ta = menu_section_add_text_item(hardware_section, _("Gateway"), device_net_data.gateway, gateway_cb);
	dns_ta = menu_section_add_text_item(hardware_section, _("Dns"), device_net_data.dns, dns_cb);
}

#if 0
static void refresh()
{
    if (menu_tree_isEmpty()) {
        return;
    }

    menu_tree_pop();
    net_eth_menu_page_destroy();
    menu_tree_push(get_menu_net_eth_page());
    net_eth_menu_page_create();
    net_eth_menu_page_show();
}
#endif

static void keyboard_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        if (kb == NULL) {
            kb = create_keyboard(&kb, obj, net_eth_menu_obj, 0, 0);
			lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUM);
        }
    }
}

static int get_ip_netmask(void)
{
    char ip_addr[32] = {0};
    char net_mask[32] = {0};

    get_network_ip_address("eth0", ip_addr);
    lv_textarea_set_text(ip_ta, ip_addr);

    get_network_netmask("eth0", net_mask);
    lv_textarea_set_text(nm_ta, net_mask);

    return 0;
}

static int get_gw_dns(void)
{
    char cmd[64] = {0};
    char readline[128] = {0};

    sprintf(cmd, "netstat -r|grep default|cut -f 10 -d \' \'");
    FILE * fp = popen(cmd, "r");
    if(NULL == fp) {
        printf("popen gateway cmd failed!\n");
        return -1;
    }

    while(NULL != fgets(readline, sizeof(readline), fp)) {
        lv_textarea_set_text(gw_ta, readline);
    }
    pclose(fp);

    fp = fopen("/run/resolv.conf", "r");
    if(NULL == fp) {
        printf("fopen /run/resolv.conf failed!\n");
        return -1;
    }

    memset(readline, 0, sizeof(readline));
    memset(cmd, 0, sizeof(cmd));
    while(NULL != fgets(readline, sizeof(readline), fp)) {
        if(strncmp("nameserver", readline, strlen("nameserver")) == 0) {
            char * start = readline + strlen("nameserver ");
            char * end = strchr(start, ' ');
            if(end)
                strncpy(cmd, start, end - start);
            else
                strcpy(cmd, start);
            lv_textarea_set_text(dns_ta, cmd);
            break;
        }
        else {
            memset(readline, 0, sizeof(readline));
            continue;
        }
    }
    fclose(fp);

    return 0;
}

static void net_mode_cb(lv_obj_t *obj, lv_event_t event)
{
	if (LV_EVENT_CLICKED == event) {
		//char buf[32];
		char dhcp_command[128];
		int value = lv_slider_get_value(obj);
        if(1 == value)
		{
			lv_label_set_text(eth_on_off_label, _("hand mode"));
			snprintf(dhcp_command, sizeof(dhcp_command), "killall udhcpc");
	        system(dhcp_command);
			device_net_data.net_mode = 0;
			cvi_set_device_config_int(CONF_KEY_NET_MODE, device_net_data.net_mode);

			cvi_get_device_config_str(CONF_KEY_NET_IP, device_net_data.ip_address, sizeof(device_net_data.ip_address));
            lv_textarea_set_text(ip_ta, device_net_data.ip_address);
			cvi_get_device_config_str(CONF_KEY_NET_MASK, device_net_data.netmask, sizeof(device_net_data.netmask));
            lv_textarea_set_text(nm_ta, device_net_data.netmask);
            cvi_get_device_config_str(CONF_KEY_GATEWAY, device_net_data.gateway, sizeof(device_net_data.gateway));
            lv_textarea_set_text(gw_ta, device_net_data.gateway);
            cvi_get_device_config_str(CONF_KEY_DNS, device_net_data.dns, sizeof(device_net_data.dns));
            lv_textarea_set_text(dns_ta, device_net_data.dns);
            load_data_flag = false;

			set_net_config();
		}
		else
		{
			lv_label_set_text(eth_on_off_label, _("auto mode"));
			snprintf(dhcp_command, sizeof(dhcp_command), "udhcpc -b -i eth0 -R &");
	        system(dhcp_command);
			device_net_data.net_mode = 1;
			change_flag = 0;
            load_data_flag = true;
			cvi_set_device_config_int(CONF_KEY_NET_MODE, device_net_data.net_mode);
            usleep(1000*1000);
            get_ip_netmask();
            get_gw_dns();
		}
    }
}

static void ip_address_cb(lv_obj_t *obj, lv_event_t event)
{
	if (1 ==device_net_data.net_mode || load_data_flag)
	{
		return ;
	}
    keyboard_cb(obj, event);

    if (LV_EVENT_VALUE_CHANGED == event) {
        const char *new_ip_address = lv_textarea_get_text(obj);
		snprintf(device_net_data.ip_address, sizeof(device_net_data.ip_address), "%s", new_ip_address);
		cvi_set_device_config_str(CONF_KEY_NET_IP, device_net_data.ip_address);
		change_flag = 1;
    }
}

static void gateway_cb(lv_obj_t *obj, lv_event_t event)
{
	if (1 ==device_net_data.net_mode || load_data_flag)
	{
		return ;
	}
    keyboard_cb(obj, event);

    if (LV_EVENT_VALUE_CHANGED == event) {
        const char *new_gateway = lv_textarea_get_text(obj);
		snprintf(device_net_data.gateway, sizeof(device_net_data.gateway), "%s", new_gateway);
		cvi_set_device_config_str(CONF_KEY_GATEWAY, device_net_data.gateway);
		change_flag = 1;
    }
}
static void netmask_cb(lv_obj_t *obj, lv_event_t event)
{
	if (1 ==device_net_data.net_mode || load_data_flag)
	{
		return ;
	}
    keyboard_cb(obj, event);

    if (LV_EVENT_VALUE_CHANGED == event) {
        const char *new_mask = lv_textarea_get_text(obj);
		snprintf(device_net_data.netmask, sizeof(device_net_data.netmask), "%s", new_mask);
		cvi_set_device_config_str(CONF_KEY_NET_MASK, device_net_data.netmask);
		change_flag = 1;
    }
}
static void dns_cb(lv_obj_t *obj, lv_event_t event)
{
	if (1 ==device_net_data.net_mode || load_data_flag)
	{
		return ;
	}
    keyboard_cb(obj, event);
	//printf("event:%d \r\n",event);

    if (LV_EVENT_VALUE_CHANGED == event) {
        const char *new_broadcast = lv_textarea_get_text(obj);
		snprintf(device_net_data.dns, sizeof(device_net_data.dns), "%s", new_broadcast);
		cvi_set_device_config_str(CONF_KEY_DNS, device_net_data.dns);
    }
}
