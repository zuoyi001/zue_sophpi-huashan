#ifndef LAUNCHER_MAIN_H
#define LAUNCHER_MAIN_H
#include "lvgl/lvgl.h"

#define LAUNCHER_MAIN_OPACITY (LV_OPA_TRANSP)
#define ALTERNATE_UI_OPACITY (LV_OPA_50)


typedef struct {
    lv_obj_t *line_ptr;
    lv_obj_t *label_ptr;
    lv_point_t points[5];
}cvi_face_rect_t;

void launcher_draw_rect(cvi_face_rect_t *rect_ptr, char *face_name, int16_t x, int16_t y, int16_t width, int16_t height);
void launcher_app_init();
void launcher_hide();
void launcher_show();
lv_obj_t * lv_launcher_desktop(void);
lv_obj_t * lv_launcher_scr(void);

#endif