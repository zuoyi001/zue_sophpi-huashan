#ifndef MENU_PAGE_H
#define MENU_PAGE_H
#include "lvgl/lvgl.h"

/*menu layout*/
lv_obj_t * menu_page_create_top_panel(lv_obj_t *par, lv_event_cb_t cb);
lv_obj_t * menu_page_create_disp_panel(lv_obj_t * par);
lv_obj_t * menu_page_create_disp_scrollable_panel(lv_obj_t * par);
lv_obj_t * menu_page_create_test_mode_top_panel(lv_obj_t *par);
lv_obj_t * menu_page_create_test_mode_bottom_panel(lv_obj_t *par);
lv_obj_t * menu_page_create_test_mode_disp_panel(lv_obj_t * par);
lv_obj_t * menu_page_create_test2_mode_top_panel(lv_obj_t *par, lv_event_cb_t cb);
lv_obj_t * create_bottom_2btn_panel(lv_obj_t *par, lv_event_cb_t cb0, lv_event_cb_t cb2);
lv_obj_t * create_bottom_2btn_panel2(lv_obj_t *par, lv_obj_t **btn1, lv_obj_t **btn2, lv_event_cb_t cb0, lv_event_cb_t cb2);
lv_obj_t * menu_page_create_bottom_panel(lv_obj_t *par,int x, int y);
lv_obj_t * create_one_btn(lv_obj_t *par, lv_obj_t **btn1,lv_obj_t **label,int x, int y, const char * text, lv_event_cb_t cb);


/*menu section*/
lv_obj_t * menu_section_create(lv_obj_t * par, const char * title_str);
lv_obj_t * menu_section_add_dd_item(lv_obj_t * section, const char * title_str, const char *options, lv_event_cb_t cb);
lv_obj_t * menu_section_add_num_item(lv_obj_t * section, const char * title_str, int def_value, int min, int max, lv_event_cb_t cb);
lv_obj_t * menu_section_add_text_item(lv_obj_t * section, const char * title_str, const char *def_value, lv_event_cb_t cb);
lv_obj_t * menu_section_add_button_item(lv_obj_t * section, const char * title_str, const char * def_value, lv_event_cb_t cb);
lv_obj_t * menu_section_add_slider_item(lv_obj_t * section, const char * title_str, int def_value, lv_event_cb_t cb);

/*menu item*/
lv_obj_t * menu_item_create_basic_item(lv_obj_t * par, const char * title_str);
lv_obj_t * menu_item_create_dropdown_list(lv_obj_t * par, const char * title_str, const char *options, lv_event_cb_t cb);
lv_obj_t * menu_item_create_num_item(lv_obj_t * par, const char * title_str, int def_value, int min, int max, lv_event_cb_t cb);
lv_obj_t * menu_item_create_text_item(lv_obj_t * par, const char * title_str, const char *def_value, lv_event_cb_t cb);
lv_obj_t * menu_item_create_button_item(lv_obj_t * par, const char * title_str, const char * value_str, lv_event_cb_t cb);
lv_obj_t * menu_item_create_slider_item(lv_obj_t * par, const char * title_str, int value, lv_event_cb_t cb);

/*item's styles*/
lv_style_t * menu_page_title_style();
lv_style_t * menu_page_item_style();
lv_style_t * menu_page_item_title_style();
lv_style_t * menu_page_section_title_rgn_style();
lv_style_t * menu_page_section_title_style();
lv_style_t * menu_page_style();
lv_style_t *menu_page_test_mode_title_style();
lv_style_t *menu_page_btn_back_title_style();
lv_style_t *menu_page_btn_label_style();

void menu_page_style_init();

#endif
