#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "cviconfig.h"


//static cvi_device_config_t device_json_cfg;
static bool cfg_inited = false;

#define CONFIG_FILE_PATH  CONFIG_PATH
#define CONFIG_TEMPLATE_STR "{\"setting\": {}}"

void config_data_init()
{
    if (!cfg_inited)
    {
        if (access(CONFIG_FILE_PATH, F_OK) == -1) {
            FILE *fp;
            if ( (fp = fopen(CONFIG_FILE_PATH, "w+")) != NULL)
            {
                fwrite(CONFIG_TEMPLATE_STR, 1, strlen(CONFIG_TEMPLATE_STR), fp);
                fclose(fp);
            }
            else
            {
                printf("error create config.json file\n");
                return;
            }
        }

        cfg_inited = true;
    }
}
