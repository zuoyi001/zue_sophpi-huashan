#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "app_ipcam_comm.h"
#include "app_ipcam_os.h"
#include "app_ipcam_mq.h"
#include "app_ipcam_cmd.h"

#define CVI_CMD_CLIENT_ID_CC_TOOL (CVI_MQ_CLIENT_ID_USER_0)
#define CVI_CMD_CHANNEL_ID_CC(cc_id) (0x00 + (cc_id))

typedef struct _optionExt_ {
    struct option opt;
    int type;
    int min;
    int max;
    const char *help;
} optionExt;

typedef enum _ARG_TYPE_ {
	ARG_INT = 0,
	ARG_STRING,
} ARG_TYPE;

char *cmd_buf[CVI_CMD_BUT] = {
        "vparam", 
        "osd",
        "cover",
        "rect",
        "audio",
        "aipd",
        "aimd",
        "aifd",
        "auto_rgbir",
        "switch_pq",
        "flip", 
        "mirror", 
        "180",
        "rotate",
        "record",
        "mp3_play",
 /*       "rtsp",   // todo 
        "cap" 
        "rec"
        "pd", 
        "md", 
        "fd", 
        "face_rec", 
        "face_reg", 
        "ir", 
        "flip", 
        "mirror", 
        "rotate" */};

static const struct option vparam_opts[] = {
            { "index",      required_argument, 0, 'i' },
            { "codec",      required_argument, 0, 'c' },
            { "width",      required_argument, 0, 'w' },
            { "height",     required_argument, 0, 'h' },
            { "rcmode",     required_argument, 0, 'r' },
            { "bitrate",    required_argument, 0, 'b' },
            { "gop",        required_argument, 0, 'g' },
            { "fps",        required_argument, 0, 'f' },
            { NULL, 0, 0, 0 },
        };

static const struct option osd_opts[] = {
            { "index",      required_argument, 0, 'i' },
            { "show",       required_argument, 0, 's' },
            { "offset",     required_argument, 0, 'o' },
            { "running",    required_argument, 0, 'r' },
            { NULL, 0, 0, 0 },
        };

static const struct option cover_opts[] = {
            { "index",      required_argument, 0, 'i' },
            { "color",      required_argument, 0, 'c' },
            { NULL, 0, 0, 0 },
        };

static const struct option rect_opts[] = {
            { "switch",     required_argument, 0, 's' },
            { NULL, 0, 0, 0 },
        };

static const struct option audio_opts[] = {
            { "chn",        required_argument, 0, 'i' },
            { "codec",      required_argument, 0, 'c' },
            { "function",   required_argument, 0, 'f' },
            { "samplerate", required_argument, 0, 's' },
            { NULL, 0, 0, 0 },
        };

static const struct option aipd_opts[] = {
            { "switch",       required_argument, 0, 's' },
            { "threshold",    required_argument, 0, 't' },
            { NULL, 0, 0, 0 },
        };
static const struct option aimd_opts[] = {
            { "switch",       required_argument, 0, 's' },
            { "threshold",    required_argument, 0, 't' },
        };
static const struct option aifd_opts[] = {
            { "switch",       required_argument, 0, 's' },
            { "threshold",    required_argument, 0, 't' },
            { NULL, 0, 0, 0 },
        };
static const struct option auto_rgbir_opts[] = {
            { "switch",       required_argument, 0, 's' },
            { NULL, 0, 0, 0 },
        };
static const struct option switch_pq_opts[] = {
            { "select",       required_argument, 0, 's' },
            { NULL, 0, 0, 0 },
        };
static const struct option flip_opts[] = {
            { "switch",       required_argument, 0, 's' },
            { NULL, 0, 0, 0 },
        };
static const struct option mirror_opts[] = {
            { "switch",       required_argument, 0, 's' },
            { NULL, 0, 0, 0 },
        };

static const struct option fm_180_opts[] = {
            { "switch",       required_argument, 0, 's' },
            { NULL, 0, 0, 0 },
        };
static const struct option rotate_opts[] = {
            { "switch",       required_argument, 0, 's' },
            { NULL, 0, 0, 0 },
        };

static const struct option record_opts[] = {
            { "record_status",    required_argument, 0, 's' },
            { "replay_status",    required_argument, 0, 'r' },
            { "check_month",      required_argument, 0, 'm' },
            { "check_days",       required_argument, 0, 'd' },
            { "check_status",     required_argument, 0, 'c' },
            { NULL, 0, 0, 0 },
        };
static const struct option mp3_play_opts[] = {
            { "switch",       required_argument, 0, 's' },
            { NULL, 0, 0, 0 },
        };
static void print_usage(int argc, char *argv[])
{
    (void)argc;
    int i = 0;
    printf("Usage... \n");
    printf("%s %s [-iwhcrbgf]\n", argv[0], cmd_buf[i++]);
    puts("  -i --index      video encode channel index (e.g. \"0/1/2...\")\n"
        "  -w --width      video encode channel width\n"
        "  -h --height     video encode channel height\n"
        "  -c --codec      video encode channel codec (e.g. \"264/265/mjp\")\n"
        "  -r --rcmode     video encode channel RC mode (e.g. \"264cbr/264vbr/264avbr/264qvbr/264qpmap/\n"
        "                        265cbr/265vbr/265avbr/265qvbr/265fixqp/265qpmap/mjpgcbr/mjpgvbr/mjpgfixqp\")\n"
        "  -b --bitrate    video encode channel bitrate (Kb)\n"
        "  -g --gop        video encode channel gop\n"
        "  -f --framerate  video encode channel framerate\n");

    printf("%s %s [-isof]\n", argv[0], cmd_buf[i++]);
    puts("  -i --index      osd channel handle index (e.g. \"0/1/2...\")\n"
        "  -s --show       osd show or hidden (e.g. 1:show; 0:hide)\n"
        "  -o --offset     osd offset step\n"
        "  -r --running    osd task running (e.g. 1:start; 0:stop)\n");

    printf("%s %s [-is]\n", argv[0], cmd_buf[i++]);
    puts("  -i --index      cover channel handle index (e.g. \"0/1/2...\")\n"
        "  -c --color      set cover color\n");

    printf("%s %s [-s]\n", argv[0], cmd_buf[i++]);
    puts("  -s --switch    AI draw Rect switch on or off\n");

    printf("%s %s [-is]\n", argv[0], cmd_buf[i++]);
    puts("  -i --chn        audio chn\n"
         "  -m --mode       audio sound mode\n"
         "  -c --codec      audio encode       (e.g. 19: g711a, 20: g711u, 21: g726 37:aac\")\n"
         "  -f --function   audio vqe function (e.g. elf 1:agc; 10:anr 100:aec)\n"
         "  -s --samplerate audio samplerate\n");

    printf("%s %s [-st]\n", argv[0], cmd_buf[i++]);
    puts("  -s --switch    Ai PD switch on / off (e.g. 1:on; 0:off)\n"
         "  -t --threshold Ai FD threshold(e.g. \"1/2/100...\")\n"); 

    printf("%s %s [-s]\n", argv[0], cmd_buf[i++]);
    puts("  -s --switch    Ai MD switch on / off (e.g. 1:on; 0:off)\n");
    printf("%s %s [-st]\n", argv[0], cmd_buf[i++]);
    puts("  -s --switch    Ai FD switch on / off (e.g. 1:on; 0:off)\n"
         "  -t --threshold Ai FD threshold(e.g. \"1/2/100...\")\n");
    
    printf("%s %s [-s]\n", argv[0], cmd_buf[i++]);
    puts("  -s --switch    auto Switch rgb_ir modle on / off (e.g. 1:on; 0:off)\n");

    printf("%s %s [-s]\n", argv[0], cmd_buf[i++]);
    puts("  -s --select    Select PQ to cat (e.g. 0:RGB; 1:IR; 2:color night 3:wdr)\n");

    printf("%s %s [-s]\n", argv[0], cmd_buf[i++]);
    puts("  -s --switch    flip Switch on / off (e.g. 1:on; 0:off)\n");

    printf("%s %s [-s]\n", argv[0], cmd_buf[i++]);
    puts("  -s --switch    mirror Switch on / off (e.g. 1:on; 0:off)\n");

    printf("%s %s [-s]\n", argv[0], cmd_buf[i++]);
    puts("  -s --switch    flip and mirror Switch on / off (e.g. 1:on; 0:off)\n");

    printf("%s %s [-s]\n", argv[0], cmd_buf[i++]);
    puts("  -s --switch    rotate Switch  on / off (e.g. 1:on; 0:off)\n");

    printf("%s %s [-srmdc]\n", argv[0], cmd_buf[i++]);
    puts("  -s --record_status      record status (e.g. 1:on; 0:off)\n"
        "  -r --replay_status       replay status (e.g. \"20220707121100\":on; \"0\":off)\n"
        "  -m --check_month         check month have record (e.g. \"202207\")\n"
        "  -d --check_days          check day have record segment (e.g. \"20220707\")\n"
        "  -c --check_status        check record status\n");

    printf("%s %s [-s]\n", argv[0], cmd_buf[i++]);
    puts("  -s --switch    mpe play  on / off (e.g. 1:on; 0:off)\n");
}

static int parse_opts(int argc, char *argv[], int cmd_id, char *param)
{
    int ret = 0;

    switch (cmd_id) {
    case CVI_CMD_VIDEO_ATTR: {
            int ch;
            while ((ch = getopt_long(argc, argv, "i:c:w:h:r:b:g:f:", vparam_opts, NULL)) != -1) {
                printf("ch = %c\t", ch);
                switch (ch) {
                    case 'i':
                        {
                            strcat(param, "i:");
                            printf("index --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                    break;
                    case 'c':
                        {
                            strcat(param, "c:");
                            printf("codec --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                    break;
                    case 'w':
                        {
                            strcat(param, "w:");
                            printf("width --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                        break;
                    case 'h':
                        {
                            strcat(param, "h:");
                            printf("height --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                        break;
                    case 'r':
                        {
                            strcat(param, "r:");
                            printf("rcmode --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                        break;
                    case 'b':
                        {
                            strcat(param, "b:");
                            printf("bitrate --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                        break;
                    case 'g':
                        {
                            strcat(param, "g:");
                            printf("gop --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                        break;
                    case 'f':
                        {
                            strcat(param, "f:");
                            printf("fps --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                        break;
                    default:
                        print_usage(argc, argv);
                        return 0;
                        break;
                }
            }
            param[strlen(param) - 1] = '\0';
        }
        break;

    case CVI_CMD_OSD_SWITCH: {
            int ch;
            while ((ch = getopt_long(argc, argv, "i:s:o:r:", osd_opts, NULL)) != -1) {
                printf("ch = %c\t", ch);
                switch (ch) {
                    case 'i':
                        {
                            strcat(param, "i:");
                            printf("index --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                    break;
                    case 's':
                        {
                            strcat(param, "s:");
                            printf("show --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                    break;
                    case 'o':
                        {
                            strcat(param, "o:");
                            printf("offset --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                    break;
                    case 'r':
                        {
                            strcat(param, "r:");
                            printf("running --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                    break;
                    default:
                        print_usage(argc, argv);
                        return 0;
                        break;
                }
            }
            param[strlen(param) - 1] = '\0';
        }
        break;

    case CVI_CMD_COVER_SWITCH: {
            int ch;
            while ((ch = getopt_long(argc, argv, "i:c:", cover_opts, NULL)) != -1) {
                printf("ch = %c\t", ch);
                switch (ch) {
                    case 'i':
                        {
                            strcat(param, "i:");
                            printf("index --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                    break;
                    case 'c':
                        {
                            strcat(param, "c:");
                            printf("color --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                    break;
                    default:
                        print_usage(argc, argv);
                        return 0;
                        break;
                }
            }
            param[strlen(param) - 1] = '\0';
        }
        break;

    case CVI_CMD_RECT_SWITCH: {
            int ch;
            while ((ch = getopt_long(argc, argv, "s:", rect_opts, NULL)) != -1) {
                printf("ch = %c\t", ch);
                switch (ch) {
                    case 's':
                        {
                            strcat(param, "s:");
                            printf("color --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                    break;
                    default:
                        print_usage(argc, argv);
                        return 0;
                        break;
                }
            }
            param[strlen(param) - 1] = '\0';
        }
        break;

    case CVI_CMD_AUDIO_SWITCH: {
            int ch;
            while ((ch = getopt_long(argc, argv, "i:m:c:f:s:", audio_opts, NULL)) != -1) {
                printf("ch = %c\t", ch);
                switch (ch) {
                    case 'i':
                        {
                            strcat(param, "i:");
                            printf("chn --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                    break;
                    case 'm':
                        {
                            strcat(param, "m:");
                            printf("chn --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                    break;
                    case 'c':
                        {
                            strcat(param, "c:");
                            printf("codec --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                    break;
                    case 'f':
                        {
                            strcat(param, "f:");
                            printf("function --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                    break;
                    case 's':
                        {
                            strcat(param, "s:");
                            printf("samplerate --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                    break;
                    default:
                        print_usage(argc, argv);
                        return 0;
                        break;
                }
            }
            param[strlen(param) - 1] = '\0';
        }
        break;

    case CVI_CMD_AI_PD_SWITCH: {
        int ch;
        while ((ch = getopt_long(argc, argv, "s:t:", aipd_opts, NULL)) != -1) {
            printf("ch = %c\t", ch);
            switch (ch) {
                case 's':
                    {
                        strcat(param, "s:");
                        printf("switch --> %s\n", optarg);
                        strcat(param, optarg);
                        strcat(param, "/");
                    }
                break;
                case 't':
                    {
                        strcat(param, "t:");
                        printf("threshold --> %s\n", optarg);
                        strcat(param, optarg);
                        strcat(param, "/");
                    }
                break;
                default:
                    print_usage(argc, argv);
                    return 0;
                    break;
            }
        }
            param[strlen(param) - 1] = '\0';
        }
        break;

    case CVI_CMD_AI_MD_SWITCH: {
        int ch;
        while ((ch = getopt_long(argc, argv, "s:", aimd_opts, NULL)) != -1) {
            printf("ch = %c\t", ch);
            switch (ch) {
                case 's':
                    {
                        strcat(param, "s:");
                        printf("switch --> %s\n", optarg);
                        strcat(param, optarg);
                        strcat(param, "/");
                    }
                break;
                default:
                    print_usage(argc, argv);
                    return 0;
                    break;
            }
        }
            param[strlen(param) - 1] = '\0';
        }
        break;
    case CVI_CMD_AI_FD_SWITCH: {
        int ch;
        while ((ch = getopt_long(argc, argv, "s:t:", aifd_opts, NULL)) != -1) {
            printf("ch = %c\t", ch);
            switch (ch) {
                case 's':
                    {
                        strcat(param, "s:");
                        printf("switch --> %s\n", optarg);
                        strcat(param, optarg);
                        strcat(param, "/");
                    }
                break;
                case 't':
                    {
                        strcat(param, "t:");
                        printf("threshold --> %s\n", optarg);
                        strcat(param, optarg);
                        strcat(param, "/");
                    }
                break;    
                default:
                    print_usage(argc, argv);
                    return 0;
                    break;
            }
        }
            param[strlen(param) - 1] = '\0';
        }
        break;

     case CVI_CMD_AUTO_RGB_IR_SWITCH: {
        int ch;
        while ((ch = getopt_long(argc, argv, "s:", auto_rgbir_opts, NULL)) != -1) {
            printf("ch = %c\t", ch);
            switch (ch) {
                case 's':
                    {
                        strcat(param, "s:");
                        printf("switch --> %s\n", optarg);
                        strcat(param, optarg);
                        strcat(param, "/");
                    }
                break;
                default:
                    print_usage(argc, argv);
                    return 0;
                    break;
            }
        }
            param[strlen(param) - 1] = '\0';
        }
        break;
        
        case CVI_CMD_PQ_CAT: {
        int ch;
        while ((ch = getopt_long(argc, argv, "s:", switch_pq_opts, NULL)) != -1) {
            printf("ch = %c\t", ch);
            switch (ch) {
                case 's':
                    {
                        strcat(param, "s:");
                        printf("select --> %s\n", optarg);
                        strcat(param, optarg);
                        strcat(param, "/");
                    }
                break;
                default:
                    print_usage(argc, argv);
                    return 0;
                    break;
            }
        }
            param[strlen(param) - 1] = '\0';
        }
        break;

        case CVI_CMD_FLIP_SWITCH: {
        int ch;
        while ((ch = getopt_long(argc, argv, "s:", flip_opts, NULL)) != -1) {
            printf("ch = %c\t", ch);
            switch (ch) {
                case 's':
                    {
                        strcat(param, "s:");
                        printf("switch --> %s\n", optarg);
                        strcat(param, optarg);
                        strcat(param, "/");
                    }
                break;
                default:
                    print_usage(argc, argv);
                    return 0;
                    break;
            }
        }
            param[strlen(param) - 1] = '\0';
        }
        break;

        case CVI_CMD_MIRROR_SWITCH: {
        int ch;
        while ((ch = getopt_long(argc, argv, "s:", mirror_opts, NULL)) != -1) {
            printf("ch = %c\t", ch);
            switch (ch) {
                case 's':
                    {
                        strcat(param, "s:");
                        printf("switch --> %s\n", optarg);
                        strcat(param, optarg);
                        strcat(param, "/");
                    }
                break;
                default:
                    print_usage(argc, argv);
                    return 0;
                    break;
            }
        }
            param[strlen(param) - 1] = '\0';
        }
        break;

        case CVI_CMD_180_SWITCH: {
        int ch;
        while ((ch = getopt_long(argc, argv, "s:", fm_180_opts, NULL)) != -1) {
            printf("ch = %c\t", ch);
            switch (ch) {
                case 's':
                    {
                        strcat(param, "s:");
                        printf("switch --> %s\n", optarg);
                        strcat(param, optarg);
                        strcat(param, "/");
                    }
                break;
                default:
                    print_usage(argc, argv);
                    return 0;
                    break;
            }
        }
            param[strlen(param) - 1] = '\0';
        }
        break;

        case CVI_CMD_ROTATE_SWITCH: {
        int ch;
        while ((ch = getopt_long(argc, argv, "s:", rotate_opts, NULL)) != -1) {
            printf("ch = %c\t", ch);
            switch (ch) {
                case 's':
                    {
                        strcat(param, "s:");
                        printf("switch --> %s\n", optarg);
                        strcat(param, optarg);
                        strcat(param, "/");
                    }
                break;
                default:
                    print_usage(argc, argv);
                    return 0;
                    break;
            }
        }
            param[strlen(param) - 1] = '\0';
        }
        break;

        case CVI_CMD_RECORD_SET:
        {
            int ch;
            while ((ch = getopt_long(argc, argv, "s:r:m:d:c", record_opts, NULL)) != -1) {
                printf("ch = %c\t", ch);
                switch (ch) {
                    case 's':
                        {
                            strcat(param, "s:");
                            printf("record --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                    break;
                    case 'r':
                        {
                            strcat(param, "r:");
                            printf("replay --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                    break;
                    case 'm':
                        {
                            strcat(param, "m:");
                            printf("time --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                        break;
                    case 'd':
                        {
                            strcat(param, "d:");
                            printf("time --> %s\n", optarg);
                            strcat(param, optarg);
                            strcat(param, "/");
                        }
                        break;
                    case 'c':
                        {
                            strcat(param, "c");
                            strcat(param, "/");
                        }
                        break;
                    default:
                        print_usage(argc, argv);
                        return 0;
                        break;
                }
            }
            param[strlen(param) - 1] = '\0';
            break;
        }

        case CVI_CMD_MP3_PLAY: {
        int ch;
        while ((ch = getopt_long(argc, argv, "s:", mp3_play_opts, NULL)) != -1) {
            printf("ch = %c\t", ch);
            switch (ch) {
                case 's':
                    {
                        strcat(param, "s:");
                        printf("switch --> %s\n", optarg);
                        strcat(param, optarg);
                        strcat(param, "/");
                    }
                break;
                default:
                    print_usage(argc, argv);
                    return 0;
                    break;
            }
        }
            param[strlen(param) - 1] = '\0';
        }
        break;

    default:
        break;
    }

    return ret;
}

static int app_ipcam_CmdMq_Send(
            int client_id,
            int client_chn,
            int cmd_id,
            int cmd_val,
            char *param)
{
    int ret = 0;

    static int i = 0;
    // TODO: implement ACK

    // test CVI_MQ_Send_RAW
    CVI_MQ_MSG_t msg = {0};

    int len = (int)CVI_MQ_MSG_HEADER_LEN;

    snprintf(msg.payload, CVI_MQ_MSG_PAYLOAD_LEN, "%s", param);
    printf("msg.payload: %s, len = %"PRIdFAST32"\n", msg.payload, strlen(msg.payload));
    msg.target_id = CVI_MQ_ID(client_id, client_chn);
    msg.arg1 = cmd_id;
    msg.arg2 = cmd_val;
    msg.needack = 0;
    msg.seq_no = i++;
    msg.len = len+strlen(param);
    uint64_t boot_time;
    app_ipcam_boot_time_get(&boot_time);
    msg.crete_time = boot_time;

    ret = CVI_MQ_Send_RAW(&msg);
    if (ret != 0) {
        return -1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    int ret = 0;
    int i = 0;
    int cmd_id = 0;
    int cmd_val = 0;

    if (argv[1] == NULL) {
        print_usage(argc, argv);
        exit(1);
    }

    for (i = 0; i < CVI_CMD_BUT; i++) {
        if (!strcmp(argv[1], cmd_buf[i])) {
            cmd_id = i;
            break;
        }
    }
    if (i >= CVI_CMD_BUT) {
        print_usage(argc, argv);
        exit(-1);
    }

    if (argv[2] != NULL) {
        cmd_val = atoi(argv[2]);
        if (cmd_val < 0) {
            printf("arg_val failed\n");
            return -1;
        }
    }

    char param[128] = {0};
    ret = parse_opts(argc, argv, cmd_id, param);
    if (ret != 0) {
        printf("%s %d failed with %d\n", __FUNCTION__, __LINE__, ret);
        exit(-1);
    }

    ret = app_ipcam_CmdMq_Send(
        CVI_CMD_CLIENT_ID_CC_TOOL, 
        CVI_CMD_CHANNEL_ID_CC(0), 
        cmd_id, 
        cmd_val, 
        param);
    if (ret != 0) {
        exit(-1);
    }

	return ret;
}

