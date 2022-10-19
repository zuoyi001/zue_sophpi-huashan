#include <pthread.h>
#include <time.h>

#include "write_mac_sn.h"
#include "lvgl/lvgl.h"
#include "../launcher/launcher.h"
#include "dialog.h"
#include "write_mac_sn_dialog.h"

static lv_obj_t *popup_wms_menu = NULL;
static pthread_mutex_t popmenu_wms_mutex = PTHREAD_MUTEX_INITIALIZER;

void hide_write_mac_sn_popmenu()
{
    pthread_mutex_lock(&popmenu_wms_mutex);
    if (popup_wms_menu) {
        lv_obj_del(popup_wms_menu);
        popup_wms_menu = NULL;
    }
    pthread_mutex_unlock(&popmenu_wms_mutex);
}

void show_write_mac_sn_popmenu()
{
    pthread_mutex_lock(&popmenu_wms_mutex);

    if (popup_wms_menu) {
        lv_obj_del(popup_wms_menu);
        popup_wms_menu = NULL;
    }

    if (popup_wms_menu == NULL) {
        popup_wms_menu = createPopupMenuWMS(lv_launcher_desktop());
    }
    pthread_mutex_unlock(&popmenu_wms_mutex);
}

