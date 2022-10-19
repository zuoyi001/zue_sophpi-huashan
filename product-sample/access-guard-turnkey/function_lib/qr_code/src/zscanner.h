#ifndef __ZSCANNER_H__
#define __ZSCANNER_H__

#include "zbar.h"
#define ZBAR_VIDEO_WIDTH   704
#define ZBAR_VIDEO_HEIGHT  396
extern zbar_image_scanner_t * zbar_scanner;
extern zbar_image_t         * zbar_image  ;


void* zscanner_init(int w, int h);
void  zscanner_exit();
char* zscanner_scan(char *image);

#endif
