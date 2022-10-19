#include "app_peripheral_comp.h"
#include "msgq.h"

static msgq_t peri_msgq;

void peri_comp_init()
{
    msgq_init(&peri_msgq,"peri_msgq");
}

void peri_comp_sendcmd(int cmd, void *args, int args_len, void *ext, int ext_len)
{
    msg_node_t msg;
    memset(&msg, 0, sizeof(msg_node_t));
    msg.msg_type = cmd;

    if (args != NULL && args_len > 0)//&& args_len <= MSG_DATA_LEN) 
    {
        memcpy(msg.data, args, args_len);
    }

    if (!msgq_send(&peri_msgq, &msg))
    {
        printf("error send cmd\n");
    }
}

void peri_comp_task_handler()
{
    do {
        msg_node_t msg;
        memset(&msg, 0, sizeof(msg_node_t));
        if (msgq_receive(&peri_msgq, &msg))
        {
            switch(msg.msg_type) {
                case CMD_PERI_COMP_UNKNOWN:
                    break;
                case CMD_PERI_TURNONLEG_WITHTIMER:
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
