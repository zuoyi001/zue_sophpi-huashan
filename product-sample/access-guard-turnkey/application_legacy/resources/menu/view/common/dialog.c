#include <time.h>

#include "dialog.h"
#include "resource.h"
#include "keyboard.h"
#include "app_utils.h"
#include "menu.h"
#include "launcher.h"
#include "menu_page.h"

typedef struct {
    lv_obj_t * dialog;
    dialog_func_cb ok_cb;
    dialog_func_cb cancel_cb;
    void * args;
} intern_dialog_t;

//local declaration
static lv_obj_t * dialog_create(lv_obj_t * parent, char *title_str, MSG_BOX_BTN_TYPE type, dialog_func_cb ok, dialog_func_cb cancel);
static void checkPasswd(lv_obj_t * obj, lv_event_t event);
static void text_area_event_handler(lv_obj_t * text_area, lv_event_t event);

static lv_obj_t * kb;
static lv_obj_t * par_cont;

lv_obj_t *passwd = NULL;


LV_FONT_DECLARE(stsong_22);
LV_FONT_DECLARE(stsong_18);
LV_FONT_DECLARE(stsong_16);

//Function
static lv_obj_t * dialog_create(lv_obj_t * parent, char *title_str, MSG_BOX_BTN_TYPE type, dialog_func_cb ok, dialog_func_cb cancel)
{
    if (parent == NULL) {
        parent = lv_scr_act();
    }

    lv_obj_t * dialog_img = lv_img_create(parent, NULL);
    //lv_mem_assert(dialog_img);
    lv_img_set_src(dialog_img, RES_IMG_DIALOG);
    lv_img_set_auto_size(dialog_img, true);
    lv_obj_set_pos(dialog_img, (LV_HOR_RES_MAX - 526) / 2, (LV_VER_RES_MAX - 307) / 2);

    if (title_str != NULL) {
        lv_obj_t * title = lv_label_create(dialog_img, NULL);
        lv_label_set_text(title, title_str);
    }

    switch (type) {
        case MSG_BOX_BTN_OK_CANCEL:
        {
            lv_obj_t * btn_ok = lv_imgbtn_create(dialog_img, NULL);
            lv_imgbtn_set_src(btn_ok, LV_BTN_STATE_PRESSED, RES_IMG_BTN_CONFIRM);
            lv_imgbtn_set_src(btn_ok, LV_BTN_STATE_RELEASED, RES_IMG_BTN_CONFIRM);
            lv_obj_align(btn_ok, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 120, -65);
            lv_obj_t * label_ok = lv_label_create(btn_ok, NULL);
            lv_label_set_text(label_ok, "#FF0000 OK#");
            lv_label_set_recolor(label_ok, true);
            lv_obj_align(label_ok, NULL, LV_ALIGN_CENTER, -10, -10);

            lv_obj_t * btn_cancel = lv_imgbtn_create(dialog_img, NULL);
            lv_imgbtn_set_src(btn_cancel, LV_BTN_STATE_PRESSED, RES_IMG_BTN_CANCEL);
            lv_imgbtn_set_src(btn_cancel, LV_BTN_STATE_RELEASED, RES_IMG_BTN_CANCEL);
            lv_obj_align(btn_cancel, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -120, -65);
            lv_obj_t * label_cancel = lv_label_create(btn_cancel, NULL);
            lv_label_set_text(label_cancel, "#FF0000 Cancel#");
            lv_label_set_recolor(label_cancel, true);
            lv_obj_align(label_cancel, NULL, LV_ALIGN_CENTER, -10, -10); 
            break;
        }
        case MSG_BOX_BTN_OK:
        default:
        {
            lv_obj_t * btn_ok = lv_imgbtn_create(dialog_img, NULL);
            lv_imgbtn_set_src(btn_ok, LV_BTN_STATE_PRESSED, RES_IMG_BTN_CONFIRM);
            lv_imgbtn_set_src(btn_ok, LV_BTN_STATE_RELEASED, RES_IMG_BTN_CONFIRM);
            lv_obj_align(btn_ok, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -65);
            lv_obj_t * label_ok = lv_label_create(btn_ok, NULL);
            lv_label_set_text(label_ok, "#FF0000 OK#");
            lv_label_set_recolor(label_ok, true);
            lv_obj_align(label_ok, NULL, LV_ALIGN_CENTER, -10, -10);
            if (ok != NULL) {
                intern_dialog_t *dia = (intern_dialog_t *) malloc(sizeof(intern_dialog_t));
                dia->dialog = dialog_img;
                dia->ok_cb = ok;
                lv_obj_set_user_data(btn_ok, (lv_obj_user_data_t)dia);
            }
            lv_obj_set_event_cb(btn_ok, checkPasswd);
            break;
        }
    }

    return dialog_img;
}

lv_obj_t * createMsgBox(lv_obj_t *par, char *title, char *msg, MSG_BOX_BTN_TYPE type, dialog_func_cb ok, dialog_func_cb cancel)
{
    if (par == NULL) {
        par = lv_scr_act();
    }

    if (title == NULL || msg == NULL) {
        return NULL;
    }

    lv_obj_t *img_dialog = dialog_create(par, title, type, NULL, NULL);
    //lv_mem_assert(img_dialog);

    return img_dialog;
}

static void checkPasswd(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        if (kb)
        {
            lv_obj_del(kb);
            kb = NULL;
        }

        intern_dialog_t *dia = (intern_dialog_t *) lv_obj_get_user_data(obj);
        lv_obj_t *dialog_obj = dia->dialog;
        const char *passwd_txt = lv_textarea_get_text((lv_obj_t *)(dia->args));
        lv_obj_set_hidden(dialog_obj, true);
        //lv_textarea_set_text((lv_obj_t *)(dia->args), "");
        if (dia->ok_cb) {
            dia->ok_cb(passwd_txt);
        }
    }
}

static void desktop_event_cb(lv_obj_t * obj, lv_event_t event)
{
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	switch (event)
	{
	    case LV_EVENT_CLICKED:
	    {
			if (kb)
	        {
	            lv_obj_del(kb);
	            kb = NULL;
	        }

			set_enter_menu_flag(OUT_MENU);
			APP_CompSendCmd(CMD_APP_COMP_RESTART, NULL, 0, NULL, 0);
			lv_textarea_set_text(passwd, "");
			
	        lv_obj_t * admin_alert = lv_obj_get_user_data(obj);
	        if (admin_alert) {
	            lv_obj_set_hidden(admin_alert, true);
	        }
			break;
	    }
		default:
			break;
	}
}

void setPasswdClear()
{
	lv_textarea_set_text(passwd, "");
}
lv_obj_t * createPasswdBox(lv_obj_t *par, char *title_str, dialog_func_cb ok)
{
    if (par == NULL) {
        par = lv_layer_top();
    }

    if (title_str == NULL) {
        return NULL;
    }

    //par_cont = par;

    static lv_style_t modal_style;
    lv_style_init(&modal_style);
    //lv_style_copy(&modal_style, &lv_style_plain_color);
    // modal_style.body.main_color = modal_style.body.grad_color = LV_COLOR_BLACK;
    // modal_style.body.opa = LV_OPA_30;
    lv_style_set_bg_color(&modal_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_opa(&modal_style, LV_STATE_DEFAULT, LV_OPA_30);

    lv_obj_t * obj = lv_obj_create(par, NULL);
    lv_obj_add_style(obj, LV_OBJ_PART_MAIN, &modal_style);
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, LV_HOR_RES, LV_VER_RES);
	//lv_obj_set_click(obj, true);
	lv_obj_set_user_data(obj, (lv_obj_user_data_t) obj);
    lv_obj_set_event_cb(obj, desktop_event_cb);
	par_cont = obj;
    // lv_obj_set_opa_scale_enable(obj, true);

    lv_obj_t * dialog_img = lv_img_create(obj, NULL);
    //lv_mem_assert(dialog_img);
    lv_img_set_src(dialog_img, RES_IMG_DIALOG);
    lv_img_set_auto_size(dialog_img, true);
    lv_obj_align(dialog_img, NULL, LV_ALIGN_CENTER, 0, 0);

    if (title_str != NULL) {
        lv_obj_t * title_obj = lv_label_create(dialog_img, NULL);
        lv_label_set_text(title_obj, title_str);
        lv_obj_align(title_obj, NULL, LV_ALIGN_CENTER, 0, -70);
    }

    passwd = lv_textarea_create(dialog_img, NULL);
    //lv_mem_assert(passwd);
    lv_textarea_set_one_line(passwd, true);
    lv_textarea_set_pwd_mode(passwd, true);
    lv_textarea_set_pwd_show_time(passwd, 500);
    lv_textarea_set_text_align(passwd, LV_LABEL_ALIGN_LEFT);
    lv_textarea_set_accepted_chars(passwd, "0123456789");
    lv_textarea_set_max_length(passwd, 16);
    //lv_ta_set_cursor_type(passwd, LV_CURSOR_LINE);
    lv_textarea_set_text(passwd, "");
    lv_obj_set_width(passwd, 280);
    lv_obj_align(passwd, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(passwd, text_area_event_handler);
    /*static lv_style_t passwd_style;
    lv_style_copy(&passwd_style, lv_ta_get_style(passwd, LV_TA_STYLE_BG));
    passwd_style.text.font = &stsong_18;
    lv_ta_set_style(passwd, LV_TA_STYLE_BG, &passwd_style);*/

    lv_obj_t * btn_ok = lv_imgbtn_create(dialog_img, NULL);
    lv_imgbtn_set_src(btn_ok, LV_BTN_STATE_PRESSED, RES_IMG_BTN_CONFIRM);
    lv_imgbtn_set_src(btn_ok, LV_BTN_STATE_RELEASED, RES_IMG_BTN_CONFIRM);
    lv_obj_align(btn_ok, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -65);
    lv_obj_t * label_ok = lv_label_create(btn_ok, NULL);
    lv_label_set_text(label_ok, "#FF0000 OK#");
    lv_label_set_recolor(label_ok, true);
    lv_obj_align(label_ok, NULL, LV_ALIGN_CENTER, -10, -10);
    if (ok != NULL) {
        intern_dialog_t *dia = (intern_dialog_t *) malloc(sizeof(intern_dialog_t));
        dia->dialog = obj;
        dia->ok_cb = ok;
        dia->args = passwd;
        lv_obj_set_user_data(btn_ok, (lv_obj_user_data_t)dia);
    }
    lv_obj_set_event_cb(btn_ok, checkPasswd);

    return obj;
}

#define AVATAR_WIDTH (150)
#define AVATAR_HEIGHT (150)
static int calcZoom(int in_height)
{
    int ratio =  AVATAR_HEIGHT / (1.0 * in_height) * 256;
    printf("zoom :%d\n", ratio);
    return ratio;
}

lv_obj_t * createPopupMenu(lv_obj_t *par, void *face_img, char *name_str, const char *result_str, int width, int height, bool bPass)
{
    if (par == NULL) {
        par = lv_launcher_desktop();
    }

	//lv_style_t style_popup;
	//lv_style_init(&style_popup);
	//lv_style_set_text_font(&style_popup, LV_STATE_DEFAULT, &stsong_36);

    lv_obj_t * dialog_img = lv_img_create(par, NULL);
    if (bPass)
        lv_img_set_src(dialog_img, RES_IMG_POPUP_BG_PASS);
    else
        lv_img_set_src(dialog_img, RES_IMG_POPUP_BG_NG);
    lv_img_set_auto_size(dialog_img, true);
    lv_obj_align(dialog_img, NULL, LV_ALIGN_CENTER, 0, 400);
    lv_obj_set_auto_realign(dialog_img, true);

    lv_obj_t * face_rect = lv_img_create(dialog_img, NULL);
    if (face_img == NULL) {
        lv_img_set_src(face_rect, RES_IMG_TEST_NG_FACE);
    } else {
        lv_img_set_src(face_rect, face_img);
        lv_img_set_zoom(face_rect, calcZoom(height));
    }
    lv_obj_align(face_rect, NULL, LV_ALIGN_IN_LEFT_MID, 20, 0);

    if (name_str != NULL) {
        lv_obj_t *name_label = lv_label_create(dialog_img, NULL);
        lv_obj_align(name_label, NULL, LV_ALIGN_IN_TOP_MID, -100, 100);
		lv_obj_add_style(name_label, LV_STATE_DEFAULT, menu_page_title_style());
        lv_label_set_text(name_label, name_str);        
    }

	if (result_str != NULL) {
        lv_obj_t *result_label = lv_label_create(dialog_img, NULL);
		lv_obj_add_style(result_label, LV_STATE_DEFAULT, menu_page_title_style());
        lv_obj_align(result_label, NULL, LV_ALIGN_IN_TOP_MID, -100, 150);
        lv_label_set_text(result_label, result_str);
    }

    char date_str[16] = {0};
    char time_str[16] = {0};
    time_t cur_time;
    time(&cur_time);
    struct tm *tm_p = localtime(&cur_time);
    snprintf(date_str, sizeof(date_str), "%04d/%02d/%02d", 1900 + tm_p->tm_year, 1 + tm_p->tm_mon, tm_p->tm_mday);
    snprintf(time_str, sizeof(time_str), "%02d:%02d", (tm_p->tm_hour)%24, tm_p->tm_min);

    lv_obj_t *date_obj = lv_label_create(dialog_img, NULL);
	lv_obj_add_style(date_obj, LV_STATE_DEFAULT, menu_page_title_style());
    lv_label_set_text(date_obj, date_str);
    lv_obj_align(date_obj, NULL, LV_ALIGN_IN_RIGHT_MID, -100, -30);

    lv_obj_t *time_obj = lv_label_create(dialog_img, NULL);
	lv_obj_add_style(time_obj, LV_STATE_DEFAULT, menu_page_title_style());
    lv_label_set_text(time_obj, time_str);
    lv_obj_align(time_obj, NULL, LV_ALIGN_IN_RIGHT_MID, -110, 20);

    return dialog_img;
}

static void text_area_event_handler(lv_obj_t * text_area, lv_event_t event)
{
    /*Text area is on the scrollable part of the page but we need the page itself*/
    // lv_obj_t * parent = par_cont;

    if(event == LV_EVENT_CLICKED) {
        if(kb == NULL) {
            kb = create_keyboard(&kb, text_area, par_cont, 0, 130);//y offset 130
            lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUM);
        }
    }
}
