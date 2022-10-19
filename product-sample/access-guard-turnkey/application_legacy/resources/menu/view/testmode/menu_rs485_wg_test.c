#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#include "menu.h"
#include "menu_common.h"
#include "menu_page.h"
#include "resource.h"
#include "lv_i18n.h"
#include "menu_test_mode.h"
#include "menu_rs485_wg_test.h"
#include "menu_radar_test.h"
#include "factory.h"
#include "app_comp.h"
#include "menu_factory_mode.h"
#include "app_peripheral.h"


/*个性化与通知反馈页面*/

static lv_obj_t *menu_rs485_wg_test_page_obj = NULL;
static lv_obj_t *bottom_panel_page_obj = NULL;
//static lv_obj_t *disp_btn_panel = NULL;
static lv_obj_t * show_label = NULL;

static lv_style_t disp_panel_style;
static lv_style_t disp_btn_panel_style;
static lv_style_t disp_rs485_wg_panel_style;
#define LAUNCHER_MAIN_OPACITY (LV_OPA_TRANSP)
#define ALTERNATE_UI_OPACITY (LV_OPA_50)


static int flag = 1;
int extiSendRecv = 1;

char* pText = NULL;
#define TEXT_LEN  (512)

static void menu_rs485_wg_test_create();
static void menu_rs485_wg_test_destroy();
static void menu_rs485_wg_test_show();
static void menu_rs485_wg_test_hide();

static menu_page_t menu_rs485_wg_test_page = {
    .onCreate = menu_rs485_wg_test_create,
    .onDestroy = menu_rs485_wg_test_destroy,
    .show = menu_rs485_wg_test_show,
    .hide = menu_rs485_wg_test_hide,
    .back = menuPageBack
};

menu_page_t get_menu_rs485_wg_test_page()
{
    return menu_rs485_wg_test_page;
}

pthread_t rs485_wg_id = 0;
pthread_t wg_recv_id = 0;

int senddata = 6000;
int recvdata = 0;
int wg_recv_status = 0;

int test_rs485_wg_exit(void)
{
	if(rs485_wg_id != 0)
	{
		pthread_cancel(rs485_wg_id);
		rs485_wg_id = 0;
	}
	if(rs485_wg_id != 0)
	{
		pthread_cancel(wg_recv_id);
		wg_recv_id = 0;
	}

	return 0;
}


void * rs485_wg_send_recv_thread(void *arg)
{
	//int count = 20;
	int ret = 0;
	int swap;
	char tmpBuf[64] = {0};
	//struct wng_receive_data wgn_recv_data;
	char uart_recv_buf[4];
	int *p_s4;
	
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	CVI_HAL_GpioSetValue(RS485_GPIO, CVI_HAL_GPIO_VALUE_H);

	while(1 == extiSendRecv)
	{
		if(NULL == pText)
		{
			break;
		}
		if(strlen(pText) >= (TEXT_LEN-32))
		{
			memset(pText, 0x00, TEXT_LEN);
		}
		
		//senddata = rand()%10000;

		if(0 == flag)
		{
			senddata++;
			CVI_Wiegand_To_Rs485(false, senddata);

			usleep(500*1000);

			ret = CVI_HAL_UartReceive(uart_recv_buf, 3, 1);
			printf("[ret:%d %s,%d]\n",ret,__FUNCTION__,__LINE__);
			if(ret > 0)
			{
				swap = uart_recv_buf[0];
				uart_recv_buf[0] = uart_recv_buf[2];
				uart_recv_buf[2] = swap;
				p_s4 = (int *)uart_recv_buf;
				recvdata = *p_s4;
			}
			else
			{
				recvdata = 0;
				//continue;
			}
			
			memset(tmpBuf, 0x00 ,sizeof(tmpBuf));
			sprintf(tmpBuf,"Wiegand send:%d  rs485 recv:%d",senddata,recvdata);
			if(1 == extiSendRecv)
			{
				tmpBuf[strlen(tmpBuf)] = '\n';
				strcat(pText,tmpBuf);
				lv_label_set_text(show_label, pText);
			}

		}
		else
		{
			//CVI_HAL_GpioSetValue(RS485_GPIO, CVI_HAL_GPIO_VALUE_H);
			//usleep(50*1000);
			if(0 == wg_recv_status)
			{
				continue;
			}
			usleep(50*1000);
			//senddata = 2567;
			senddata++;
			printf("[senddata:%d %s,%d]\n",senddata,__FUNCTION__,__LINE__);
			CVI_Rs485_To_Wiegand(false, senddata);
		}
		
		sleep(1);
		//printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	}
	
	return NULL;
}

void * wg_recv_thread(void *arg)
{
	//int count = 20;
	int ret = 0;
	//int swap;
	char tmpBuf[64] = {0};
	struct wng_receive_data wgn_recv_data;
	//char uart_recv_buf[4];
	
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);

	while((1 == extiSendRecv) &&(1 == flag))
	{
		if(NULL == pText)
		{
			break;
		}
		if(strlen(pText) >= (TEXT_LEN-32))
		{
			memset(pText, 0x00, TEXT_LEN);
		}

		wg_recv_status = 1;
		ret = CVI_HAL_WiegandReceive(&wgn_recv_data, 1);
		wg_recv_status = 0;
		printf("[ret:%d %s,%d]\n",ret,__FUNCTION__,__LINE__);
		if(0 == ret)
		{
			recvdata = (wgn_recv_data.rx_data >> 1) & 0xFFFFFF;
		}
		else
		{
			recvdata = 0;
		}
		
		memset(tmpBuf, 0x00 ,sizeof(tmpBuf));
		sprintf(tmpBuf,"rs485 send:%d  Wiegand recv:%d",senddata,recvdata);
		printf("%s \n",tmpBuf);
		if(1 == extiSendRecv)
		{
			tmpBuf[strlen(tmpBuf)] = '\n';
			strcat(pText,tmpBuf);
			lv_label_set_text(show_label, pText);
		}
				
		sleep(1);
		//printf("[%s,%d]\n",__FUNCTION__,__LINE__);
	}
	
	return NULL;
}


/*事件回调处理函数*/
static void change_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
	if (LV_EVENT_RELEASED == event)
    {
        printf("[%s,%d]\n",__FUNCTION__,__LINE__);

		(0 == flag)?(flag = 1):(flag=0);

		if(pText != NULL)
		{
			memset(pText, 0x00, TEXT_LEN);
		}
		
		if(0 == flag)
		{
			if(wg_recv_id != 0)
			{
				pthread_cancel(wg_recv_id);
				wg_recv_id = 0;
			}
			CVI_HAL_GpioSetValue(RS485_GPIO, CVI_HAL_GPIO_VALUE_L);
			lv_label_set_text(show_label, "Test Wiegand send...");
		}
		else
		{
			CVI_HAL_GpioSetValue(RS485_GPIO, CVI_HAL_GPIO_VALUE_H);
			lv_label_set_text(show_label, "Test rs485 send...");
			pthread_create(&wg_recv_id,NULL,wg_recv_thread,NULL);
		}
		
    }
}

static void pass_btn_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (LV_EVENT_RELEASED == event) {

		//menu_test_mode_PageBack();
		if (menu_tree_isEmpty())
                return;
		if(get_test_mode_page_flag() == 1)
		{
			menu_btn_flag[MENU_ID_RS485_WG] = 0;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
            menu_tree_push(get_menu_radar_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
		}
		extiSendRecv = 0;
		test_rs485_wg_exit();
		
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
			menu_btn_flag[MENU_ID_RS485_WG] = 1;
			menu_test_mode_PageBack();
		}
		else
		{
            menu_tree_get_top().hide();
			menu_tree_push(get_menu_radar_test_page());
            menu_tree_get_top().onCreate();
            menu_tree_get_top().show();
		}
		extiSendRecv = 0;
		test_rs485_wg_exit();
    }
}



/*内部样式*/
static bool styleInited = false;
static void _style_init()
{
    if (!styleInited)
    {
    	lv_style_init(&disp_panel_style);
		lv_style_set_text_font(&disp_panel_style, LV_STATE_DEFAULT, &stsong_36);
		lv_style_set_bg_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
		lv_style_set_text_color(&disp_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);

		lv_style_init(&disp_btn_panel_style);
		lv_style_set_text_font(&disp_btn_panel_style, LV_STATE_DEFAULT, &stsong_36);
		lv_style_set_bg_color(&disp_btn_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLUE);
		lv_style_set_bg_color(&disp_btn_panel_style, LV_STATE_PRESSED, LV_COLOR_RED);
		lv_style_set_text_color(&disp_btn_panel_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);

		lv_style_init(&disp_rs485_wg_panel_style);
		lv_style_set_bg_opa(&disp_rs485_wg_panel_style, LV_STATE_DEFAULT, LAUNCHER_MAIN_OPACITY);
		lv_style_set_opa_scale(&disp_rs485_wg_panel_style, LV_STATE_DEFAULT, ALTERNATE_UI_OPACITY);

		
        styleInited = true;
    }
}

/*页面内部layout*/
static void create_top_panel(lv_obj_t * par)
{
    lv_obj_t *top_panel = menu_page_create_test_mode_top_panel(par);

    lv_obj_t *page_title = lv_label_create(top_panel, NULL);
    lv_label_set_text(page_title, _("RS485/wg test"));
    lv_obj_align(page_title, NULL, LV_ALIGN_CENTER, -20, 0);
    lv_obj_add_style(page_title, LV_LABEL_PART_MAIN, menu_page_title_style());
}
static void create_disp_panel(lv_obj_t * par)
{
    //lv_obj_t *disp_panel = menu_page_create_disp_panel(par);
	//lv_obj_t *disp_panel = lv_cont_create(par, back_btn_event_cb);
	lv_obj_t *disp_panel = lv_cont_create(par, NULL);
	lv_obj_set_event_cb(disp_panel, change_btn_event_cb);
    lv_obj_set_size(disp_panel, LV_HOR_RES_MAX, LV_VER_RES_MAX - 2*TOP_TEST_MODE_PANEL_H);
    lv_obj_set_pos(disp_panel, 0, TOP_TEST_MODE_PANEL_H);
	lv_obj_add_style(disp_panel, LV_CONT_PART_MAIN, &disp_panel_style);
    lv_cont_set_layout(disp_panel, LV_LAYOUT_OFF);
	//_lv_obj_set_style_local_color(lv_obj_t * obj, uint8_t part, lv_style_property_t prop, lv_color_t color)

	show_label = lv_label_create(disp_panel, NULL);
    //lv_obj_add_style(vs_label, LV_STATE_DEFAULT, menu_page_title_style());
    lv_obj_add_style(show_label, LV_STATE_DEFAULT, &disp_panel_style);
    lv_label_set_text(show_label, "���CVI_Test_Wiegand...");
    //lv_label_set_align(vs_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(show_label, NULL, LV_ALIGN_IN_TOP_LEFT, 100, 20);
	
}

/// 页面生命周期函数 start
static void menu_rs485_wg_test_create()
{
	printf("[%s,%d]\n",__FUNCTION__,__LINE__);
    _style_init();
	extiSendRecv = 1;
	flag = 1;

	pText = malloc(TEXT_LEN);
	if(pText != NULL)
	{
		memset(pText, 0x00, TEXT_LEN);
	}

	//���������߳�
	//CVI_Test_Production_Init();
	
	pthread_create(&wg_recv_id,NULL,wg_recv_thread,NULL);
	pthread_create(&rs485_wg_id,NULL,rs485_wg_send_recv_thread,NULL);
	
    menu_rs485_wg_test_page_obj = lv_cont_create(get_test_mode_page_flag()==1 ? lv_scr_menu_test_mode():lv_scr_menu_factory_mode(), NULL);
    lv_obj_add_style(menu_rs485_wg_test_page_obj, LV_CONT_PART_MAIN, menu_page_test_mode_title_style());
    lv_obj_set_size(menu_rs485_wg_test_page_obj, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	
	create_top_panel(menu_rs485_wg_test_page_obj);
    create_disp_panel(menu_rs485_wg_test_page_obj);
    
	bottom_panel_page_obj = create_bottom_2btn_panel(menu_rs485_wg_test_page_obj, pass_btn_event_cb, error_btn_event_cb);
	//lv_obj_set_hidden(bottom_panel_page_obj, false);
	lv_obj_set_hidden(bottom_panel_page_obj, false);
	//create_bottom_2btn_panel(menu_lcd_test_page_obj, pass_btn_event_cb, error_btn_event_cb);

    lv_obj_set_hidden(menu_rs485_wg_test_page_obj, true);
}

lv_obj_t * lv_scr_menu_rs485_wg_mode()
{
    return menu_rs485_wg_test_page_obj;
}

static void menu_rs485_wg_test_destroy()
{
    if (menu_rs485_wg_test_page_obj)
    {
        lv_obj_del(menu_rs485_wg_test_page_obj);
        menu_rs485_wg_test_page_obj = NULL;
		show_label = NULL;
		if(pText != NULL)
		{
			free(pText);
			pText = NULL;
		}
		extiSendRecv = 0;
    }
}

static void menu_rs485_wg_test_show()
{
    if (menu_rs485_wg_test_page_obj)
    {
        lv_obj_set_hidden(menu_rs485_wg_test_page_obj, false);
    }
}

static void menu_rs485_wg_test_hide()
{
    if (menu_rs485_wg_test_page_obj)
    {
        lv_obj_set_hidden(menu_rs485_wg_test_page_obj, true);
    }
}
/// 页面生命周期函数 end


