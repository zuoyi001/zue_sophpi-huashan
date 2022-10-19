#ifndef __APP_RTSP_H__
#define __APP_RTSP_H__

//int start_rtsp_server(int *VencChn, int num);
//void stop_rtsp_server(int *VencChn, int num);
int APP_InitVenc(void);
int APP_RgbVideoVencInit(void);
int APP_IrVideoVencInit(void);
int RTSP_CreateServer(int *VencChn, int num);
int RTSP_StopServer(int *VencChn, int num);

#endif