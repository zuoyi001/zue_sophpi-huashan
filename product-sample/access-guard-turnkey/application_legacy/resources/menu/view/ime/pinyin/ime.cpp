#include "ime.h"
#include "pinyinime.h"
#include "utf.h"
using namespace ime_pinyin;

#define DICT_PINYIN_PATH "./res/data/dict_pinyin.dat"
#define DICT_PINYIN_USER_PATH "./res/data/dict_pinyin_user.dat"
#define MAX_SPELL_LEN (128)
#define MAX_OUTPUT_LEN (128)

bool ime_init()
{
    bool ret = im_open_decoder(DICT_PINYIN_PATH, DICT_PINYIN_USER_PATH);
    if (ret == false)
        return ret;

    im_set_max_lens(MAX_SPELL_LEN, MAX_OUTPUT_LEN);
    im_reset_search();

    return ret;
}

void ime_reset_search()
{
    im_reset_search();
}

unsigned int ime_search(char *spell, int len)
{
    if (spell == NULL || len <= 0)
    {
        return 0;
    }

    size_t cand_num = im_search(spell, len);

    return cand_num;
}

unsigned int ime_add_letter(char ch)
{
    return im_add_letter(ch);
}

unsigned char * ime_get_candidate(unsigned int cand_id, unsigned char * cand_str, unsigned int max_len)
{
    if (cand_str == NULL || max_len <= 0)
    {
        return NULL;
    }

    unsigned short str_utf16[MAX_OUTPUT_LEN + 1] = {0};
    if (NULL != im_get_candidate(cand_id, str_utf16, max_len))
    {
        if (conversionOK == Utf16_To_Utf8(str_utf16, cand_str, max_len, strictConversion))
        {
            return cand_str;
        }
    }
    return NULL;
}