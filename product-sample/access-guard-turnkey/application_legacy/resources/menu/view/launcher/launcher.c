#include "launcher.h"
#include "menu_setting.h"
#include "menu_test_mode.h"
#include "menu.h"
#include "menu_factory_mode.h"
#include "resource.h"
#include "app_utils.h"
#include "dialog.h"
#include "lv_i18n.h"

/*Local Macro*/
#define DESKTOP_HOR_RES (LV_HOR_RES_MAX)
#define DESKTOP_VER_RES (LV_VER_RES_MAX)
#define tr(text) lv_i18n_get_text(text)

/*Local declaration*/
void launcher_init_vol_slider();
static void launcher_ui_init();
static void launcher_layout_init();
static void launcher_retranslateUI();

/*static variable*/
static lv_obj_t * desktop;
static lv_obj_t * scr_launcher;
static lv_style_t launcher_style;
#if 0
static lv_style_t facename_label_style;
#endif
static lv_obj_t * launcher_title;

static void menu_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	//static uint8_t en=0;
	switch (event)
	{
    case LV_EVENT_CLICKED:
    {
        lv_obj_t * admin_alert = lv_obj_get_user_data(obj);
        if (admin_alert) {
            lv_obj_set_hidden(admin_alert, false);
			set_enter_menu_flag(ENTER_MENU);
			APP_CompSendCmd(CMD_APP_COMP_VIDEOSTOP, NULL, 0, NULL, 0);
        }
		break;
    }
	default:
		break;
	}
}

void launcher_app_init()
{
    launcher_layout_init();
    launcher_ui_init();
}

#if 0
static void vol_slider_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
    }
}
#endif

void launcher_init_vol_slider()
{
    lv_obj_t * img_vol_sym = lv_img_create(lv_launcher_desktop(), NULL);
    lv_img_set_src(img_vol_sym, RES_IMG_VOL_SYMBOL);
    lv_img_set_auto_size(img_vol_sym, true);
    lv_obj_set_pos(img_vol_sym, 0, 118 + 5);

    lv_obj_t * img_vol_bar = lv_img_create(lv_launcher_desktop(), NULL);
    lv_img_set_src(img_vol_bar, RES_IMG_VOL_BAR_VOL24);
    lv_img_set_auto_size(img_vol_bar, true);
    lv_obj_set_pos(img_vol_bar, 0, 118 + 55);
}

static void launcher_layout_init()
{
    /*set screen*/
    scr_launcher = lv_cont_create(NULL, NULL);
    lv_disp_load_scr(scr_launcher);
    // lv_style_copy(&launcher_style, &lv_style_scr);
    // launcher_style.body.opa = LAUNCHER_MAIN_OPACITY;
    lv_style_init(&launcher_style);
    lv_style_set_bg_opa(&launcher_style, LV_STATE_DEFAULT, LAUNCHER_MAIN_OPACITY);
    lv_style_set_opa_scale(&launcher_style, LV_STATE_DEFAULT, ALTERNATE_UI_OPACITY);
    lv_obj_add_style(scr_launcher, LV_CONT_PART_MAIN, &launcher_style);
    lv_obj_set_size(scr_launcher, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_cont_set_fit(scr_launcher, LV_FIT_NONE);

    /*set main desktop*/
    desktop = lv_cont_create(scr_launcher, NULL);
    lv_obj_add_style(desktop, LV_CONT_PART_MAIN, &launcher_style);
    lv_obj_set_size(desktop, DESKTOP_HOR_RES, DESKTOP_VER_RES);
    lv_cont_set_fit(desktop, LV_FIT_NONE);
    lv_cont_set_layout(desktop, LV_LAYOUT_OFF);
    // lv_obj_set_opa_scale_enable(desktop, true);
    // lv_cont_set_opa_scale(desktop, ALTERNATE_UI_OPACITY);
}

void validatePassWord(const void *arg)
{
    char * t = (char *)arg;
    printf("passwd: %s\n", t);
    //menu_scr_show();
	if(strcmp(t,"") == 0)
	{
		set_enter_menu_flag(ENTER_MENU);
    	menu_scr_show();
	}else if(strcmp(t,"1") == 0)
	{
		printf("welcome to test mode!");
		set_enter_menu_flag(ENTER_MENU);
		menu_test_mode_scr_show();
	}
	else if(strcmp(t,"2") == 0)
	{
		printf("welcome to test mode!");
		set_enter_menu_flag(ENTER_MENU);
		menu_factory_mode_scr_show();
	}
	else
	{
		set_enter_menu_flag(OUT_MENU);
		APP_CompSendCmd(CMD_APP_COMP_RESTART, NULL, 0, NULL, 0);
	}

	setPasswdClear();
}

pthread_t launcher_test_id = 0;

void * launcher_test_thread(void *arg)
{
	
	sleep(10);

	while(1)
	{
		APP_CompSendCmd(CMD_APP_COMP_VIDEOSTOP, NULL, 0, NULL, 0);
		sleep(1);
		APP_CompSendCmd(CMD_APP_COMP_RESTART, NULL, 0, NULL, 0);
		sleep(1);
	}
	return NULL;
}

static void launcher_ui_init()
{
    if (!desktop) {
        return;
    }
    
    lv_obj_t * admin_alert = createPasswdBox(NULL, "Please Input Admin Password", validatePassWord);
    lv_obj_set_hidden(admin_alert, true);

    lv_obj_t * img_top_left = lv_img_create(lv_launcher_desktop(), NULL);
    lv_img_set_src(img_top_left, RES_IMG_TOP_LEFT);
    lv_img_set_auto_size(img_top_left, true);
    lv_obj_set_pos(img_top_left, 0, 0);

    lv_obj_t * img_top_mid = lv_img_create(lv_launcher_desktop(), NULL);
    lv_img_set_src(img_top_mid, RES_IMG_TOP_MID);
    lv_img_set_auto_size(img_top_mid, true);
    //lv_obj_set_pos(img_top_mid, (DESKTOP_HOR_RES - 496) / 2, 0);
    lv_obj_align(img_top_mid, NULL, LV_ALIGN_IN_TOP_MID, 20, 0);

    lv_obj_t * img_top_right = lv_img_create(lv_launcher_desktop(), NULL);
    lv_img_set_src(img_top_right, RES_IMG_TOP_RIGHT);
    lv_img_set_auto_size(img_top_right, true);
    lv_obj_set_pos(img_top_right, DESKTOP_HOR_RES - 82, 0);
    lv_obj_set_click(img_top_right, true);
    lv_obj_set_user_data(img_top_right, (lv_obj_user_data_t) admin_alert);
    lv_obj_set_event_cb(img_top_right, menu_btn_event_cb);

    lv_obj_t * img_left = lv_img_create(lv_launcher_desktop(), NULL);
    lv_img_set_src(img_left, RES_IMG_LEFT);
    lv_img_set_auto_size(img_left, true);
    lv_obj_align(img_left, NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);

    lv_obj_t * img_right = lv_img_create(lv_launcher_desktop(), NULL);
    lv_img_set_src(img_right, RES_IMG_RIGHT);
    lv_img_set_auto_size(img_right, true);
    lv_obj_align(img_right, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);

    lv_obj_t * img_bot_left = lv_img_create(lv_launcher_desktop(), NULL);
    lv_img_set_src(img_bot_left, RES_IMG_BOT_LEFT);
    lv_img_set_auto_size(img_bot_left, true);
    lv_obj_set_pos(img_bot_left, 0, DESKTOP_VER_RES - 172);

    lv_obj_t * img_bot_mid = lv_img_create(lv_launcher_desktop(), NULL);
    lv_img_set_src(img_bot_mid, RES_IMG_BOT_MID);
    lv_img_set_auto_size(img_bot_mid, true);
    lv_obj_set_pos(img_bot_mid, (DESKTOP_HOR_RES - 488) / 2, DESKTOP_VER_RES - 28);

    lv_obj_t * img_bot_right = lv_img_create(lv_launcher_desktop(), NULL);
    lv_img_set_src(img_bot_right, RES_IMG_BOT_RIGHT);
    lv_img_set_auto_size(img_bot_right, true);
    lv_obj_set_pos(img_bot_right, DESKTOP_HOR_RES - 118, DESKTOP_VER_RES - 118);

    lv_obj_t * img_bot_link = lv_img_create(lv_launcher_desktop(), NULL);
    lv_img_set_src(img_bot_link, RES_IMG_BOT_LOCAL_LINK_OFF);
    lv_img_set_auto_size(img_bot_link, true);
    lv_obj_set_pos(img_bot_link, DESKTOP_HOR_RES - 108, DESKTOP_VER_RES - 108);

    LV_FONT_DECLARE(stsong_22);
    static lv_style_t title_style;
    lv_style_init(&title_style);
    lv_style_set_text_font(&title_style, LV_STATE_DEFAULT, &stsong_22);
    launcher_title = lv_label_create(img_top_mid, NULL);
    lv_obj_add_style(launcher_title, LV_LABEL_PART_MAIN, &title_style);
    lv_label_set_text(launcher_title, _("Welcome to CviTek"));
    lv_label_set_recolor(launcher_title, true);
    lv_obj_align(launcher_title, NULL, LV_ALIGN_CENTER, 0, 0);

    launcher_init_vol_slider();

	//pthread_create(&launcher_test_id,NULL,launcher_test_thread,NULL);
}

void launcher_hide()
{
}

void launcher_show()
{
    if (scr_launcher) {
        lv_scr_load(scr_launcher);
        launcher_retranslateUI();
    }
}

lv_obj_t * lv_launcher_scr(void)
{
    return scr_launcher;
}

lv_obj_t * lv_launcher_desktop(void)
{
    return desktop;
}

static void launcher_retranslateUI()
{
    lv_label_set_text(launcher_title, _("Welcome to CviTek"));
    lv_obj_realign(launcher_title);
}

#if 0
#define LABEL_OFFSET_X (0)
#define LABEL_OFFSET_Y (20)
void launcher_draw_rect(cvi_face_rect_t *rect_ptr, char *face_name, int16_t x, int16_t y, int16_t width, int16_t height)
{
    if (NULL == rect_ptr)
        return;
    
    rect_ptr->points[0].x = x;
    rect_ptr->points[0].y = y;
    rect_ptr->points[1].x = x + width;
    rect_ptr->points[1].y = y;
    rect_ptr->points[2].x = x + width;
    rect_ptr->points[2].y = y + height;
    rect_ptr->points[3].x = x;
    rect_ptr->points[3].y = y + height;
    rect_ptr->points[4].x = x;
    rect_ptr->points[4].y = y;

    if (rect_ptr->line_ptr == NULL) {
        rect_ptr->line_ptr = lv_line_create(scr_launcher, NULL);
    }
    lv_line_set_points(rect_ptr->line_ptr, rect_ptr->points, 5);

    if (rect_ptr->label_ptr == NULL) {
        rect_ptr->label_ptr = lv_label_create(scr_launcher, NULL);
    }
    lv_obj_set_pos(rect_ptr->label_ptr, x - LABEL_OFFSET_X, y - LABEL_OFFSET_Y);
    lv_style_copy(&facename_label_style, lv_obj_get_style(rect_ptr->label_ptr));
    facename_label_style.text.color = LV_COLOR_RED;
    lv_label_set_style(rect_ptr->label_ptr, LV_LABEL_STYLE_MAIN, &facename_label_style);
    lv_label_set_text(rect_ptr->label_ptr, face_name);
}

void face_rect_del(cvi_face_rect_t *rect_ptr)
{
    lv_obj_del(rect_ptr->line_ptr);
    rect_ptr->line_ptr = NULL;
    lv_obj_del(rect_ptr->label_ptr);
    rect_ptr->label_ptr = NULL;
}
#endif
