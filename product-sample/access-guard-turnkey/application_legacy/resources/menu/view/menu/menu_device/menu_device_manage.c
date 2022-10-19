#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "menu_device_manage.h"
#include "cvi_audio.h"
#include "lv_i18n.h"
#include "resource.h"
#include "menu_common.h"
#include "menu_page.h"
#include "cviconfig.h"
#include "keyboard.h"
#include "sys.h"
#include "cvi_hal_face_api.h"
#include "app_peripheral.h"


static void dev_manage_menu_page_create();
static void dev_manage_menu_page_destroy();
static void dev_manage_menu_page_show();
static void dev_manage_menu_page_hide();

static lv_obj_t *dev_manage_menu_obj = NULL;
static menu_page_t dev_manage_menu_page = {
    .onCreate = dev_manage_menu_page_create,
    .onDestroy = dev_manage_menu_page_destroy,
    .show = dev_manage_menu_page_show,
    .hide = dev_manage_menu_page_hide,
    .back = menuPageBack
};

typedef struct {
    char time_from_boot[32];
    char version_name[64];
    char device_name[256];
    char device_type[32];
    char device_serial[64];
    char release_time[64];
    char mac_address[32];
    char ip_address[32];
    char netmask[32];
    char broadcast_address[32];
    int voice_sound;
    int display_brightness;
    int display_enter_lock_time;
    int display_enter_close_time;
    char device_password[256];
    int available_storage;
    int occupancy_storage;
    int register_people_num;
} device_t;

static device_t device_data;
static lv_obj_t *kb;

// const int SCROLL_BAR_WIDTH = 50;
const int MAX_PEOPLE_NUMBER = 5000;
char *lang_en = "en-US";
char *lang_zh = "zh-CN";

//static const char* wifi_password;
//static const char* wifi_ssid;

extern hal_facelib_handle_t facelib_handle;

/*****************************
    PRIVATE STATIC FUNCTIONS
 *****************************/

static lv_obj_t * create_menu_container(lv_obj_t *parent);
static void load_data();
static void load_about_data();
static void load_hardware_data();
static void load_storage_data();
static void load_account_data();
static void create_top_panel(lv_obj_t *parent);
static void back_btn_event_cb(lv_obj_t * obj, lv_event_t event);
static void create_disp_panel(lv_obj_t *parent);
static void create_about_section(lv_obj_t *parent);
static void create_hardware_section(lv_obj_t *parent);
//static void create_wifi_section(lv_obj_t *parent);
//static void create_storage_section(lv_obj_t *parent);
static void create_system_section(lv_obj_t *parent);
//static void create_account_section(lv_obj_t *parent);
static void create_other_section(lv_obj_t *parent);
static void refresh();
static void keyboard_cb(lv_obj_t *obj, lv_event_t event);
static void voice_sound_cb(lv_obj_t *obj, lv_event_t event);
static void display_brightness_cb(lv_obj_t *obj, lv_event_t event);
static void reboot_cb(lv_obj_t *obj, lv_event_t event);
//static void shutdown_cb(lv_obj_t *obj, lv_event_t event);
static void ntp_cb(lv_obj_t *obj, lv_event_t event);
static void langs_cb(lv_obj_t *obj, lv_event_t event);
//static void device_name_cb(lv_obj_t *obj, lv_event_t event);
static void upgrade_cb(lv_obj_t *obj, lv_event_t event);
//static void ip_address_cb(lv_obj_t *obj, lv_event_t event);
//static void broadcast_address_cb(lv_obj_t *obj, lv_event_t event);
//static void netmask_cb(lv_obj_t *obj, lv_event_t event);
static void manual_date_cb(lv_obj_t *obj, lv_event_t event);
static void display_enter_lock_time_cb(lv_obj_t *obj, lv_event_t event);
static void display_enter_close_time_cb(lv_obj_t *obj, lv_event_t event);
//static void wifi_ssid_cb(lv_obj_t *obj, lv_event_t event);
//static void wifi_password_cb(lv_obj_t *obj, lv_event_t event);
//static void connect_wifi_cb(lv_obj_t *obj, lv_event_t event);
//static void password_cb(lv_obj_t *obj, lv_event_t event);
static void reset_cb(lv_obj_t *obj, lv_event_t event);


menu_page_t get_dev_manage_menu_page()
{
    return dev_manage_menu_page;
}

static void dev_manage_menu_page_create()
{
    load_data();

    lv_obj_t *parent = lv_scr_menu();
    lv_obj_t *menu_container = create_menu_container(parent);
    create_top_panel(menu_container);
    create_disp_panel(menu_container);
}

static void dev_manage_menu_page_destroy()
{
	if (kb != NULL) {
        lv_obj_del(kb);
        kb = NULL;
    }
    lv_obj_del(dev_manage_menu_obj);
    dev_manage_menu_obj = NULL;
}

static void dev_manage_menu_page_show()
{
    if (dev_manage_menu_obj == NULL) {
        return;
    }

    lv_obj_set_hidden(dev_manage_menu_obj, false);
}

static void dev_manage_menu_page_hide()
{
    if (dev_manage_menu_obj == NULL) {
        return;
    }

    lv_obj_set_hidden(dev_manage_menu_obj, true);
}

static lv_obj_t * create_menu_container(lv_obj_t *parent)
{
    dev_manage_menu_obj = lv_cont_create(parent, NULL);
    lv_obj_add_style(dev_manage_menu_obj, LV_CONT_PART_MAIN, menu_page_style());
    lv_obj_set_size(dev_manage_menu_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_hidden(dev_manage_menu_obj, true);

    return dev_manage_menu_obj;
}

static void create_top_panel(lv_obj_t * parent)
{
    lv_obj_t *top_panel = menu_page_create_top_panel(parent, back_btn_event_cb);

    lv_obj_t *page_title = lv_label_create(top_panel, NULL);
    lv_label_set_text(page_title, _("Device Manage"));
    lv_label_set_align(page_title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(page_title, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(page_title, true);
    lv_obj_add_style(page_title, LV_LABEL_PART_MAIN, menu_page_title_style());
}

static void create_disp_panel(lv_obj_t *parent)
{
    lv_obj_t *disp_panel = menu_page_create_disp_scrollable_panel(parent);

    create_hardware_section(disp_panel);
    //create_wifi_section(disp_panel);
    //create_storage_section(disp_panel);
    create_system_section(disp_panel);
    //create_account_section(disp_panel);
    create_other_section(disp_panel);
	create_about_section(disp_panel);
}

static void load_data()
{
    load_about_data();
    load_hardware_data();
    load_storage_data();
    load_account_data();
}

static void load_about_data()
{
    get_system_boot_time(device_data.time_from_boot);
    cvi_get_device_config_str(CONF_KEY_DEVICE_NAME, device_data.device_name,
        sizeof(device_data.device_name));
    snprintf(device_data.device_type, sizeof(device_data.device_type), "cv1835");
    cvi_get_device_config_str(CONF_KEY_VERSION_NAME, device_data.version_name,
        sizeof(device_data.version_name));
    char card[] = "eth0";
    get_mac(card, device_data.mac_address);
    char serial[64] = "SN";
    for (int i = 0, j = 2; i < sizeof(device_data.mac_address); i++) {
        if (device_data.mac_address[i] != ':') {
            serial[j++] = (('a' <= device_data.mac_address[i]) && ('z' >= device_data.mac_address[i])) ?
                toupper(device_data.mac_address[i]) : device_data.mac_address[i];
        }
    }
    snprintf(device_data.device_serial, sizeof(device_data.device_serial), serial);
}

static void load_hardware_data()
{
    int ret;
    get_network_ip_address("eth0", device_data.ip_address);
    get_network_netmask("eth0", device_data.netmask);
    get_network_broadcast("eth0", device_data.broadcast_address);
    cvi_get_device_config_int(CONF_KEY_VOICE_SOUND, &device_data.voice_sound);
    ret = cvi_get_device_config_int(CONF_KEY_DISPLAY_BRIGHTNESS, &device_data.display_brightness);
    if(ret < 0)
        device_data.display_brightness = 100;
    cvi_get_device_config_int(CONF_KEY_DISPLAY_ENTER_LOCK_TIME, &device_data.display_enter_lock_time);
    cvi_get_device_config_int(CONF_KEY_DISPLAY_ENTER_CLOSE_TIME, &device_data.display_enter_close_time);
}

static void load_storage_data()
{
    int total_storage = get_partition_total_size("/mnt/data/");
    device_data.available_storage = get_partition_available_size("/mnt/data/");
    device_data.occupancy_storage = total_storage - device_data.available_storage;
    device_data.register_people_num = HAL_FACE_GetRepoNum(facelib_handle);
}

static void load_account_data()
{
    cvi_get_device_config_str(CONF_KEY_DEVICE_PASSWORD, device_data.device_password,
        sizeof(device_data.device_password));
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

static void create_about_section(lv_obj_t *parent)
{
    lv_obj_t *about_section = menu_section_create(parent, _("About"));
    lv_obj_set_width(about_section, lv_obj_get_width(about_section) - SCROLL_BAR_WIDTH);
/*
	menu_section_add_text_item(about_section, _("Boot time"), device_data.time_from_boot, NULL);
    menu_section_add_text_item(about_section, _("Version"), device_data.version_name, NULL);
    menu_section_add_text_item(about_section, _("Device name"), device_data.device_name, device_name_cb);
    menu_section_add_text_item(about_section, _("Device type"), device_data.device_type, NULL);
    menu_section_add_text_item(about_section, _("Device serial"), device_data.device_serial, NULL);
    menu_section_add_text_item(about_section, _("Release time"), device_data.release_time, NULL);
    menu_section_add_text_item(about_section, _("Mac address"), device_data.mac_address, NULL);
*/
    menu_section_add_button_item(about_section, _("Upgrade"), _("Upgrade"), upgrade_cb);
}

static void create_hardware_section(lv_obj_t *parent)
{
    lv_obj_t *hardware_section = menu_section_create(parent, _("Hardware"));
    lv_obj_set_width(hardware_section, lv_obj_get_width(hardware_section) - SCROLL_BAR_WIDTH);
    lv_obj_t *voice_sound_item = menu_section_add_slider_item(hardware_section, _("Voice sound"),
        device_data.voice_sound, voice_sound_cb);
    lv_obj_set_width(voice_sound_item, lv_obj_get_width(voice_sound_item) - SCROLL_BAR_WIDTH/2);
    lv_slider_set_range(voice_sound_item, 0, 15);
    lv_obj_t *display_brightness_item = menu_section_add_slider_item(hardware_section, _("Display brightness"),
        device_data.display_brightness, display_brightness_cb);
    lv_obj_set_width(display_brightness_item, lv_obj_get_width(display_brightness_item) - SCROLL_BAR_WIDTH/2);
    char display_enter_lock_time[32];
    sprintf(display_enter_lock_time, "%d", device_data.display_enter_lock_time);
    menu_section_add_text_item(hardware_section, _("Enter display lock time"),
        display_enter_lock_time, display_enter_lock_time_cb);
    char display_enter_close_time[32];
    sprintf(display_enter_close_time, "%d", device_data.display_enter_close_time);
    menu_section_add_text_item(hardware_section, _("Enter display close time"),
        display_enter_close_time, display_enter_close_time_cb);
    //menu_section_add_dd_item(hardware_section, _("Camera"), "", NULL);
}

#if 0
static void create_wifi_section(lv_obj_t *parent)
{
    lv_obj_t *wifi_section = menu_section_create(parent, _("Wifi"));
    lv_obj_set_width(wifi_section, lv_obj_get_width(wifi_section) - SCROLL_BAR_WIDTH);
    lv_obj_t *ssid_item = menu_section_add_text_item(wifi_section, _("SSID"), "", wifi_ssid_cb);
    wifi_ssid = lv_textarea_get_text(ssid_item);
    lv_obj_t *password_item = menu_section_add_text_item(wifi_section, _("Password"), "", wifi_password_cb);
    wifi_password = lv_textarea_get_text(password_item);
    menu_section_add_button_item(wifi_section, _("Connect"), _("Connect"), connect_wifi_cb);
}


static void create_storage_section(lv_obj_t *parent)
{
    lv_obj_t *storage_section = menu_section_create(parent, _("Storage"));
    lv_obj_set_width(storage_section, lv_obj_get_width(storage_section) - SCROLL_BAR_WIDTH);

    char storage_info[64];
    char available_storage[32];
    char occupancy_storage[32];
    sprintf(available_storage, "%d", device_data.available_storage);
    sprintf(occupancy_storage, "%d", device_data.occupancy_storage);
    snprintf(storage_info, sizeof(storage_info), "%s/%s MB", occupancy_storage, available_storage);
    menu_section_add_text_item(storage_section, _("Occupancy/Remaining"), storage_info, NULL);

    char register_info[64];
    char register_people_num[32];
    char max_people_num[32];
    sprintf(register_people_num, "%d", device_data.register_people_num);
    sprintf(max_people_num, "%d", MAX_PEOPLE_NUMBER);
    snprintf(register_info, sizeof(register_info), "%s/%s", register_people_num, max_people_num);
    menu_section_add_text_item(storage_section, _("Registered people/Max"), register_info, NULL);
}
#endif

static void create_system_section(lv_obj_t *parent)
{
    lv_obj_t *system_section = menu_section_create(parent, _("System"));
    lv_obj_set_width(system_section, lv_obj_get_width(system_section) - SCROLL_BAR_WIDTH);
    char langs_options[64];
    snprintf(langs_options, sizeof(langs_options), "%s\n%s", lang_en, lang_zh);
    lv_obj_t *langs_dropdown_item = menu_section_add_dd_item(system_section, _("Language"), langs_options, langs_cb);
    const char *current_locale = lv_i18n_get_current_locale();
    if (strcmp(current_locale, lang_en) == 0) {
        lv_dropdown_set_selected(langs_dropdown_item, 0);
    } else {
        lv_dropdown_set_selected(langs_dropdown_item, 1);
    }
    menu_section_add_button_item(system_section, _("Syn NTP"), _("Start"), ntp_cb);
    //menu_section_add_dd_item(system_section, _("Time zone"), "Taiwan\n China", NULL);
    char current_date[64];
    memset(current_date,0,sizeof(current_date));
    get_current_date(current_date);
    menu_section_add_text_item(system_section, _("Manual date"), current_date, manual_date_cb);
    //menu_section_add_button_item(system_section, _("Log export"), _("Export"), NULL);
}

#if 0
static void create_account_section(lv_obj_t *parent)
{
    lv_obj_t *account_section = menu_section_create(parent, _("Account"));
    lv_obj_set_width(account_section, lv_obj_get_width(account_section) - SCROLL_BAR_WIDTH);
    lv_obj_t *password_item = menu_section_add_text_item(account_section, _("Password"), "", password_cb);
    lv_textarea_set_pwd_mode(password_item, true);
    lv_textarea_set_text(password_item, device_data.device_password);
}
#endif

static void create_other_section(lv_obj_t *parent)
{
    lv_obj_t *other_section = menu_section_create(parent, _("Others"));
    lv_obj_set_width(other_section, lv_obj_get_width(other_section) - SCROLL_BAR_WIDTH);
    menu_section_add_button_item(other_section, _("Reboot"), _("Reboot"), reboot_cb);
    //menu_section_add_button_item(other_section, _("Shutdown"), _("Shutdown"), shutdown_cb);
    menu_section_add_button_item(other_section, _("Reset"), _("Reset"), reset_cb);
}

static void refresh()
{
    if (menu_tree_isEmpty()) {
        return;
    }

    menu_tree_pop();
    dev_manage_menu_page_destroy();
    menu_tree_push(get_dev_manage_menu_page());
    dev_manage_menu_page_create();
    dev_manage_menu_page_show();
}

static void keyboard_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        if (kb == NULL) {
            kb = create_keyboard(&kb, obj, dev_manage_menu_obj, 0, 0);
			lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUM);
        }
    }
}

static void voice_sound_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_VALUE_CHANGED == event) {
        int value = lv_slider_get_value(obj);
        cvi_set_device_config_int(CONF_KEY_VOICE_SOUND, value);
        //CVI_AO_SetVolume(1, value);
    }
}

static void display_brightness_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_VALUE_CHANGED == event) {
        int value = lv_slider_get_value(obj);
        cvi_set_device_config_int(CONF_KEY_DISPLAY_BRIGHTNESS, value);

		CVI_HAL_PwmDisable(0, 1);
		CVI_HAL_PwmUnExport(0, 1);
		CVI_HAL_PwmExport(0, 1);
		CVI_HAL_PwmSetParm(0, 1, PWM_LCD_PERIOD, PWM_LCD_DUTY_BASE * ((value == 0) ? 1 : value));
		CVI_HAL_PwmEnable(0, 1);
    }
}

static void reboot_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        system("reboot -f");
    }
}

#if 0
static void shutdown_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        //system("shutdown now");
        //system("shutdown");
    }
}
#endif
lv_obj_t * mbox1 = NULL;
#if 0
static void yes_no_handler(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        printf("Button: %s\n", lv_msgbox_get_active_btn_text(obj));
		lv_obj_del(mbox1);
   		mbox1 = NULL;
		
    }
}
#endif
static void reset_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
		/*
        static const char * btns[] ={"Apply", "Close", ""};

	    mbox1 = lv_msgbox_create(lv_scr_act(), NULL);
	    lv_msgbox_set_text(mbox1, "A message box with two buttons.");
	    lv_msgbox_add_btns(mbox1, btns);
	    lv_obj_set_width(mbox1, 500);
		lv_obj_set_height(mbox1, 500);
	    //lv_obj_set_event_cb(mbox1, yes_no_handler);
	    lv_obj_set_event_cb(mbox1, NULL);
	    lv_obj_align(mbox1, NULL, LV_ALIGN_CENTER, 0, 0); 
		*/
    }
}

static void ntp_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        system("ntpd -g -q time.pool.aliyun.com");
    }
}

static void langs_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_VALUE_CHANGED == event) {
        char buf[32];
        lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
        if (strcmp(buf, lang_en) == 0) {
            lv_i18n_set_locale(lang_en);
            mainMenu_retranslateUI();
            refresh();
        } else if (strcmp(buf, lang_zh) == 0) {
            lv_i18n_set_locale(lang_zh);
            mainMenu_retranslateUI();
            refresh();
        }
    }
}

#if 0
static void device_name_cb(lv_obj_t *obj, lv_event_t event)
{
    keyboard_cb(obj, event);

    if (LV_EVENT_VALUE_CHANGED == event) {
        const char *new_device_name = lv_textarea_get_text(obj);
        cvi_set_device_config_str(CONF_KEY_DEVICE_NAME, (char *)new_device_name);
    }
}
#endif

static void upgrade_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        system("devmem 0x03005D00 32 0x55425355");
        system("reboot -f");
    }
}

static char* find_chars_or_comment(const char* s, const char* chars)
{
    while (*s && (!chars || !strchr(chars, *s))) {
        s++;
    }

    return (char*)s;
}

static int check_data_format(const char* date)
{
    char tmp[8] = "";
    int value = 0;
    char * start;
    char * end;

    if(strlen(date) < 14 || strlen(date) > 19)
        return -1;

    start = (char *)date;
    end = find_chars_or_comment(start, "-");
    strncpy(tmp, start, end - start);
    value = atoi(tmp);
    if(1970 > value || value > 2038)
        return -1;

    start = end + 1;
    end = find_chars_or_comment(start, "-");
    memset(tmp, 0, sizeof(tmp));
    strncpy(tmp, start, end - start);
    value = atoi(tmp);
    if(0 > value || value > 12)
        return -1;

    start = end + 1;
    end = find_chars_or_comment(start, " ");
        memset(tmp, 0, sizeof(tmp));
    strncpy(tmp, start, end - start);
    value = atoi(tmp);
    if(0 > value || value > 31)
        return -1;

    start = end + 1;
    end = find_chars_or_comment(start, ":");
        memset(tmp, 0, sizeof(tmp));
    strncpy(tmp, start, end - start);
    value = atoi(tmp);
    if(0 > value || value > 23)
        return -1;

    start = end + 1;
    end = find_chars_or_comment(start, ":");
        memset(tmp, 0, sizeof(tmp));
    strncpy(tmp, start, end - start);
    value = atoi(tmp);
    if(0 > value || value > 59)
        return -1;

    start = end + 1;
        memset(tmp, 0, sizeof(tmp));
    strcpy(tmp, start);
    value = atoi(tmp);
    if(0 > value || value > 60)
        return -1;

    return 0;
}

static void manual_date_cb(lv_obj_t *obj, lv_event_t event)
{
    keyboard_cb(obj, event);
    int ret = 0;

    if (LV_EVENT_VALUE_CHANGED == event) {
        const char *new_date = lv_textarea_get_text(obj);
        ret = check_data_format(new_date);
        if(ret == -1)
            return;
        char date_command[128];
        snprintf(date_command, sizeof(date_command), "date -s \"%s\"", new_date);
        system(date_command);
    }
}

static void display_enter_lock_time_cb(lv_obj_t *obj, lv_event_t event)
{
    keyboard_cb(obj, event);

    if (LV_EVENT_VALUE_CHANGED == event) {
        const char *new_value = lv_textarea_get_text(obj);
        cvi_set_device_config_int(CONF_KEY_DISPLAY_ENTER_LOCK_TIME, atoi(new_value));
    }
}

static void display_enter_close_time_cb(lv_obj_t *obj, lv_event_t event)
{
    keyboard_cb(obj, event);

    if (LV_EVENT_VALUE_CHANGED == event) {
        const char *new_value = lv_textarea_get_text(obj);
        cvi_set_device_config_int(CONF_KEY_DISPLAY_ENTER_CLOSE_TIME, atoi(new_value));
    }
}

#if 0
static void wifi_ssid_cb(lv_obj_t *obj, lv_event_t event)
{
    keyboard_cb(obj, event);

    if (LV_EVENT_VALUE_CHANGED == event) {
        wifi_ssid = lv_textarea_get_text(obj);
    }
}

static void wifi_password_cb(lv_obj_t *obj, lv_event_t event)
{
    keyboard_cb(obj, event);

    if (LV_EVENT_VALUE_CHANGED == event) {
        wifi_password = lv_textarea_get_text(obj);
    }
}

static int connect_wifi(const char *ssid, const char *password)
{
    int network_id;
    char command[1024];
    const int sleep_time = 1;

    system("ifconfig wlan0 up");
    sleep(sleep_time);
    system("echo \"ctrl_interface=/var/run/wpa_supplicant\" >/tmp/wpa_supplicant.conf");
    system("wpa_supplicant -B -iwlan0 -Dnl80211 -c /tmp/wpa_supplicant.conf");
    sleep(sleep_time);

    FILE *fp = popen("wpa_cli add_network", "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }
    char line[64];
    fgets(line, sizeof(line), fp);
    fscanf(fp, "%d", &network_id);
    pclose(fp);

    snprintf(command, sizeof(command), "wpa_cli set_network %d ssid \'\"%s\"\'", network_id, ssid);
    system(command);
    snprintf(command, sizeof(command), "wpa_cli set_network %d psk \'\"%s\"\'", network_id, password);
    system(command);
    snprintf(command, sizeof(command), "wpa_cli select_network %d", network_id);
    system(command);
    system("udhcpc -b -i wlan0 -R &");
}

static void connect_wifi_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
       connect_wifi(wifi_ssid, wifi_password);
    }
}

static void password_cb(lv_obj_t *obj, lv_event_t event)
{
    keyboard_cb(obj, event);

    if (LV_EVENT_VALUE_CHANGED == event) {
        const char *new_password = lv_textarea_get_text(obj);
        cvi_set_device_config_str(CONF_KEY_DEVICE_PASSWORD, (char *)new_password);
    }
}
#endif