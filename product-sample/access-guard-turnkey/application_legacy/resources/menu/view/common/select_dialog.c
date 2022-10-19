#include "select_dialog.h"
#include "lvgl/lvgl.h"
#include "resource.h"
#include "lv_i18n.h"
#include "app_utils.h"
#include "menu.h"
#include "cvi_hal_face_api.h"
#include "menu_common.h"
#include "net.h"
#include "public.h"
#include "cviconfigsystem.h"

static lv_obj_t *select_dialog = NULL;

static void out_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
		device_cfg_system.signType = 1;
		cvi_set_device_config_system_int(CONF_KEY_SIGN_TYPE, device_cfg_system.signType);
        menu_tree_get_top().show();
        lv_obj_del(select_dialog);
    }
}

static void in_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
		device_cfg_system.signType = 2;
		cvi_set_device_config_system_int(CONF_KEY_SIGN_TYPE, device_cfg_system.signType);
        menu_tree_get_top().show();
        lv_obj_del(select_dialog);
    }
}

static void inout_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
		device_cfg_system.signType = 3;
		cvi_set_device_config_system_int(CONF_KEY_SIGN_TYPE, device_cfg_system.signType);
				
        menu_tree_get_top().show();
        lv_obj_del(select_dialog);
    }
}

static void cancel_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
		
        menu_tree_get_top().show();
        lv_obj_del(select_dialog);
    }
}

static lv_style_t modal_style;
static lv_style_t dialog_style;
static lv_style_t label_style;
static lv_style_t item_style;
static void _style_init()
{
    static bool style_init = false;

    if (!style_init)
    {
        lv_style_init(&modal_style);
        lv_style_set_bg_color(&modal_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        //lv_style_set_bg_opa(&modal_style, LV_STATE_DEFAULT, LV_OPA_30);

        lv_style_init(&item_style);
        lv_style_set_bg_opa(&item_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
		//lv_style_set_bg_color(&item_style, LV_STATE_DEFAULT, COLOR_BLUE);

        lv_style_init(&dialog_style);
        //lv_style_set_bg_opa(&dialog_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        //lv_style_set_image_opa(&dialog_style, LV_STATE_DEFAULT, LV_OPA_100);
		lv_style_set_bg_color(&dialog_style, LV_STATE_DEFAULT, COLOR_TOP_PANEL);
        lv_style_set_pad_inner(&dialog_style, LV_STATE_DEFAULT, 0);
        lv_style_set_pad_right(&dialog_style, LV_STATE_DEFAULT, 0);
        lv_style_set_pad_left(&dialog_style, LV_STATE_DEFAULT, 0);
        lv_style_set_pad_top(&dialog_style, LV_STATE_DEFAULT, 0);
        lv_style_set_pad_bottom(&dialog_style, LV_STATE_DEFAULT, 0);

        lv_style_init(&label_style);
        lv_style_set_text_font(&label_style, LV_STATE_DEFAULT, &stsong_36);

        style_init = true;
    }
}

lv_obj_t * create_select_dialog(lv_obj_t *par, char *target_ssid)
{
	int btn_height = 150;
    if (par == NULL) {
        par = lv_layer_top();
    }
	
    _style_init();

    select_dialog = lv_obj_create(lv_layer_top(), NULL);
    lv_obj_add_style(select_dialog, LV_OBJ_PART_MAIN, &modal_style);
    lv_obj_set_pos(select_dialog, 0, LV_VER_RES-600-6);
    lv_obj_set_size(select_dialog, LV_HOR_RES, 606);

	lv_obj_t * out_panel = lv_cont_create(select_dialog, NULL);
    lv_obj_set_size(out_panel, LV_HOR_RES_MAX, btn_height);
    lv_obj_set_pos(out_panel, 0, 0);
	lv_obj_add_style(out_panel, LV_CONT_PART_MAIN, &dialog_style);
	lv_obj_set_click(out_panel, true);
	lv_obj_set_event_cb(out_panel, out_btn_event_cb);
    lv_obj_t * out_txt = lv_label_create(out_panel, NULL);
    lv_obj_add_style(out_txt, LV_LABEL_PART_MAIN, &label_style);
	lv_label_set_text(out_txt, _("Out"));
    lv_obj_align(out_txt, NULL, LV_ALIGN_CENTER, 0, 20);

	lv_obj_t * in_panel = lv_cont_create(select_dialog, NULL);
    lv_obj_set_size(in_panel, LV_HOR_RES_MAX, btn_height);
    lv_obj_set_pos(in_panel, 0, btn_height+2);
	lv_obj_add_style(in_panel, LV_CONT_PART_MAIN, &dialog_style);
	lv_obj_set_click(in_panel, true);
	lv_obj_set_event_cb(in_panel, in_btn_event_cb);
    lv_obj_t * in_txt = lv_label_create(in_panel, NULL);
    lv_obj_add_style(in_txt, LV_LABEL_PART_MAIN, &label_style);
	lv_label_set_text(in_txt, _("In"));
    lv_obj_align(in_txt, NULL, LV_ALIGN_CENTER, 0, 20);

	lv_obj_t * inout_panel = lv_cont_create(select_dialog, NULL);
    lv_obj_set_size(inout_panel, LV_HOR_RES_MAX, 150);
    lv_obj_set_pos(inout_panel, 0, 2*(btn_height+2));
	lv_obj_add_style(inout_panel, LV_CONT_PART_MAIN, &dialog_style);
	lv_obj_set_click(inout_panel, true);
	lv_obj_set_event_cb(inout_panel, inout_btn_event_cb);
    lv_obj_t * inout_txt = lv_label_create(inout_panel, NULL);
    lv_obj_add_style(inout_txt, LV_LABEL_PART_MAIN, &label_style);
	lv_label_set_text(inout_txt, _("In and Out"));
    lv_obj_align(inout_txt, NULL, LV_ALIGN_CENTER, 0, 20);

	lv_obj_t * cancel_panel = lv_cont_create(select_dialog, NULL);
    lv_obj_set_size(cancel_panel, LV_HOR_RES_MAX, 150);
    lv_obj_set_pos(cancel_panel, 0, 3*(btn_height+2));
	lv_obj_add_style(cancel_panel, LV_CONT_PART_MAIN, &dialog_style);
	lv_obj_set_click(cancel_panel, true);
	lv_obj_set_event_cb(cancel_panel, cancel_btn_event_cb);
    lv_obj_t * cancel_txt = lv_label_create(cancel_panel, NULL);
    lv_obj_add_style(cancel_txt, LV_LABEL_PART_MAIN, &label_style);
	lv_label_set_text(cancel_txt, _("Cancel"));
    lv_obj_align(cancel_txt, NULL, LV_ALIGN_CENTER, 0, 20);
	
    return select_dialog;
}
