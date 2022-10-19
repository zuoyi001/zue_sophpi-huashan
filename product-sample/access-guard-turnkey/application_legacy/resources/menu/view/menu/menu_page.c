#include <stdio.h>
#include "menu_page.h"
#include "menu_common.h"
#include "resource.h"
#include "lv_i18n.h"

static lv_style_t style_sb;
static lv_style_t top_panel_style;
static lv_style_t btn_label_style;
static lv_style_t page_title_style;
static lv_style_t item_title_style;
static lv_style_t item_style;
static lv_style_t section_title_rgn_style;
static lv_style_t section_title_style;
static lv_style_t page_style;
static lv_style_t page_test_mode_title_style;
static lv_style_t page_button_title_style;
static lv_style_t btn_label_bottom_style;
static lv_style_t page_btn_back_title_style;



static bool style_inited = false;
void menu_page_style_init()
{
    if (!style_inited)
    {
        lv_style_init(&style_sb);
        lv_style_set_bg_color(&style_sb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_bg_grad_color(&style_sb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_border_color(&style_sb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_border_width(&style_sb, LV_STATE_DEFAULT, 1);
        lv_style_set_border_opa(&style_sb, LV_STATE_DEFAULT, LV_OPA_70);
        lv_style_set_bg_opa(&style_sb, LV_STATE_DEFAULT, LV_OPA_60);
        lv_style_set_pad_right(&style_sb, LV_STATE_DEFAULT, 3);
        lv_style_set_pad_bottom(&style_sb, LV_STATE_DEFAULT, 3);
        lv_style_set_pad_inner(&style_sb, LV_STATE_DEFAULT, 8);

        lv_style_init(&page_style);
        lv_style_set_bg_color(&page_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);

        lv_style_init(&top_panel_style);
        lv_style_set_bg_color(&top_panel_style, LV_STATE_DEFAULT, COLOR_TOP_PANEL);

        lv_style_init(&btn_label_style);
        lv_style_set_text_font(&btn_label_style, LV_STATE_DEFAULT, &stsong_28);

        lv_style_init(&page_title_style);
        lv_style_set_text_font(&page_title_style, LV_STATE_DEFAULT, &stsong_36);
        lv_style_set_text_color(&page_title_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);

        lv_style_init(&item_title_style);
        lv_style_set_text_font(&item_title_style, LV_STATE_DEFAULT, &stsong_28);
        lv_style_set_text_color(&item_title_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);

        lv_style_init(&item_style);
        lv_style_set_clip_corner(&item_style, LV_STATE_DEFAULT, false);
        lv_style_set_pad_left(&item_style, LV_STATE_DEFAULT, 20);
        lv_style_set_pad_right(&item_style, LV_STATE_DEFAULT, 20);
        lv_style_set_pad_top(&item_style, LV_STATE_DEFAULT, 0);
        lv_style_set_pad_bottom(&item_style, LV_STATE_DEFAULT, 0);
        lv_style_set_pad_inner(&item_style, LV_STATE_DEFAULT, 0);
        lv_style_set_border_side(&item_style, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
        lv_style_set_border_color(&item_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
        lv_style_set_border_color(&item_style, LV_STATE_FOCUSED, LV_COLOR_GRAY);
        lv_style_set_border_width(&item_style, LV_STATE_DEFAULT, 1);

        lv_style_init(&section_title_rgn_style);
        lv_style_set_border_side(&section_title_rgn_style, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
        lv_style_set_border_color(&section_title_rgn_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
        lv_style_set_border_color(&section_title_rgn_style, LV_STATE_FOCUSED, LV_COLOR_GRAY);
        lv_style_set_border_width(&section_title_rgn_style, LV_STATE_DEFAULT, 1);

        lv_style_init(&section_title_style);
        lv_style_set_text_font(&section_title_style, LV_STATE_DEFAULT, &stsong_26);
        lv_style_set_text_color(&section_title_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);

		lv_style_init(&page_test_mode_title_style);
        lv_style_set_text_font(&page_test_mode_title_style, LV_STATE_DEFAULT, &stsong_26);
        lv_style_set_text_color(&page_test_mode_title_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		lv_style_set_bg_color(&page_test_mode_title_style, LV_STATE_DEFAULT, COLOR_TOP_PANEL);

		lv_style_init(&page_btn_back_title_style);
        lv_style_set_text_font(&page_btn_back_title_style, LV_STATE_DEFAULT, &stsong_26);
        lv_style_set_text_color(&page_btn_back_title_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		lv_style_set_bg_color(&page_btn_back_title_style, LV_STATE_DEFAULT, COLOR_TOP_PANEL);
		lv_style_set_bg_color(&page_btn_back_title_style, LV_STATE_PRESSED, COLOR_GREEN);

		lv_style_init(&page_button_title_style);
        lv_style_set_text_font(&page_button_title_style, LV_STATE_DEFAULT, &stsong_28);
        lv_style_set_text_color(&page_button_title_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		lv_style_set_bg_color(&page_button_title_style, LV_STATE_DEFAULT, COLOR_GREEN);
		lv_style_set_bg_color(&page_button_title_style, LV_STATE_PRESSED, COLOR_RED);

		lv_style_init(&btn_label_bottom_style);
		lv_style_set_text_color(&btn_label_bottom_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
		lv_style_set_text_color(&btn_label_bottom_style, LV_STATE_PRESSED, LV_COLOR_WHITE);
		lv_style_set_text_font(&btn_label_bottom_style, LV_STATE_DEFAULT, &stsong_28);

        style_inited = true;
    }
}

lv_obj_t * menu_page_create_top_panel(lv_obj_t *par, lv_event_cb_t cb)
{
    lv_obj_t * top_panel = lv_obj_create(par, NULL);
    lv_obj_set_size(top_panel, LV_HOR_RES_MAX, TOP_PANEL_H);
    lv_obj_set_pos(top_panel, 0, 0);
    lv_obj_add_style(top_panel, LV_CONT_PART_MAIN, &top_panel_style);

	lv_obj_t * top_panel2 = lv_obj_create(par, NULL);
    lv_obj_set_size(top_panel2, 200, TOP_PANEL_H);
    lv_obj_set_pos(top_panel2, 0, 0);
	lv_obj_set_event_cb(top_panel2, cb);
    lv_obj_add_style(top_panel2, LV_CONT_PART_MAIN, &page_btn_back_title_style);

    lv_obj_t *back_img = lv_img_create(top_panel2, NULL);
    lv_img_set_src(back_img, RES_IMG_BACK);
    lv_img_set_auto_size(back_img, true);
    lv_obj_set_click(back_img, true);
    lv_obj_align(back_img, NULL, LV_ALIGN_IN_TOP_LEFT, 30, (TOP_PANEL_H - 30 ) / 2);
    //lv_obj_set_event_cb(back_img, cb);

    return top_panel;
}

lv_obj_t * create_one_btn(lv_obj_t *par, lv_obj_t **btn1,lv_obj_t **label,int x, int y, const char * text, lv_event_cb_t cb)
{
	lv_obj_t * bottom1_panel = menu_page_create_bottom_panel(par,x, y);
	lv_obj_set_event_cb(bottom1_panel, cb);

	lv_obj_t *next_label = lv_label_create(bottom1_panel, NULL);
    //lv_obj_set_click(next_label, true);
	lv_obj_set_size(next_label, 200, TOP_TEST_MODE_PANEL_H);
    lv_obj_align(next_label, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
    lv_obj_add_style(next_label, LV_LABEL_PART_MAIN, &btn_label_bottom_style);
    lv_label_set_text(next_label, text);

	if(btn1 != NULL)
	{
		*btn1 = bottom1_panel;
	}

	if(label != NULL)
	{
		*label = next_label;
	}
	return bottom1_panel;
}


lv_obj_t * create_bottom_2btn_panel(lv_obj_t *par, lv_event_cb_t cb0, lv_event_cb_t cb2)
{
	lv_obj_t * bottom_panel = menu_page_create_test_mode_bottom_panel(par);

	lv_obj_t * bottom1_panel = menu_page_create_bottom_panel(par,0, LV_VER_RES_MAX-TOP_TEST_MODE_PANEL_H);
	lv_obj_set_event_cb(bottom1_panel, cb2);

	lv_obj_t * bottom2_panel = menu_page_create_bottom_panel(par,2*(LV_HOR_RES_MAX/3), LV_VER_RES_MAX-TOP_TEST_MODE_PANEL_H);
	lv_obj_set_event_cb(bottom2_panel, cb0);

	lv_obj_t *next_label = lv_label_create(bottom2_panel, NULL);
    //lv_obj_set_click(next_label, true);
	lv_obj_set_size(next_label, 200, TOP_TEST_MODE_PANEL_H);
    lv_obj_align(next_label, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
    //lv_obj_set_event_cb(next_label, cb0);
    lv_obj_add_style(next_label, LV_LABEL_PART_MAIN, &btn_label_bottom_style);
    lv_label_set_text(next_label, _("pass"));

    lv_obj_t *prev_label = lv_label_create(bottom1_panel, NULL);
    //lv_obj_set_click(prev_label, true);
	lv_obj_set_size(prev_label, 200, TOP_TEST_MODE_PANEL_H);
    lv_obj_align(prev_label, NULL, LV_ALIGN_IN_LEFT_MID, 100, 0);
    //lv_obj_set_event_cb(prev_label, cb2);
    lv_obj_add_style(prev_label, LV_LABEL_PART_MAIN, &btn_label_bottom_style);
    lv_label_set_text(prev_label, _("fail"));

	return bottom_panel;
}

lv_obj_t * create_bottom_2btn_panel2(lv_obj_t *par, lv_obj_t **btn1, lv_obj_t **btn2, lv_event_cb_t cb0, lv_event_cb_t cb2)
{
	lv_obj_t * bottom_panel = menu_page_create_test_mode_bottom_panel(par);

	lv_obj_t * bottom1_panel = menu_page_create_bottom_panel(par,0, LV_VER_RES_MAX-TOP_TEST_MODE_PANEL_H);
	lv_obj_set_event_cb(bottom1_panel, cb2);

	lv_obj_t * bottom2_panel = menu_page_create_bottom_panel(par,2*(LV_HOR_RES_MAX/3), LV_VER_RES_MAX-TOP_TEST_MODE_PANEL_H);
	lv_obj_set_event_cb(bottom2_panel, cb0);

	lv_obj_t *next_label = lv_label_create(bottom2_panel, NULL);
	lv_obj_set_size(next_label, 200, TOP_TEST_MODE_PANEL_H);
    lv_obj_align(next_label, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);
    lv_obj_add_style(next_label, LV_LABEL_PART_MAIN, &btn_label_bottom_style);
    lv_label_set_text(next_label, _("pass"));

    lv_obj_t *prev_label = lv_label_create(bottom1_panel, NULL);
	lv_obj_set_size(prev_label, 200, TOP_TEST_MODE_PANEL_H);
    lv_obj_align(prev_label, NULL, LV_ALIGN_IN_LEFT_MID, 100, 0);
    lv_obj_add_style(prev_label, LV_LABEL_PART_MAIN, &btn_label_bottom_style);
    lv_label_set_text(prev_label, _("fail"));

	if(btn1 != NULL)
	{
		*btn1 = bottom1_panel;
	}
	if(btn2 != NULL)
	{
		*btn2 = bottom2_panel;
	}

	return bottom_panel;
}


lv_obj_t * menu_page_create_test_mode_top_panel(lv_obj_t *par)
{
    lv_obj_t * top_panel = lv_obj_create(par, NULL);
    lv_obj_set_size(top_panel, LV_HOR_RES_MAX, TOP_TEST_MODE_PANEL_H);
    lv_obj_set_pos(top_panel, 0, 0);
    lv_obj_add_style(top_panel, LV_CONT_PART_MAIN, &page_test_mode_title_style);
	

    //lv_obj_t *back_img = lv_img_create(top_panel, NULL);
    //lv_img_set_src(back_img, RES_IMG_BACK);
    //lv_img_set_auto_size(back_img, true);
    //lv_obj_set_click(back_img, true);
    //lv_obj_align(back_img, NULL, LV_ALIGN_IN_TOP_LEFT, 30, 0/*(TOP_PANEL_H - 30 ) / 2*/);
    //lv_obj_set_event_cb(back_img, cb);

    return top_panel;
}

lv_obj_t * menu_page_create_test2_mode_top_panel(lv_obj_t *par, lv_event_cb_t cb)
{
    lv_obj_t * top_panel = lv_obj_create(par, NULL);
    lv_obj_set_size(top_panel, LV_HOR_RES_MAX, TOP_TEST_MODE_PANEL_H);
    lv_obj_set_pos(top_panel, 0, 0);
    lv_obj_add_style(top_panel, LV_CONT_PART_MAIN, &page_test_mode_title_style);

	lv_obj_t * top_panel2 = lv_obj_create(par, NULL);
    lv_obj_set_size(top_panel2, 200, TOP_TEST_MODE_PANEL_H);
    lv_obj_set_pos(top_panel2, 0, 0);
	lv_obj_set_event_cb(top_panel2, cb);
    lv_obj_add_style(top_panel2, LV_CONT_PART_MAIN, &page_btn_back_title_style);

    lv_obj_t *back_img = lv_img_create(top_panel2, NULL);
    lv_img_set_src(back_img, RES_IMG_BACK);
    lv_img_set_auto_size(back_img, true);
    //lv_obj_set_click(back_img, true);
    lv_obj_align(back_img, NULL, LV_ALIGN_IN_TOP_LEFT, 50, (TOP_TEST_MODE_PANEL_H - 30 ) / 2);
    //lv_obj_set_event_cb(back_img, cb);

    return top_panel;
}


lv_obj_t * menu_page_create_bottom_panel(lv_obj_t *par,int x, int y)
{
    lv_obj_t * bottom_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom_panel, LV_HOR_RES_MAX/3, TOP_TEST_MODE_PANEL_H);
	lv_obj_set_pos(bottom_panel, x, y);
	lv_obj_add_style(bottom_panel, LV_CONT_PART_MAIN, &page_button_title_style);

    return bottom_panel;
}

lv_obj_t * menu_page_create_test_mode_bottom_panel(lv_obj_t *par)
{
    lv_obj_t * bottom_panel = lv_obj_create(par, NULL);
	lv_obj_set_size(bottom_panel, LV_HOR_RES_MAX, TOP_TEST_MODE_PANEL_H);
	lv_obj_set_pos(bottom_panel, 0, LV_VER_RES_MAX-TOP_TEST_MODE_PANEL_H);
	lv_obj_add_style(bottom_panel, LV_CONT_PART_MAIN, &page_test_mode_title_style);

    return bottom_panel;
}

lv_obj_t * menu_page_create_disp_panel(lv_obj_t * par)
{
    lv_obj_t *disp_panel = lv_cont_create(par, NULL);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX, LV_VER_RES_MAX - TOP_TEST_MODE_PANEL_H);
    lv_obj_set_pos(disp_panel, 0, TOP_TEST_MODE_PANEL_H);
    lv_cont_set_layout(disp_panel, LV_LAYOUT_COLUMN_LEFT);

    return disp_panel;
}
lv_obj_t * menu_page_create_test_mode_disp_panel(lv_obj_t * par)
{
    lv_obj_t *disp_panel = lv_cont_create(par, NULL);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_obj_set_pos(disp_panel, 0, TOP_TEST_MODE_PANEL_H);
    //lv_cont_set_layout(disp_panel, LV_LAYOUT_COLUMN_MID);

    return disp_panel;
}

lv_obj_t * menu_page_create_disp_scrollable_panel(lv_obj_t * par)
{
    lv_obj_t *page = lv_page_create(par, NULL);
    lv_obj_set_size(page, LV_HOR_RES_MAX, LV_VER_RES_MAX - TOP_PANEL_H);
    lv_obj_set_pos(page, 0, TOP_PANEL_H);
    lv_page_set_scrollbar_mode(page, LV_SCRLBAR_MODE_AUTO);

    lv_obj_t *disp_panel = lv_list_create(page, NULL);
    lv_cont_set_fit2(disp_panel, LV_FIT_PARENT, LV_FIT_TIGHT);
    lv_cont_set_layout(disp_panel, LV_LAYOUT_COLUMN_LEFT);
    lv_list_set_scroll_propagation(disp_panel, true);
    lv_obj_set_drag(disp_panel, true);
    lv_page_glue_obj(disp_panel, true);

    return disp_panel;
}

lv_style_t *menu_page_title_style()
{
    return &page_title_style;
}

lv_style_t *menu_page_item_title_style()
{
    return &item_title_style;
}

lv_style_t *menu_page_item_style()
{
    return &item_style;
}

lv_style_t *menu_page_section_title_rgn_style()
{
    return &section_title_rgn_style;
}

lv_style_t *menu_page_section_title_style()
{
    return &section_title_style;
}

lv_style_t *menu_page_style()
{
    return &page_style;
}
lv_style_t *menu_page_test_mode_title_style()
{
    return &page_test_mode_title_style;
}

lv_style_t *menu_page_btn_back_title_style()
{
    return &page_btn_back_title_style;
}
lv_style_t *menu_page_btn_label_style()
{
    return &btn_label_style;
}

/*menu item related*/
lv_obj_t * menu_item_create_basic_item(lv_obj_t * par, const char * title_str)
{
    lv_obj_t * cont = lv_cont_create(par, NULL);
    lv_obj_set_size(cont, LV_HOR_RES_MAX, MENU_ITEM_H);
    lv_obj_add_style(cont, LV_CONT_PART_MAIN, menu_page_item_style());
    lv_obj_set_drag(cont, true);
    lv_page_glue_obj(cont, true);

    lv_obj_t * title = lv_label_create(cont, NULL);
    lv_obj_add_style(title, LV_LABEL_PART_MAIN, menu_page_item_title_style());
    lv_label_set_text(title, title_str);
    lv_obj_align(title, NULL, LV_ALIGN_IN_LEFT_MID, ITEM_TILE_PAD, 0);

    return cont;
}

lv_obj_t * menu_item_create_dropdown_list(lv_obj_t * par, const char * title_str, const char *options, lv_event_cb_t cb)
{
    lv_obj_t * item = menu_item_create_basic_item(par, title_str);

    lv_obj_t * dropdown = lv_dropdown_create(item, NULL);
    lv_dropdown_set_options(dropdown, options);
    lv_dropdown_set_draw_arrow(dropdown, true);
    lv_obj_set_width(dropdown, ITEM_DROPDOWN_WIDTH);
    lv_obj_align(dropdown, NULL, LV_ALIGN_IN_RIGHT_MID, -50, 0);
    if (cb) {
        lv_obj_set_event_cb(dropdown, cb);
    }

    return dropdown;
}

lv_obj_t * menu_item_create_num_item(lv_obj_t * par, const char * title_str, int def_value, int min, int max, lv_event_cb_t cb)
{
    lv_obj_t * item = menu_item_create_basic_item(par, title_str);

    lv_obj_t * ta = lv_textarea_create(item, NULL);
    lv_textarea_set_max_length(ta, 5);
    lv_textarea_set_accepted_chars(ta, "0123456789");
    lv_textarea_set_one_line(ta, true);
    lv_obj_set_width(ta, ITEM_VALUE_WIDTH);
    lv_textarea_set_cursor_hidden(ta, true);
    char def_str[8] = {0};
    snprintf(def_str, sizeof(def_str), "%d", def_value);
    lv_textarea_set_text(ta, def_str);
    lv_obj_align(ta, NULL, LV_ALIGN_IN_RIGHT_MID, -50, 0);
    if (cb) {
        lv_obj_set_event_cb(ta, cb);
    }

    return ta;
}

lv_obj_t * menu_item_create_text_item(lv_obj_t * par, const char * title_str, const char *def_value, lv_event_cb_t cb)
{
    lv_obj_t * item = menu_item_create_basic_item(par, title_str);

    lv_obj_t * ta = lv_textarea_create(item, NULL);
    lv_textarea_set_max_length(ta, 40);
    lv_textarea_set_one_line(ta, true);
    lv_obj_set_width(ta, 2*ITEM_VALUE_WIDTH);
    lv_textarea_set_text(ta, def_value);
    lv_textarea_set_cursor_hidden(ta, true);
    lv_obj_align(ta, NULL, LV_ALIGN_IN_RIGHT_MID, -50, 0);
    if (cb) {
        lv_obj_set_event_cb(ta, cb);
    }

    return ta;
}

lv_obj_t * menu_item_create_button_item(lv_obj_t * par, const char * title_str, const char * value_str, lv_event_cb_t cb)
{
    lv_obj_t * item = menu_item_create_basic_item(par, title_str);

    lv_obj_t * button = lv_btn_create(item, NULL);
    lv_obj_set_width(button, ITEM_VALUE_WIDTH);
    lv_obj_align(button, NULL, LV_ALIGN_IN_RIGHT_MID, -50, 0);
    if (cb) {
        lv_obj_set_event_cb(button, cb);
    }
    lv_obj_t * label;
    label = lv_label_create(button, NULL);
    lv_obj_add_style(label, LV_LABEL_PART_MAIN, menu_page_item_title_style());
    lv_label_set_text(label, value_str);

    return button;
}

lv_obj_t * menu_item_create_slider_item(lv_obj_t * par, const char * title_str, int value, lv_event_cb_t cb)
{
    lv_obj_t * item = menu_item_create_basic_item(par, title_str);

    lv_obj_t * slider = lv_slider_create(item, NULL);
    lv_slider_set_value(slider, value, LV_ANIM_OFF);
    lv_obj_set_width(slider, ITEM_VALUE_WIDTH);
    lv_obj_align(slider, NULL, LV_ALIGN_IN_RIGHT_MID, -50, 0);
    if (cb) {
        lv_obj_set_event_cb(slider, cb);
    }

    return slider;
}

lv_obj_t * menu_section_create(lv_obj_t * par, const char * title_str)
{
    lv_obj_t * cont = lv_cont_create(par, NULL);
    lv_obj_set_size(cont, LV_HOR_RES_MAX, MENU_SEC_HEADER_H);
    lv_cont_set_layout(cont, LV_LAYOUT_COLUMN_LEFT);
    lv_obj_set_drag(cont, true);
    lv_page_glue_obj(cont, true);

    lv_obj_t * rgn = lv_cont_create(cont, NULL);
    lv_obj_set_size(rgn, LV_HOR_RES_MAX, MENU_SEC_HEADER_H);
    lv_obj_add_style(rgn, LV_CONT_PART_MAIN, menu_page_section_title_rgn_style());

    lv_obj_t * title = lv_label_create(rgn, NULL);
    lv_obj_add_style(title, LV_LABEL_PART_MAIN, menu_page_item_title_style());
    lv_label_set_text(title, title_str);
    lv_obj_align(title, NULL, LV_ALIGN_IN_LEFT_MID, ITEM_TILE_PAD, 0);
    lv_obj_add_style(title, LV_LABEL_PART_MAIN, menu_page_section_title_style());

    return cont;
}

lv_obj_t * menu_section_add_dd_item(lv_obj_t * section, const char * title_str, const char *options, lv_event_cb_t cb)
{
    lv_coord_t height_org = lv_obj_get_height(section);
    lv_obj_set_height(section, height_org + MENU_ITEM_H);
    return menu_item_create_dropdown_list(section, title_str, options, cb);
}

lv_obj_t * menu_section_add_num_item(lv_obj_t * section, const char * title_str, int def_value, int min, int max, lv_event_cb_t cb)
{
    lv_coord_t height_org = lv_obj_get_height(section);
    lv_obj_set_height(section, height_org + MENU_ITEM_H);
    return menu_item_create_num_item(section, title_str, def_value, min, max, cb);
}

lv_obj_t * menu_section_add_text_item(lv_obj_t * section, const char * title_str, const char *def_value, lv_event_cb_t cb)
{
    lv_coord_t height_org = lv_obj_get_height(section);
    lv_obj_set_height(section, height_org + MENU_ITEM_H);
    return menu_item_create_text_item(section, title_str, def_value, cb);
}

lv_obj_t * menu_section_add_button_item(lv_obj_t * section, const char * title_str, const char * def_value, lv_event_cb_t cb)
{
    lv_coord_t height_org = lv_obj_get_height(section);
    lv_obj_set_height(section, height_org + MENU_ITEM_H);
    return menu_item_create_button_item(section, title_str, def_value, cb);
}

lv_obj_t * menu_section_add_slider_item(lv_obj_t * section, const char * title_str, int def_value, lv_event_cb_t cb)
{
    lv_coord_t height_org = lv_obj_get_height(section);
    lv_obj_set_height(section, height_org + MENU_ITEM_H);
    return menu_item_create_slider_item(section, title_str, def_value, cb);
}
