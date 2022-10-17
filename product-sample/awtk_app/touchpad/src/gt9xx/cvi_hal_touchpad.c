#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/input.h>
#include <errno.h>
#include <sys/wait.h>

#include "cvi_hal_touchpad.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif  /* End of #ifdef __cplusplus */

/*  touchpad init state */
static bool s_bTOUCHPADInitState = false;
static int s_s32TOUCHPADfd = -1;
static bool s_bHALTPUCHPADEnable = true; /**< touchpad enable flag */

#define HAL_TOUCHPAD_KO_PATH        "/mnt/system/ko/3rd/gt9xx.ko"
#define HAL_TOUCHAPD_DEV           ("/dev/input/event0")
#define HAL_TOUCHAPD_MAX_POINT_NUM (1)
#define HAL_TOUCHAPD_BITLONG       (32)
#define HAL_TOUCHAPD_BITWORD(a) ((a) / HAL_TOUCHAPD_BITLONG)

/** multi touch info */
typedef struct tagHAL_TOUCHPAD_MTSAMPLE_S
{
    int s32ID;
    int s32X;
    int s32Y;
    int u32Pressure;
    struct timeval tv;
} HAL_TOUCHPAD_MTSAMPLE_S;

typedef struct tagHAL_TOUCHPAD_MTINPUT_S
{
    int as32Filled[HAL_TOUCHAPD_MAX_POINT_NUM];/**< MARK filled flag,array index is id */
    int as32X[HAL_TOUCHAPD_MAX_POINT_NUM];
    int as32Y[HAL_TOUCHAPD_MAX_POINT_NUM];
    int au32Pressure[HAL_TOUCHAPD_MAX_POINT_NUM];
    int s32Slot;/**< represent input id */
} HAL_TOUCHPAD_MTINPUT_S;

static  HAL_TOUCHPAD_MTINPUT_S s_stHALTOUCHPADMtInput;

static void HAL_TOUCHPAD_PinoutInit(void)
{
    printf("Set Pin Reuse OK.\n\n");
}

static inline int HAL_TOUCHPAD_InputBitCheck(int bit, const volatile uint32_t* addr)
{
    return 1UL & (addr[HAL_TOUCHAPD_BITWORD(bit)] >> (bit & (HAL_TOUCHAPD_BITLONG - 1)));
}

static int HAL_TOUCHPAD_PushSamp(HAL_TOUCHPAD_MTINPUT_S* pstMtInput, struct input_event* pstInputEvent, HAL_TOUCHPAD_MTSAMPLE_S* pstMtSamp)
{
    int s32Num = 0;
    int s32Slot = 0;

    for (s32Slot = 0; s32Slot < HAL_TOUCHAPD_MAX_POINT_NUM; s32Slot++)
    {
        if (!pstMtInput->as32Filled[s32Slot])
        {
            continue;/**<s32Slot index has no upadte data*/
        }

        pstMtSamp[s32Num].s32X = pstMtInput->as32X[s32Slot];
        pstMtSamp[s32Num].s32Y = pstMtInput->as32Y[s32Slot];
        pstMtSamp[s32Num].u32Pressure = pstMtInput->au32Pressure[s32Slot];
        pstMtSamp[s32Num].tv = pstInputEvent->time;
        pstMtSamp[s32Num].s32ID = s32Slot;

        pstMtInput->as32Filled[s32Slot] = 0;
        s32Num++;
    }
    return s32Num;
}

static int HAL_TOUCHPAD_ReadInputEventStatic(HAL_TOUCHPAD_MTSAMPLE_S* pstMtSamp)
{
    struct input_event stInputEvent;
    int s32Ret = 0;
    bool bDataReadCompleted = false;
    int s32Pushed = 0;
    int pen_up = 0;
    int type = 0;
    /** parm ps32Level check */
    if (NULL == pstMtSamp)
    {
        printf("pstMtSamp is null\n");
        return -1;
    }
    /** init check */
    if (-1 == s_s32TOUCHPADfd)
    {
        printf("touchpad not initialized\n");
        return -1;
    }
    while (false == bDataReadCompleted)
    {
        s32Ret = read(s_s32TOUCHPADfd, &stInputEvent, sizeof(struct input_event));

        if (s32Ret < (int)sizeof(struct input_event))
        {
            /** no more data */
            printf("s32Ret(%d) <sizeof(struct input_event)(%d)\n",(int)s32Ret,(int)sizeof(struct input_event));
            return -1;
        }

        switch (stInputEvent.type)
        {
                /** key 0x1*/
            case EV_KEY:

                //printf("event=EV_KEY:\n");
                switch (stInputEvent.code)
                {
                        /** 14a 0 */
                    case BTN_TOUCH:
                        (&s_stHALTOUCHPADMtInput)->au32Pressure[(&s_stHALTOUCHPADMtInput)->s32Slot] = stInputEvent.value;
                        (&s_stHALTOUCHPADMtInput)->as32Filled[(&s_stHALTOUCHPADMtInput)->s32Slot] = 1;
                        break;
                }

                break;

                /** 0x00 */
            case EV_SYN:
                //printf("stInputEvent.code == %d SYN_REPORT = %d SYN_MT_REPORT = %d\n",stInputEvent.code,SYN_REPORT,SYN_MT_REPORT);
                if (stInputEvent.code == SYN_REPORT) {
                        /* Fill out a new complete event */
                        if (pen_up) {
                            (&s_stHALTOUCHPADMtInput)->as32X[(&s_stHALTOUCHPADMtInput)->s32Slot] = 0;
                            (&s_stHALTOUCHPADMtInput)->as32Y[(&s_stHALTOUCHPADMtInput)->s32Slot] = 0;
                            (&s_stHALTOUCHPADMtInput)->au32Pressure[(&s_stHALTOUCHPADMtInput)->s32Slot] = 0;
                            (&s_stHALTOUCHPADMtInput)->as32Filled[(&s_stHALTOUCHPADMtInput)->s32Slot] = 1;
                            pen_up = 0;
                        }
                        s32Pushed = HAL_TOUCHPAD_PushSamp(&s_stHALTOUCHPADMtInput, &stInputEvent, pstMtSamp);
                        pstMtSamp += s32Pushed;
                        bDataReadCompleted = true;
                    } else if (stInputEvent.code == SYN_MT_REPORT) {
                        if (!type)
                            break;

                        if (type == 1) { /* no data: pen-up */
                            pen_up = 1;

                        } else {
                            type = 1;
                        }
                }
                break;

                /** 0x3 */
            case EV_ABS:
                switch (stInputEvent.code)
                {
                        /** 0x3a Pressure on contact area */
                    case ABS_PRESSURE:
                        //printf("event.code=ABS_PRESSURE.v=%d\n", stInputEvent.value);
                        (&s_stHALTOUCHPADMtInput)->au32Pressure[(&s_stHALTOUCHPADMtInput)->s32Slot] = stInputEvent.value;
                        break;
                    case ABS_MT_PRESSURE:
                        (&s_stHALTOUCHPADMtInput)->au32Pressure[(&s_stHALTOUCHPADMtInput)->s32Slot] = stInputEvent.value;
                        break;
                        /** 0x2f MT slot being modified */
                    case ABS_MT_SLOT:
                        //printf("event.code=ABS_MT_SLOT.v=%d\n", stInputEvent.value);
                        if (stInputEvent.value < 0)
                        {
                            break;
                        }

                        (&s_stHALTOUCHPADMtInput)->s32Slot = stInputEvent.value;
                        if ((&s_stHALTOUCHPADMtInput)->s32Slot >= HAL_TOUCHAPD_MAX_POINT_NUM)
                        {
                            //printf("slot limit error. MAX_POINT_NUM=%d. temp slot=%d\n", HAL_TOUCHAPD_MAX_POINT_NUM, (&s_stmtinput)->temp_slot);
                            (&s_stHALTOUCHPADMtInput)->s32Slot = 0;
                        }
                        break;

                        /** 0X30 Major axis of touching ellipse */
                    case ABS_MT_TOUCH_MAJOR:
                        //printf("event.code=ABS_MT_TOUCH_MAJOR.v=%d\n", stInputEvent.value);
                        if (stInputEvent.value == 0)
						    (&s_stHALTOUCHPADMtInput)->au32Pressure[(&s_stHALTOUCHPADMtInput)->s32Slot] = 0;
                        (&s_stHALTOUCHPADMtInput)->as32Filled[(&s_stHALTOUCHPADMtInput)->s32Slot] = 1;
                        break;

                        /** 0X35 */
                    case ABS_MT_POSITION_X:
                        //printf("event.code=ABS_MT_POSITION_X.v=%d\n", stInputEvent.value);
                        type++;
                        (&s_stHALTOUCHPADMtInput)->as32X[(&s_stHALTOUCHPADMtInput)->s32Slot] = stInputEvent.value;
                        (&s_stHALTOUCHPADMtInput)->as32Filled[(&s_stHALTOUCHPADMtInput)->s32Slot] = 1;

                        break;

                        /** 0X36 */
                    case ABS_MT_POSITION_Y:
                        //printf("event.code=ABS_MT_POSITION_Y.v=%d\n", stInputEvent.value);
                        type++;
                        (&s_stHALTOUCHPADMtInput)->as32Y[(&s_stHALTOUCHPADMtInput)->s32Slot] = stInputEvent.value;
                        (&s_stHALTOUCHPADMtInput)->as32Filled[(&s_stHALTOUCHPADMtInput)->s32Slot] = 1;
                        break;

                        /** 0X39 */
                    case ABS_MT_TRACKING_ID:
                        //printf("event.code=ABS_MT_TRACKING_ID.v=%d\n", stInputEvent.value);
                        if (stInputEvent.value == -1)
                        {
                            (&s_stHALTOUCHPADMtInput)->au32Pressure[(&s_stHALTOUCHPADMtInput)->s32Slot] = 0;
                            (&s_stHALTOUCHPADMtInput)->as32Filled[(&s_stHALTOUCHPADMtInput)->s32Slot] = 1;
                        }
                        break;
                }

                break;
        }
    }

    return 0;
}

int CVI_HAL_TOUCHPAD_Init(void)
{
    //int s32Ret = 0;

    if (false == s_bTOUCHPADInitState)
    {
        HAL_TOUCHPAD_PinoutInit();
        /** insmod touchpad driver */
        #if 0
        s32Ret = cvi_insmod(HAL_TOUCHPAD_KO_PATH,NULL);
        if(0 != s32Ret)
        {
            printf("insmod touchpad:failed, errno(%d)\n", errno);
            return -1;
        }
        #endif
        s_bTOUCHPADInitState = true;
    }
    else
    {
        printf("touchapd already init\n");
        return -1;
    }
    return 0;
}

int CVI_HAL_TOUCHPAD_Suspend(void)
{
    s_bHALTPUCHPADEnable = false;
    printf("touchpad suspend\n");
    return 0;
}

int CVI_HAL_TOUCHPAD_Resume(void)
{
    s_bHALTPUCHPADEnable = true;
    printf("touchpad resumme\n");
    return 0;
}

int CVI_HAL_TOUCHPAD_Start(int* ps32Fd)
{
    int s32Ret = 0;
    uint32_t au32Inputbits[(ABS_MAX + 31) / 32];
    if (-1 != s_s32TOUCHPADfd)
    {
        printf("touchpad already started\n");
        return -1;
    }
    memset(&s_stHALTOUCHPADMtInput, 0, sizeof(HAL_TOUCHPAD_MTINPUT_S));
    s_s32TOUCHPADfd = open(HAL_TOUCHAPD_DEV, O_RDONLY);
    *ps32Fd = s_s32TOUCHPADfd;
    if (s_s32TOUCHPADfd < 0)
    {
        printf("open /dev/input/event0 err,errno(%d)\n",errno);
        return -1;
    }

    s32Ret = ioctl(s_s32TOUCHPADfd, EVIOCGBIT(EV_ABS, sizeof(au32Inputbits)), au32Inputbits);
    if (s32Ret < 0)
    {
        printf("open ioctl err,errno(%d)\n",errno);
        close(s_s32TOUCHPADfd);
        s_s32TOUCHPADfd = -1;
        return -1;
    }

    if ( ( 0 == HAL_TOUCHPAD_InputBitCheck(ABS_MT_POSITION_X, au32Inputbits) )
         || ( 0 == HAL_TOUCHPAD_InputBitCheck(ABS_MT_POSITION_Y, au32Inputbits) )
         || ( 0 == HAL_TOUCHPAD_InputBitCheck(ABS_MT_TOUCH_MAJOR, au32Inputbits) )
       )
    {
        printf("error: could not support the device\n");
        printf("EV_SYN=%d\n", HAL_TOUCHPAD_InputBitCheck(EV_SYN, au32Inputbits));
        printf("EV_ABS=%d\n", HAL_TOUCHPAD_InputBitCheck(EV_ABS, au32Inputbits));
        printf("ABS_MT_POSITION_X=%d\n", HAL_TOUCHPAD_InputBitCheck(ABS_MT_POSITION_X, au32Inputbits));
        printf("ABS_MT_POSITION_Y=%d\n", HAL_TOUCHPAD_InputBitCheck(ABS_MT_POSITION_Y, au32Inputbits));
        printf("ABS_MT_TOUCH_MAJOR=%d\n", HAL_TOUCHPAD_InputBitCheck(ABS_MT_TOUCH_MAJOR, au32Inputbits));
        close(s_s32TOUCHPADfd);
        s_s32TOUCHPADfd = -1;
        return -1;
    }
    
    return 0;
}

int CVI_HAL_TOUCHPAD_Stop(void)
{
    int s32Ret = 0;
    s32Ret = close(s_s32TOUCHPADfd);
    if (0 != s32Ret)
    {
        printf("close err,errno(%d)\n",errno);
        return -1;
    }
    s_s32TOUCHPADfd = -1;
    return 0;
}

int CVI_HAL_TOUCHPAD_ReadInputEvent(CVI_HAL_TOUCHPAD_INPUTINFO_S* pstInputData)
{
    int s32Ret = 0;

    HAL_TOUCHPAD_MTSAMPLE_S astMtSample[HAL_TOUCHAPD_MAX_POINT_NUM];

    memset(&astMtSample,'\0',sizeof(astMtSample));
    s32Ret = HAL_TOUCHPAD_ReadInputEventStatic(astMtSample);
    if (0 != s32Ret)
    {
        printf("read_input_event err\n");
        return -1;
    }

    if (true == s_bHALTPUCHPADEnable)
    {
        pstInputData->s32ID = astMtSample[0].s32ID;
        pstInputData->u32Pressure = astMtSample[0].u32Pressure;
        pstInputData->s32X = 440 - astMtSample[0].s32Y;
        pstInputData->s32Y = astMtSample[0].s32X;
        pstInputData->u32TimeStamp = (astMtSample[0].tv.tv_sec) * 1000 + (astMtSample[0].tv.tv_usec) / 1000;
    }
    else
    {
        pstInputData->s32ID = 0;
        pstInputData->u32Pressure = 0;
        pstInputData->s32X = 0;
        pstInputData->s32Y = 0;
        pstInputData->u32TimeStamp = 0;
    }

    return 0;
}

int CVI_HAL_TOUCHPAD_Deinit(void)
{
    if (-1 != s_s32TOUCHPADfd)
    {
        printf("touchpad need stop first\n");
        return -1;
    }
    #if 0
    int s32Ret = 0;
    /** Deinitial touchpad Device */
    
    s32Ret = cvi_rmmod(HAL_TOUCHPAD_KO_PATH);
    if(0 != s32Ret)
    {
        printf("rmmod touchpad:failed, errno(%d)\n", s32Ret);
        return -1;
    }
    #endif
    s_bTOUCHPADInitState = false;
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif /* __cplusplus */
