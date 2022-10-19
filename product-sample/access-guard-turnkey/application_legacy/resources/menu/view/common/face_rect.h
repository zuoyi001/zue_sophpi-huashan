#ifndef FACE_RECT_H
#define FACE_RECT_H
#include <stdbool.h>

#define DEFAULT_RECT_COLOR_R          (53. / 255.)
#define DEFAULT_RECT_COLOR_G          (208. / 255.)
#define DEFAULT_RECT_COLOR_B          (217. / 255.)
#define DEFAULT_LIVENESS_THRESHOLD    0.6
#define DEFAULT_RECT_THINKNESS        4
#define DEFAULT_ALPHABET_IMAGES_PATH  "./labels"

typedef struct {
    int x;
    int y;
    int width;
    int height;
    int line_id;
    int rect_color;
} face_rect_s;

typedef struct {
    void    *face_img;
    char    *name_str;
    const char *result_str;
    int     width;
    int     height;
    bool    bPass;
} pop_menu_info_s;

void launcher_draw_face_4_points(int x, int y, int width, int height,int line_id,int rect_color);
//void launcher_draw_face_rect(int x, int y, int width, int height,int line_id);
void launcher_hide_face_rect();
void launcher_draw_name(int start_x,int start_y,int name_id,char * name);
void launcher_hide_name();
void launcher_draw_obj_rect(int x, int y, int width, int height,int line_id);
void launcher_hide_obj_rect();
void show_popmenu(void *face_img, char *name_str, const char *result_str, int width, int height, bool bPass);
void hide_popmenu();
void check_popmenu();
void face_rect_draw_face_4_points(int x, int y, int width, int height,int line_id,int rect_color);
void face_rect_hide_rects(void);
void face_rect_hide_name(void);
#endif