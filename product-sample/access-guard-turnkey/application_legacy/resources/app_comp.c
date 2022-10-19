#include <assert.h>
#include "app_utils.h"
#include "app_comp.h"
#include "msgq.h"
#include "dirent.h"
#include "menu.h"


static msgq_t app_main_msgq;
extern hal_facelib_handle_t facelib_handle ;
extern bool flagSetVpssAttr;

void APP_CompTaskHandler(void)
{
    do {
        msg_node_t msg;
        cvi_person_t person;
        int repo_num = 0;
	    CVI_S32 s32Ret = CVI_SUCCESS;

        //memset(&msg, 0, sizeof(msg_node_t));
        if (msgq_receive(&app_main_msgq, &msg))
        {
            switch(msg.msg_type) {
                case CMD_APP_COMP_UNKNOWN:
                    break;
                case CMD_APP_COMP_VIDEOSTOP:

                    if(get_app_comp_sm()!=CVI_STOP&&get_app_comp_sm()!=CVI_UNKNOW)
                    {
                        printf("CMD_APP_COMP_VIDEOSTOP event \n");
                        set_app_comp_sm(CVI_STOP);
                    }
                    msgq_clear(&app_main_msgq);
                    break;
                case CMD_APP_COMP_RESTART:
                    #ifndef CONFIG_CVI_SOC_CV182X
                    APP_SetVpssAspect(0,0,0,DISP_WIDTH,DISP_HEIGHT);
                    #else
                    CVI_APP_RecoverVpssAttr();
                    #endif
                    #ifdef DUAL_SENSOR
                    APP_SetIrVpssAspect(0,0,0,270,480,PIXEL_FORMAT_BGR_888);
                    #endif
                    set_register_status(false);
                    APP_SetFreezeStatus(false);

                    set_app_comp_sm(CVI_INIT);
                    break;
				case CMD_APP_COMP_GBR_START:
                    APP_SetVpssAspect(0,0,0,DISP_WIDTH,DISP_HEIGHT);
                    set_register_status(false);
                    set_app_comp_sm(CVI_GBR_INIT);
                    break;
				case CMD_APP_COMP_IR_START:
                    #ifndef CONFIG_CVI_SOC_CV182X
                    APP_SetIrVpssAspect(0,0,0,DISP_WIDTH,DISP_HEIGHT,PIXEL_FORMAT_YUV_PLANAR_420);
                    #else
                    APP_SetIrVpssAspect(0,0,0,DISP_WIDTH,DISP_HEIGHT,PIXEL_FORMAT_NV21);
                    #endif
                    set_register_status(false);
                    set_app_comp_sm(CVI_IR_INIT);
                    break;
                case CMD_APP_COMP_VIDEORESIZE:
                    set_register_status(true);
                    #ifndef CONFIG_CVI_SOC_CV182X
                    APP_SetVpssAspect(0,110,280,500,800);
                    #else
                    if(flagSetVpssAttr == false)
                        CVI_APP_SetVpssAttr();
                    #endif
                    set_app_comp_sm(CVI_INIT);
                    break;
                case CMD_APP_COMP_VIDEOFREEZE:
                    APP_SetFreezeStatus(true);
                    set_app_comp_sm(CVI_STOP);
                    break;

                case CMD_APP_COMP_FACE_REGISTER:

                    memcpy(&person, msg.data, sizeof(cvi_person_t));

                    if(SUCCESS != HAL_FACE_Register(facelib_handle,"/tmp/register.jpg",person))
                    {
                        DBG_PRINTF(CVI_ERROR, "register face fail\n");
                    }
                    else
                    {
                        DBG_PRINTF(CVI_ERROR, "register face successfully\n");
                    }

                    set_app_comp_sm(CVI_UNKNOW);
                    set_register_status(false);    

                    menu_tree_get_top().show();
                    CVI_APP_RecoverVpssAttr();
                    DBG_PRINTF(CVI_INFO, "Reg face done\n");

                    break;
                case CMD_APP_COMP_FACE_REGISTER_MULTI:

                    facelib_config.is_face_quality = true;

                    s32Ret = HAL_FACE_LibOpen(&facelib_config);
                    if (s32Ret != CVI_SUCCESS) {
                        DBG_PRINTF(CVI_ERROR, "CviFaceLibOpen failed with %d\n", s32Ret);
                        assert(0);
                    }

                    repo_num = HAL_FACE_GetRepoNum(facelib_handle);
                    int tmp = repo_num;
                    DIR *dir = NULL;
                    struct dirent *entry;
                    if((dir = opendir(FilePath))==NULL)
                    {
                        DBG_PRINTF(CVI_ERROR, "opendir failed!");

                        break;
                    }
                    else
                    {
                        while((entry = readdir(dir)))
                        {
                            char file_path[200];
                            bzero(file_path,200);
                            strcat(file_path,FilePath);
                            strcat(file_path,"/");
                            strcat(file_path,entry->d_name);
                            if(!strcmp(entry->d_name,".")||!strcmp(entry->d_name,".."))
                                continue;

                            // DBG_PRINTF(CVI_ERROR, "register %s...\n",entry->d_name); 
                            int i=0;
                            char *fn1 = NULL;
                            fn1 = entry->d_name;

                            while(true)
                            {
                                if(*(fn1+i)=='.')
                                {
                                    break;
                                }
                                i++;
                            }
                            *(fn1+i)='\0';

                            cvi_person_t person;
                            strcpy(person.name,fn1);
                            char serial_id[50]={0};
                            memset(serial_id,0,sizeof(serial_id));
                            sprintf(serial_id,"%d",repo_num++);
                            strncpy(person.serial, serial_id,strlen(serial_id));
                            int ret = 0;
                            ret = HAL_FACE_Register(facelib_handle,file_path,person);

                            if(SUCCESS != ret)
                            {
                                repo_num--;
                                if(ret == FAIL_MULTI_FACE)
                                {
                                    DBG_PRINTF(CVI_ERROR, "register %s...(multiface)\n",entry->d_name);
                                }
                                DBG_PRINTF(CVI_ERROR, "register fail\n");
                            }

                            DBG_PRINTF(CVI_ERROR, "register successfully\n");
                        }
                    }

                    facelib_config.is_face_quality = false;
                    //clean_network_face_quality();
                    set_register_status(false);
                    menu_tree_get_top().show();
                    DBG_PRINTF(CVI_ERROR, "Reg multi face done, total %d faces registed\n",(repo_num-tmp));

                    break;

                default:
                {
                    printf("default case\n");
                    break;
                }
            }

            if (msg.ext != NULL) {
                free(msg.ext);
                msg.ext = NULL;
            }
        }
    }while(0);
}

void APP_CompInit()
{
    msgq_init(&app_main_msgq, "app_comp_msgq");
    #if 0
    pthread_t h_app_comp_task;
    if (pthread_create(&h_app_comp_task, NULL, main_app_comp_task, NULL) == -1)
    {
        printf("create main app_comp task fail\n");
    }
    #endif
}

void APP_CompSendCmd(int cmd, void *args, int args_len, void *ext, int ext_len)
{
    msg_node_t msg;
    memset(&msg, 0, sizeof(msg_node_t));
    msg.msg_type = cmd;
    if (args != NULL && args_len > 0)//&& args_len <= MSG_DATA_LEN)
    {
        memcpy(msg.data, args, args_len);
    }

    if (!msgq_send(&app_main_msgq, &msg))
    {
        printf("error send cmd\n");
    }
}
