#include "face_edit_dialog.h"
#include "lvgl/lvgl.h"
#include "resource.h"
#include "lv_i18n.h"
#include "app_utils.h"
#include "menu.h"
#include "cvi_hal_face_api.h"
#include "keyboard.h"

extern hal_facelib_handle_t facelib_handle;

static lv_obj_t *face_edit_dialog = NULL;
static cvi_person_t *person = NULL;
static lv_obj_t *kb = NULL;
const int ITEM_VALUE_WIDTH = 150;

static void confirm_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        CviSetPerson(facelib_handle, person);
        menu_tree_get_top().show();
        lv_obj_del(face_edit_dialog);
        kb = NULL;
    }
}

static void cancel_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        lv_obj_del(face_edit_dialog);
        kb = NULL;
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

static void keyboard_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        if (kb == NULL) {
            kb = create_keyboard(&kb, obj, face_edit_dialog, 0, 0);
        }
    }
}

static void add_item(lv_obj_t *par, const char *title, char *value, int y, lv_event_cb_t cb)
{
    lv_obj_t * container = lv_cont_create(par, NULL);
    lv_obj_align(container, NULL, LV_ALIGN_IN_TOP_MID, 0, y);
    lv_obj_add_style(container, LV_CONT_PART_MAIN, &item_style);
    lv_cont_set_fit2(container, LV_FIT_PARENT, LV_FIT_TIGHT);
    lv_obj_t * title_labal = lv_label_create(container, NULL);
    lv_obj_align(title_labal, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0);
    lv_label_set_text(title_labal, title);
    lv_obj_t * value_textarea = lv_textarea_create(container, NULL);
    lv_textarea_set_cursor_hidden(value_textarea, true);
    lv_obj_align(value_textarea, NULL, LV_ALIGN_IN_RIGHT_MID, 50, 0);
    lv_textarea_set_max_length(value_textarea, 40);
    lv_textarea_set_one_line(value_textarea, true);
    lv_obj_set_width(value_textarea, ITEM_VALUE_WIDTH);
    lv_textarea_set_text(value_textarea, value);
    if (cb) {
        lv_obj_set_event_cb(value_textarea, cb);
    }
}

static void name_cb(lv_obj_t *obj, lv_event_t event)
{
    keyboard_cb(obj, event);

    if (LV_EVENT_VALUE_CHANGED == event) {
        const char *value = lv_textarea_get_text(obj);
        snprintf(person->name, sizeof(person->name), "%s", value);
    }
}

static void serial_cb(lv_obj_t *obj, lv_event_t event)
{
    keyboard_cb(obj, event);

    if (LV_EVENT_VALUE_CHANGED == event) {
        const char *value = lv_textarea_get_text(obj);
        snprintf(person->serial, sizeof(person->serial), "%s", value);
    }
}

static void type_cb(lv_obj_t *obj, lv_event_t event)
{
    keyboard_cb(obj, event);

    if (LV_EVENT_VALUE_CHANGED == event) {
        const char *value = lv_textarea_get_text(obj);
        snprintf(person->type, sizeof(person->type), "%s", value);
    }
}

static void ic_card_cb(lv_obj_t *obj, lv_event_t event)
{
    keyboard_cb(obj, event);

    if (LV_EVENT_VALUE_CHANGED == event) {
        const char *value = lv_textarea_get_text(obj);
        snprintf(person->ic_card, sizeof(person->ic_card), "%s", value);
    }
}

lv_obj_t * create_face_edit_dialog(lv_obj_t *par, cvi_person_t *target_person)
{
    if (par == NULL) {
        par = lv_layer_top();
    }

    person = target_person;

    _style_init();

    face_edit_dialog = lv_obj_create(lv_layer_top(), NULL);
    lv_obj_add_style(face_edit_dialog, LV_OBJ_PART_MAIN, &modal_style);
    lv_obj_set_pos(face_edit_dialog, 0, 0);
    lv_obj_set_size(face_edit_dialog, LV_HOR_RES, LV_VER_RES);

    lv_obj_t * dialog_img = lv_img_create(face_edit_dialog, NULL);
    lv_img_set_src(dialog_img, RES_IMG_DIALOG);
    lv_img_set_auto_size(dialog_img, true);
    lv_obj_align(dialog_img, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(dialog_img, LV_IMG_PART_MAIN, &dialog_style);

    lv_obj_t * title_obj = lv_label_create(dialog_img, NULL);
    lv_obj_add_style(title_obj, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_text(title_obj, _("Edit Face Info"));
    lv_obj_align(title_obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);

    const int item_start_height = 50;
    const int item_height = 30;
    int current_item_height = item_start_height;
    add_item(dialog_img,  _("Name"), person->name, current_item_height, name_cb);
    current_item_height+=item_height;
    add_item(dialog_img,  _("Serial"), person->serial, current_item_height, serial_cb);
    current_item_height+=item_height;
    add_item(dialog_img,  _("type"), person->type, current_item_height, type_cb);
    current_item_height+=item_height;
    add_item(dialog_img,  _("ic_card"), person->ic_card, current_item_height, ic_card_cb);
    current_item_height+=item_height;

    lv_obj_t * btn_confirm = lv_imgbtn_create(dialog_img, NULL);
    lv_imgbtn_set_src(btn_confirm, LV_BTN_STATE_PRESSED, RES_IMG_BTN_CONFIRM);
    lv_imgbtn_set_src(btn_confirm, LV_BTN_STATE_RELEASED, RES_IMG_BTN_CONFIRM);
    lv_obj_align(btn_confirm, NULL, LV_ALIGN_IN_TOP_MID, 0, 180);
    lv_obj_set_event_cb(btn_confirm, confirm_btn_event_cb);
    lv_obj_t * label_confirm = lv_label_create(btn_confirm, NULL);
    lv_obj_add_style(label_confirm, LV_STATE_DEFAULT, &label_style);
    lv_label_set_text(label_confirm, _("Confirm"));

    lv_obj_t * btn_cancel = lv_imgbtn_create(dialog_img, NULL);
    lv_imgbtn_set_src(btn_cancel, LV_BTN_STATE_PRESSED, RES_IMG_BTN_CONFIRM);
    lv_imgbtn_set_src(btn_cancel, LV_BTN_STATE_RELEASED, RES_IMG_BTN_CONFIRM);
    lv_obj_align(btn_cancel, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    lv_obj_set_event_cb(btn_cancel, cancel_btn_event_cb);
    lv_obj_t * label_cancel = lv_label_create(btn_cancel, NULL);
    lv_obj_add_style(label_cancel, LV_STATE_DEFAULT, &label_style);
    lv_label_set_text(label_cancel, _("Cancel"));
    lv_obj_align(label_cancel, NULL, LV_ALIGN_CENTER, -10, -10);

    return face_edit_dialog;
}