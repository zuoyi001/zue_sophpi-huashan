#include "face_capture_page.h"
#include "resource.h"
#include "lv_i18n.h"
#include "menu.h"
#include "app_utils.h"
#include "app_comp.h"
#include "menu_common.h"
#include "face_register_dialog.h"

static void show_buttons();
static void hide_buttons();
static void showCapButton();
static void face_capture_page_create();
static void face_capture_page_del();
static void face_capture_page_show();
static void face_capture_page_hide();

static lv_obj_t * face_capture_page = NULL;
static menu_page_t faceCapturePage = {
    .onCreate = face_capture_page_create,
    .onDestroy = face_capture_page_del,
    .show = face_capture_page_show,
    .hide = face_capture_page_hide,
    .back = menuPageBack
};

menu_page_t getFaceCapturePage()
{
    return faceCapturePage;
}

static void back_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	switch (event)
	{
	case LV_EVENT_RELEASED:
        printf("back_btn_event_cb.\n");
        menuPageBack();
		break;
	default:
		break;
	}
}

static void cap_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
        APP_CompSendCmd(CMD_APP_COMP_VIDEOFREEZE, NULL, 0, NULL, 0);
        lv_obj_set_hidden(obj, true);
        show_buttons();       
    }
}

static void confirm_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {

        //menuPageBack();
        createFaceRegisterDialog(NULL);
    }
}

static void cancel_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
		APP_CompSendCmd(CMD_APP_COMP_VIDEORESIZE, NULL, 0, NULL, 0);
        hide_buttons();
        showCapButton();
    }
}

#define WIDTH (LV_HOR_RES_MAX)
#define HEIGHT (LV_VER_RES_MAX - TOP_PANEL_H)

static lv_obj_t *page_title = NULL;
static lv_obj_t *confirm_btn;
static lv_obj_t *cancel_btn;
static lv_obj_t *cap_btn;

static bool style_inited = false;
static lv_style_t bg_style;
static lv_style_t top_panel_style;
static void _style_init()
{
    if (!style_inited)
    {
        lv_style_init(&bg_style);
        lv_style_set_bg_opa(&bg_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);

        lv_style_init(&top_panel_style);
        lv_style_set_bg_color(&top_panel_style, LV_STATE_DEFAULT, COLOR_TOP_PANEL);
        style_inited = true;
    }
}

static void face_capture_page_create()
{
    lv_obj_t * parent = lv_scr_menu();

    _style_init();

    face_capture_page = lv_obj_create(parent, NULL);
    lv_obj_set_size(face_capture_page, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_add_style(face_capture_page, LV_OBJ_PART_MAIN, &bg_style);

    lv_obj_t * top_panel = lv_obj_create(face_capture_page, NULL);
    lv_obj_set_size(top_panel, LV_HOR_RES_MAX, TOP_PANEL_H);
    lv_obj_set_pos(top_panel, 0, 0);
    lv_obj_add_style(top_panel, LV_CONT_PART_MAIN, &top_panel_style);

    lv_obj_t *back_img = lv_img_create(top_panel, NULL);
    lv_img_set_src(back_img, RES_IMG_BACK);
    lv_img_set_auto_size(back_img, true);
    lv_obj_set_click(back_img, true);
    lv_obj_align(back_img, NULL, LV_ALIGN_IN_TOP_LEFT, 15, (TOP_PANEL_H - 30 ) / 2);
    lv_obj_set_event_cb(back_img, back_btn_event_cb);

    page_title = lv_label_create(top_panel, NULL);
    static lv_style_t title_style;
    lv_style_init(&title_style);
    lv_style_set_text_font(&title_style, LV_STATE_DEFAULT, &stsong_18);
    lv_obj_add_style(page_title, LV_LABEL_PART_MAIN, &title_style);
    lv_label_set_text(page_title, _("Capture"));
    lv_obj_align(page_title, NULL, LV_ALIGN_CENTER, 0, 0);

    static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_INDEXED_1BIT(WIDTH, HEIGHT)];
    lv_obj_t *canvas = lv_canvas_create(face_capture_page, NULL);
    lv_obj_set_pos(canvas, 0, TOP_PANEL_H);
    lv_canvas_set_buffer(canvas, cbuf, WIDTH, HEIGHT, LV_IMG_CF_INDEXED_1BIT);
    lv_canvas_set_palette(canvas, 0, LV_COLOR_TRANSP);
    lv_canvas_set_palette(canvas, 1, LV_COLOR_WHITE);
    lv_color_t c0;
    lv_color_t c1;
    c0.full = 0;
    c1.full = 1;
    lv_canvas_fill_bg(canvas, c1, LV_OPA_100);
    int w = 500;
    int h = 800;
    int x = (WIDTH - w)/2;
    int y = (HEIGHT - h)/2;
    for (int j = y; j < y + h; j++){
        for (int i = x; i < x + w ; i++) {
            lv_canvas_set_px(canvas, i, j, c0);
        }
    }

    cap_btn = lv_btn_create(canvas, NULL);
    lv_obj_align(cap_btn, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -(y + 20));
    lv_obj_t *cap_label = lv_label_create(cap_btn, NULL);
    lv_label_set_text(cap_label, "Capture");
    lv_obj_set_event_cb(cap_btn, cap_btn_event_cb);

    confirm_btn = lv_btn_create(canvas, NULL);
    lv_obj_align(confirm_btn, NULL, LV_ALIGN_IN_BOTTOM_MID, -100, -(y + 20));
    lv_obj_t *confirm_label = lv_label_create(confirm_btn, NULL);
    lv_label_set_text(confirm_label, "Confirm");
    lv_obj_set_event_cb(confirm_btn, confirm_btn_event_cb);

    cancel_btn = lv_btn_create(canvas, NULL);
    lv_obj_align(cancel_btn, NULL, LV_ALIGN_IN_BOTTOM_MID, 100, -(y + 20));
    lv_obj_t *cancel_label = lv_label_create(cancel_btn, NULL);
    lv_label_set_text(cancel_label, "Cancel");
    lv_obj_set_event_cb(cancel_btn, cancel_btn_event_cb);
    
    lv_obj_set_hidden(confirm_btn, true);
    lv_obj_set_hidden(cancel_btn, true);
}

static void face_capture_page_del()
{
    lv_obj_del(face_capture_page);
    face_capture_page = NULL;
}

static void face_capture_page_show()
{
    if (face_capture_page) {
        lv_obj_set_hidden(face_capture_page, false);
    }
}

static void face_capture_page_hide()
{
    if (face_capture_page) {
        lv_obj_set_hidden(face_capture_page, true);
    }
}

static void showCapButton()
{
    lv_obj_set_hidden(cap_btn, false);
}

static void show_buttons()
{
    lv_obj_set_hidden(confirm_btn, false);
    lv_obj_set_hidden(cancel_btn, false);
}

static void hide_buttons()
{
    lv_obj_set_hidden(confirm_btn, true);
    lv_obj_set_hidden(cancel_btn, true);
}
