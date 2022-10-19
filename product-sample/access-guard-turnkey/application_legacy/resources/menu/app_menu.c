#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <assert.h>

#include "app_utils.h"
#include "lv_i18n.h"
#include "keyboard.h"
#include "ime.h"
#include "lib_main.h"
#include "app_menu.h"
#include "core.h"
#include "config_data.h"
#include "menu_page.h"
#include "menu.h"
#include "core.h"

static void _MainMenu()
{
    lv_i18n_init(lv_i18n_language_pack);
    lv_i18n_set_locale("zh-CN");

    /*Input method*/
    bool ret = ime_init();
    printf("IME init ret %d\n", ret);

    /*set theme*/
    lv_theme_t *theme = lv_theme_ag_init(LV_COLOR_NAVY, LV_COLOR_NAVY, 0,
        &lv_font_montserrat_14, &lv_font_montserrat_16, &lv_font_montserrat_16, &lv_font_montserrat_16);
    lv_theme_set_act(theme);
    //lv_theme_t *th = lv_theme_get_act();

    gui_comp_init();
    keyboard_init();

	cvi_set_device_config_str(CONF_KEY_VERSION_NAME, (char *)VERSION_NAME);

    /*Start Launcher UI*/
    menu_page_style_init();
    launcher_app_init();
    menu_app_init();
}

static void *_MainMenuTask(void *arg)
{
    char szThreadName[20]="main_gui_task";
	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);

    main_lvgl_init();

    /*app ui entry*/
    _MainMenu();

    /*Handle tasks (tickless mode)*/
    while(1) {
        gui_task_handler();
        lv_task_handler();
        usleep(3000);
    }

    return NULL;
}

void APP_InitMenu(void) {
    pthread_t h_gui_task;
    if (pthread_create(&h_gui_task, NULL, _MainMenuTask, NULL) == -1)
    {
        DBG_PRINTF(CVI_ERROR, "create main ui task fail\n");
        assert(0);
    }
	#if 0
	struct sched_param param;
	param.__sched_priority = 99;
	if (pthread_setschedparam(h_gui_task, SCHED_RR, &param))
	{
		printf("pthread_setschedparam fail\n");
	}
    #endif
	
}
