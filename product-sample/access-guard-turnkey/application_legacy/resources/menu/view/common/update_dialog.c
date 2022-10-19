#include "update_dialog.h"
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
#include "launcher.h"


static lv_obj_t *update_dialog = NULL;

//static char snBuf[64] = {0};
//static char macBuf[64] = {0};
lv_obj_t * updatelable = NULL;

static pthread_t update_id = 0;

void * update_check_thread(void *arg)
{
	char bufTemp[64] = {0};
	int flag = 0;

	while(true)
	{
		if(flag < 10)
		{
			if(0 == flag )
			{
				sprintf(bufTemp,"%s","loading .");
			}
			else if(1 == flag )
			{
				sprintf(bufTemp,"%s","loading ..");
			}
			else if(2 == flag )
			{
				sprintf(bufTemp,"%s","loading ...");
			}
			else if(3 == flag )
			{
				sprintf(bufTemp,"%s","loading ....");
			}
			else if(4 == flag )
			{
				sprintf(bufTemp,"%s","loading .....");
			}
			else if(5 == flag )
			{
				sprintf(bufTemp,"%s","loading ......");
			}
			else
			{
				flag = 0;
				sprintf(bufTemp,"%s","loading .");
			}
			flag++;
			printf("[%s,%d]\n",__FUNCTION__,__LINE__);
			
			if(getUpdateResult() == 1)
			{
				sprintf(bufTemp,"%s",_("load finish"));
				flag = 10;
			}
			else if(getUpdateResult() < 0)
			{
				sprintf(bufTemp,"%s",_("load fail"));
			}
		}
		else
		{
			sprintf(bufTemp,"%s",_("update tips"));
			sleep(2);
		}

		if(updatelable != NULL)
		{
	    	lv_label_set_text(updatelable, bufTemp);
		}

		sleep(1);
	}
							
	return NULL;
}



static void confirm_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	//char bufTemp[64] = {0};
    if (event == LV_EVENT_CLICKED) {

		if(getUpdateResult() != 1)
		{
			return;
		}
		
		if(update_id != 0)
		{
			pthread_cancel(update_id);
			update_id = 0;
		}
		hide_update_popmenu();
		updatelable = NULL;
		system("reboot");
    }
}

static void cancel_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        hide_update_popmenu();
		updatelable = NULL;
		if(update_id != 0)
		{
			pthread_cancel(update_id);
			update_id = 0;
			unlink(UPDATE_FOFT_FILE_NAME);
		}
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
        lv_style_set_text_font(&label_style, LV_STATE_DEFAULT, &stsong_36);

        style_init = true;
    }
}

lv_obj_t * createPopupMenuUpdate(lv_obj_t *par)
{
	//char bufTemp[64] = {0};
    if (par == NULL) {
        par = lv_layer_top();
    }

    _style_init();

	APP_CompSendCmd(CMD_APP_COMP_VIDEOSTOP, NULL, 0, NULL, 0);

	pthread_create(&update_id,NULL,update_check_thread,NULL);

    update_dialog = lv_obj_create(lv_layer_top(), NULL);
    lv_obj_add_style(update_dialog, LV_OBJ_PART_MAIN, &modal_style);
    lv_obj_set_pos(update_dialog, 0, 0);
    lv_obj_set_size(update_dialog, LV_HOR_RES, LV_VER_RES);

	lv_obj_t *disp_panel = lv_cont_create(update_dialog, NULL);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX-100, LV_VER_RES_MAX/3);
    lv_obj_set_pos(disp_panel, 50, LV_VER_RES_MAX/3);
	lv_obj_add_style(disp_panel, LV_CONT_PART_MAIN, &dialog_style);

    lv_obj_t * title_obj = lv_label_create(disp_panel, NULL);
    lv_obj_add_style(title_obj, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_text(title_obj, _("update"));
    lv_obj_align(title_obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
  
    updatelable = lv_label_create(disp_panel, NULL);
    lv_obj_add_style(updatelable, LV_LABEL_PART_MAIN, &label_style);
	//sprintf(bufTemp,"%s",_("loading"));
    lv_label_set_text(updatelable, "");
    lv_obj_align(updatelable, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 180);
	

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

    return update_dialog;
}

static lv_obj_t *popup_update_menu = NULL;
static pthread_mutex_t popmenu_update_mutex = PTHREAD_MUTEX_INITIALIZER;

void hide_update_popmenu()
{
    pthread_mutex_lock(&popmenu_update_mutex);
    if (popup_update_menu) {
        lv_obj_del(popup_update_menu);
        popup_update_menu = NULL;
    }
    pthread_mutex_unlock(&popmenu_update_mutex);
	APP_CompSendCmd(CMD_APP_COMP_RESTART, NULL, 0, NULL, 0);
}

void show_update_popmenu()
{
    pthread_mutex_lock(&popmenu_update_mutex);

    if (popup_update_menu) {
        lv_obj_del(popup_update_menu);
        popup_update_menu = NULL;
    }

    if (popup_update_menu == NULL) {
        popup_update_menu = createPopupMenuUpdate(lv_launcher_desktop());
    }
    pthread_mutex_unlock(&popmenu_update_mutex);
}

