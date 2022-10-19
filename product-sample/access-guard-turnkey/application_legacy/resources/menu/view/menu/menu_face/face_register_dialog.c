#include "face_register_dialog.h"
#include "resource.h"
#include "keyboard.h"
#include "lv_i18n.h"
#include "app_utils.h"

static lv_obj_t *parent;
static lv_obj_t *kb_name = NULL;
static lv_obj_t *kb_id = NULL;

static lv_obj_t *ta_name = NULL;
static lv_obj_t *ta_id = NULL;

static void ta_name_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
        if (kb_id) {
            lv_textarea_set_cursor_hidden(ta_id, true);
            lv_obj_del(kb_id);
            kb_id = NULL;
        }

        if (kb_name == NULL) {
            kb_name = create_keyboard(&kb_name, obj, parent, 0, 0);
        }
    } /*else if (LV_EVENT_DEFOCUSED == event) {
        if (kb_name != NULL) {
            lv_obj_del(kb_name);
            kb_name = NULL;
        }
    }*/
}

static void ta_id_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
        if (kb_name) {
            lv_textarea_set_cursor_hidden(ta_name, true);
            lv_obj_del(kb_name);
            kb_name = NULL;
        }

        if (kb_id == NULL) {
            kb_id = create_keyboard(&kb_id, obj, parent, 0, 0);
        }
    } /*else if (LV_EVENT_DEFOCUSED == event) {
        if (kb_id != NULL) {
            lv_obj_del(kb_id);
            kb_id = NULL;
        }
    }*/
}
cvi_person_t person;
static void commit_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
        const char *person_name = lv_textarea_get_text(ta_name);
        int name_len = strlen(person_name);
        if (name_len == 0 || name_len > 20)
        {
            printf("please input valid name\n");
            return;
        }

        const char *person_id = lv_textarea_get_text(ta_id);
        int id_len = strlen(person_id);
        if (id_len == 0 || id_len > 20)
        {
            printf("please input valid id\n");
            return;
        }

        if (kb_name) {
            lv_obj_del(kb_name);
            kb_name = NULL;
        }

        if (kb_id) {
            lv_obj_del(kb_id);
            kb_id = NULL;
        }

        memset(&person, 0, sizeof(cvi_person_t));
        
        strcpy(person.name, person_name);
        strcpy(person.serial,person_id);

  
        lv_obj_user_data_t user_data = lv_obj_get_user_data(obj);
        lv_obj_del((lv_obj_t *)user_data);
        APP_CompSendCmd(CMD_APP_COMP_FACE_REGISTER, &person, sizeof(cvi_person_t), NULL, 0);

    }
}

lv_obj_t * createFaceRegisterDialog(lv_obj_t *par)
{
    if (par == NULL) {
        par = lv_layer_top();
    }
    parent = par;

    static lv_style_t modal_style;
    // lv_style_copy(&modal_style, &lv_style_plain_color);
    // modal_style.body.main_color = modal_style.body.grad_color = LV_COLOR_BLACK;
    // modal_style.body.opa = LV_OPA_30;
    lv_style_init(&modal_style);
    lv_style_set_bg_color(&modal_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_grad_color(&modal_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_opa(&modal_style, LV_STATE_DEFAULT, LV_OPA_30);

    lv_obj_t * obj = lv_obj_create(parent, NULL);
    lv_obj_add_style(obj, LV_OBJ_PART_MAIN, &modal_style);
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, LV_HOR_RES, LV_VER_RES);

    lv_obj_t * dialog_img = lv_img_create(obj, NULL);
    lv_img_set_src(dialog_img, RES_IMG_DIALOG);
    lv_img_set_auto_size(dialog_img, true);
    lv_obj_align(dialog_img, NULL, LV_ALIGN_CENTER, 0, 0);
    static lv_style_t dialog_style;
    // lv_style_copy(&dialog_style, lv_img_get_style(dialog_img, LV_IMG_PART_MAIN));
    lv_style_init(&dialog_style);
    // dialog_style.body.opa = LV_OPA_30;
    lv_style_set_bg_opa(&dialog_style, LV_STATE_DEFAULT, LV_OPA_30);
    lv_obj_add_style(dialog_img, LV_IMG_PART_MAIN, &dialog_style);

    lv_obj_t * title1 = lv_label_create(dialog_img, NULL);
    static lv_style_t title_style;
    // lv_style_copy(&title_style, lv_label_get_style(title1, LV_LABEL_PART_MAIN));
    lv_style_init(&title_style);
    // title_style.text.font = &stsong_22;
    lv_style_set_text_font(&title_style, LV_STATE_DEFAULT, &stsong_22);
    lv_obj_add_style(title1, LV_LABEL_PART_MAIN, &title_style);
    lv_label_set_text(title1, _("Please Input Employee's Name:"));
    lv_obj_align(title1, NULL, LV_ALIGN_IN_TOP_LEFT, 30, 30);

    ta_name = lv_textarea_create(dialog_img, NULL);
    static lv_style_t ta_style;
    // lv_style_copy(&ta_style, lv_ta_get_style(ta_name, LV_TEXTAREA_PART_BG));
    lv_style_init(&ta_style);
    // ta_style.text.font = &stsong_22;
    lv_textarea_set_cursor_hidden(ta_name, true);
    lv_style_set_text_font(&ta_style, LV_STATE_DEFAULT, &stsong_22);
    lv_obj_add_style(ta_name, LV_TEXTAREA_PART_BG, &ta_style);
    lv_textarea_set_max_length(ta_name, 20);
    lv_textarea_set_one_line(ta_name, true);
    lv_obj_set_size(ta_name, 200, 40);
    lv_textarea_set_text_align(ta_name, LV_LABEL_ALIGN_LEFT);
    lv_textarea_set_text(ta_name, "");
    lv_obj_align(ta_name, NULL, LV_ALIGN_IN_TOP_LEFT, 30, 80);
    lv_obj_set_event_cb(ta_name, ta_name_event_cb);

    lv_obj_t * title2 = lv_label_create(dialog_img, NULL);
    lv_obj_add_style(title2, LV_LABEL_PART_MAIN, &title_style);
    lv_label_set_text(title2, _("Please Input Employee's ID:"));
    lv_obj_align(title2, NULL, LV_ALIGN_IN_TOP_LEFT, 30, 160);

    ta_id = lv_textarea_create(dialog_img, NULL);
    lv_textarea_set_cursor_hidden(ta_id, true);
    lv_textarea_set_max_length(ta_id, 20);
    lv_textarea_set_one_line(ta_id, true);
    lv_obj_set_size(ta_id, 200, 40);
    lv_textarea_set_text_align(ta_id, LV_LABEL_ALIGN_LEFT);
    lv_textarea_set_text(ta_id, "");
    lv_obj_align(ta_id, NULL, LV_ALIGN_IN_TOP_LEFT, 30, 200);
    lv_obj_set_event_cb(ta_id, ta_id_event_cb);

    lv_obj_t *commit = lv_imgbtn_create(dialog_img, NULL);
    lv_imgbtn_set_src(commit, LV_BTN_STATE_PRESSED, RES_IMG_BTN_CONFIRM);
    lv_imgbtn_set_src(commit, LV_BTN_STATE_RELEASED, RES_IMG_BTN_CONFIRM);
    lv_obj_align(commit, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    lv_obj_set_user_data(commit, obj);
    lv_obj_set_event_cb(commit, commit_btn_event_cb);
    lv_obj_t * commit_label = lv_label_create(commit, NULL);
    lv_label_set_text(commit_label, _("Commit"));
    lv_obj_align(commit_label, NULL, LV_ALIGN_CENTER, 0, 0);

    static lv_style_t btn_style_rel;
    //static lv_style_t btn_style_pr;
    // lv_style_copy(&btn_style_rel, lv_imgbtn_get_style(commit, LV_IMGBTN_PART_MAIN));
    // lv_style_copy(&btn_style_pr, lv_imgbtn_get_style(commit, LV_IMGBTN_STYLE_PR));
    lv_style_init(&btn_style_rel);
    lv_style_set_text_font(&btn_style_rel, LV_BTN_STATE_PRESSED, &stsong_18);
    lv_style_set_text_font(&btn_style_rel, LV_BTN_STATE_RELEASED, &stsong_18);
    // btn_style_rel.text.font = &stsong_18;
    // btn_style_pr.text.font = &stsong_18;
    lv_obj_add_style(commit, LV_IMGBTN_PART_MAIN, &btn_style_rel);
    // lv_imgbtn_set_style(commit, LV_IMGBTN_STYLE_PR, &btn_style_pr);

    return NULL;
}