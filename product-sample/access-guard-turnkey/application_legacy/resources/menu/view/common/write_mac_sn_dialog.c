#include "write_mac_sn_dialog.h"
#include "write_mac_sn.h"
#include "lvgl/lvgl.h"
#include "resource.h"
#include "lv_i18n.h"
#include "app_utils.h"
#include "menu.h"
#include "cvi_hal_face_api.h"
#include "menu_common.h"
#include "net.h"
#include "public.h"
#include "keyboard.h"
#include "cviconfigsystem.h"


//extern hal_facelib_handle_t facelib_handle;

static lv_obj_t *write_mac_sn_dialog = NULL;

static char snBuf[64] = {0};
static char macBuf[64] = {0};


static void confirm_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	//char bufTemp[64] = {0};
    if (event == LV_EVENT_CLICKED) {
		if(strlen(snBuf) > 0)
		{
			set_sn(snBuf, strlen(snBuf));
			setsnbeenuse();
		}
		//printf("read sn:%s", snBuf);
		if(strlen(macBuf) > 0)
		{
			set_mac(macBuf);
			setmacbeenuse();
		}
		//printf("read mac:%s", macBuf);
		hide_write_mac_sn_popmenu();
    }
}

static void cancel_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        hide_write_mac_sn_popmenu();
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
        lv_style_set_bg_opa(&modal_style, LV_STATE_DEFAULT, LV_OPA_30);

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
        lv_style_set_text_font(&label_style, LV_STATE_DEFAULT, &stsong_28);

        style_init = true;
    }
}

lv_obj_t * createPopupMenuWMS(lv_obj_t *par)
{
	char bufTemp[64] = {0};
    if (par == NULL) {
        par = lv_layer_top();
    }

    _style_init();

    write_mac_sn_dialog = lv_obj_create(lv_layer_top(), NULL);
    lv_obj_add_style(write_mac_sn_dialog, LV_OBJ_PART_MAIN, &modal_style);
    lv_obj_set_pos(write_mac_sn_dialog, 0, 0);
    lv_obj_set_size(write_mac_sn_dialog, LV_HOR_RES, LV_VER_RES);

	lv_obj_t *disp_panel = lv_cont_create(write_mac_sn_dialog, NULL);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX-100, LV_VER_RES_MAX/3);
    lv_obj_set_pos(disp_panel, 50, LV_VER_RES_MAX/3);
	lv_obj_add_style(disp_panel, LV_CONT_PART_MAIN, &dialog_style);

    lv_obj_t * title_obj = lv_label_create(disp_panel, NULL);
    lv_obj_add_style(title_obj, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_text(title_obj, _("write mac and sn"));
    lv_obj_align(title_obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);

  //  const int item_start_height = -100;
    //const int item_height = 30;
    //int current_item_height = item_start_height;
   // add_item(disp_panel, input_data.data, current_item_height, input_dialog_cb);

    lv_obj_t * mac = lv_label_create(disp_panel, NULL);
    lv_obj_add_style(mac, LV_LABEL_PART_MAIN, &label_style);
	sprintf(bufTemp,"%s: %s",_("now mac"),get_mac1());
    lv_label_set_text(mac, bufTemp);
    lv_obj_align(mac, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 60);

	lv_obj_t * sn = lv_label_create(disp_panel, NULL);
    lv_obj_add_style(sn, LV_LABEL_PART_MAIN, &label_style);
	sprintf(bufTemp,"%s: %s",_("now sn"),get_sn());
    lv_label_set_text(sn, bufTemp);
    lv_obj_align(sn, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 100);

	lv_obj_t * mac_write = lv_label_create(disp_panel, NULL);
    lv_obj_add_style(mac_write, LV_LABEL_PART_MAIN, &label_style);
	memset(macBuf, 0x00, sizeof(macBuf));
	getonelinemac(macBuf, sizeof(macBuf));
	printf("macBuf:%s\n",macBuf);
	sprintf(bufTemp,"%s: %s",_("write mac"),macBuf);
	printf("macBuf:%s\n",bufTemp);
    lv_label_set_text(mac_write, bufTemp);
    lv_obj_align(mac_write, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 160);

	lv_obj_t * sn_write = lv_label_create(disp_panel, NULL);
    lv_obj_add_style(sn_write, LV_LABEL_PART_MAIN, &label_style);
	memset(snBuf, 0x00, sizeof(snBuf));
	getonelinesn(snBuf, sizeof(snBuf));
	sprintf(bufTemp,"%s: %s",_("write sn"),snBuf);
    lv_label_set_text(sn_write, bufTemp);
    lv_obj_align(sn_write, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 200);

    lv_obj_t * btn_confirm = lv_imgbtn_create(disp_panel, NULL);
    lv_imgbtn_set_src(btn_confirm, LV_BTN_STATE_PRESSED, RES_IMG_BTN_CONFIRM);
    lv_imgbtn_set_src(btn_confirm, LV_BTN_STATE_RELEASED, RES_IMG_BTN_CONFIRM);
    lv_obj_align(btn_confirm, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 100);
    lv_obj_set_event_cb(btn_confirm, confirm_btn_event_cb);
    lv_obj_t * label_confirm = lv_label_create(btn_confirm, NULL);
    lv_obj_add_style(label_confirm, LV_STATE_DEFAULT, &label_style);
    lv_label_set_text(label_confirm, _("Confirm"));

    lv_obj_t * btn_cancel = lv_imgbtn_create(disp_panel, NULL);
    lv_imgbtn_set_src(btn_cancel, LV_BTN_STATE_PRESSED, RES_IMG_BTN_CONFIRM);
    lv_imgbtn_set_src(btn_cancel, LV_BTN_STATE_RELEASED, RES_IMG_BTN_CONFIRM);
    lv_obj_align(btn_cancel, NULL, LV_ALIGN_IN_LEFT_MID, 100, 100);
    lv_obj_set_event_cb(btn_cancel, cancel_btn_event_cb);
    lv_obj_t * label_cancel = lv_label_create(btn_cancel, NULL);
    lv_obj_add_style(label_cancel, LV_STATE_DEFAULT, &label_style);
    lv_label_set_text(label_cancel, _("Cancel"));
    lv_obj_align(label_cancel, NULL, LV_ALIGN_CENTER, -10, -10);

    return write_mac_sn_dialog;
}
