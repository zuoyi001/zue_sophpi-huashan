#include <stdio.h>
#include <stdint.h>

#include "menu.h"
#include "menu_common.h"
#include "menu_page.h"
#include "resource.h"
#include "lv_i18n.h"
#include "menu_notification.h"

/*个性化与通知反馈页面*/

static lv_obj_t *menu_notification_page_obj;
static lv_obj_t * voice_notice_mode;
static lv_obj_t * name_disp_mode;
static lv_obj_t * company_name_item;

static void menu_notification_create();
static void menu_notification_destroy();
static void menu_notification_show();
static void menu_notification_hide();

static menu_page_t menu_notification_page = {
    .onCreate = menu_notification_create,
    .onDestroy = menu_notification_destroy,
    .show = menu_notification_show,
    .hide = menu_notification_hide,
    .back = menuPageBack
};

menu_page_t get_menu_notification_page()
{
    return menu_notification_page;
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
        if (voice_notice_mode == obj)
        {
            lv_dropdown_get_selected(voice_notice_mode);
            //int voice_mode = lv_dropdown_get_selected(voice_notice_mode);
            //cvi_set_device_config_int(CONFIG_VOICE_MODE, voice_mode);
        }
        else if (name_disp_mode == obj)
        {
            lv_dropdown_get_selected(name_disp_mode);
            //int display_mode = lv_dropdown_get_selected(name_disp_mode);
            //cvi_set_device_config_int(CONFIG_DISPLAY_MODE, display_mode);
        }
        else if (company_name_item == obj)
        {
            const char * company_name = lv_textarea_get_text(company_name_item);
            cvi_set_device_config_str(CONFIG_COMPANY_NAME, company_name);
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
    {
        int voice_mode = 0;
        //cvi_get_device_config_int(CONFIG_VOICE_MODE, &voice_mode);
        if (voice_mode >= VOICE_NOTICE_MODE_NONE && voice_mode < VOICE_NOTICE_MODE_END)
            lv_dropdown_set_selected(voice_notice_mode, voice_mode);
        
        int display_mode = 0;
        //cvi_get_device_config_int(CONFIG_DISPLAY_MODE, &display_mode);
        if (display_mode >= NAME_DISPLAY_MODE_FULL && display_mode < NAME_DISPLAY_MODE_END)
            lv_dropdown_set_selected(name_disp_mode, display_mode);

        char company_name[256] = {0};
        cvi_get_device_config_str(CONFIG_COMPANY_NAME, company_name, sizeof(company_name));
        lv_textarea_set_text(company_name_item, company_name);
    }
    #endif
}

/*页面内部layout*/
static void create_top_panel(lv_obj_t * par)
{
    lv_obj_t *top_panel = menu_page_create_top_panel(par, back_btn_event_cb);

    lv_obj_t *page_title = lv_label_create(top_panel, NULL);
    lv_label_set_text(page_title, _("Profile & Notification"));
    lv_label_set_align(page_title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(page_title, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(page_title, true);
    lv_obj_add_style(page_title, LV_LABEL_PART_MAIN, menu_page_title_style());
}

static void create_disp_panel(lv_obj_t * par)
{
    lv_obj_t *disp_panel = menu_page_create_disp_scrollable_panel(par);

    lv_obj_t * profile_sec = menu_section_create(disp_panel, _("Profile"));
    lv_obj_set_width(profile_sec, lv_obj_get_width(profile_sec) - SCROLL_BAR_WIDTH);
    voice_notice_mode = menu_section_add_dd_item(profile_sec, _("Voice Notice"), "No Notice\nMusic Notice", sample_event_cb);
    name_disp_mode = menu_section_add_dd_item(profile_sec, _("Name Display"), "Full Name\nName Only", sample_event_cb);
    company_name_item = menu_section_add_text_item(profile_sec, _("Company Name"), " ", sample_event_cb);
}

/// 页面生命周期函数 start
static void menu_notification_create()
{printf("[%s,%d]\n",__FUNCTION__,__LINE__);
    _style_init();

    menu_notification_page_obj = lv_cont_create(lv_scr_menu(), NULL);
    lv_obj_add_style(menu_notification_page_obj, LV_CONT_PART_MAIN, menu_page_style());
    lv_obj_set_size(menu_notification_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);

    create_top_panel(menu_notification_page_obj);
    create_disp_panel(menu_notification_page_obj);

    _load_page_data();
    lv_obj_set_hidden(menu_notification_page_obj, true);
}

static void menu_notification_destroy()
{
    if (menu_notification_page_obj)
    {
        lv_obj_del(menu_notification_page_obj);
        menu_notification_page_obj = NULL;
    }
}

static void menu_notification_show()
{
    if (menu_notification_page_obj)
    {
        lv_obj_set_hidden(menu_notification_page_obj, false);
    }
}

static void menu_notification_hide()
{
    if (menu_notification_page_obj)
    {
        lv_obj_set_hidden(menu_notification_page_obj, true);
    }
}
/// 页面生命周期函数 end


