#ifndef _PERI_COMP_H_
#define _PERI_COMP_H_

enum {
    CMD_PERI_COMP_UNKNOWN,
    CMD_PERI_TURNONLEG_WITHTIMER,
};

void peri_comp_init();
void peri_comp_sendcmd(int cmd, void *args, int args_len, void *ext, int ext_len);
void peri_comp_task_handler();

#endif