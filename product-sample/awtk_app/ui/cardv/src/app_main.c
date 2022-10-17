#include <sys/prctl.h>
#include <pthread.h>
#include "awtk_app_conf.h"
#include "awtk.h"
#include "window_common.h"
#include "../res/assets_res_480_272.inc"
//#include "../res/assets.inc"
static pthread_t s_UiThread;

static int gui_app_start(int w, int h)
{
    int32_t lcd_w = w;
    int32_t lcd_h = h;
    #ifdef NDEBUG
    log_set_log_level(LOG_LEVEL_INFO);
    #else
    log_set_log_level(LOG_LEVEL_DEBUG);
    #endif /*NDEBUG*/

    log_debug("APP_RES_ROOT = %s\n",APP_RES_ROOT);
    tk_init(lcd_w, lcd_h, APP_TYPE, APP_NAME, APP_RES_ROOT);

    #if defined(WITH_LCD_PORTRAIT)
    if (lcd_w > lcd_h) {
        tk_set_lcd_orientation(LCD_ORIENTATION_90);
    }
    #endif /*WITH_LCD_PORTRAIT*/

    #ifdef WITH_LCD_LANDSCAPE
    if (lcd_w < lcd_h) {
        tk_set_lcd_orientation(LCD_ORIENTATION_270);
    }
    #endif /*WITH_LCD_PORTRAIT*/

    system_info_set_default_font(system_info(), APP_DEFAULT_FONT);
    assets_init();

    #ifndef WITHOUT_EXT_WIDGETS
    tk_ext_widgets_init();
    #endif /*WITHOUT_EXT_WIDGETS*/
    log_info("Build at: %s %s\n", __DATE__, __TIME__);

    #ifdef ENABLE_CURSOR
    window_manager_set_cursor(window_manager(), "cursor");
    #endif /*ENABLE_CURSOR*/

    //on_change_locale
    locale_info_change(locale_info(), "zh", "CN");

    application_init();
    tk_run();
    application_exit();


    return 0;
}

static void *start_uiapp(void *arg)
{
    prctl(PR_SET_NAME, __FUNCTION__, 0, 0, 0);
    gui_app_start(LCD_WIDTH, LCD_HEIGHT);

    return NULL;
}

int uiapp_start(void)
{
    int s32Ret = 0;
    s32Ret = pthread_create(&s_UiThread, NULL, start_uiapp, NULL);
    if (0 != s32Ret)
    {
        printf("pthread_create start_uiapp Failed, s32Ret=%#x\n", s32Ret);
        return -1;
    }

    return 0;
}

int uiapp_stop(void)
{
    int s32Ret = 0;

    tk_quit();
    s32Ret = pthread_join(s_UiThread, NULL);
    if (0 != s32Ret)
    {
        printf("pthread_join start_uiapp Failed, s32Ret=%#x\n", s32Ret);
        return -1;
    }

    return 0;
}
