/**
 * @file main.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include <linux/input.h>

//#define DISP_BUF_SIZE (128 * 1024)

#define PANEL_HOR_RES_MAX   (720)
#define PANEL_VER_RES_MAX   (1280)
#define DISP_BUF_SIZE (PANEL_HOR_RES_MAX * PANEL_VER_RES_MAX)
//#define FB_DISP_ROTATION
#define INPUT_DEV_FILENAME "/proc/bus/input/devices"

static char *str_trim_end(char *str)
{
    int len = strlen(str);
    char *p = &str[len-1];
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
        p--;
    }
    *(p+1) = 0;

    return str;
}

static bool search_evdev()
{
    bool find_mouse = false;
    char evdev_path[64];
    char cat_buffer[256];
    memset(evdev_path, 0, sizeof(evdev_path));

    FILE *fp = fopen(INPUT_DEV_FILENAME, "r+");
    if (NULL == fp)
    {
        return false;
    }

    while(NULL != fgets(cat_buffer, sizeof(cat_buffer) - 1, fp)) {
        #if 0
        char *start = strstr(cat_buffer, "mouse");
        if (start == NULL)
            continue;
        #endif
        char *end = strstr(cat_buffer, "event");
        if (end != NULL) {
            sprintf(evdev_path, "/dev/input/%s", end);
            str_trim_end(evdev_path);
            printf("to set indev %s---\n", evdev_path);
            evdev_set_file(evdev_path);
            find_mouse = true;
            break;
        }
    }

    return find_mouse;
}


static void indev_init()
{
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER; /*See below.*/
    indev_drv.read_cb = evdev_read;            /*See below.*/

    evdev_init();
    bool bfindmouse = search_evdev();//if mouse pluged, use mouse eventX, or use event0
    lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv); /*Register the driver in LittlevGL*/
    if (bfindmouse) {
        //LV_IMG_DECLARE(mouse_cursor_icon);                          /*Declare the image file.*/
        lv_obj_t * cursor_obj =  lv_img_create(lv_layer_sys()); /*Create an image object for the cursor */
        lv_img_set_src(cursor_obj, LV_SYMBOL_GPS);             /*Set the image source*/
        lv_indev_set_cursor(mouse_indev, cursor_obj);               /*Connect the image  object to the driver*/
    }
}

void main_lvgl_init(void)
{
    /*LittlevGL init*/
    lv_init();

    /*Linux frame buffer device init*/
    fbdev_init();

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;
    disp_drv.flush_cb   = fbdev_flush;
    disp_drv.hor_res    = PANEL_HOR_RES_MAX;
    disp_drv.ver_res    = PANEL_VER_RES_MAX;
    #if defined(FB_DISP_ROTATION)
    disp_drv.sw_rotate  = 1;
    lv_disp_set_rotation(lv_disp_drv_register(&disp_drv), LV_DISP_ROT_90);
    #else
    lv_disp_drv_register(&disp_drv);
    #endif

    /*Input Device Init and Register, mouse and touchpad*/
    indev_init();

    /*Create a Demo*/
    //lv_demo_widgets();

    /*lv lib init*/
    lv_png_init();
    return;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
