#include "face_add_dialog.h"
#include "lvgl/lvgl.h"
#include "resource.h"
#include "lv_i18n.h"
#include "face_capture_page.h"
#include "app_utils.h"

static void gallery_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{

}

static void capture_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_RELEASED) {
        
        if (menu_tree_isEmpty())
            return;
        menu_tree_get_top().hide();
        menu_tree_push(getFaceCapturePage());
        menu_tree_get_top().onCreate();
        menu_tree_get_top().show();

        lv_obj_user_data_t con = lv_obj_get_user_data(obj);
        lv_obj_del((lv_obj_t *)con);
        printf("click capture button\n");

    }
}

static void batch_reg_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        APP_CompSendCmd(CMD_APP_COMP_VIDEOSTOP, NULL, 0, NULL, 0);
        usleep(2000);
        APP_CompSendCmd(CMD_APP_COMP_FACE_REGISTER_MULTI, NULL, 0, NULL, 0);
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

lv_obj_t * createFaceAddWindow(lv_obj_t *par)
{
    if (par == NULL) {
        par = lv_layer_top();
    }

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
    lv_label_set_text(title_obj, _("Add Face Info"));
    lv_obj_align(title_obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);

    lv_obj_t *gallery_btn = lv_imgbtn_create(dialog_img, NULL);
    lv_imgbtn_set_src(gallery_btn, LV_BTN_STATE_PRESSED, RES_IMG_BTN_CONFIRM);
    lv_imgbtn_set_src(gallery_btn, LV_BTN_STATE_RELEASED, RES_IMG_BTN_CONFIRM);
    lv_obj_align(gallery_btn, NULL, LV_ALIGN_IN_TOP_MID, 0, 60);
    lv_obj_t *gallery_title = lv_label_create(gallery_btn, NULL);
    lv_obj_add_style(gallery_title, LV_STATE_DEFAULT, &label_style);
    lv_label_set_text(gallery_title, _("Gallery"));
    lv_obj_align(gallery_title, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(gallery_btn, gallery_btn_event_cb);

    lv_obj_t *capture_btn = lv_imgbtn_create(dialog_img, NULL);
    lv_imgbtn_set_src(capture_btn, LV_BTN_STATE_PRESSED, RES_IMG_BTN_CONFIRM);
    lv_imgbtn_set_src(capture_btn, LV_BTN_STATE_RELEASED, RES_IMG_BTN_CONFIRM);
    lv_obj_align(capture_btn, NULL, LV_ALIGN_IN_TOP_MID, 0, 120);
    lv_obj_t *capture_title = lv_label_create(capture_btn, NULL);
    lv_obj_add_style(capture_title, LV_STATE_DEFAULT, &label_style);
    lv_label_set_text(capture_title, _("Capture"));
    lv_obj_align(capture_title, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_user_data(capture_btn, obj);
    lv_obj_set_event_cb(capture_btn, capture_btn_event_cb);

    lv_obj_t *batch_register_btn = lv_imgbtn_create(dialog_img, NULL);
    lv_imgbtn_set_src(batch_register_btn, LV_BTN_STATE_PRESSED, RES_IMG_BTN_CONFIRM);
    lv_imgbtn_set_src(batch_register_btn, LV_BTN_STATE_RELEASED, RES_IMG_BTN_CONFIRM);
    lv_obj_align(batch_register_btn, NULL, LV_ALIGN_IN_TOP_MID, 0, 180);
    lv_obj_t *batch_register_title = lv_label_create(batch_register_btn, NULL);
    lv_obj_add_style(batch_register_title, LV_STATE_DEFAULT, &label_style);
    lv_label_set_text(batch_register_title, _("Batch Register"));
    lv_obj_align(batch_register_title, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(batch_register_btn, batch_reg_btn_event_cb);

    lv_obj_t * btn_cancel = lv_imgbtn_create(dialog_img, NULL);
    lv_imgbtn_set_src(btn_cancel, LV_BTN_STATE_PRESSED, RES_IMG_BTN_CONFIRM);
    lv_imgbtn_set_src(btn_cancel, LV_BTN_STATE_RELEASED, RES_IMG_BTN_CONFIRM);
    lv_obj_align(btn_cancel, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    lv_obj_set_user_data(btn_cancel, obj);
    lv_obj_set_event_cb(btn_cancel, cancel_btn_event_cb);
    lv_obj_t * label_cancel = lv_label_create(btn_cancel, NULL);
    lv_obj_add_style(label_cancel, LV_STATE_DEFAULT, &label_style);
    lv_label_set_text(label_cancel, _("Cancel"));
    lv_obj_align(label_cancel, NULL, LV_ALIGN_CENTER, -10, -10);

    APP_CompSendCmd(CMD_APP_COMP_VIDEORESIZE, NULL, 0, NULL, 0);

    return obj;
}