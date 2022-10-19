#include <stdlib.h>
#include <stdio.h>
#include "zscanner.h"

zbar_image_scanner_t *zbar_scanner;
zbar_image_t         *zbar_image  ;

void* zscanner_init(int w, int h)
{


    /* zbar scanner init */
    zbar_scanner = zbar_image_scanner_create();
    zbar_image_scanner_set_config(zbar_scanner, ZBAR_NONE  , ZBAR_CFG_ENABLE, 0);
    zbar_image_scanner_set_config(zbar_scanner, ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);

    /* wrap image data */
    zbar_image = zbar_image_create();
    zbar_image_set_format(zbar_image, *(int*)"Y800");
    zbar_image_set_size  (zbar_image, w, h);
}

void zscanner_exit()
{
    zbar_image_destroy(zbar_image);
    zbar_image_scanner_destroy(zbar_scanner);
}

char* zscanner_scan(char *image)
{
    const zbar_symbol_t *symbol = NULL;
    char *data = NULL;
    int   ret;

    /* set zbar image data */
    zbar_image_set_data(zbar_image, image, 640*360, zbar_image_free_data);

    /* scan the image for barcodes */
    ret = zbar_scan_image(zbar_scanner, zbar_image);
    if (ret <= 0) return data;

    /* extract results */
    symbol = zbar_image_first_symbol(zbar_image);
    for(; symbol; symbol = zbar_symbol_next(symbol)) {
        /* do something useful with results */
        zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
        data = (char*)zbar_symbol_get_data(symbol);
        printf("decoded %s symbol \"%s\"\n", zbar_get_symbol_name(typ), data);
    }

    return data;
}

