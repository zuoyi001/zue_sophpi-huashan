#include <stdlib.h>
#include "menu.h"
#include "menu_setting.h"
#include "menu_test_mode.h"
#include "menu_factory_mode.h"

static menu_tree *s_menuTree;

int enter_menu_flag = 0;
void set_enter_menu_flag(int flag)
{
	enter_menu_flag = flag;
}
int get_enter_menu_flag()
{
	return enter_menu_flag;
}
void menu_tree_init()
{
    s_menuTree = NULL;
}

void menu_tree_push(menu_page_t e)
{
    stackNode *p = (stackNode *)malloc(sizeof(stackNode));
    p->page = e;
    p->next = s_menuTree;
    s_menuTree = p;
}

void menu_tree_pop()
{
    if (!menu_tree_isEmpty()) {
        stackNode *e = s_menuTree;
        s_menuTree = s_menuTree->next;
        free(e);
    }
}

menu_page_t menu_tree_get_top()
{
    menu_page_t page = {0};
    if (!menu_tree_isEmpty()) {
        stackNode *e = s_menuTree;
        return e->page;
    }

    return page;
}

void menu_tree_destroy()
{
    stackNode *e = NULL;
    while(s_menuTree != NULL) {
        e = s_menuTree;
        s_menuTree = s_menuTree->next;
        free(e);
    }
}

bool menu_tree_isEmpty()
{
    return s_menuTree == NULL;
}

void menu_app_init()
{
    menu_tree_init();
    menu_scr_init();
	menu_test_mode_scr_init();
	menu_factory_mode_scr_init();
}
