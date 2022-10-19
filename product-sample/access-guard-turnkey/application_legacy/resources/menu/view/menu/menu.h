#ifndef _MENU_H_
#define _MENU_H_

#include "lvgl/lvgl.h"

#define VERSION_NAME  "v1.0.0.10"
#define ENTER_MENU		1
#define OUT_MENU    	0

typedef struct {
    void (*onCreate)(void);
    void (*onDestroy)(void);
    void (*show)(void);
    void (*hide)(void);
    void (*back)(void);
} menu_page_t;

typedef struct stackNode
{
    menu_page_t page;
    struct stackNode *next;
}stackNode;

typedef stackNode menu_tree;

void menu_tree_init();
void menu_tree_push(menu_page_t e);
void menu_tree_pop();
menu_page_t menu_tree_get_top();
void menu_tree_destroy();
bool menu_tree_isEmpty();
void menu_app_init();

void set_enter_menu_flag(int flag);
int get_enter_menu_flag();

#endif