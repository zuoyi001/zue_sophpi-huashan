#ifndef _IME_H_
#define _IME_H_

#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif
bool ime_init();
void ime_reset_search();
unsigned int ime_search(char *spell, int len);
unsigned int ime_add_letter(char ch);
unsigned char * ime_get_candidate(unsigned int cand_id, unsigned char * cand_str, unsigned int max_len);

#ifdef	__cplusplus
}
#endif

#endif