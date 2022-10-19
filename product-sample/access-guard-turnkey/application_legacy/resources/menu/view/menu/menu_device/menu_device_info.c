#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "menu_device_info.h"
#include "cviconfigsystem.h"
#include "cvi_audio.h"
#include "lv_i18n.h"
#include "resource.h"
#include "menu_common.h"
#include "menu_page.h"
#include "cviconfig.h"
#include "keyboard.h"
#include "sys.h"
#include "cvi_hal_face_api.h"
#include "public.h"

static void dev_info_menu_page_create();
static void dev_info_menu_page_destroy();
static void dev_info_menu_page_show();
static void dev_info_menu_page_hide();

static lv_obj_t *dev_info_menu_obj = NULL;
static menu_page_t dev_info_menu_page = {
    .onCreate = dev_info_menu_page_create,
    .onDestroy = dev_info_menu_page_destroy,
    .show = dev_info_menu_page_show,
    .hide = dev_info_menu_page_hide,
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
    int available_storage;
    int occupancy_storage;
    int register_people_num;
} device_t;

static device_t device_data;
//static lv_obj_t *kb;

// const int SCROLL_BAR_WIDTH = 50;
#define MAX_USER_NUMBER  5000

extern hal_facelib_handle_t facelib_handle;

/*****************************
    PRIVATE STATIC FUNCTIONS
 *****************************/

static lv_obj_t * create_menu_container(lv_obj_t *parent);
static void load_data();
static void load_about_data();
static void load_storage_data();
static void create_top_panel(lv_obj_t *parent);
static void back_btn_event_cb(lv_obj_t * obj, lv_event_t event);
static void create_disp_panel(lv_obj_t *parent);
static void create_about_section(lv_obj_t *parent);
static void create_storage_section(lv_obj_t *parent);
//static void refresh();


menu_page_t get_dev_info_menu_page()
{
    return dev_info_menu_page;
}

static void dev_info_menu_page_create()
{
    load_data();

    lv_obj_t *parent = lv_scr_menu();
    lv_obj_t *menu_container = create_menu_container(parent);
    create_top_panel(menu_container);
    create_disp_panel(menu_container);
}

static void dev_info_menu_page_destroy()
{
    lv_obj_del(dev_info_menu_obj);
    dev_info_menu_obj = NULL;
}

static void dev_info_menu_page_show()
{
    if (dev_info_menu_obj == NULL) {
        return;
    }

    lv_obj_set_hidden(dev_info_menu_obj, false);
}

static void dev_info_menu_page_hide()
{
    if (dev_info_menu_obj == NULL) {
        return;
    }

    lv_obj_set_hidden(dev_info_menu_obj, true);
}

static lv_obj_t * create_menu_container(lv_obj_t *parent)
{
    dev_info_menu_obj = lv_cont_create(parent, NULL);
    lv_obj_add_style(dev_info_menu_obj, LV_CONT_PART_MAIN, menu_page_style());
    lv_obj_set_size(dev_info_menu_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_hidden(dev_info_menu_obj, true);

    return dev_info_menu_obj;
}

static void create_top_panel(lv_obj_t * parent)
{
    lv_obj_t *top_panel = menu_page_create_top_panel(parent, back_btn_event_cb);

    lv_obj_t *page_title = lv_label_create(top_panel, NULL);
    lv_label_set_text(page_title, _("Device Information"));
    lv_label_set_align(page_title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(page_title, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(page_title, true);
    lv_obj_add_style(page_title, LV_LABEL_PART_MAIN, menu_page_title_style());
}

static void create_disp_panel(lv_obj_t *parent)
{
    lv_obj_t *disp_panel = menu_page_create_disp_scrollable_panel(parent);

    create_about_section(disp_panel);
    create_storage_section(disp_panel);
}

static void load_data()
{
    load_about_data();
    load_storage_data();
}

static void load_about_data()
{
    get_system_boot_time(device_data.time_from_boot);
    //cvi_get_device_config_str(CONF_KEY_DEVICE_NAME, device_data.device_name,
    //    sizeof(device_data.device_name));
    snprintf(device_data.device_type, sizeof(device_data.device_type), "cv1835");
    cvi_get_device_config_str(CONF_KEY_VERSION_NAME, device_data.version_name,
        sizeof(device_data.version_name));
    char card[] = "eth0";
    get_mac(card, device_data.mac_address);

   cvi_get_device_config_system_str(CONF_KEY_DEVICE_NAME, device_cfg_system.deviceName,
        sizeof(device_cfg_system.deviceName));

	
   // char serial[64] = "SN";
	/*
    for (int i = 0, j = 2; i < sizeof(device_data.mac_address); i++) {
        if (device_data.mac_address[i] != ':') {
            serial[j++] = (('a' <= device_data.mac_address[i]) && ('z' >= device_data.mac_address[i])) ?
                toupper(device_data.mac_address[i]) : device_data.mac_address[i];
        }
    }
	*/
    snprintf(device_data.device_serial, sizeof(device_data.device_serial), get_sn());
    sprintf(device_data.release_time, "%s %s", __DATE__,__TIME__);
}
static void load_storage_data()
{
    int total_storage = get_partition_total_size("/mnt/data/");
    device_data.available_storage = get_partition_available_size("/mnt/data/");
    device_data.occupancy_storage = total_storage - device_data.available_storage;
    device_data.register_people_num = HAL_FACE_GetRepoNum(facelib_handle);
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
    menu_section_add_text_item(about_section, _("Boot time"), device_data.time_from_boot, NULL);
    menu_section_add_text_item(about_section, _("Version"), device_data.version_name, NULL);
    menu_section_add_text_item(about_section, _("Device name"), device_cfg_system.deviceName, NULL);
    menu_section_add_text_item(about_section, _("Device type"), device_data.device_type, NULL);
    menu_section_add_text_item(about_section, _("Device serial"), device_data.device_serial, NULL);
    menu_section_add_text_item(about_section, _("Release time"), device_data.release_time, NULL);
    menu_section_add_text_item(about_section, _("Mac address"), device_data.mac_address, NULL);
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
    sprintf(max_people_num, "%d", MAX_USER_NUMBER);
    snprintf(register_info, sizeof(register_info), "%s/%s", register_people_num, max_people_num);
    menu_section_add_text_item(storage_section, _("Registered people/Max"), register_info, NULL);
}
/*
static void refresh()
{
    if (menu_tree_isEmpty()) {
        return;
    }

    menu_tree_pop();
    dev_info_menu_page_destroy();
    menu_tree_push(get_dev_info_menu_page());
    dev_info_menu_page_create();
    dev_info_menu_page_show();
}

*/
