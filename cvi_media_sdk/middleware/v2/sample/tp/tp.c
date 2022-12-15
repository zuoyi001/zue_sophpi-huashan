#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <inttypes.h>
#include <errno.h>
#include <pthread.h>

#include "tp.h"
//#include "dbg.h"

#define MAX_BUF 64
#define PANEL_H 1280
#define PANEL_W 720

static int tp_fd = -1;

static pthread_t tp_t;

static int Display_W;
static int Display_H;

/*If EVDEV_XXX_MIN > EVDEV_XXX_MAX the XXX axis is automatically inverted*/
#define EVDEV_HOR_MIN   1
#define EVDEV_VER_MIN   8
#define HLPrintf1 printf

int Tp_Postion_Mapping(int x, int in_min, int in_max, int out_min, int out_max)
{
	div_t val;
	val = div((out_max - out_min), (in_max - in_min));
	
	return (x - in_min) * val.quot + out_min;
}

int Tp_Postion_Mapping_(int x, uint32_t s_x, uint32_t d_x)
{
    int x_ = ((float) s_x / (float) d_x) * x;

	return x_;
}

int Tp_Init(int node)
{
	char buf[MAX_BUF]={'\0'};
	snprintf(buf, MAX_BUF, "/dev/input/event%d", node);
	printf("operate tp node :%s.\n", buf);

	tp_fd = open(buf, O_RDONLY);
	if (tp_fd < 0) {
		printf("open %s failed\n", buf);
		return -1;
	}
	return 0;
}


int Tp_Postion_Get(int *x, int *y)
{
	int touch_ret = -1 ;	
    //1、定义一个结构体变量用来描述ft5x0x触摸屏事件  
    struct input_event tp_event ;  
    
    //2、读事件  
    touch_ret = read(tp_fd, &tp_event, sizeof(tp_event));  
    if (touch_ret < 0) {
		printf("read touch fair!\n");
    }
	//printf("event info, type:%x, code:%x, tp_event.value:%d\n", tp_event.type, tp_event.code, tp_event.value);
    //3、判断事件类型
    switch(tp_event.type) {  
		case EV_SYN:  
			//printf("EV_SYN type:%x, code:%x, tp_event.value:%d\n", tp_event.type, tp_event.code, tp_event.value);
			break ;

		case EV_ABS:
			if (tp_event.code == ABS_MT_POSITION_X) {
				*x = tp_event.value;
				//printf("X type:%x,code:%x,tp_event.value:%d\n", tp_event.type, tp_event.code, tp_event.value);
			}
			if (tp_event.code == ABS_MT_POSITION_Y) {
				*y = tp_event.value;
				//printf("Y type:%x,code:%x,tp_event.value:%d\n", tp_event.type, tp_event.code, tp_event.value);
			}
			break;

        default:
			//printf("default type:%x,code:%x,tp_event.value:%d\n", tp_event.type, tp_event.code, tp_event.value);
			break;
    }
    
    return 0;
}

static void *tp_postion_get_thread()
{
	char szTdName[MAX_BUF] = "tp_pos_get_thread";
	prctl(PR_SET_NAME, szTdName, 0, 0, 0);

	int tp_x, tp_y;
	int tp_map_x, tp_map_y;
    int tp_map_xx, tp_map_yy;

	while (1) {
		////Obtain the absolute coordinate points on the screen
		Tp_Postion_Get(&tp_x, &tp_y);
        //Modify coordinates according to scale
		tp_map_x = Tp_Postion_Mapping_(tp_x , PANEL_H, Display_H);
		tp_map_y = Tp_Postion_Mapping_(tp_y , PANEL_W, Display_W);

        //Modify 0 point position
        tp_map_x = PANEL_H - tp_map_x;
        tp_map_y = PANEL_W - tp_map_y;

        //Modify x-axis and y-axis and set offset
        tp_map_xx = tp_map_y - EVDEV_HOR_MIN;
        tp_map_yy = tp_map_x - EVDEV_VER_MIN;

        printf("tp_map_xy(%d,%d) \n", tp_map_xx, tp_map_yy);
		usleep(100);
	}

	return 0;
}

int Tp_Postion_Looping()
{
	int ret = 0;
	pthread_create(&tp_t, NULL, tp_postion_get_thread, NULL);

	return ret;
}

static int Tp_Option_Handle(int node, int op)
{
    int ret = 0;

    switch (op) {
        case 0:
            ret = Tp_Init(node);
            break;

		case 1:
			ret = Tp_Postion_Looping();
            break;	

        default:
            HLPrintf1("op :%d not valid, pls try again \n", op);
            break;
    }

    return ret;
}

void main(int argc, char *argv[], int v[])
{
    int op = 0;
    int node;

	if ((argc == 2) || (strcmp((const char *)argv[2], "--help") == 0) || (strcmp((const char *)argv[2], "help") == 0)) {  
        printf("pls input panel resolution (ex: 1280 720): \n");
		if (scanf("%d %d", &Display_W, &Display_H) == 0) {
			printf("input value not match current format (%%d)\n");
		}  	
        HLPrintf1("Set screen resolution to WxH(%dx%d) \n", Display_W, Display_H);
		printf("pls input operate node id[0/1/2] : \n");
		if (scanf("%d", &node) == 0) {
			printf("input value not match current format (%%d)\n");
		}
        do {
            HLPrintf1(" 0: tp_event_%d init         \n", node);
            HLPrintf1(" 1: tp_%d postion get looping\n", node);

            printf("pls input operate case from[0~1]: \n");
            if (scanf("%d", &op) == 0) {
				printf("input value not match current format (%%d)\n");
			}

            Tp_Option_Handle(node, op);
        } while(op != 255);

	} else {
        HLPrintf1("input ./sample tp help ; %d", v[3]);
    }
}
