#include <stdlib.h>
#include "face_rect.h"
#include "core.h"
#include "stdio.h"
#include <string.h>

void gui_draw_face_rect(int x, int y, int width, int height,int line_id,int rect_color)
{
    face_rect_s rect;
    memset(&rect, 0, sizeof(face_rect_s));
    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;
    rect.line_id = line_id;
    rect.rect_color = rect_color;
    gui_comp_sendcmd(CMD_UI_DRAW_FACE_RECT, &rect, sizeof(face_rect_s), NULL, 0);
}

void gui_clear_face_rect()
{
    gui_comp_sendcmd(CMD_UI_CLEAR_FACE_RECT, NULL, 0, NULL, 0);
}

void gui_show_popmenu(void *face_img, char *name_str, const char *result_str, bool bPass,int width,int height)
{
    // int width = 200;
    // int height = 200;
    pop_menu_info_s info;
    memset(&info, 0, sizeof(pop_menu_info_s));
    info.face_img = face_img;
    info.name_str = name_str;
    info.result_str = result_str;
    info.width = width;
    info.height = height;
    info.bPass = bPass;
    gui_comp_sendcmd(CMD_UI_SHOW_POPMENU, &info, sizeof(pop_menu_info_s), NULL, 0);
}

void gui_hide_popmenu()
{
    gui_comp_sendcmd(CMD_UI_HIDE_POPMENU, NULL, 0, NULL, 0);
}