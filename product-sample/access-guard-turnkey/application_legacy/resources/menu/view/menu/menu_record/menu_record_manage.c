#include "unistd.h"
#include "keyboard.h"
#include "menu_face_lib.h"
#include "resource.h"
#include "lv_i18n.h"
#include "face_add_dialog.h"
#include "face_edit_dialog.h"
#include "face_delete_dialog.h"
#include "menu.h"
#include "menu_common.h"
#include "menu_page.h"
#include "cvi_hal_face_api.h"
#include "cvi_hal_record.h"
#include "public.h"
#include "app_utils.h"


static void menu_record_ui_create();
static void menu_record_ui_destroy();
static void menu_record_ui_show();
static void menu_record_ui_hide();

extern hal_facelib_handle_t facelib_handle;

/*Static var*/
static lv_obj_t * record_lib_page = NULL;
//static page_return_cb s_back_func_cb = NULL;
static lv_obj_t * kb_id = NULL;
static lv_obj_t * par_cont = NULL;
static lv_obj_t * search_label = NULL;
static lv_obj_t * search_box = NULL;
static int current_record_start_index = 0;
static int current_record_end_index = 0;
static int total_record_num = 0;
static cvi_record_t *record_list = NULL;
lv_obj_t *record_disp_panel = NULL;
static const char *search_text = "";
const int load_record_once_limit = 7;

static void load_data();
static void refresh();
//static void edit_record_cb(lv_obj_t *obj, lv_event_t event);
//static void delete_record_cb(lv_obj_t *obj, lv_event_t event);
void create_next_record_list();
int create_record_list(int start_index, int limit, int *end_index);
void menu_record_retranslateUI(void);

static menu_page_t menu_record_page = {
    .onCreate = menu_record_ui_create,
    .onDestroy = menu_record_ui_destroy,
    .show = menu_record_ui_show,
    .hide = menu_record_ui_hide,
    .back = menuPageBack
};

menu_page_t get_menu_record_page()
{
    return menu_record_page;
}

/*Static function*/
#if 0
static void btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    //int id = (int) (obj->user_data);
	switch (event)
	{
	case LV_EVENT_RELEASED:
		break;
	default:
		break;
	}
}
#endif

static void back_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	switch (event)
	{
	case LV_EVENT_RELEASED:
        menuPageBack();
		break;
	default:
		break;
	}
}

static void searchbox_event_cb(lv_obj_t * text_area, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
         if(kb_id == NULL) {
            kb_id = create_keyboard(&kb_id, text_area, par_cont, 0, 0);
        }
    } /*else if (LV_EVENT_DEFOCUSED == event) {
        if (kb != NULL) {
            lv_obj_del(kb);
            kb = NULL;
        }
    }*/
}

static void searchbtn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
        lv_obj_t *textarea = (lv_obj_t *)lv_obj_get_user_data(obj);
        search_text = lv_textarea_get_text(textarea);
        refresh();
    }
}

#if 0
static void add_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
        lv_obj_t *face_info_window = createFaceAddWindow(NULL);
        lv_obj_set_hidden(face_info_window, false);
    }
}
#endif

static bool style_inited = false;
static lv_style_t top_panel_style;
static lv_style_t searchbox_style;
static lv_style_t label_style;
static void _style_init()
{
    if (!style_inited) {
        lv_style_init(&top_panel_style);
        lv_style_set_bg_color(&top_panel_style, LV_STATE_DEFAULT, COLOR_TOP_PANEL);

        lv_style_init(&searchbox_style);
        lv_style_set_text_font(&searchbox_style, LV_STATE_DEFAULT, &stsong_18);

        lv_style_init(&label_style);
        lv_style_set_text_color(&label_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_text_color(&label_style, LV_STATE_PRESSED, LV_COLOR_WHITE);
        lv_style_set_text_font(&label_style, LV_STATE_DEFAULT, &stsong_22);
        style_inited = true;
    }
}

static void create_top_panel(lv_obj_t * par)
{
    //load_data();
    _style_init();

/*
    lv_obj_t * top_panel = lv_obj_create(par, NULL);
    lv_obj_set_size(top_panel, LV_HOR_RES_MAX, TOP_PANEL_H);
    lv_obj_set_pos(top_panel, 0, 0);
    lv_obj_add_style(top_panel, LV_CONT_PART_MAIN, &top_panel_style);
*/
	lv_obj_t * top_panel = menu_page_create_top_panel(par, back_btn_event_cb);

    lv_obj_t *back_img = lv_img_create(top_panel, NULL);
    lv_img_set_src(back_img, RES_IMG_BACK);
    lv_img_set_auto_size(back_img, true);
    lv_obj_set_click(back_img, true);
    lv_obj_align(back_img, NULL, LV_ALIGN_IN_TOP_LEFT, 15, (TOP_PANEL_H - 30 ) / 2);

    search_box = lv_textarea_create(top_panel, NULL);
    lv_textarea_set_one_line(search_box, true);
    lv_obj_set_size(search_box, 300, 30);
    lv_textarea_set_max_length(search_box, 32);
    lv_textarea_set_text_align(search_box, LV_LABEL_ALIGN_LEFT);
    lv_textarea_set_text(search_box, "");
    lv_obj_align(search_box, NULL, LV_ALIGN_IN_TOP_MID, 0, (TOP_PANEL_H - 30) / 2);
    lv_obj_set_event_cb(search_box, searchbox_event_cb);
    lv_obj_add_style(search_box, LV_TEXTAREA_PART_BG, &searchbox_style);

    search_label = lv_label_create(top_panel, NULL);
    lv_label_set_text(search_label, _("Search"));
    lv_obj_set_click(search_label, true);
    lv_obj_align(search_label, NULL, LV_ALIGN_CENTER, 200, 0);
    lv_obj_add_style(search_label, LV_LABEL_PART_MAIN, &label_style);
    lv_obj_set_user_data(search_label, (lv_obj_user_data_t)search_box);
    lv_obj_set_event_cb(search_label, searchbtn_event_cb);

}

static void load_data()
{
    total_record_num = cvi_hal_get_record_list(&record_list, 0, RECORD_CNT);

	printf("[total_record_num:%d %s,%d] \n",total_record_num,__FUNCTION__,__LINE__);
}

static void refresh()
{
    load_data();
    usleep(500 * 1000);
    current_record_start_index = 0;
    current_record_end_index = 0;
    create_next_record_list();
}

static bool has_next_match_record()
{
    bool find = false;
    int i = current_record_end_index;
    for (; i < total_record_num; i++) {
        //cvi_person_t *record = &record_list[i];
        cvi_record_t *record = &record_list[i]; //liang.wang 2021-03-15
        if (strlen(search_text) == 0) {
            find = true;
            break;
        } else {
            char *find_name_pos = strstr(record->name, search_text);
            char *find_serial_pos = strstr(record->serial, search_text);
            if ((find_name_pos != NULL) || (find_serial_pos != NULL)) {
                find = true;
                break;
            }
        }
    }

    return find;
}

static void load_next_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
        if (current_record_end_index >= total_record_num) {
            return;
        }

        if ((strlen(search_text) != 0) && (false == has_next_match_record())) {
            return;
        }

        current_record_start_index = current_record_end_index;
        create_next_record_list();
    }
}

static int find_prev_record_start_index()
{
    int find_record_count = 0;
    int i = current_record_start_index;
    for (; i > 0; i--) {
        if (find_record_count >= load_record_once_limit) {
            break;
        }
        //cvi_person_t *record = &record_list[i];
        cvi_record_t *record = &record_list[i]; //liang.wang 2021-03-15
        if (strlen(search_text) == 0) {
            find_record_count++;
        } else {
            char *find_name_pos = strstr(record->name, search_text);
            char *find_serial_pos = strstr(record->serial, search_text);
            if ((find_name_pos != NULL) || (find_serial_pos != NULL)) {
                find_record_count++;
            }
        }
    }

    return i;
}

static void load_prev_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
        if (current_record_start_index < load_record_once_limit) {
            return;
        }

        current_record_start_index = find_prev_record_start_index();
        if (current_record_start_index < 0) {
            current_record_start_index = 0;
        }
        create_next_record_list();
    }
}

static void create_control_panel(lv_obj_t * par)
{
    lv_obj_t * control_panel = lv_obj_create(par, NULL);
    lv_obj_set_size(control_panel, LV_HOR_RES_MAX, TOP_PANEL_H);
    lv_obj_set_pos(control_panel, 0, LV_VER_RES_MAX - TOP_PANEL_H);
    lv_obj_add_style(control_panel, LV_CONT_PART_MAIN, &top_panel_style);

	lv_obj_t * top_panel1 = lv_obj_create(par, NULL);
    lv_obj_set_size(top_panel1, 200, TOP_PANEL_H);
    lv_obj_set_pos(top_panel1, LV_HOR_RES_MAX -200,  LV_VER_RES_MAX - TOP_PANEL_H);
	lv_obj_set_event_cb(top_panel1, load_next_event_cb);
    lv_obj_add_style(top_panel1, LV_CONT_PART_MAIN, menu_page_btn_back_title_style());
    lv_obj_t *next_label = lv_label_create(top_panel1, NULL);
    lv_obj_align(next_label, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_add_style(next_label, LV_CONT_PART_MAIN, &label_style);
    lv_label_set_text(next_label, "Next >");

	lv_obj_t * top_panel2 = lv_obj_create(par, NULL);
    lv_obj_set_size(top_panel2, 200, TOP_PANEL_H);
    lv_obj_set_pos(top_panel2, 0,  LV_VER_RES_MAX - TOP_PANEL_H);
	lv_obj_set_event_cb(top_panel2, load_prev_event_cb);
    lv_obj_add_style(top_panel2, LV_CONT_PART_MAIN, menu_page_btn_back_title_style());
    lv_obj_t *prev_label = lv_label_create(top_panel2, NULL);
    lv_obj_align(prev_label, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0);
    lv_obj_add_style(prev_label, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_text(prev_label, "Prev <");
}

static void create_display_panel(lv_obj_t * par)
{
    record_disp_panel = menu_page_create_disp_scrollable_panel(par);
}

static char *check_and_create_png(char *img_path)
{
    static char png_path[64];
    strcpy(png_path, img_path);
    char *find_pos = strstr(png_path, ".jpg");
    if (find_pos == NULL) {
        return png_path;
    }

    strncpy(find_pos, ".png", 4);

    FILE* fp = fopen(png_path, "r");
    if (fp == NULL) {
        int width, height;
        //int s32Ret = Cvi_Jpg2PNG(img_path, png_path, &width, &height);
        Cvi_Jpg2PNG(img_path, png_path, &width, &height); //liang.wang 2021-03-15
    }

    return png_path;
}

static void create_record_item(lv_obj_t *parent, cvi_record_t *record)
{
	char aTempBuf[256] ={0};
    const int image_size = 160;
    lv_obj_t * item = lv_cont_create(parent, NULL);
    lv_obj_set_size(item, LV_HOR_RES_MAX, image_size);
    lv_obj_add_style(item, LV_CONT_PART_MAIN, menu_page_item_style());
    lv_obj_set_width(item, lv_obj_get_width(item)-50);

    lv_obj_t *avatar_img = lv_img_create(item, NULL);
    char *avatar_img_path = check_and_create_png(record->image_path);
	
    lv_img_set_src(avatar_img, avatar_img_path);
    int original_img_width = lv_obj_get_width(avatar_img);
    int original_img_height = lv_obj_get_height(avatar_img);
    lv_img_set_zoom(avatar_img, ((float)image_size/original_img_height)*256);
    lv_obj_align(avatar_img, NULL, LV_ALIGN_IN_LEFT_MID, -(original_img_width-image_size)/2, 0);

    lv_obj_t * title = lv_label_create(item, NULL);
    lv_obj_add_style(title, LV_LABEL_PART_MAIN, menu_page_item_title_style());
    lv_label_set_text(title, record->name);
    lv_obj_align(title, NULL, LV_ALIGN_IN_LEFT_MID, image_size + ITEM_TILE_PAD, -20);

    lv_obj_t *serial_label = lv_label_create(item, NULL);
    lv_obj_add_style(serial_label, LV_LABEL_PART_MAIN, menu_page_item_title_style());
    lv_label_set_text(serial_label, record->serial);
    lv_obj_align(serial_label, NULL, LV_ALIGN_IN_LEFT_MID, image_size + ITEM_TILE_PAD, 20);

    lv_obj_t *edit_label = lv_label_create(item, NULL);
    lv_obj_add_style(edit_label, LV_LABEL_PART_MAIN, menu_page_item_title_style());
	lv_label_set_align(edit_label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(edit_label, NULL, LV_ALIGN_IN_TOP_LEFT, 300, 70);
	GetDateFromTime(record->timestamp/1000, aTempBuf);
    lv_label_set_text(edit_label, aTempBuf);
/*
    lv_obj_t *delete_button = lv_btn_create(item, NULL);
    lv_obj_set_width(delete_button, ITEM_VALUE_WIDTH);
    lv_obj_align(delete_button, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);
    lv_obj_set_user_data(delete_button, (lv_obj_user_data_t)record);
    lv_obj_set_event_cb(delete_button, delete_record_cb);
    lv_obj_t *delete_label = lv_label_create(delete_button, NULL);
    lv_obj_add_style(delete_label, LV_LABEL_PART_MAIN, menu_page_item_title_style());
    lv_label_set_text(delete_label, "Delete");
*/
}

void create_next_record_list()
{
    lv_list_clean(record_disp_panel);
    create_record_list(current_record_start_index, load_record_once_limit, &current_record_end_index);
}

int create_record_list(int start_index, int limit, int *end_index)
{
    int loaded_count = 0;
    int i = start_index;
    for (; i < total_record_num; i++) {
        if (loaded_count >= limit) {
            break;
        }

        cvi_record_t *record = &record_list[i];
        if (strlen(search_text) == 0) {
            create_record_item(record_disp_panel, record);
            loaded_count++;
        } else {
            char *find_name_pos = strstr(record->name, search_text);
            char *find_serial_pos = strstr(record->serial, search_text);
            if ((find_name_pos != NULL) || (find_serial_pos != NULL)) {
                create_record_item(record_disp_panel, record);
                loaded_count++;
            }
        }
    }

    *end_index = i;

    return loaded_count;
}

#if 0
static void edit_record_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        cvi_record_t *record = (cvi_record_t *)lv_obj_get_user_data(obj);
        cvi_person_t person; //liang.wang 2021-03-15
        memcpy(&person, record, sizeof(cvi_person_t));
        create_face_edit_dialog(NULL, &person);
    }
}

static void delete_record_cb(lv_obj_t *obj, lv_event_t event)
{
    if (LV_EVENT_CLICKED == event) {
        cvi_record_t *record = (cvi_record_t *)lv_obj_get_user_data(obj);
        cvi_person_t person; //liang.wang 2021-03-15
        memcpy(&person, record, sizeof(cvi_person_t));
        create_face_delete_dialog(NULL, &person);
    }
}
#endif

static void menu_record_ui_create()
{
    lv_obj_t * parent = lv_scr_menu();

    _style_init();

    record_lib_page = lv_obj_create(parent, NULL);
	par_cont = record_lib_page;
	
    lv_obj_set_size(record_lib_page, LV_HOR_RES_MAX, LV_VER_RES_MAX);

    create_top_panel(record_lib_page);
    create_display_panel(record_lib_page);
    create_control_panel(record_lib_page);

    lv_obj_set_hidden(record_lib_page, true);
}

static void menu_record_ui_destroy()
{
    lv_obj_del(record_lib_page);
    record_lib_page = NULL;
    search_label = NULL;
    par_cont = NULL;
	if(record_list != NULL)
	{
		free(record_list);
		record_list = NULL;
	}
}

static void menu_record_ui_show()
{
    if (record_lib_page) {
        menu_record_retranslateUI();
        lv_obj_set_hidden(record_lib_page, false);
        refresh();
    }
}

static void menu_record_ui_hide()
{
    if (record_lib_page) {
        lv_obj_set_hidden(record_lib_page, true);
        search_text = "";
        lv_textarea_set_text(search_box, search_text);
    }
}

void menu_record_retranslateUI(void)
{
    lv_label_set_text(search_label, _("Search"));
    lv_obj_realign(search_label);
}

