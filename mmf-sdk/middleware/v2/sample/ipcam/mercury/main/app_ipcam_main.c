#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "app_ipcam_paramparse.h"
#include "app_ipcam_gpio.h"
#include "app_ipcam_selftest.h"

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define VERSION_NUMBER          "V1.12.7"
#define VERSION_LAST_UPDATE     "2022-07-01 PM"
#define VERSION_CHANGE_NOTE     "added SC401AI sensor"
#define IPCAM_VERSION_PRINT                                             \
    do {                                                                \
        puts("***************************************************");    \
        printf("    Version:      %s\n", VERSION_NUMBER);               \
        printf("    last update:  %s\n", VERSION_LAST_UPDATE);          \
        printf("    change notes: %s\n", VERSION_CHANGE_NOTE);          \
        puts("/***************************************************\n"); \
    } while(0)

/* 182x EVB IR-CUT switch control */
#define IR_CUT_A    CVI_GPIOC_17
#define IR_CUT_B    CVI_GPIOC_16
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct APP_PARAM_MAIN_CTRL_S {
    bool bInit;
    char config_ini[64];
    pthread_mutex_t mutexLock;
} APP_PARAM_MAIN_CTRL_T;

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
APP_PARAM_MAIN_CTRL_T g_stMainCtrl = {
    .bInit = CVI_FALSE,
    .mutexLock = PTHREAD_MUTEX_INITIALIZER,
};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static int app_ipcam_Exit(void);

APP_PARAM_MAIN_CTRL_T *app_ipcam_MainCtrl_Param_Get(void)
{
    return &g_stMainCtrl;
}

static CVI_VOID app_ipcam_ExitSig_handle(CVI_S32 signo)
{
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    
    if ((SIGINT == signo) || (SIGTERM == signo)) {
        app_ipcam_Exit();
        APP_PROF_LOG_PRINT(LEVEL_INFO, "ipcam receive a signal(%d) from terminate\n", signo);
    }

    exit(-1);
}

static CVI_VOID app_ipcam_Usr1Sig_handle(CVI_S32 signo)
{    
    if (SIGUSR1 == signo) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "ipcam receive a signal(%d) from terminate and start trigger a picture\n", signo);
        app_ipcam_JpgCapFlag_Set(CVI_TRUE);
    }
}

static CVI_VOID app_ipcam_Usr2Sig_handle(CVI_S32 signo)
{    
    if (SIGUSR2 == signo) {
        APP_PROF_LOG_PRINT(LEVEL_INFO, "ipcam receive a signal(%d) from terminate and switch video size\n", signo);
        app_ipcam_VencSize_Set();
    }
}
static int app_ipcam_Peripheral_Init(void)
{
    /* do peripheral Init here */

    // app_ipcam_IrCut_Switch(IR_CUT_A, IR_CUT_B, CVI_TRUE);

    return CVI_SUCCESS;
}

static int app_ipcam_Exit(void)
{
    #ifdef RECORD_SUPPORT
    /* start SD Record */
    APP_CHK_RET(app_ipcam_Record_UnInit(), "running SD Record");
    #endif
    
    #ifdef AI_SUPPORT
    APP_CHK_RET(app_ipcam_Ai_PD_Stop(), "PD Stop");

    APP_CHK_RET(app_ipcam_Ai_MD_Stop(), "MD Stop");

    APP_CHK_RET(app_ipcam_Ai_FD_Stop(), "FD Stop");
    #endif

    APP_CHK_RET(app_ipcam_rtsp_Server_Destroy(), "RTSP Server Destroy");

    APP_CHK_RET(app_ipcam_Venc_Stop(APP_VENC_ALL), "Venc Stop");

    #ifdef AUDIO_SUPPORT
    APP_CHK_RET(app_ipcam_Audio_UnInit(), "Audio Stop");
    #endif

    APP_CHK_RET(app_ipcam_Osd_DeInit(), "OSD DeInit");

    APP_CHK_RET(app_ipcam_Cover_DeInit(), "Cover DeInit");

    APP_CHK_RET(app_ipcam_Vpss_DeInit(), "Vpss DeInit");

    APP_CHK_RET(app_ipcam_Vi_DeInit(), "Vi DeInit");

    APP_CHK_RET(app_ipcam_Sys_DeInit(), "System DeInit");

    return CVI_SUCCESS;

}

static int app_ipcam_Init(void)
{
    if (g_stMainCtrl.bInit) {
        APP_PROF_LOG_PRINT(LEVEL_WARN, "app_ipcam_Init initialized \n");
        return CVI_SUCCESS;
    }

    APP_CHK_RET(app_ipcam_Peripheral_Init(), "init peripheral");

    APP_CHK_RET(app_ipcam_Sys_Init(), "init systerm");

    APP_CHK_RET(app_ipcam_Vi_Init(), "init vi module");

    APP_CHK_RET(app_ipcam_Vpss_Init(), "init vpss module");

    APP_CHK_RET(app_ipcam_Osd_Init(), "init OSD");

    APP_CHK_RET(app_ipcam_Cover_Init(), "init Cover");

    APP_CHK_RET(app_ipcam_Venc_Init(APP_VENC_ALL), "init video encode");

    #ifdef AUDIO_SUPPORT
    APP_CHK_RET(app_ipcam_Audio_Init(), "start audio processing");
    #endif

    g_stMainCtrl.bInit = CVI_TRUE;

    return CVI_SUCCESS;
}

int main(int argc, char *argv[])
{
    IPCAM_VERSION_PRINT;

    APP_CHK_RET(app_ipcam_Opts_Parse(argc, argv), "parse optinos");

    signal(SIGINT, app_ipcam_ExitSig_handle);
    signal(SIGTERM, app_ipcam_ExitSig_handle);
    signal(SIGUSR1, app_ipcam_Usr1Sig_handle);
    signal(SIGUSR2, app_ipcam_Usr2Sig_handle);

    /* load each moudles parameter from param_config.ini */
    APP_CHK_RET(app_ipcam_Param_Load(), "load global parameter");

    /* init modules include <Peripheral; Sys; VI; VB; OSD; Venc; AI; Audio; etc.> */
    APP_CHK_RET(app_ipcam_Init(), "app_ipcam_Init");

    /* create rtsp server */
    APP_CHK_RET(app_ipcam_Rtsp_Server_Create(), "create rtsp server");

    /* start video encode */
    APP_CHK_RET(app_ipcam_Venc_Start(APP_VENC_ALL), "start video processing");

    #ifdef AI_SUPPORT
    /* start AI PD (Pedestrian Detection) */
    APP_CHK_RET(app_ipcam_Ai_PD_Start(), "running AI PD");

    /* start AI MD (Motion Detection)*/
    APP_CHK_RET(app_ipcam_Ai_MD_Start(), "running AI MD");

    /* start AI FD (Face Detection)*/
    APP_CHK_RET(app_ipcam_Ai_FD_Start(), "running AI FD");
    #endif

    #ifdef RECORD_SUPPORT
    /* start SD Record */
    APP_CHK_RET(app_ipcam_Record_Init(), "running SD Record");
    #endif

    /* enable receive a command form another progress for test ipcam */
    APP_CHK_RET(app_ipcam_CmdTask_Create(), "running cmd test");

    while (1) {
        sleep(1);
    };
    
    return CVI_SUCCESS;
}
