#include "lvgl/lvgl.h"
#include "keyboard.h"
#include "ime_kb.h"

static lv_style_t style_kb;
static lv_style_t style_kb_btn;
static void keyboard_event_cb(lv_obj_t * keyboard, lv_event_t event);
//static void ime_keyboard_event_cb(lv_obj_t * keyboard, lv_event_t event);
#if LV_USE_ANIMATION
//static void kb_hide_anim_end(lv_anim_t * a);
//static void ime_kb_hide_anim_end(lv_anim_t * a);
#endif

void keyboard_init()
{
    lv_style_init(&style_kb);
    lv_style_set_bg_opa(&style_kb, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_bg_color(&style_kb, LV_STATE_DEFAULT, lv_color_hex3(0x333));
    lv_style_set_bg_grad_color(&style_kb, LV_STATE_DEFAULT, lv_color_hex3(0x333));
    lv_style_set_pad_left(&style_kb, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_right(&style_kb, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_top(&style_kb, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&style_kb, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_inner(&style_kb, LV_STATE_DEFAULT, 0);

    lv_style_init(&style_kb_btn);
    lv_style_set_bg_opa(&style_kb_btn, LV_STATE_DEFAULT, LV_OPA_70);
    lv_style_set_radius(&style_kb_btn, LV_STATE_DEFAULT, 0);
    lv_style_set_border_width(&style_kb_btn, LV_STATE_DEFAULT, 1);
    lv_style_set_border_color(&style_kb_btn, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    lv_style_set_border_opa(&style_kb_btn, LV_STATE_DEFAULT, LV_OPA_50);
    lv_style_set_bg_color(&style_kb_btn, LV_STATE_DEFAULT, lv_color_hex3(0x333));
    lv_style_set_bg_grad_color(&style_kb_btn, LV_STATE_DEFAULT, lv_color_hex3(0x333));
    lv_style_set_text_color(&style_kb_btn, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_pad_left(&style_kb_btn, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_right(&style_kb_btn, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_top(&style_kb_btn, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_bottom(&style_kb_btn, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_inner(&style_kb_btn, LV_STATE_DEFAULT, 0);

    lv_style_set_bg_opa(&style_kb_btn, LV_STATE_PRESSED, LV_OPA_50);
    lv_style_set_radius(&style_kb_btn, LV_STATE_PRESSED, 0);
    lv_style_set_border_width(&style_kb_btn, LV_STATE_PRESSED, 1);
    lv_style_set_border_color(&style_kb_btn, LV_STATE_PRESSED, LV_COLOR_SILVER);
    lv_style_set_bg_color(&style_kb_btn, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_style_set_bg_grad_color(&style_kb_btn, LV_STATE_PRESSED, LV_COLOR_WHITE);
    lv_style_set_pad_left(&style_kb_btn, LV_STATE_PRESSED, 0);
    lv_style_set_pad_right(&style_kb_btn, LV_STATE_PRESSED, 0);
    lv_style_set_pad_top(&style_kb_btn, LV_STATE_PRESSED, 0);
    lv_style_set_pad_bottom(&style_kb_btn, LV_STATE_PRESSED, 0);
    lv_style_set_pad_inner(&style_kb_btn, LV_STATE_PRESSED, 0);
}

#if 0
lv_obj_t * create_pinyin_keyboard(lv_obj_t **handle, lv_obj_t * text_area, lv_obj_t *parent, lv_coord_t offs_x, lv_coord_t offs_y)
{
    lv_obj_t *kb = ime_kb_create(parent, NULL);
    //lv_obj_set_size(kb, lv_obj_get_width_fit(parent), lv_obj_get_height_fit(parent) / 3);
    ime_kb_set_ta(kb, text_area);
    ime_kb_set_event_cb(kb, ime_keyboard_event_cb);

#if LV_USE_ANIMATION
    lv_anim_t a;
    a.var = kb;
    a.start = LV_VER_RES;
    a.end = lv_obj_get_y(kb);
    a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
    a.path.cb = lv_anim_path_linear;
    a.ready_cb = NULL;
    a.act_time = 0;
    a.time = 300;
    // a.playback = 0;
    // a.playback_pause = 0;
    // a.repeat = 0;
    // a.repeat_pause = 0;
    //lv_anim_start(&a);
#endif

    if(handle) {
        lv_obj_set_user_data(kb, handle);
    }

    ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);
    int h = lv_obj_get_height(lv_obj_get_parent(ext->kb)) / 5;
    int offset = 2;
    int kb_width = offset + lv_obj_get_height(kb) / 5 * 4;
    printf("kb width:%d \n", kb_width);
    
    lv_obj_set_size(ext->kb, lv_obj_get_width(lv_obj_get_parent(ext->kb)), kb_width);
    lv_obj_set_pos(ext->kb, 0, h - offset);
    lv_obj_align(ext->kb, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_obj_align(kb, parent, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    ime_kb_set_style(kb, LV_KB_STYLE_BG, &style_kb);
    ime_kb_set_style(kb, LV_KB_STYLE_BTN_REL, &style_kb_rel);
    ime_kb_set_style(kb, LV_KB_STYLE_BTN_PR, &style_kb_pr);

    return kb;
}

static void ime_keyboard_event_cb(lv_obj_t * keyboard, lv_event_t event)
{
    lv_obj_t * par = lv_obj_get_parent(keyboard);
    ime_kb_def_event_cb(keyboard, event);

    if(event == LV_EVENT_APPLY || event == LV_EVENT_CANCEL) {
#if LV_USE_ANIMATION
        lv_anim_t a;
        a.var = par;
        a.start = lv_obj_get_y(par);
        a.end = LV_VER_RES;
        a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
        a.path_cb = lv_anim_path_linear;
        a.ready_cb = ime_kb_hide_anim_end;
        a.act_time = 0;
        a.time = 300;
        a.playback = 0;
        a.playback_pause = 0;
        a.repeat = 0;
        a.repeat_pause = 0;
        lv_anim_start(&a);
#else
        lv_obj_user_data_t data = lv_obj_get_user_data(par);
        lv_obj_del(par);
        lv_obj_t **handle = (lv_obj_t **)data;
        if (handle) 
            *handle = NULL;
#endif
    }
}

#if LV_USE_ANIMATION
static void ime_kb_hide_anim_end(lv_anim_t * a)
{
    lv_obj_t **handle = (lv_obj_t **) lv_obj_get_user_data(a->var);
    lv_obj_del(a->var);
    if (handle != NULL)
    {
        *handle = NULL;
    }
}
#endif
#endif

lv_obj_t * create_keyboard(lv_obj_t **handle, lv_obj_t * text_area, lv_obj_t *parent, lv_coord_t offs_x, lv_coord_t offs_y)
{
    lv_obj_t *kb = lv_keyboard_create(parent, NULL);
    lv_keyboard_set_cursor_manage(kb, true);
    lv_obj_set_size(kb, lv_obj_get_width_fit(parent), lv_obj_get_height_fit(parent) / 4);
    lv_obj_align(kb, parent, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_keyboard_set_textarea(kb, text_area);
    
    lv_obj_add_style(kb, LV_KEYBOARD_PART_BG, &style_kb);
    lv_obj_add_style(kb, LV_KEYBOARD_PART_BTN, &style_kb_btn);
    lv_obj_set_event_cb(kb, keyboard_event_cb);

    if(handle) {
        lv_obj_set_user_data(kb, handle);
    }

    return kb;
}

static void keyboard_event_cb(lv_obj_t * keyboard, lv_event_t event)
{
    static bool lv_release_flag = false;
    if(false == lv_release_flag && event == LV_EVENT_VALUE_CHANGED) {
        lv_keyboard_def_event_cb(keyboard, event);
        lv_release_flag = true;
    }
    if(event == LV_EVENT_APPLY || event == LV_EVENT_CANCEL) {
        lv_obj_user_data_t data = lv_obj_get_user_data(keyboard);
        lv_obj_del(keyboard);
        lv_keyboard_set_cursor_manage(keyboard, false);
        lv_release_flag = false;
        lv_obj_t **handle = (lv_obj_t **)data;
        if (handle) 
            *handle = NULL;
    }
    if(event == LV_EVENT_RELEASED) {
        lv_release_flag = false;
    }
}

#if LV_USE_ANIMATION
/*
static void kb_hide_anim_end(lv_anim_t * a)
{
    lv_obj_t **handle = (lv_obj_t **) lv_obj_get_user_data(a->var);
    lv_obj_del(a->var);
    if (handle != NULL)
    {
        *handle = NULL;
    }
}
*/
#endif