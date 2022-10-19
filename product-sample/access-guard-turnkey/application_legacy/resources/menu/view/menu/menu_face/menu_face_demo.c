#include "menu_face_demo.h"
#include "app_utils.h"
#include "menu.h"

static void face_demo_create();
static void face_demo_destroy();
static void face_demo_show();
static void face_demo_hide();

static lv_obj_t * face_demo_page = NULL;
//static page_return_cb s_back_func_cb = NULL;

static menu_page_t faceDemoMenuPage = {
    .onCreate = face_demo_create,
    .onDestroy = face_demo_destroy,
    .show = face_demo_show,
    .hide = face_demo_hide,
    .back = menuPageBack
};

menu_page_t getFaceDemoMenuPage()
{
    return faceDemoMenuPage;
}

static void btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	switch (event)
	{
	case LV_EVENT_RELEASED:
    {
        intptr_t id = (intptr_t) lv_obj_get_user_data(obj);
        if (id == 1) {
            printf("btn_face_detect clicked. \n");
        } else if (id == 2) {
            printf("btn_face_reg clicked.\n");
            facelib_config.facereg_en = true;
        } else if (id == 3) {
            printf("btn_face_compare clicked.\n");
        } else if (id == 4) {
            printf("btn_yolo clicked.\n");
        }
		break;
    }
	default:
		break;
	}
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

static void face_demo_create()
{
    lv_obj_t * parent = lv_scr_menu();

    face_demo_page = lv_cont_create(parent, NULL);
    lv_obj_set_size(face_demo_page, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_cont_set_layout(face_demo_page, LV_LAYOUT_COLUMN_LEFT);

    lv_obj_t * back = lv_btn_create(face_demo_page, NULL);
    lv_obj_t * back_label = lv_label_create(back, NULL);
    lv_label_set_text(back_label, "<Back");
    lv_btn_set_fit(back, LV_FIT_TIGHT);
    lv_obj_set_event_cb(back, back_btn_event_cb);
    
    lv_obj_t * btn_face_detect = lv_btn_create(face_demo_page, NULL);
    lv_btn_set_fit(btn_face_detect, LV_FIT_TIGHT);
    lv_obj_t * label_btn1 = lv_label_create(btn_face_detect, NULL);
    lv_label_set_text(label_btn1, "Face Detect");
    lv_obj_set_user_data(btn_face_detect, (lv_obj_user_data_t)1);
    lv_obj_set_event_cb(btn_face_detect, btn_event_cb);

    lv_obj_t * btn_face_reg = lv_btn_create(face_demo_page, NULL);
    lv_btn_set_fit(btn_face_reg, LV_FIT_TIGHT);
    lv_obj_t * btn_face_reg_label = lv_label_create(btn_face_reg, NULL);
    lv_label_set_text(btn_face_reg_label, "Face Register");
    lv_obj_set_user_data(btn_face_reg, (lv_obj_user_data_t)2);
    lv_obj_set_event_cb(btn_face_reg, btn_event_cb);

    lv_obj_t * btn_face_compare = lv_btn_create(face_demo_page, NULL);
    lv_btn_set_fit(btn_face_compare, LV_FIT_TIGHT);
    lv_obj_t * btn_face_compare_label = lv_label_create(btn_face_compare, NULL);
    lv_label_set_text(btn_face_compare_label, "Face Compare");
    lv_obj_set_user_data(btn_face_compare, (lv_obj_user_data_t)3);
    lv_obj_set_event_cb(btn_face_compare, btn_event_cb);

    lv_obj_t * btn_yolo = lv_btn_create(face_demo_page, NULL);
    lv_btn_set_fit(btn_yolo, LV_FIT_TIGHT);
    lv_obj_t * btn_yolo_label = lv_label_create(btn_yolo, NULL);
    lv_label_set_text(btn_yolo_label, "Yolo");
    lv_obj_set_user_data(btn_yolo, (lv_obj_user_data_t)4);
    lv_obj_set_event_cb(btn_yolo, btn_event_cb);

    lv_obj_set_hidden(face_demo_page, true);
}

static void face_demo_destroy()
{
    lv_obj_del(face_demo_page);
    face_demo_page = NULL;
}

static void face_demo_show()
{
    if (face_demo_page == NULL) {
        return;
    }

    lv_obj_set_hidden(face_demo_page, false);
}

static void face_demo_hide()
{
    if (face_demo_page == NULL) {
        return;
    }

    lv_obj_set_hidden(face_demo_page, true);
}