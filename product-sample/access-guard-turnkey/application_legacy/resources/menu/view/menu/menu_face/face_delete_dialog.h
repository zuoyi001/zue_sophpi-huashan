#ifndef FACE_DELETE_DIALOG_H
#define FACE_DELETE_DIALOG_H

#include "lvgl/lvgl.h"
#include "cvi_hal_db_repo.h"

lv_obj_t * create_face_delete_dialog(lv_obj_t *par, cvi_person_t *target_person);

#endif