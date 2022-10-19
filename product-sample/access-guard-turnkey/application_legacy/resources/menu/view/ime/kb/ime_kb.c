
/**
 * @file ime_kb.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "ime_kb.h"
#include "ime.h"
#if 0 //LV_USE_KB != 0

//#include "lv_debug.h"
//#include "lv_theme.h"
//#include "lv_ta.h"

/*********************
 *      DEFINES
 *********************/
#define LV_OBJX_NAME "ime_kb"

#define IME_KB_CTRL_BTN_FLAGS (LV_BTNMATRIX_CTRL_NO_REPEAT | LV_BTNMATRIX_CTRL_CLICK_TRIG)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_res_t ime_kb_signal(lv_obj_t * kb, lv_signal_t sign, void * param);
static void updatePiyinSearch(lv_obj_t * kb, char ch);
static void updatePiyinSearchtxt(lv_obj_t * kb, char* txt, int len);
static void reset_pinyin();
static void clearSpellCand(lv_obj_t * kb);

static char spell_buffer[128+1] = {0};
static int spell_char_num = 0;
static lv_style_t spell_label_style;
static lv_style_t cand_label_style;

#define CAND_LABEL_OFFSET (5)

LV_FONT_DECLARE(stsong_22);
LV_FONT_DECLARE(stsong_18);
LV_FONT_DECLARE(stsong_16);
/**********************
 *  STATIC VARIABLES
 **********************/
static lv_signal_cb_t ancestor_signal;
/* clang-format off */
static const char * kb_map_ch[] = {"1#", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", LV_SYMBOL_BACKSPACE, "\n",
                                   "En", "a", "s", "d", "f", "g", "h", "j", "k", "l", LV_SYMBOL_NEW_LINE, "\n",
                                   "_", "-", "z", "x", "c", "v", "b", "n", "m", ".", ",", ":", "\n",
                                   LV_SYMBOL_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, ""};

static const lv_btnmatrix_ctrl_t kb_ctrl_ch_map[] = {
    IME_KB_CTRL_BTN_FLAGS | 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7,
    IME_KB_CTRL_BTN_FLAGS | 6, 3, 3, 3, 3, 3, 3, 3, 3, 3, 7,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    IME_KB_CTRL_BTN_FLAGS | 2, 2, 6, 2, IME_KB_CTRL_BTN_FLAGS | 2};

static const char * kb_map_lc[] = {"1#", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", LV_SYMBOL_BACKSPACE, "\n",
                                   "Ch", "ABC", "a", "s", "d", "f", "g", "h", "j", "k", "l", LV_SYMBOL_NEW_LINE, "\n",
                                   "_", "-", "z", "x", "c", "v", "b", "n", "m", ".", ",", ":", "\n",
                                   LV_SYMBOL_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, ""};

static const lv_btnmatrix_ctrl_t kb_ctrl_lc_map[] = {
    IME_KB_CTRL_BTN_FLAGS | 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7,
    IME_KB_CTRL_BTN_FLAGS | 6, IME_KB_CTRL_BTN_FLAGS | 6, 3, 3, 3, 3, 3, 3, 3, 3, 3, 7,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    IME_KB_CTRL_BTN_FLAGS | 2, 2, 6, 2, IME_KB_CTRL_BTN_FLAGS | 2};

static const char * kb_map_uc[] = {"1#", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", LV_SYMBOL_BACKSPACE, "\n",
                                   "Ch", "abc", "A", "S", "D", "F", "G", "H", "J", "K", "L", LV_SYMBOL_NEW_LINE, "\n",
                                    "_", "-", "Z", "X", "C", "V", "B", "N", "M", ".", ",", ":", "\n",
                                   LV_SYMBOL_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, ""};

static const lv_btnmatrix_ctrl_t kb_ctrl_uc_map[] = {
    IME_KB_CTRL_BTN_FLAGS | 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7,
    IME_KB_CTRL_BTN_FLAGS | 6, IME_KB_CTRL_BTN_FLAGS | 6, 3, 3, 3, 3, 3, 3, 3, 3, 3, 7,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    IME_KB_CTRL_BTN_FLAGS | 2, 2, 6, 2, IME_KB_CTRL_BTN_FLAGS | 2};

static const char * kb_map_spec[] = {"0", "1", "2", "3", "4" ,"5", "6", "7", "8", "9", LV_SYMBOL_BACKSPACE, "\n",
                                     "abc", "+", "-", "/", "*", "=", "%", "!", "?", "#", "<", ">", "\n",
                                     "\\",  "@", "$", "(", ")", "{", "}", "[", "]", ";", "\"", "'", "\n",
                                     LV_SYMBOL_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, ""};

static const lv_btnmatrix_ctrl_t kb_ctrl_spec_map[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, IME_KB_CTRL_BTN_FLAGS | 2,
    IME_KB_CTRL_BTN_FLAGS | 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    IME_KB_CTRL_BTN_FLAGS | 2, 2, 6, 2, IME_KB_CTRL_BTN_FLAGS | 2};

static const char * kb_map_num[] = {"1", "2", "3", LV_SYMBOL_CLOSE, "\n",
                                    "4", "5", "6", LV_SYMBOL_OK, "\n",
                                    "7", "8", "9", LV_SYMBOL_BACKSPACE, "\n",
                                    "+/-", "0", ".", LV_SYMBOL_LEFT, LV_SYMBOL_RIGHT, ""};

static const lv_btnmatrix_ctrl_t kb_ctrl_num_map[] = {
        1, 1, 1, IME_KB_CTRL_BTN_FLAGS | 2,
        1, 1, 1, IME_KB_CTRL_BTN_FLAGS | 2,
        1, 1, 1, 2,
        1, 1, 1, 1, 1};
/* clang-format on */

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a keyboard objects
 * @param par pointer to an object, it will be the parent of the new keyboard
 * @param copy pointer to a keyboard object, if not NULL then the new object will be copied from it
 * @return pointer to the created keyboard
 */
lv_obj_t * ime_kb_create(lv_obj_t * par, const lv_obj_t * copy)
{
    reset_pinyin();
    LV_LOG_TRACE("keyboard create started");
    /*create parent container*/
    lv_obj_t * container = lv_cont_create(par, NULL);
    lv_obj_set_size(container, lv_obj_get_width(lv_obj_get_parent(container)),
                        lv_obj_get_height(lv_obj_get_parent(container)) / 3);
    //lv_cont_set_layout(container, LV_LAYOUT_COL_L);

    lv_obj_t * spell = lv_label_create(container, NULL);
    lv_obj_align(spell, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_obj_set_size(spell, lv_obj_get_width(container), lv_obj_get_height(container)/5/2);
    lv_label_set_text(spell, "");

    lv_obj_t * cand = lv_cont_create(container, NULL);
    lv_obj_align(cand, NULL, LV_ALIGN_IN_TOP_LEFT, 0, lv_obj_get_height(container)/5/2);
    lv_obj_set_size(cand, lv_obj_get_width(container), lv_obj_get_height(container)/5/2);

    /*Create the ancestor of keyboard*/
    lv_obj_t * new_kb = lv_btnm_create(container, NULL);
    LV_ASSERT_MEM(new_kb);
    if(new_kb == NULL) return NULL;

    if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(container);

    /*Allocate the keyboard type specific extended data*/
    ime_kb_ext_t * ext2 = lv_obj_allocate_ext_attr(new_kb, sizeof(ime_kb_ext_t));
    LV_ASSERT_MEM(ext2);
    if(ext2 == NULL) return NULL;
    /*Initialize the allocated 'ext' */
    ext2->kb         = new_kb;
    ext2->spell      = spell;
    ext2->cand       = cand;
    ext2->ta         = NULL;
    ext2->mode       = IME_KB_MODE_CH;
    ext2->cursor_mng = 0;

    ime_kb_ext_t * ext = lv_obj_allocate_ext_attr(container, sizeof(ime_kb_ext_t));
    LV_ASSERT_MEM(ext);
    if(ext == NULL) return NULL;
    memcpy(ext, ext2, sizeof(ime_kb_ext_t));

    /*The signal and design functions are not copied so set them here*/
    lv_obj_set_signal_cb(container, ime_kb_signal);

    /*Init the new keyboard keyboard*/
    /* Set a size which fits into the parent.
        * Don't use `par` directly because if the window is created on a page it is moved to the
        * scrollable so the parent has changed */
    //move here avoid crash 
    lv_btnm_set_map(new_kb, kb_map_ch);
    lv_btnm_set_ctrl_map(new_kb, kb_ctrl_ch_map);
    //lv_obj_set_size(new_kb, lv_obj_get_width_fit(lv_obj_get_parent(new_kb)),
    //                    lv_obj_get_height_fit(lv_obj_get_parent(new_kb)) / 5 * 4);
    lv_obj_align(new_kb, container, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    ime_kb_set_event_cb(container, ime_kb_def_event_cb);
    lv_obj_set_base_dir(new_kb, LV_BIDI_DIR_LTR);

    LV_LOG_INFO("keyboard created");

    return container;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Assign a Text Area to the Keyboard. The pressed characters will be put there.
 * @param kb pointer to a Keyboard object
 * @param ta pointer to a Text Area object to write there
 */
void ime_kb_set_ta(lv_obj_t * kb, lv_obj_t * ta)
{
    LV_ASSERT_OBJ(kb, LV_OBJX_NAME);
    if(ta) LV_ASSERT_OBJ(ta, "lv_ta");

    ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);
    ime_kb_ext_t * ext_real = lv_obj_get_ext_attr(ext->kb);

    /*Hide the cursor of the old Text area if cursor management is enabled*/
    if(ext->ta && ext->cursor_mng) {
        lv_textarea_set_cursor_hidden(ext->ta, true);
    }

    ext->ta = ta;
    ext_real->ta = ta;

    /*Show the cursor of the new Text area if cursor management is enabled*/
    if(ext->ta && ext->cursor_mng) {
        // lv_ta_set_cursor_type(ext->ta, cur_type & (~LV_CURSOR_HIDDEN));
        lv_textarea_set_cursor_hidden(ext->ta, false);
    }
}

/**
 * Set a new a mode (text or number map)
 * @param kb pointer to a Keyboard object
 * @param mode the mode from 'ime_kb_mode_t'
 */
void ime_kb_set_mode(lv_obj_t * kb, ime_kb_mode_t mode)
{
    LV_ASSERT_OBJ(kb, LV_OBJX_NAME);

    ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);
    if(ext->mode == mode) return;

    ime_kb_ext_t * ext2 = lv_obj_get_ext_attr(ext->kb);

    ext->mode = mode;
    ext2->mode = mode;
    if(mode == IME_KB_MODE_TEXT) {
        lv_btnm_set_map(ext->kb, kb_map_lc);
        lv_btnm_set_ctrl_map(ext->kb, kb_ctrl_lc_map);
    } else if(mode == IME_KB_MODE_NUM) {
        lv_btnm_set_map(ext->kb, kb_map_num);
        lv_btnm_set_ctrl_map(ext->kb, kb_ctrl_num_map);
    } else if(mode == IME_KB_MODE_TEXT_UPPER) {
        lv_btnm_set_map(ext->kb, kb_map_uc);
        lv_btnm_set_ctrl_map(ext->kb, kb_ctrl_uc_map);
    } else if(mode == IME_KB_MODE_SPEC) {
        lv_btnm_set_map(ext->kb, kb_map_spec);
        lv_btnm_set_ctrl_map(ext->kb, kb_ctrl_spec_map);
    }else if(mode == IME_KB_MODE_CH) {
        lv_btnm_set_map(ext->kb, kb_map_ch);
        lv_btnm_set_ctrl_map(ext->kb, kb_ctrl_ch_map);
    }
}

/**
 * Automatically hide or show the cursor of Text Area
 * @param kb pointer to a Keyboard object
 * @param en true: show cursor on the current text area, false: hide cursor
 */
void ime_kb_set_cursor_manage(lv_obj_t * kb, bool en)
{
    LV_ASSERT_OBJ(kb, LV_OBJX_NAME);

    ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);
    if(ext->cursor_mng == en) return;

    ext->cursor_mng = en == false ? 0 : 1;

    if(ext->ta) {
        // lv_cursor_type_t cur_type;
        // cur_type = lv_ta_get_cursor_type(ext->ta);

        if(ext->cursor_mng) {
            lv_textarea_set_cursor_hidden(ext->ta, false);
        } else {
            lv_textarea_set_cursor_hidden(ext->ta, true);
        }
    }
}

/**
 * Set a style of a keyboard
 * @param kb pointer to a keyboard object
 * @param type which style should be set
 * @param style pointer to a style
 */
void ime_kb_set_style(lv_obj_t * kb, ime_kb_style_t type, const lv_style_t * style)
{
    LV_ASSERT_OBJ(kb, LV_OBJX_NAME);
    ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);

    switch(type) {
        case IME_KB_STYLE_BG: 
            lv_btnm_set_style(ext->kb, LV_BTNMATRIX_PART_BG, style);
            break;
        case IME_KB_STYLE_BTN_REL: 
            lv_btnm_set_style(ext->kb, LV_BTNMATRIX_PART_BTN, style);
            lv_cont_set_style(kb, LV_CONT_PART_MAIN, style);
            lv_cont_set_style(ext->cand, LV_CONT_PART_MAIN, style);
            break;
        case IME_KB_STYLE_BTN_PR: lv_btnm_set_style(ext->kb, LV_BTNMATRIX_PART_BTN, style); break;
        case IME_KB_STYLE_BTN_TGL_REL: lv_btnm_set_style(ext->kb, LV_BTNMATRIX_PART_BTN, style); break;
        case IME_KB_STYLE_BTN_TGL_PR: lv_btnm_set_style(ext->kb, LV_BTNMATRIX_PART_BTN, style); break;
        case IME_KB_STYLE_BTN_INA: lv_btnm_set_style(ext->kb, LV_BTNMATRIX_PART_BTN, style); break;
    }
}

void ime_kb_set_event_cb(lv_obj_t * obj, lv_event_cb_t event_cb)
{
    LV_ASSERT_OBJ(obj, LV_OBJX_NAME);
    ime_kb_ext_t * ext = lv_obj_get_ext_attr(obj);
    lv_obj_set_event_cb(ext->kb, event_cb);
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Assign a Text Area to the Keyboard. The pressed characters will be put there.
 * @param kb pointer to a Keyboard object
 * @return pointer to the assigned Text Area object
 */
lv_obj_t * ime_kb_get_ta(const lv_obj_t * kb)
{
    LV_ASSERT_OBJ(kb, LV_OBJX_NAME);

    ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);
    return ext->ta;
}

/**
 * Set a new a mode (text or number map)
 * @param kb pointer to a Keyboard object
 * @return the current mode from 'ime_kb_mode_t'
 */
ime_kb_mode_t ime_kb_get_mode(const lv_obj_t * kb)
{
    LV_ASSERT_OBJ(kb, LV_OBJX_NAME);

    ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);
    return ext->mode;
}

/**
 * Get the current cursor manage mode.
 * @param kb pointer to a Keyboard object
 * @return true: show cursor on the current text area, false: hide cursor
 */
bool ime_kb_get_cursor_manage(const lv_obj_t * kb)
{
    LV_ASSERT_OBJ(kb, LV_OBJX_NAME);

    ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);
    return ext->cursor_mng == 0 ? false : true;
}

/**
 * Get a style of a keyboard
 * @param kb pointer to a keyboard object
 * @param type which style should be get
 * @return style pointer to a style
 */
const lv_style_t * ime_kb_get_style(const lv_obj_t * kb, ime_kb_style_t type)
{
    LV_ASSERT_OBJ(kb, LV_OBJX_NAME);
    ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);

    const lv_style_t * style = NULL;

    switch(type) {
        case IME_KB_STYLE_BG: style = lv_btnm_get_style(ext->kb, LV_BTNM_STYLE_BG); break;
        case IME_KB_STYLE_BTN_REL: style = lv_btnm_get_style(ext->kb, LV_BTNM_STYLE_BTN_REL); break;
        case IME_KB_STYLE_BTN_PR: style = lv_btnm_get_style(ext->kb, LV_BTNM_STYLE_BTN_PR); break;
        case IME_KB_STYLE_BTN_TGL_REL: style = lv_btnm_get_style(ext->kb, LV_BTNM_STYLE_BTN_TGL_REL); break;
        case IME_KB_STYLE_BTN_TGL_PR: style = lv_btnm_get_style(ext->kb, LV_BTNM_STYLE_BTN_TGL_PR); break;
        case IME_KB_STYLE_BTN_INA: style = lv_btnm_get_style(ext->kb, LV_BTNM_STYLE_BTN_INA); break;
        default: style = NULL; break;
    }

    return style;
}

/*=====================
 * Other functions
 *====================*/

/**
 * Default keyboard event to add characters to the Text area and change the map.
 * If a custom `event_cb` is added to the keyboard this function be called from it to handle the
 * button clicks
 * @param kb pointer to a  keyboard
 * @param event the triggering event
 */
void ime_kb_def_event_cb(lv_obj_t * kb, lv_event_t event)
{
    if(event != LV_EVENT_VALUE_CHANGED) return;

    ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);
    uint16_t btn_id   = lv_btnm_get_active_btn(kb);
    if(btn_id == LV_BTNM_BTN_NONE) return;
    if(lv_btnm_get_btn_ctrl(kb, btn_id, LV_BTNM_CTRL_HIDDEN | LV_BTNM_CTRL_INACTIVE)) return;
    if(lv_btnm_get_btn_ctrl(kb, btn_id, LV_BTNM_CTRL_NO_REPEAT) && event == LV_EVENT_LONG_PRESSED_REPEAT) return;

    const char * txt = lv_btnm_get_active_btn_text(kb);
    if(txt == NULL) return;

    lv_obj_t * par = lv_obj_get_parent(kb);
    /*Do the corresponding action according to the text of the button*/
    if(strcmp(txt, "abc") == 0) {
        //lv_btnm_set_map(kb, kb_map_lc);
        //lv_btnm_set_ctrl_map(kb, kb_ctrl_lc_map);
        ime_kb_set_mode(par, IME_KB_MODE_TEXT);
        return;
    } else if(strcmp(txt, "ABC") == 0) {
        //lv_btnm_set_map(kb, kb_map_uc);
        //lv_btnm_set_ctrl_map(kb, kb_ctrl_uc_map);
        ime_kb_set_mode(par, IME_KB_MODE_TEXT_UPPER);
        return;
    } else if(strcmp(txt, "1#") == 0) {
        //lv_btnm_set_map(kb, kb_map_spec);
        //lv_btnm_set_ctrl_map(kb, kb_ctrl_spec_map);
        ime_kb_set_mode(par, IME_KB_MODE_SPEC);
        return;
    } else if(strcmp(txt, "Ch") == 0) {
        //lv_btnm_set_map(kb, kb_map_ch);
        //lv_btnm_set_ctrl_map(kb, kb_ctrl_ch_map);
        ime_kb_set_mode(par, IME_KB_MODE_CH);
        return;
    } else if(strcmp(txt, "En") == 0) {
        //lv_btnm_set_map(kb, kb_map_lc);
        //lv_btnm_set_ctrl_map(kb, kb_ctrl_lc_map);
        ime_kb_set_mode(par, IME_KB_MODE_TEXT);
        return;
    }else if(strcmp(txt, LV_SYMBOL_CLOSE) == 0) {
        if(kb->event_cb != ime_kb_def_event_cb) {
            lv_res_t res = lv_event_send(kb, LV_EVENT_CANCEL, NULL);
            if(res != LV_RES_OK) return;
        } else {
            ime_kb_set_ta(kb, NULL); /*De-assign the text area  to hide it cursor if needed*/
            lv_obj_del(kb);
            return;
        }
        return;
    } else if(strcmp(txt, LV_SYMBOL_OK) == 0) {
        if(kb->event_cb != ime_kb_def_event_cb) {
            lv_res_t res = lv_event_send(kb, LV_EVENT_APPLY, NULL);
            if(res != LV_RES_OK) return;
        } else {
            ime_kb_set_ta(kb, NULL); /*De-assign the text area to hide it cursor if needed*/
        }
        return;
    }

    /*Add the characters to the text area if set*/
    if(ext->ta == NULL) return;

    if(strcmp(txt, "Enter") == 0 || strcmp(txt, LV_SYMBOL_NEW_LINE) == 0)
        lv_ta_add_char(ext->ta, '\n');
    else if(strcmp(txt, LV_SYMBOL_LEFT) == 0)
    {
        if (ext->mode != IME_KB_MODE_CH)
            lv_ta_cursor_left(ext->ta);
    }
    else if(strcmp(txt, LV_SYMBOL_RIGHT) == 0)
    {
        if (ext->mode != IME_KB_MODE_CH)
            lv_ta_cursor_right(ext->ta);
    }
    else if(strcmp(txt, LV_SYMBOL_BACKSPACE) == 0) 
    {
        lv_ta_del_char(ext->ta);
        if (ext->mode == IME_KB_MODE_CH)
        {
            if (spell_char_num > 0)
            {
                --spell_char_num;
                spell_buffer[spell_char_num] = 0;
                updatePiyinSearchtxt(kb, spell_buffer, strlen(spell_buffer));
            }
        }
    }
    else if(strcmp(txt, "+/-") == 0) {
        uint16_t cur        = lv_ta_get_cursor_pos(ext->ta);
        const char * ta_txt = lv_ta_get_text(ext->ta);
        if(ta_txt[0] == '-') {
            lv_ta_set_cursor_pos(ext->ta, 1);
            lv_ta_del_char(ext->ta);
            lv_ta_add_char(ext->ta, '+');
            lv_ta_set_cursor_pos(ext->ta, cur);
        } else if(ta_txt[0] == '+') {
            lv_ta_set_cursor_pos(ext->ta, 1);
            lv_ta_del_char(ext->ta);
            lv_ta_add_char(ext->ta, '-');
            lv_ta_set_cursor_pos(ext->ta, cur);
        } else {
            lv_ta_set_cursor_pos(ext->ta, 0);
            lv_ta_add_char(ext->ta, '-');
            lv_ta_set_cursor_pos(ext->ta, cur + 1);
        }
    } else {
        if (ext->mode == IME_KB_MODE_CH && strlen(txt) == 1)
        {
            updatePiyinSearch(kb, txt[0]);
        }
        lv_ta_add_text(ext->ta, txt);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void reset_pinyin()
{
    memset(spell_buffer, 0, sizeof(spell_buffer));
    spell_char_num = 0;
    ime_reset_search();
}

static void clearSpellCand(lv_obj_t * kb)
{
    ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);
    lv_obj_t *spell = ext->spell;
    lv_obj_t *cand = ext->cand;

    lv_label_set_text(spell, "");
    lv_obj_clean(cand);
    reset_pinyin();
}

static void cand_label_event_cb(lv_obj_t * obj, lv_event_t event)
{
	switch (event)
	{
	case LV_EVENT_RELEASED:
    {
        lv_obj_t *kb = (lv_obj_t *)(obj->user_data);
        ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);
        char *txt = lv_label_get_text(obj);
        int len = strlen(lv_label_get_text(ext->spell));
        for(int i=0; i<len; i++)
        {
            lv_ta_del_char(ext->ta);
        }
        lv_ta_add_text(ext->ta, txt);
        clearSpellCand(kb);
    }
    break;
    }
}

static void updatePiyinSearchtxt(lv_obj_t * kb, char* txt, int len)
{
    ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);
    lv_obj_t *spell = ext->spell;
    lv_obj_t *cand = ext->cand;

    if (cand == NULL || spell == NULL) {
        return;
    }
    lv_label_set_text(spell, txt);

    ime_reset_search();
    int cand_num = ime_search(txt, len);
    lv_obj_t *obj;
    char str[128+1];
    int w = lv_obj_get_width_fit(cand)/5;
    int h = lv_obj_get_height_fit(cand);

    lv_obj_clean(cand);
    for (int i = 0; i < cand_num && i < 5; i++)
    {
        memset(str, 0, sizeof(str));
        if (NULL != ime_get_candidate(i, str, 128))
        {
            obj = lv_label_create(cand, NULL);
            lv_obj_set_size(obj, w, h);
            lv_obj_set_pos(obj, i*w, CAND_LABEL_OFFSET);
            lv_obj_set_user_data(obj, kb);
            lv_obj_set_event_cb(obj, cand_label_event_cb);
            lv_style_copy(&cand_label_style, lv_label_get_style(obj, LV_LABEL_STYLE_MAIN));
            cand_label_style.text.font = &stsong_18;
            cand_label_style.text.color = LV_COLOR_WHITE;
            lv_label_set_style(obj, LV_LABEL_STYLE_MAIN, &cand_label_style);
            lv_label_set_text(obj, str);
        }
    }
}

static void updatePiyinSearch(lv_obj_t * kb, char ch)
{
    ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);
    lv_obj_t *spell = ext->spell;
    lv_obj_t *cand = ext->cand;

    if (cand == NULL || spell == NULL) {
        return;
    }

    if (spell_char_num < 128) {
        spell_buffer[spell_char_num++] = ch;
        spell_buffer[spell_char_num] = 0;
    }
    lv_style_copy(&spell_label_style, lv_label_get_style(spell, LV_LABEL_STYLE_MAIN));
    spell_label_style.text.color = LV_COLOR_WHITE;
    lv_label_set_style(spell, LV_LABEL_STYLE_MAIN, &spell_label_style);
    lv_label_set_text(spell, spell_buffer);
    
    int cand_num = ime_search(spell_buffer, strlen(spell_buffer));
    lv_obj_t *obj;
    char str[128+1];
    int w = lv_obj_get_width_fit(cand)/5;
    int h = lv_obj_get_height_fit(cand);

    lv_obj_clean(cand);
    for (int i = 0; i < cand_num && i < 5; i++)
    {
        memset(str, 0, sizeof(str));
        if (NULL != ime_get_candidate(i, str, 128))
        {
            obj = lv_label_create(cand, NULL);
            lv_obj_set_size(obj, w, h);
            lv_obj_set_pos(obj, i*w, CAND_LABEL_OFFSET);
            lv_obj_set_user_data(obj, kb);
            lv_obj_set_click(obj, true);
            lv_obj_set_event_cb(obj, cand_label_event_cb);
            lv_style_copy(&cand_label_style, lv_label_get_style(obj, LV_LABEL_PART_MAIN));
            // cand_label_style.text.font = &stsong_18;
            // cand_label_style.text.color = LV_COLOR_WHITE;
            lv_style_set_text_font(&cand_label_style, LV_STATE_DEFAULT, &stsong_18);
            lv_style_set_text_color(&cand_label_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_label_set_style(obj, LV_LABEL_PART_MAIN, &cand_label_style);
            lv_label_set_text(obj, str);
        }
    }
}

/**
 * Signal function of the keyboard
 * @param kb pointer to a keyboard object
 * @param sign a signal type from lv_signal_t enum
 * @param param pointer to a signal specific variable
 * @return LV_RES_OK: the object is not deleted in the function; LV_RES_INV: the object is deleted
 */
static lv_res_t ime_kb_signal(lv_obj_t * kb, lv_signal_t sign, void * param)
{
    lv_res_t res;
    ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);

    /* Include the ancient signal function */
    res = ancestor_signal(ext->kb, sign, param);
    if(res != LV_RES_OK) return res;
    if(sign == LV_SIGNAL_GET_TYPE) return lv_obj_handle_get_type_signal(param, LV_OBJX_NAME);

    if(sign == LV_SIGNAL_CLEANUP) {
        /*Nothing to cleanup. (No dynamically allocated memory in 'ext')*/
    } else if(sign == LV_SIGNAL_FOCUS) {
        ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);
        /*Show the cursor of the new Text area if cursor management is enabled*/
        if(ext->ta && ext->cursor_mng) {
            lv_cursor_type_t cur_type = lv_ta_get_cursor_type(ext->ta);
            lv_ta_set_cursor_type(ext->ta, cur_type & (~LV_CURSOR_HIDDEN));
        }
    } else if(sign == LV_SIGNAL_DEFOCUS) {
        ime_kb_ext_t * ext = lv_obj_get_ext_attr(kb);
        /*Show the cursor of the new Text area if cursor management is enabled*/
        if(ext->ta && ext->cursor_mng) {
            lv_cursor_type_t cur_type = lv_ta_get_cursor_type(ext->ta);
            lv_ta_set_cursor_type(ext->ta, cur_type | LV_CURSOR_HIDDEN);
        }
    }

    return res;
}

#endif
