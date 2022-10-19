#include <time.h>

#include "down_user_dialog.h"
#include "write_mac_sn.h"
#include "lvgl/lvgl.h"
#include "resource.h"
#include "lv_i18n.h"
#include "app_utils.h"
#include "menu.h"
#include "cvi_hal_face_api.h"
#include "menu_common.h"
#include "menu.h"
#include "net.h"
#include "public.h"
#include "keyboard.h"
#include "cviconfigsystem.h"
#include "launcher.h"

extern hal_facelib_handle_t facelib_handle;

static lv_obj_t *down_user_dialog = NULL;

lv_obj_t * down_user_lable = NULL;

static pthread_t down_user_id = 0;
static time_t start_down_user_time = 0;

void set_down_user_time(time_t times)
{
	start_down_user_time = times;
}
time_t get_down_user_time()
{
	return start_down_user_time;
}

void * down_user_check_thread(void *arg)
{
	char bufTemp[64] = {0};

	while(true)
	{
		if(time(NULL) - get_down_user_time() >= 5)
		{
			break;
		}
		sleep(1);
	}

	if(down_user_lable != NULL)
	{
    	lv_label_set_text(down_user_lable, bufTemp);
	}

	if (facelib_handle != NULL) {
		CviLoadIdentify(facelib_handle);
	}

	hide_down_user_popmenu();
	sleep(1);
	set_down_user_time(0);

	return NULL;
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
        lv_style_set_text_font(&label_style, LV_STATE_DEFAULT, &stsong_36);

        style_init = true;
    }
}

lv_obj_t * createPopupMenuDownUser(lv_obj_t *par)
{
	//char bufTemp[64] = {0};
    if (par == NULL) {
        par = lv_layer_top();
    }

    _style_init();

	pthread_create(&down_user_id,NULL,down_user_check_thread,NULL);

    down_user_dialog = lv_obj_create(lv_layer_top(), NULL);
    lv_obj_add_style(down_user_dialog, LV_OBJ_PART_MAIN, &modal_style);
    lv_obj_set_pos(down_user_dialog, 0, 0);
    lv_obj_set_size(down_user_dialog, LV_HOR_RES, LV_VER_RES);

	lv_obj_t *disp_panel = lv_cont_create(down_user_dialog, NULL);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX-100, LV_VER_RES_MAX/3);
    lv_obj_set_pos(disp_panel, 50, LV_VER_RES_MAX/3);
	lv_obj_add_style(disp_panel, LV_CONT_PART_MAIN, &dialog_style);

    lv_obj_t * title_obj = lv_label_create(disp_panel, NULL);
    lv_obj_add_style(title_obj, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_text(title_obj, _("Down User"));
    lv_obj_align(title_obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);

    down_user_lable = lv_label_create(disp_panel, NULL);
    lv_obj_add_style(down_user_lable, LV_LABEL_PART_MAIN, &label_style);
	//sprintf(bufTemp,"%s",_("loading"));
    lv_label_set_text(down_user_lable, _("Downloading users"));
    lv_obj_align(down_user_lable, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 180);

    return down_user_dialog;
}

static lv_obj_t *popup_down_user_menu = NULL;
static pthread_mutex_t popmenu_down_user_mutex = PTHREAD_MUTEX_INITIALIZER;

void hide_down_user_popmenu()
{
    pthread_mutex_lock(&popmenu_down_user_mutex);
    if (popup_down_user_menu) {
        lv_obj_del(popup_down_user_menu);
        popup_down_user_menu = NULL;
    }
    pthread_mutex_unlock(&popmenu_down_user_mutex);
	if(get_enter_menu_flag() == OUT_MENU)
	{
		APP_CompSendCmd(CMD_APP_COMP_RESTART, NULL, 0, NULL, 0);
	}
}

void show_down_user_popmenu()
{
    pthread_mutex_lock(&popmenu_down_user_mutex);

    if (popup_down_user_menu) {
        lv_obj_del(popup_down_user_menu);
        popup_down_user_menu = NULL;
    }

    if (popup_down_user_menu == NULL) {
        popup_down_user_menu = createPopupMenuDownUser(lv_launcher_desktop());
    }
    pthread_mutex_unlock(&popmenu_down_user_mutex);
}

