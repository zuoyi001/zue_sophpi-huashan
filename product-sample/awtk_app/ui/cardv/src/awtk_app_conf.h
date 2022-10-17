#ifndef __AWTK_APP_CONF_H__
#define __AWTK_APP_CONF_H__

#define WITHOUT_WINDOW_ANIMATORS 1
#define WITHOUT_DIALOG_HIGHLIGHTER 1
#define WITHOUT_WIDGET_ANIMATORS 1
#define WITHOUT_CLIPBOARD 1
#define WITH_BITMAP_BGRA 1
//WITH_NULL_IM
//#define NDEBUG
#define WITH_NANOVG_AGGE
#define WITH_VGCANVAS

#define HAS_STD_MALLOC 1
#define WITH_UNICODE_BREAK 1
#define LINUX 1
#define WITH_ASSET_LOADER 1
#define WITH_FS_RES 1
#define STBTT_STATIC 1
#define STB_IMAGE_STATIC 1
#define WITH_STB_IMAGE 1
#define WITH_STB_FONT 1

#define WITH_IME_PINYIN 1
#define HAS_PTHREAD 1
//#define ENABLE_CURSOR 0
#define WITH_DATA_READER_WRITER 1
#define WITH_EVENT_RECORDER_PLAYER 1
#define WITH_WIDGET_TYPE_CHECK 1
//#define WITHOUT_EXT_WIDGETS 1
#define WITH_LCD_LANDSCAPE

#define APP_TYPE APP_MOBILE

#ifndef APP_THEME
#define APP_THEME "default"
#endif /*APP_THEME*/

#ifndef APP_DEFAULT_FONT
#define APP_DEFAULT_FONT "default"
#endif /*APP_DEFAULT_FONT*/

#ifndef APP_NAME
#define APP_NAME "awtk"
#endif /*APP_NAME*/

#ifndef APP_RES_ROOT
#define APP_RES_ROOT "/mnt/data/install/bin/res"
#endif /*APP_RES_ROOT*/

#define LCD_WIDTH 480
#define LCD_HEIGHT 272

#else
#error "include awtk_app_conf twice"
#endif /* End of #ifndef __AWTK_APP_CONF_H__*/
