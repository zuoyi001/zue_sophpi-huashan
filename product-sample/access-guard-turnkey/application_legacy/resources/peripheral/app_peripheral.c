#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <sys/prctl.h>

//#include "utils.h"
#include "app_peripheral_comp.h"
#include "factory.h"
// #include "aealgo.h"
#include "app_peripheral.h"

extern CVI_S16 AE_GetCurrentLvX100(CVI_U8 sID);

#define PRINT_HEX(data, lens)                             \
        do {                                              \
            for (int i = 0; i < lens; i++) {              \
                DBG_PRINTF(CVI_DEBUG, "%02X", data[i]);                  \
            }                                             \
        } while (0)

unsigned char card[4];
pthread_mutex_t getcard_lock;

static void gpio_init()
{
#ifdef ENABLE_PERI

    //SPK Unmute
    PERIPHERAL_GPIO_Export(SPK_MUTE);
    PERIPHERAL_GPIO_DirectionOutput(SPK_MUTE);
    CVI_HAL_GpioSetValue(SPK_MUTE, CVI_HAL_GPIO_VALUE_H);

    //IR LED
    PERIPHERAL_GPIO_Export(IRLED_GPIO);
    PERIPHERAL_GPIO_DirectionOutput(IRLED_GPIO);
    CVI_HAL_GpioSetValue(IRLED_GPIO, CVI_HAL_GPIO_VALUE_H);

    //relay_c
    PERIPHERAL_GPIO_Export(RELAY_GPIO);
    PERIPHERAL_GPIO_DirectionOutput(RELAY_GPIO);

    //door_key
    PERIPHERAL_GPIO_Export(DOORKEY_GPIO);
    CVI_HAL_GpioDirectionInput(DOORKEY_GPIO);

    //door_mag
    PERIPHERAL_GPIO_Export(DOORMAG_GPIO);
    CVI_HAL_GpioDirectionInput(DOORMAG_GPIO);

    //remove button
    PERIPHERAL_GPIO_Export(RM_GPIO);
    CVI_HAL_GpioDirectionInput(RM_GPIO);

    #if (CUSTOMIZED_PCBA == 1)
    //fire alarm in
    PERIPHERAL_GPIO_Export(ALARM_GPIO);
    CVI_HAL_GpioDirectionInput(ALARM_GPIO);

    //rs485
    PERIPHERAL_GPIO_Export(RS485_GPIO);
    PERIPHERAL_GPIO_DirectionOutput(RS485_GPIO);
	
    #elif (CUSTOMIZED_PCBB == 1)
    //LED_R_EN
    PERIPHERAL_GPIO_Export(LED_R);
    PERIPHERAL_GPIO_DirectionOutput(LED_R);

    //LED_G_EN
    PERIPHERAL_GPIO_Export(LED_G);
    PERIPHERAL_GPIO_DirectionOutput(LED_G);

    //LED_B_EN
    PERIPHERAL_GPIO_Export(LED_B);
    PERIPHERAL_GPIO_DirectionOutput(LED_B);

    //BT_RST_N
    PERIPHERAL_GPIO_Export(BT_RST);
    PERIPHERAL_GPIO_DirectionOutput(BT_RST);
   
    #endif
	
#endif
}
#if defined(NFC_SUPPORT)
static pthread_t nfc_tid = 0;
static int nfc_test(unsigned char * uid)
{
    unsigned char MIFARE_KEY[6], BLOCK_BUF[16], VALUE_BUF[4], INC_BUF[4], DEC_BUF[4], result;

    memcpy(MIFARE_KEY, "\xFF\xFF\xFF\xFF\xFF\xFF", 6); //设置认证密钥数组

    result = CVI_NFC_Mifare_Auth(0, 0x01, MIFARE_KEY, uid); //扇区1进�?�KEY A 认证
    if (result != 0)
    {
        DBG_PRINTF(CVI_DEBUG, "-> Sector [1] Auth -1\r\n");
        return -1;
    }

    DBG_PRINTF(CVI_DEBUG, "-> Sector [1] Auth OK\r\n");

    memcpy(BLOCK_BUF, "\x11\x22\x33\x44\x55\x66\x77\x88\x11\x22\x33\x44\x55\x66\x77\x88", 
        16); //设置待写入的16�?字节数组

    result = CVI_NFC_Mifare_Blockwrite(0x04, BLOCK_BUF); //块地址0x04（扇�?1，块0）写块操�?
    if (result != 0)
    {
        DBG_PRINTF(CVI_DEBUG, "-> Block [4] Write -1\r\n");
        return -1;
    }

    DBG_PRINTF(CVI_DEBUG, "-> Block [4] Write OK \r\n");

    result = CVI_NFC_Mifare_Blockread(0x04, BLOCK_BUF); //块地址0x04（扇�?1，块0）�?�块操作
    if (result != 0)
    {
        DBG_PRINTF(CVI_DEBUG, "-> Block [4] Read -1\r\n");
        return -1;
    }

    DBG_PRINTF(CVI_DEBUG, "-> Block [4] Read Data = ");
    PRINT_HEX(BLOCK_BUF, 16);
    DBG_PRINTF(CVI_DEBUG, "\r\n");

    memcpy(VALUE_BUF, "\x44\x44\x44\x44", 4); //数值块设置�?44 44 44 44
    result = CVI_NFC_Mifare_Blockset(0x04, VALUE_BUF);
    if (result != 0)
    {
        DBG_PRINTF(CVI_DEBUG, "-> Block [4] Set -1\r\n");
        return -1;
    }

    DBG_PRINTF(CVI_DEBUG, "-> Block [4] Set OK\r\n");

    memcpy(VALUE_BUF, "\x44\x44\x44\x44", 4); //数值块设置�?44 44 44 44
    result = CVI_NFC_Mifare_Blockset(0x05, VALUE_BUF);
    if (result != 0)
    {
        DBG_PRINTF(CVI_DEBUG, "-> Block [5] Set -1\r\n");
        return -1;
    }

    DBG_PRINTF(CVI_DEBUG, "-> Block [5] Set OK\r\n");

    result = CVI_NFC_Mifare_Blockread(0x04, BLOCK_BUF); //块地址0x04（扇�?1，块0）�?�块操作
    if (result != 0)
    {
        DBG_PRINTF(CVI_DEBUG, "-> Block [4] Read -1\r\n");
        return -1;
    }

    DBG_PRINTF(CVI_DEBUG, "-> Block [4] Read Data = ");
    PRINT_HEX(BLOCK_BUF, 16);
    DBG_PRINTF(CVI_DEBUG, "\r\n");

    memcpy(INC_BUF, "\x01\x02\x03\x04", 4); //增�? 01 02 03 04

    result = CVI_NFC_Mifare_Blockinc(0x04, INC_BUF); //块地址0x04（扇�?1，块0）�?�值操�?
    if (result != 0)
    {
        DBG_PRINTF(CVI_DEBUG, "-> Block [4] Inc -1\r\n");
        return -1;
    }

    DBG_PRINTF(CVI_DEBUG, "-> Block [4] Inc OK\r\n");

    result = CVI_NFC_Mifare_Transfer(0x04); //块地址0x04（扇�?1，块0）传输操�?
    if (result != 0)
    {
        DBG_PRINTF(CVI_DEBUG, "->  Block [4] Transfer -1\r\n");
        return -1;
    }

    DBG_PRINTF(CVI_DEBUG, "-> Block [4] Transfer OK\r\n");

    result = CVI_NFC_Mifare_Blockread(0x04, BLOCK_BUF); //块地址0x04（扇�?1，块0）�?�块操作
    if (result != 0)
    {
        DBG_PRINTF(CVI_DEBUG, "-> Block [4] Read -1\r\n");
        return -1;
    }

    DBG_PRINTF(CVI_DEBUG, "-> Block [4] Read Data = ");
    PRINT_HEX(BLOCK_BUF, 16);
    DBG_PRINTF(CVI_DEBUG, "\r\n");

    memcpy(DEC_BUF, "\x00\x12\x34\x56", 4); //减�? 00 12 34 56
    result = CVI_NFC_Mifare_Blockdec(0x04, DEC_BUF); //块地址0x04（扇�?1，块0）�?�值操�?
    if (result != 0)
    {
        DBG_PRINTF(CVI_DEBUG, "->  Block [4] Dec -1\r\n");
        return -1;
    }

    DBG_PRINTF(CVI_DEBUG, "-> Block [4] Dec OK\r\n");

    result = CVI_NFC_Mifare_Transfer(0x04); //块地址0x04（扇�?1，块0）传输操�?
    if (result != 0)
    {
        DBG_PRINTF(CVI_ERROR, "->  Block [4] Transfer -1\r\n");
        return -1;
    }

    DBG_PRINTF(CVI_DEBUG, "-> Block [4] Transfer OK\r\n");

    result = CVI_NFC_Mifare_Blockread(0x04, BLOCK_BUF); //块地址0x04（扇�?1，块0）�?�块操作
    if (result != 0)
    {
        DBG_PRINTF(CVI_ERROR, "-> Block [4] Read -1\r\n");
        return -1;
    }

    DBG_PRINTF(CVI_DEBUG, "-> Block [4] Read Data = ");
    PRINT_HEX(BLOCK_BUF, 16);
    DBG_PRINTF(CVI_DEBUG, "\r\n");

    result = CVI_NFC_Mifare_Restore(0x05); //块地址0x05（扇�?1，块0）恢复操�?
    if (result != 0)
    {
        DBG_PRINTF(CVI_ERROR, "->  Block [5] Restore -1\r\n");
        return -1;
    }

    DBG_PRINTF(CVI_DEBUG, "-> Block [5] Restore OK\r\n");

    result = CVI_NFC_Mifare_Transfer(0x04); //块地址0x04（扇�?1，块0）传输操�?
    if (result != 0)
    {
        DBG_PRINTF(CVI_ERROR, "->  Block [4] Transfer -1\r\n");
        return -1;
    }

    DBG_PRINTF(CVI_DEBUG, "-> Block [4] Transfer OK\r\n");

    result = CVI_NFC_Mifare_Blockread(0x04, BLOCK_BUF); //块地址0x04（扇�?1，块0）�?�块操作
    if (result != 0)
    {
        DBG_PRINTF(CVI_DEBUG, "-> Block [4] Read -1\r\n");
        return -1;
    }

    DBG_PRINTF(CVI_DEBUG, "-> Block [4] Read Data = ");
    PRINT_HEX(BLOCK_BUF, 16);
    DBG_PRINTF(CVI_DEBUG, "\r\n");
    return 0;
}

static void * nfc_thread(void * arg)
{
    unsigned char uid[4];
    char szThreadName[20] = "nfc_thread";

	pthread_mutex_init(&getcard_lock, NULL);

    prctl(PR_SET_NAME, szThreadName, 0, 0, 0);

    while (1)
    {
        if (0 == CVI_NFC_CardActivate(uid))
        {
            printf("UID = ");
            PRINT_HEX(uid, 4);
            printf("\r\n");
			set_card_no(uid);

            //printf("---NFC Test Begin---\r\n");
            //nfc_test(uid);
            //printf("---NFC Test END---\r\n");

            CVI_NFC_CardHalt();
        }
        usleep(500 * 1000);
    }
    return NULL;
}
#endif

static void nfc_init()
{
#ifdef ENABLE_PERI
    int ret;

    ret = CVI_NFC_Init();
    if (ret < 0)
    {
        perror("nfc_init fail");
        return;
    }

    ret = pthread_create(&nfc_tid, NULL, nfc_thread, NULL);
    if (ret != 0)
    {
        perror("pthread_creat fail");
    }

    return;

#endif
}

static void wiegand_init()
{
    struct wgn_rx_cfg rx_cfg;
    struct wgn_tx_cfg tx_cfg;

    //40ns per bit
    rx_cfg.rx_debounce = 0x32; //2us
    rx_cfg.rx_idle_timeout = 0x2625A0; //100ms
    rx_cfg.rx_bitcount = 26; //26bits
    rx_cfg.rx_msb1st = 1;

    tx_cfg.tx_lowtime = 0x00186A; //250us
    tx_cfg.tx_hightime = 0x00C350; //2ms
    tx_cfg.tx_bitcount = 26; //26bits
    tx_cfg.tx_msb1st = 1;
    tx_cfg.tx_opendrain = 0;

    if (CVI_HAL_WiegandInit(&tx_cfg, &rx_cfg) < 0)
        DBG_PRINTF(CVI_ERROR, "wiegand init failed\n");
}

static void rs485_init()
{
    if (CVI_HAL_UartInit("/dev/ttyS1") < 0)
        DBG_PRINTF(CVI_ERROR, "uart init failed\n");

    /* factory test should set 9600 */
    /* if you want to change baud rate, remember to call this funtion */
    if (CVI_HAL_UartSetParam(9600, 0, 8, 1, 'N') < 0)
        DBG_PRINTF(CVI_DEBUG, "uart set param failed\n");
}

static void *watchdog_control_thread(void *p)
{
	char szThreadName[20] = "watchdog_thread";

	prctl(PR_SET_NAME, szThreadName, 0, 0, 0);

	while (1) {
		CVI_HAL_WatchdogKeepalive();
		sleep(8);
	}
    return NULL;
}

int watchdog_init(void)
{
	int res;
	pthread_t id;

	if (CVI_HAL_WatchdogOpen("/dev/watchdog") < 0) {
		DBG_PRINTF(CVI_ERROR, "watchdog open failed\n");
		return -1;
	}

	if (CVI_HAL_WatchdogEnable() < 0) {
		DBG_PRINTF(CVI_ERROR, "watchdog enable failed\n");
		return -1;
	}

    /* call this funtion with parameter less than 8s to reboot system*/
	if (CVI_HAL_WatchdogSetTimeout(WATCHDOG_TIMEOUT) < 0) {
		DBG_PRINTF(CVI_ERROR, "watchdog set timeout failed\n");
		return -1;
	}

	res = pthread_create(&id, 0, watchdog_control_thread, 0);
	if (res)
		DBG_PRINTF(CVI_DEBUG, "%s\n", strerror(res));
        
	pthread_detach(id);

	return 0;
}

#if 0
static void * screen_saver_thread(void * arg)
{
    while (true)
    {
        if (facelib_config.fd_en)
        { // only check while fd enabled 

            // status = get 24G radar is distance
            // if(status++ == THRESHOLD) 
            // {
            // 	enable screen saver.
            // }
            // else
            // {
            // 	disable screen saver
            // }
        }
    }
}
#endif

bool timer_start = false;
time_t start;

void *ledon_timer_thread(void * arg)
{
    time_t current;

    time(&start);
    timer_start = true;
    CVI_Test_Led(50);
    do 
    {
        //设置当前时间
        time(&current);
        usleep(2000*1000);
    }
    while((current - start) <= TURNONLEG_TIMER);

    timer_start = false;
    CVI_Test_Led(0);

    return NULL;
}

int Get_CurrentLv()
{
    int val = AE_GetCurrentLvX100(0);

    DBG_PRINTF(CVI_DEBUG, "light val = %d\n", val);

    return val;
}

int LED_OnOFF(bool onoff)
{
    int ret = 0;

    if (onoff)
    {
        if (timer_start)
        {
            time(&start);
        }
        else 
        {
            job_t * job = (job_t *)malloc(sizeof(job_t));
            job->job_function = ledon_timer_thread;
            job->data = NULL;
            thread_pool_add(&pool, job);
        }

    }
    else // turn off led
    {
        CVI_Test_Led(0);
    }

    return ret;
}

void set_card_no(unsigned char * pUID)
{
	pthread_mutex_lock(&getcard_lock);
	memcpy(card,pUID,4);
	pthread_mutex_unlock(&getcard_lock);
}

void get_card_no(unsigned char * pUID)
{
	pthread_mutex_lock(&getcard_lock);
	if(strlen((char *)card)!= 0)
	{
		memcpy(pUID,card,4);
		memset(card, 0x00, sizeof(card));
	}
	pthread_mutex_unlock(&getcard_lock);
}

int IR_OnOFF(bool onoff)
{
    int ret = 0;
    #ifdef ENABLE_PERI
    if (onoff)
    {
        CVI_HAL_GpioSetValue(IRLED_GPIO, CVI_HAL_GPIO_VALUE_H);
    }
    else
    {
        CVI_HAL_GpioSetValue(IRLED_GPIO, CVI_HAL_GPIO_VALUE_L);
    }
    #endif
    return ret;
}

static int panel_pwm_init(int pwmGrp, int pwmId, int duty_Range)
{
	int duty_cycle;

	CVI_HAL_PwmDisable(pwmGrp, pwmId);
	CVI_HAL_PwmUnExport(pwmGrp, pwmId);

	if (duty_Range <= 0) {
		return 0;
	} else if (duty_Range >= PWM_LCD_DUTY_RANGE) {
		duty_cycle = PWM_LCD_PERIOD;
	} else {
		duty_cycle = PWM_LCD_DUTY_BASE * duty_Range;
	}

	CVI_HAL_PwmExport(pwmGrp, pwmId);
	CVI_HAL_PwmSetParm(pwmGrp, pwmId, PWM_LCD_PERIOD, duty_cycle);
	CVI_HAL_PwmEnable(pwmGrp, pwmId);

	return 0;
}
int peri_init(void)
{
    int ret = 0;

    gpio_init();

    nfc_init();

    // watchdog_init();

    rs485_init();

    wiegand_init();

    panel_pwm_init(0, 1, 50);

    return ret;
}

int get_radar_status(void)
{
    int ret = 1;
    return 1;  //radar issue
    #ifdef ENABLE_PERI
    PERIPHERAL_GPIO_NUM_E value;
    ret = CVI_HAL_GpioGetValue(RADAR_GPIO, &value);
    if(ret != 0)
    {
        DBG_PRINTF(CVI_ERROR, "get gpio value error,%s(%d)\n",__func__,__LINE__);
        return -1;
    }

    if(value == CVI_HAL_GPIO_VALUE_H)
    {
        ret = 1;
    }
    else 
    {
        ret = 2; 
        DBG_PRINTF(CVI_ERROR, "no person from radar\n");
    }
    #endif
    return ret;
}
