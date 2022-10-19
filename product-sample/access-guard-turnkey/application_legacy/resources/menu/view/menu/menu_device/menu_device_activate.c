#include "menu_device_activate.h"
#include "menu.h"


static void devActMenuPageCreate();
static void devActMenuPageDestroy();
static void devActMenuPageShow();
static void devActMenuPageHide();

static lv_obj_t * devActMenuPage_obj = NULL;
static menu_page_t devActMenuPage = {
    .onCreate = devActMenuPageCreate,
    .onDestroy = devActMenuPageDestroy,
    .show = devActMenuPageShow,
    .hide = devActMenuPageHide,
    .back = menuPageBack
};

menu_page_t getDevActMenuPage()
{
    return devActMenuPage;
}

static void back_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	switch (event)
	{
	case LV_EVENT_RELEASED:
        //Todo
        menuPageBack();
		break;
	default:
		break;
	}
}

static void devActMenuPageCreate()
{
    lv_obj_t * parent = lv_scr_menu();

    devActMenuPage_obj =  lv_cont_create(parent, NULL);
    lv_obj_set_size(devActMenuPage_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_cont_set_fit(devActMenuPage_obj, LV_FIT_NONE);
    lv_cont_set_layout(devActMenuPage_obj, LV_LAYOUT_OFF);

    lv_obj_t * back = lv_btn_create(devActMenuPage_obj, NULL);
    lv_obj_t * back_label = lv_label_create(back, NULL);
    lv_label_set_text(back_label, "<Back");
    lv_btn_set_fit(back, LV_FIT_TIGHT);
    lv_obj_set_event_cb(back, back_btn_event_cb);

    lv_obj_set_hidden(devActMenuPage_obj, true);
}

static void devActMenuPageDestroy()
{
    lv_obj_del(devActMenuPage_obj);
    devActMenuPage_obj = NULL;
}

static void devActMenuPageShow()
{
    if (devActMenuPage_obj == NULL) {
        return;
    }

    lv_obj_set_hidden(devActMenuPage_obj, false);
}

static void devActMenuPageHide()
{
    if (devActMenuPage_obj == NULL) {
        return;
    }

    lv_obj_set_hidden(devActMenuPage_obj, true);
}
