#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "menu.h"
#include "menu_common.h"
#include "menu_page.h"
#include "resource.h"
#include "lv_i18n.h"
#include "menu_test_mode.h"
#include "menu_rs485_wg_test.h"
#include "menu_eth_test.h"
#include "menu_audio_test.h"
#include "factory.h"
#include "app_comp.h"
#include "menu_factory_mode.h"
#include "ping.h"
#include "sys.h"

#define PING_FILE  "/tmp/ping.txt"

/*个性化与通知反馈页面*/

static lv_obj_t *menu_eth_test_page_obj = NULL;
static lv_obj_t *bottom_panel_page_obj = NULL;
//static lv_obj_t *disp_btn_panel = NULL;
static lv_obj_t * show_label = NULL;

static lv_style_t disp_panel_style;
static lv_style_t disp_btn_panel_style;
static lv_style_t disp_eth_panel_style;
#define LAUNCHER_MAIN_OPACITY (LV_OPA_TRANSP)
#define ALTERNATE_UI_OPACITY (LV_OPA_50)


static int flag = 0;

char bufGw[64] = {0};
static void menu_eth_test_create();
static void menu_eth_test_destroy();
static void menu_eth_test_show();
static void menu_eth_test_hide();

static menu_page_t menu_eth_test_page = {
    .onCreate = menu_eth_test_create,
    .onDestroy = menu_eth_test_destroy,
    .show = menu_eth_test_show,
    .hide = menu_eth_test_hide,
    .back = menuPageBack
};

menu_page_t get_menu_eth_test_page()
{
    return menu_eth_test_page;
}
static pthread_t eth_id = 0;


void getGw(char* buf, int size)
{
	char Cmd[100]={0};
	char readline[100]={0};
	memset( Cmd, 0, sizeof( Cmd ) );
	sprintf( Cmd,"netstat -r|grep default|cut -f 10 -d \' \'");
	FILE* fp = popen( Cmd, "r" );

	if ( NULL == fp )
	{
		return;
	}

	//memset( buf, 0, sizeof( buf ) );
	memset(buf, 0x0, size); //liang.wang 2021-03-15
	while ( NULL != fgets( readline,sizeof( readline ),fp ))
	{
		printf("gateway=%s\n",readline);
		//snprintf(buf,size,"%s",readline);
		if(size > strlen(readline)-1)
		{
			memcpy(buf,readline,strlen(readline)-1);
		}
		break;
	}
	pclose(fp);
}
void * eth_check_thread(void *arg)
{
	char 
tmpBuf[64]  = {0};
	char* pBuf = NULL;
	int count = 10;

	while(0 == flag)
	{
		count = 20;
		snprintf(tmpBuf,sizeof(tmpBuf),"%s %s","rm",PING_FILE);
		system(tmpBuf);
		sleep(1);
		memset(tmpBuf, 0x00 ,sizeof(tmpBuf));
		snprintf(tmpBuf,sizeof(tmpBuf),"ping %s >> %s&",bufGw,PING_FILE);
		//printf("cmd:%s \n",tmpBuf);
		//snprintf(tmpBuf,sizeof(tmpBuf),"ping %s >> /tmp/ping.txt&","192.168.1.1");
		system(tmpBuf);

		while((0 == flag) && (count-- > 0))
		{
			struct stat st;
			if(stat( PING_FILE, &st ) == 0)
			{
				int len = st.st_size;
				if ( len > 0 )
				{
					pBuf = malloc(len+1);
					if(pBuf != NULL)
					{
						//memset(pBuf, 0x00 ,sizeof(pBuf));
						memset(pBuf, 0x00 ,len+1);//liang.wang 2021-03-15
						int fd = open( PING_FILE,  O_RDONLY );
						if (read( fd, pBuf, len ) == len)
						{
							if(show_label != NULL)
							{
								lv_label_set_text(show_label, pBuf);
								//printf("[%s,%d]\n",__FUNCTION__,__LINE__);
							}
						}
						free(pBuf);
						pBuf = NULL;
						close( fd );
					}
				}
			}
			//pingEx("192.168.1.1");
			sleep(1);
		};
		memset(tmpBuf, 0x00 ,sizeof(tmpBuf));
		snprintf(tmpBuf,sizeof(tmpBuf),"%s","killall ping&");
		system(tmpBuf);
		sleep(1);
	}
	return NULL;
}
static void pass_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
		printf("[%s,%d]\n",__FUNCTION__,__LINE__);

		//menu_test_mode_PageBack();
		if (menu_tree_isEmpty())
                return;
		if(get_test_mode_page_flag() == 1)
		{
			menu_btn_flag[MENU_ID_ETH] = 0;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_audio_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
		}
		flag = 1;
    }
}

static void error_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {
        printf("[%s,%d]\n",__FUNCTION__,__LINE__);

		if (menu_tree_isEmpty())
                return;
		if(get_test_mode_page_flag() == 1)
		{
			menu_btn_flag[MENU_ID_ETH] = 1;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
			menu_tree_push(get_menu_audio_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
		}
		flag = 1;
    }
}



/*内部样式*/
static bool styleInited = false;
static void _style_init()
{
    if (!styleInited)
    {
    	lv_style_init(&disp_panel_style);
		lv_style_set_text_font(&disp_panel_style, LV_STATE_DEFAULT, &stsong_28);
		lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
		lv_style_set_text_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);

		lv_style_init(&disp_btn_panel_style);
		lv_style_set_text_font(&disp_btn_panel_style, LV_STATE_DEFAULT, &stsong_36);
		lv_style_set_bg_color(&disp_btn_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLUE);
		lv_style_set_bg_color(&disp_btn_panel_style, LV_STATE_PRESSED, LV_COLOR_RED);
		lv_style_set_text_color(&disp_btn_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);

		lv_style_init(&disp_eth_panel_style);
		lv_style_set_bg_opa(&disp_eth_panel_style, LV_STATE_DEFAULT, LAUNCHER_MAIN_OPACITY);
		lv_style_set_opa_scale(&disp_eth_panel_style, LV_STATE_DEFAULT, ALTERNATE_UI_OPACITY);

		
        styleInited = true;
    }
	show_label = NULL;
}

/*页面内部layout*/
static void create_top_panel(lv_obj_t * par)
{
    lv_obj_t *top_panel = menu_page_create_test_mode_top_panel(par);

    lv_obj_t *page_title = lv_label_create(top_panel, NULL);
    lv_label_set_text(page_title, _("eth test"));
    lv_obj_align(page_title, NULL, LV_ALIGN_CENTER, -20, 0);
    lv_obj_add_style(page_title, LV_LABEL_PART_MAIN, menu_page_title_style());
}

static void create_disp_panel(lv_obj_t * par)
{
	char tmpBuf[128] = {0};
	char tmpBuf1[64] = {0};
    //lv_obj_t *disp_panel = menu_page_create_disp_panel(par);
	//lv_obj_t *disp_panel = lv_cont_create(par, back_btn_event_cb);
	lv_obj_t *disp_panel = lv_cont_create(par, NULL);
	//lv_obj_set_event_cb(disp_panel, change_btn_event_cb);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_obj_set_pos(disp_panel, 0, TOP_TEST_MODE_PANEL_H);
	lv_obj_add_style(disp_panel, LV_CONT_PART_MAIN, &disp_panel_style);
    lv_cont_set_layout(disp_panel, LV_LAYOUT_OFF);
	//_lv_obj_set_style_local_color(lv_obj_t * obj, uint8_t part, lv_style_property_t prop, lv_color_t color)

	get_network_ip_address("eth0", tmpBuf1);
	getGw(bufGw,sizeof(bufGw));
	snprintf(tmpBuf,sizeof(tmpBuf),"%s:%s  %s:%s",_("Ip"),tmpBuf1,_("Gateway"),bufGw);
	lv_obj_t * show_ip_gw_label = lv_label_create(disp_panel, NULL);
    //lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
    lv_obj_add_style(show_ip_gw_label, LV_STATE_DEFAULT, &disp_panel_style);
    lv_label_set_text(show_ip_gw_label, tmpBuf);
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(show_ip_gw_label, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 10);

	 /*Create a page*/
    lv_obj_t * page = lv_page_create(disp_panel, NULL);
    lv_obj_set_size(page, 720, 1000);
    lv_obj_align(page, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 80);

    /*Create a label on the page*/
    show_label = lv_label_create(page, NULL);
    lv_label_set_long_mode(show_label, LV_LABEL_LONG_BREAK);            /*Automatically break long lines*/
	lv_obj_add_style(show_label, LV_STATE_DEFAULT, &disp_panel_style);
    lv_obj_set_width(show_label, lv_page_get_width_fit(page));          /*Set the label width to max value to not show hor. scroll bars*/
    lv_label_set_text(show_label, "");
	
}

/// 页面生命周期函数 start
static void menu_eth_test_create()
{
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
    _style_init();
	flag = 0;
	
    menu_eth_test_page_obj = lv_cont_create(get_test_mode_page_flag()==1 ? lv_scr_menu_test_mode():lv_scr_menu_factory_mode(), NULL);
    lv_obj_add_style(menu_eth_test_page_obj, LV_CONT_PART_MAIN, menu_page_test_mode_title_style());
    lv_obj_set_size(menu_eth_test_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	//lv_obj_set_event_cb(menu_radar_test_page_obj, change_btn_event_cb);
	create_top_panel(menu_eth_test_page_obj);
    create_disp_panel(menu_eth_test_page_obj);
    
	bottom_panel_page_obj = create_bottom_2btn_panel(menu_eth_test_page_obj, pass_btn_event_cb, error_btn_event_cb);
	//lv_obj_set_hidden(bottom_panel_page_obj, false);
	lv_obj_set_hidden(bottom_panel_page_obj, false);
	//create_bottom_2btn_panel(menu_lcd_test_page_obj, pass_btn_event_cb, error_btn_event_cb);

    lv_obj_set_hidden(menu_eth_test_page_obj, true);

	pthread_create(&eth_id,NULL,eth_check_thread,NULL);
}

lv_obj_t * lv_scr_menu_eth_mode()
{
    return menu_eth_test_page_obj;
}

static void menu_eth_test_destroy()
{
    if (menu_eth_test_page_obj)
    {
        lv_obj_del(menu_eth_test_page_obj);
        menu_eth_test_page_obj = NULL;
		flag = 1;
    }
}

static void menu_eth_test_show()
{
    if (menu_eth_test_page_obj)
    {
        lv_obj_set_hidden(menu_eth_test_page_obj, false);
    }
}

static void menu_eth_test_hide()
{
    if (menu_eth_test_page_obj)
    {
        lv_obj_set_hidden(menu_eth_test_page_obj, true);
    }
}
/// 页面生命周期函数 end


