#ifndef INPUT_DIALOG_H
#define INPUT_DIALOG_H

#include "lvgl/lvgl.h"

typedef struct {
    int id;
    char data[128];
} input_dialog_t;

enum {
    INPUT_ID_WEB = 0,
	INPUT_ID_REGISTID,
	INPUT_ID_INOUT
};
extern input_dialog_t input_data;
lv_obj_t * create_input_dialog(lv_obj_t *par, char *target_ssid);

#endif