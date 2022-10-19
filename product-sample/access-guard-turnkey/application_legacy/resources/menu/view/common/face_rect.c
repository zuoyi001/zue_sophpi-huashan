#include <pthread.h>
#include <time.h>

#include "face_rect.h"
#include "lvgl/lvgl.h"
#include "../launcher/launcher.h"
#include "dialog.h"

#define POPMENU_SHOW_PERIOD_SEC (3)
#define MAX_RECT 10
static lv_obj_t *face_line[MAX_RECT*4];
static lv_obj_t *obj_line[MAX_RECT];
static lv_point_t points[MAX_RECT*4][5];
static lv_obj_t *label[MAX_RECT];

static lv_obj_t *popup_menu = NULL;
static pthread_mutex_t popmenu_mutex = PTHREAD_MUTEX_INITIALIZER;
static time_t start_time = 0;

void hide_popmenu()
{
    pthread_mutex_lock(&popmenu_mutex);
    if (popup_menu) {
        lv_obj_del(popup_menu);
        popup_menu = NULL;
    }
    pthread_mutex_unlock(&popmenu_mutex);
}

void show_popmenu(void *face_img, char *name_str, const char *result_str, int width, int height, bool bPass)
{
    pthread_mutex_lock(&popmenu_mutex);

    if (popup_menu) {
        lv_obj_del(popup_menu);
        popup_menu = NULL;
    }

    if (popup_menu == NULL) {
        time(&start_time);
        popup_menu = createPopupMenu(lv_launcher_desktop(), face_img, name_str, result_str, width, height, bPass);
    }
    pthread_mutex_unlock(&popmenu_mutex);
}

void check_popmenu()
{
    time_t cur;
    time(&cur);
    int diff = cur - start_time;
    //printf("time diff:%d\n", diff);
    if (diff > POPMENU_SHOW_PERIOD_SEC) {
        hide_popmenu();
    }
}

lv_style_t style_line;
bool bstyle_init = false;
void face_rect_draw_face_4_points(int x, int y, int width, int height,int line_id,int rect_color)
{
    /*Create new style (thick dark blue)*/
    //lv_style_copy(&style_line, &lv_style_plain);
    
	lv_style_init(&style_line);
	bstyle_init = true;

    if(0 == rect_color) // match fail
    {
        lv_style_set_line_color(&style_line, LV_STATE_DEFAULT, LV_COLOR_MAKE(255, 0, 0));
        //style_line.line.color = LV_COLOR_MAKE(255, 0, 0);
    }
    else if(1 == rect_color) //match pass
    {
        lv_style_set_line_color(&style_line, LV_STATE_DEFAULT, LV_COLOR_MAKE(0, 255, 0));
        // style_line.line.color = LV_COLOR_MAKE(0, 255, 0);
    }
    else if(2 == rect_color)
    {
        lv_style_set_line_color(&style_line, LV_STATE_DEFAULT, LV_COLOR_MAKE(233, 208, 23));
        // style_line.line.color = LV_COLOR_MAKE(233, 208, 23);
    } 
    // style_line.line.width = 5;
    // style_line.line.rounded = 5;
    lv_style_set_line_width(&style_line, LV_STATE_DEFAULT, 5);
    lv_style_set_line_rounded(&style_line, LV_STATE_DEFAULT, 5);

    for(int i=0;i<4;i++){
        if (face_line[line_id+i] != NULL) {
            lv_obj_del(face_line[line_id+i]);
            face_line[line_id+i] = NULL;
        }
    }

    points[line_id][0].x = x;
    points[line_id][0].y = y+12;
    points[line_id][1].x = x;
    points[line_id][1].y = y;
    points[line_id][2].x = x +12;
    points[line_id][2].y = y;

    /*Copy the previous line and apply the new style*/
    face_line[line_id] = lv_line_create(lv_launcher_scr(), NULL);

    // static lv_point_t line_points[] = { {x, y+5}, {x,y}, {x+5,y}};
    lv_line_set_points(face_line[line_id], points[line_id], 3);     /*Set the points*/
    lv_obj_add_style(face_line[line_id], LV_LINE_PART_MAIN, &style_line);
    // lv_obj_align(face_line[line_id], NULL, LV_ALIGN_CENTER, 0, 0);
    line_id++;
    points[line_id][0].x = x+ width-11;
    points[line_id][0].y = y;
    points[line_id][1].x = x + width;
    points[line_id][1].y = y;
    points[line_id][2].x = x + width;
    points[line_id][2].y = y + 11;

    face_line[line_id] = lv_line_create(lv_launcher_scr(), NULL);

    lv_line_set_points(face_line[line_id], points[line_id], 3);     /*Set the points*/
    lv_obj_add_style(face_line[line_id], LV_LINE_PART_MAIN, &style_line);

    line_id++;
    points[line_id][0].x = x;
    points[line_id][0].y = y+height-11;
    points[line_id][1].x = x ;
    points[line_id][1].y = y+height;
    points[line_id][2].x = x+11;
    points[line_id][2].y = y+height;
    face_line[line_id] = lv_line_create(lv_launcher_scr(), NULL);

    lv_line_set_points(face_line[line_id], points[line_id], 3);     /*Set the points*/
    lv_obj_add_style(face_line[line_id], LV_LINE_PART_MAIN, &style_line);

    line_id++;
    points[line_id][0].x = x+ width-11;
    points[line_id][0].y = y+height;
    points[line_id][1].x = x + width;
    points[line_id][1].y = y+height;
    points[line_id][2].x = x + width;
    points[line_id][2].y = y + height - 11;
    face_line[line_id] = lv_line_create(lv_launcher_scr(), NULL);

    lv_line_set_points(face_line[line_id], points[line_id], 3);     /*Set the points*/
    lv_obj_add_style(face_line[line_id], LV_LINE_PART_MAIN, &style_line);
}

//void launcher_hide_face_rect()
void face_rect_hide_rects(void)
{
    for(int i=0;i <MAX_RECT*4 ;i++)
    {
        if (face_line[i] != NULL) {
            lv_obj_del(face_line[i]);
            face_line[i] = NULL;
        }
    }
	if(bstyle_init)
	{
		lv_style_reset(&style_line);
		bstyle_init = false;
	}
}

void launcher_draw_name(int start_x,int start_y,int name_id,char * name)
{
    if (label[name_id] != NULL) {
        lv_obj_del(label[name_id]);
        label[name_id] = NULL;
    }

    label[name_id] = lv_label_create(lv_launcher_scr(), NULL);
    //printf("launcher_draw_face_name %d %d\n",start_x,start_y);
    lv_label_set_long_mode(label[name_id], LV_LABEL_LONG_BREAK);
    lv_obj_set_pos(label[name_id], start_x,start_y);
    lv_label_set_text(label[name_id], name);
}

void face_rect_hide_name(void)
{
    for(int i=0;i <MAX_RECT ;i++)
    {
        if (label[i] != NULL) {
            lv_obj_del(label[i]);
            label[i] = NULL;
        }
    }
}

void launcher_draw_obj_rect(int x, int y, int width, int height,int line_id)
{
    points[line_id][0].x = x;
    points[line_id][0].y = y;
    points[line_id][1].x = x + width;
    points[line_id][1].y = y;
    points[line_id][2].x = x + width;
    points[line_id][2].y = y + height;
    points[line_id][3].x = x;
    points[line_id][3].y = y + height;
    points[line_id][4].x = x;
    points[line_id][4].y = y;
    if (obj_line[line_id] != NULL) {
        lv_obj_del(obj_line[line_id]);
        obj_line[line_id] = NULL;
    }
    obj_line[line_id] = lv_line_create(lv_launcher_scr(), NULL);
    lv_line_set_points(obj_line[line_id], points[line_id], 5);
}

void launcher_hide_obj_rect()
{
    for(int i=0;i <MAX_RECT ;i++)
    {
        if (obj_line[i] != NULL) {
            lv_obj_del(obj_line[i]);
            obj_line[i] = NULL;
        }
    }
}