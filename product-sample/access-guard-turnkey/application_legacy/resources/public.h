#ifndef _PUBLIC_H_
#define _PUBLIC_H_
#include <time.h>
#include <sys/time.h>
extern cvi_device_config_t gDevice_Cfg;

#define STORAGE_USB_PATH    	"/mnt/usb"
#define UPDATE_FOFT_FILE_NAME   "/mnt/data/softw.tar.gz"

/*************************************************************
 * �������ƣ� CheckUsbExist
 * ���ܣ� check usb�豸�Ƿ����
 * ������ pDev: ����豸�� ��ΪNULL
 * ���أ� ����Ϊ1 ������Ϊ0
 ***************************************************************/
int CheckUsbExist(char *pDev);
void get_mac_show();
char* get_mac1();
char* get_sn();
int set_sn(char* psn, int size);
int set_mac(char* pmac);
int getonelinesn(char* sn , int snlen);
int setsnbeenuse();
int getonelinemac(char* mac , int maclen);
int setmacbeenuse();
void GetDateFromTime(time_t t, char* buf);
int Update();
int getUpdateResult();
#endif