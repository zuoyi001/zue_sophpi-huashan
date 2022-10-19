#ifndef _DRAW_HELPER_H_
#define _DRAW_HELPER_H_

void gui_draw_face_rect(int x, int y, int width, int height,int line_id,int rect_color);
void gui_clear_face_rect();
void gui_show_popmenu(void *face_img, char *name_str, const char *result_str, bool bPass,int width,int height);
void gui_hide_popmenu();
#endif