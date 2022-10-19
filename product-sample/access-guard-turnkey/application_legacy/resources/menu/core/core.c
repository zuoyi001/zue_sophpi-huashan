#include "core.h"
#include "msgq.h"
#include "face_rect.h"
#include "write_mac_sn.h"
#include "update_dialog.h"
#include "down_user_dialog.h"


static msgq_t gui_msgq;

void gui_comp_init()
{
    msgq_init(&gui_msgq, "gui_msgq");
}

void gui_comp_sendcmd(int cmd, void *args, int args_len, void *ext, int ext_len)
{
    msg_node_t msg;
    memset(&msg, 0, sizeof(msg_node_t));
    msg.msg_type = cmd;
    if (args != NULL && args_len > 0 && args_len <= MSG_DATA_LEN) {
        memcpy(msg.data, args, args_len);
    }

    if (!msgq_send(&gui_msgq, &msg))
    {
        printf("error send cmd\n");
    }
}

void gui_task_handler()
{
    do {
        check_popmenu();

        msg_node_t msg;
        memset(&msg, 0, sizeof(msg_node_t));
        if (msgq_receive(&gui_msgq, &msg))
        {
            switch(msg.msg_type) {
                case CMD_UI_DRAW_FACE_RECT:
                {
                    face_rect_s rect;
                    memcpy(&rect, msg.data, sizeof(face_rect_s));
                    face_rect_draw_face_4_points(rect.x, rect.y, rect.width,\
                                rect.height, rect.line_id, rect.rect_color);
                    break;
                }
                case CMD_UI_CLEAR_FACE_RECT:
                {
                    face_rect_hide_rects();
                    face_rect_hide_name();
                    break;
                }
                case CMD_UI_SHOW_POPMENU:
                {
                    pop_menu_info_s info;
                    memcpy(&info, msg.data, sizeof(pop_menu_info_s));
                    show_popmenu(info.face_img, info.name_str, info.result_str,\
                                info.width, info.height, info.bPass);
                    break;
                }
                case CMD_UI_HIDE_POPMENU:
                {
                    hide_popmenu();
                    break;
                }
				case CMD_UI_MAC_SN_POPMENU:
                {
                    show_write_mac_sn_popmenu();
                    break;
                }
				case CMD_UI_UPDATE_POPMENU:
                {
                    show_update_popmenu();
                    break;
                }
				case CMD_UI_DOWN_USER_POPMENU:
                {
                    show_down_user_popmenu();
                    break;
                }
                case CMD_UI_UNKNOWN:
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
        
    } while(msgq_get_len(&gui_msgq));
}
