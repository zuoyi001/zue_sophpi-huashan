#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "menu.h"
#include "menu_common.h"
#include "menu_page.h"
#include "resource.h"
#include "lv_i18n.h"
#include "config_data.h"
#include "keyboard.h"


/*门禁控制页面*/

static lv_obj_t *menu_access_control_page_obj;
static lv_obj_t *kb = NULL;


/*控制选项*/
static lv_obj_t * auth_method_list;
static lv_obj_t * elec_ctrl_list;
static lv_obj_t * weigand_list;
//static lv_obj_t * com_select;
//static lv_obj_t * com_enable;
//static lv_obj_t * baudrate;
//static lv_obj_t * bits_item;
//static lv_obj_t * checkmode_item;
//static lv_obj_t * stop_bit_item;
//static lv_obj_t * protocol_item;
static lv_obj_t * signal_item;
static lv_obj_t * open_delay_item;
static lv_obj_t * open_close_item;
static lv_obj_t * alarmer_item;
static lv_obj_t * door_btn_item;
static lv_obj_t * open_period_item;
static lv_obj_t * fr_gap_item;

static void menu_access_control_create();
static void menu_access_control_destroy();
static void menu_access_control_show();
static void menu_access_control_hide();

static void create_auth_sec(lv_obj_t *disp_panel);
static void create_interface_sec(lv_obj_t *disp_panel);
//static void create_uart_sec(lv_obj_t *disp_panel);
static void create_other_sec(lv_obj_t *disp_panel);

static menu_page_t menu_access_control_page = {
    .onCreate = menu_access_control_create,
    .onDestroy = menu_access_control_destroy,
    .show = menu_access_control_show,
    .hide = menu_access_control_hide,
    .back = menuPageBack
};

menu_page_t get_menu_access_control_page()
{
    return menu_access_control_page;
}


/*事件回调处理函数*/
static void back_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	if (LV_EVENT_RELEASED == event)
    {
        printf("back_btn_event_cb.\n");
        menuPageBack();
    }

}

static void sample_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_VALUE_CHANGED == event)
    {
        #ifdef USE_CONFIG_DATA
        int idx = 0;
        if (auth_method_list == obj) {
            idx = lv_dropdown_get_selected(auth_method_list);
			printf("idx:%d \n",idx);
            cvi_set_device_config_int(CONFIG_AUTH_METHOD, idx);
        }
        else if (elec_ctrl_list == obj) {
            idx = lv_dropdown_get_selected(elec_ctrl_list);
            cvi_set_device_config_int(CONFIG_RELAY_CTRL, idx);
        }
        else if (weigand_list == obj) {
            idx = lv_dropdown_get_selected(weigand_list);
            cvi_set_device_config_int(CONFIG_WIEGAND_CTRL, idx);
        }
		/*
        else if (com_select == obj) {
            idx = lv_dropdown_get_selected(com_select);
            cvi_set_device_config_int(CONFIG_COM_INDEX, idx);
        }
        else if (com_enable == obj) {
            idx = lv_dropdown_get_selected(com_enable);
            cvi_set_device_config_int(CONFIG_COM_ENABLE, idx);
        }
        else if (baudrate == obj) {
            idx = lv_dropdown_get_selected(baudrate);
            cvi_set_device_config_int(CONFIG_BAUDRATE, idx);
        }
        else if (bits_item == obj) {
            idx = lv_dropdown_get_selected(bits_item);
            cvi_set_device_config_int(CONFIG_DATA_BITS_NUM, idx);
        }
        else if (checkmode_item == obj) {
            idx = lv_dropdown_get_selected(checkmode_item);
            cvi_set_device_config_int(CONFIG_CHECK_MODE, idx);
        }
        else if (stop_bit_item == obj) {
            idx = lv_dropdown_get_selected(stop_bit_item);
            cvi_set_device_config_int(CONFIG_STOP_BIT, idx);
        }
        else if (protocol_item == obj) {
            idx = lv_dropdown_get_selected(protocol_item);
            cvi_set_device_config_int(CONFIG_PROTOCOL, idx);
        }
		*/
        else if (signal_item == obj) {
            idx = lv_dropdown_get_selected(signal_item);
            cvi_set_device_config_int(CONFIG_MAG_DETECT, idx);
        }
        else if (open_delay_item == obj) {
            const char *txt = lv_textarea_get_text(open_delay_item);
            cvi_set_device_config_int(CONFIG_OPEN_DELAY, atoi(txt));
        }
        else if (open_close_item == obj) {
            idx = lv_dropdown_get_selected(open_close_item);
            cvi_set_device_config_int(CONFIG_GATE_ACTION, idx);
        }
        else if (alarmer_item == obj) {
            idx = lv_dropdown_get_selected(alarmer_item);
            cvi_set_device_config_int(CONFIG_ALARMER, idx);
        }
        else if (door_btn_item == obj) {
            idx = lv_dropdown_get_selected(door_btn_item);
            cvi_set_device_config_int(CONFIG_GATE_BTN_CTRL, idx);
        }
        else if (open_period_item == obj) {
            const char *txt = lv_textarea_get_text(open_period_item);
            cvi_set_device_config_int(CONFIG_GATE_OPEN_PERIOD, atoi(txt));
        }
        else if (fr_gap_item == obj) {
            const char *txt = lv_textarea_get_text(fr_gap_item);
            cvi_set_device_config_int(CONFIG_FR_PERIOD, atoi(txt));
        }
        #endif
    }
}
static void hide_keyboard()
{
    if (kb != NULL) {
        lv_obj_del(kb);
        kb = NULL;
    }
}

static void show_keyboard(lv_obj_t *obj, lv_event_t event, lv_obj_t * parent)
{
    if (event == LV_EVENT_CLICKED) {
        if (kb == NULL) {
            kb = create_keyboard(&kb, obj, parent, 0, 0);
			lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUM);
        }
    }
}
static void other_item_event_cb(lv_obj_t * obj, lv_event_t event)
{
    show_keyboard(obj, event, menu_access_control_page_obj);

    if (LV_EVENT_VALUE_CHANGED == event)
    {
        #ifdef USE_CONFIG_DATA
        if (open_period_item == obj) {
            const char *txt = lv_textarea_get_text(open_period_item);
            cvi_set_device_config_int(CONFIG_GATE_OPEN_PERIOD, atoi(txt));
        }
        else if (fr_gap_item == obj) {
            const char *txt = lv_textarea_get_text(fr_gap_item);
            cvi_set_device_config_int(CONFIG_FR_PERIOD, atoi(txt));
        }
        #endif
    }
}

static void open_delay_event_cb(lv_obj_t * obj, lv_event_t event)
{
    show_keyboard(obj, event, menu_access_control_page_obj);

    if (LV_EVENT_VALUE_CHANGED == event)
    {
        #ifdef USE_CONFIG_DATA
        if (open_delay_item == obj) {
            const char *txt = lv_textarea_get_text(open_delay_item);
            cvi_set_device_config_str(CONFIG_OPEN_DELAY, txt);
        }
        #endif
    }
}


/*内部样式*/
static bool styleInited = false;
static void _style_init()
{
    if (!styleInited)
    {
        styleInited = true;
    }
}

/*刷新页面数据*/
static void _load_page_data()
{
    #ifdef USE_CONFIG_DATA
    int value = 0;

	cvi_get_device_config_int(CONFIG_AUTH_METHOD, &value);
	lv_dropdown_set_selected(auth_method_list, value);

    cvi_get_device_config_int(CONFIG_RELAY_CTRL, &value);
    lv_dropdown_set_selected(elec_ctrl_list, value);

    cvi_get_device_config_int(CONFIG_WIEGAND_CTRL, &value);
    lv_dropdown_set_selected(weigand_list, value);
	/*
    cvi_get_device_config_int(CONFIG_COM_INDEX, &value);
    lv_dropdown_set_selected(com_select, value);
    cvi_get_device_config_int(CONFIG_COM_ENABLE, &value);
    lv_dropdown_set_selected(baudrate, value);
    cvi_get_device_config_int(CONFIG_DATA_BITS_NUM, &value);
    lv_dropdown_set_selected(bits_item, value);
    cvi_get_device_config_int(CONFIG_CHECK_MODE, &value);
    lv_dropdown_set_selected(checkmode_item, value);
    cvi_get_device_config_int(CONFIG_STOP_BIT, &value);
    lv_dropdown_set_selected(stop_bit_item, value);
    cvi_get_device_config_int(CONFIG_PROTOCOL, &value);
    lv_dropdown_set_selected(protocol_item, value);
	*/
    cvi_get_device_config_int(CONFIG_MAG_DETECT, &value);
    lv_dropdown_set_selected(signal_item, value);
    
	char open_delay_str[8] = {0};
    cvi_get_device_config_str(CONFIG_OPEN_DELAY, open_delay_str,sizeof(open_delay_str));
    lv_textarea_set_text(open_delay_item, open_delay_str);

    cvi_get_device_config_int(CONFIG_GATE_ACTION, &value);
    lv_dropdown_set_selected(open_close_item, value);
    cvi_get_device_config_int(CONFIG_ALARMER, &value);
    lv_dropdown_set_selected(alarmer_item, value);
    cvi_get_device_config_int(CONFIG_GATE_BTN_CTRL, &value);
    lv_dropdown_set_selected(door_btn_item, value);

    cvi_get_device_config_int(CONFIG_GATE_OPEN_PERIOD, &value);
    char open_period_str[8] = {0};
    snprintf(open_period_str, sizeof(open_period_str), "%d", value);
    lv_textarea_set_text(open_period_item, open_period_str);

    int period = 0;
    cvi_get_device_config_int(CONFIG_FR_PERIOD, &period);
    char period_str[8] = {0};
    snprintf(period_str, sizeof(period_str), "%d", period);
    lv_textarea_set_text(fr_gap_item, period_str);
    #endif
}

/*内部layout*/
static void create_top_panel(lv_obj_t * par)
{
    lv_obj_t *top_panel = menu_page_create_top_panel(par, back_btn_event_cb);

    lv_obj_t *page_title = lv_label_create(top_panel, NULL);
    lv_label_set_text(page_title, _("Access Control"));
    lv_label_set_align(page_title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(page_title, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(page_title, true);
    lv_obj_add_style(page_title, LV_LABEL_PART_MAIN, menu_page_title_style());
}

static void create_disp_panel(lv_obj_t * par)
{
    lv_obj_t *disp_panel = menu_page_create_disp_scrollable_panel(par);

    create_auth_sec(disp_panel);
    create_interface_sec(disp_panel);
    //create_uart_sec(disp_panel);
    create_other_sec(disp_panel);

    _load_page_data();
}

static void create_auth_sec(lv_obj_t *disp_panel)
{
    lv_obj_t * auth_sec = menu_section_create(disp_panel, _("Authorization"));
    lv_obj_set_width(auth_sec, lv_obj_get_width(auth_sec) - SCROLL_BAR_WIDTH);
    auth_method_list = menu_section_add_dd_item(auth_sec, _("Auth Method"), "Standard FR\nFR + ID Card", sample_event_cb);
}

static void create_interface_sec(lv_obj_t *disp_panel)
{
    lv_obj_t * device_interface_sec = menu_section_create(disp_panel, _("Device Interface"));
    lv_obj_set_width(device_interface_sec, lv_obj_get_width(device_interface_sec) - SCROLL_BAR_WIDTH);
    elec_ctrl_list = menu_section_add_dd_item(device_interface_sec, _("Electronic Control"), "Open\nClose", sample_event_cb);
    weigand_list = menu_section_add_dd_item(device_interface_sec, _("Wiegand"), "26\n34", sample_event_cb);
}

#if 0
static void create_uart_sec(lv_obj_t *disp_panel)
{
    lv_obj_t * uart_sec = menu_section_create(disp_panel, _("Uart"));
    lv_obj_set_width(uart_sec, lv_obj_get_width(uart_sec) - SCROLL_BAR_WIDTH);

	com_select = menu_section_add_dd_item(uart_sec, _("Com"), "Com1\nCom2", sample_event_cb);
    com_enable = menu_section_add_dd_item(uart_sec, _("Com Enable"), "Enable\nDisable", sample_event_cb);
    baudrate = menu_section_add_dd_item(uart_sec, _("Baud Rate"), "9600\n115200\n", sample_event_cb);
    bits_item = menu_section_add_dd_item(uart_sec, _("Bits"), "4\n5\n6\n7\n8", sample_event_cb);
    checkmode_item = menu_section_add_dd_item(uart_sec, _("Check Mode"), "Odd\nEven\nNone", sample_event_cb);
    stop_bit_item = menu_section_add_dd_item(uart_sec, _("Stop bit"), "0\n1", sample_event_cb);
    protocol_item = menu_section_add_dd_item(uart_sec, _("Protocol"), "A\nB\nC", sample_event_cb);
}
#endif

static void create_other_sec(lv_obj_t *disp_panel)
{
    lv_obj_t * others_sec = menu_section_create(disp_panel, _("Others"));
    lv_obj_set_width(others_sec, lv_obj_get_width(others_sec) - SCROLL_BAR_WIDTH);
    signal_item = menu_section_add_dd_item(others_sec, _("Signal"), "On\nOff", sample_event_cb);
    open_delay_item = menu_section_add_num_item(others_sec, _("Open Delay(ms)"), 100, 100, 10000, open_delay_event_cb);
    open_close_item = menu_section_add_dd_item(others_sec, _("Open/Close Setting"), "Always Open\nAlways Closed\nAuto", sample_event_cb);
    alarmer_item = menu_section_add_dd_item(others_sec, _("Alarmer"), "On\nOff", sample_event_cb);
    door_btn_item = menu_section_add_dd_item(others_sec, _("Door Button"), "On\nOff", sample_event_cb);
    open_period_item = menu_section_add_num_item(others_sec, _("Open Period(ms)"), 2, 2, 10, other_item_event_cb);
    fr_gap_item = menu_section_add_num_item(others_sec, _("FR Gap Time(ms)"), 30, 10, 1000, other_item_event_cb);
}

///页面生命周期函数 start
static void menu_access_control_create()
{
    _style_init();

    menu_access_control_page_obj = lv_cont_create(lv_scr_menu(), NULL);
    lv_obj_add_style(menu_access_control_page_obj, LV_CONT_PART_MAIN, menu_page_style());
    lv_obj_set_size(menu_access_control_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);

    create_top_panel(menu_access_control_page_obj);
    create_disp_panel(menu_access_control_page_obj);

    lv_obj_set_hidden(menu_access_control_page_obj, true);
}

static void menu_access_control_destroy()
{
	hide_keyboard();
    if (menu_access_control_page_obj)
    {
        lv_obj_del(menu_access_control_page_obj);
        menu_access_control_page_obj = NULL;
    }
}

static void menu_access_control_show()
{
    if (menu_access_control_page_obj)
    {
        lv_obj_set_hidden(menu_access_control_page_obj, false);
    }
}

static void menu_access_control_hide()
{
    if (menu_access_control_page_obj)
    {
        lv_obj_set_hidden(menu_access_control_page_obj, true);
    }
}

///页面生命周期函数 end
