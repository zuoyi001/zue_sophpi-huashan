#include "face_delete_dialog.h"
#include "lvgl/lvgl.h"
#include "resource.h"
#include "lv_i18n.h"
#include "app_utils.h"
#include "menu.h"
#include "cvi_hal_face_api.h"

extern hal_facelib_handle_t facelib_handle;

static cvi_person_t *person = NULL;

static void confirm_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
		if (facelib_handle != NULL) {
			HAL_FACE_RepoRemoveIdentity(facelib_handle, person->id);
			CviLoadIdentify(facelib_handle);
		}
        menu_tree_get_top().show();
        lv_obj_user_data_t con = lv_obj_get_user_data(obj);
        lv_obj_del((lv_obj_t *)con);
    }
}

static void cancel_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        lv_obj_user_data_t con = lv_obj_get_user_data(obj);
        lv_obj_del((lv_obj_t *)con);
    }
}

static lv_style_t modal_style;
static lv_style_t dialog_style;
static lv_style_t label_style;
static void _style_init()
{
    static bool style_init = false;

    if (!style_init)
    {
        lv_style_init(&modal_style);
        lv_style_set_bg_color(&modal_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        lv_style_set_bg_opa(&modal_style, LV_STATE_DEFAULT, LV_OPA_30);

        lv_style_init(&dialog_style);
        lv_style_set_bg_opa(&dialog_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
        lv_style_set_image_opa(&dialog_style, LV_STATE_DEFAULT, LV_OPA_100);
        lv_style_set_pad_inner(&dialog_style, LV_STATE_DEFAULT, 0);
        lv_style_set_pad_right(&dialog_style, LV_STATE_DEFAULT, 0);
        lv_style_set_pad_left(&dialog_style, LV_STATE_DEFAULT, 0);
        lv_style_set_pad_top(&dialog_style, LV_STATE_DEFAULT, 0);
        lv_style_set_pad_bottom(&dialog_style, LV_STATE_DEFAULT, 0);

        lv_style_init(&label_style);
        lv_style_set_text_font(&label_style, LV_STATE_DEFAULT, &stsong_18);

        style_init = true;
    }
}

lv_obj_t * create_face_delete_dialog(lv_obj_t *par, cvi_person_t *target_person)
{
    if (par == NULL) {
        par = lv_layer_top();
    }

    person = target_person;

    _style_init();

    lv_obj_t * obj = lv_obj_create(lv_layer_top(), NULL);
    lv_obj_add_style(obj, LV_OBJ_PART_MAIN, &modal_style);
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, LV_HOR_RES, LV_VER_RES);

    lv_obj_t * dialog_img = lv_img_create(obj, NULL);
    lv_img_set_src(dialog_img, RES_IMG_DIALOG);
    lv_img_set_auto_size(dialog_img, true);
    lv_obj_align(dialog_img, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(dialog_img, LV_IMG_PART_MAIN, &dialog_style);

    lv_obj_t * title_obj = lv_label_create(dialog_img, NULL);
    lv_obj_add_style(title_obj, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_text(title_obj, _("Delete Face"));
    lv_obj_align(title_obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);

    lv_obj_t * btn_confirm = lv_imgbtn_create(dialog_img, NULL);
    lv_imgbtn_set_src(btn_confirm, LV_BTN_STATE_PRESSED, RES_IMG_BTN_CONFIRM);
    lv_imgbtn_set_src(btn_confirm, LV_BTN_STATE_RELEASED, RES_IMG_BTN_CONFIRM);
    lv_obj_align(btn_confirm, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_user_data(btn_confirm, obj);
    lv_obj_set_event_cb(btn_confirm, confirm_btn_event_cb);
    lv_obj_t * label_confirm = lv_label_create(btn_confirm, NULL);
    lv_obj_add_style(label_confirm, LV_STATE_DEFAULT, &label_style);
    lv_label_set_text(label_confirm, _("Confirm"));
    
    lv_obj_t * btn_cancel = lv_imgbtn_create(dialog_img, NULL);
    lv_imgbtn_set_src(btn_cancel, LV_BTN_STATE_PRESSED, RES_IMG_BTN_CONFIRM);
    lv_imgbtn_set_src(btn_cancel, LV_BTN_STATE_RELEASED, RES_IMG_BTN_CONFIRM);
    lv_obj_align(btn_cancel, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    lv_obj_set_user_data(btn_cancel, obj);
    lv_obj_set_event_cb(btn_cancel, cancel_btn_event_cb);
    lv_obj_t * label_cancel = lv_label_create(btn_cancel, NULL);
    lv_obj_add_style(label_cancel, LV_STATE_DEFAULT, &label_style);
    lv_label_set_text(label_cancel, _("Cancel"));
    
    return obj;
}