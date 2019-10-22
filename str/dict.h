#ifndef _DICT_H_
#define _DICT_H_

// typedef struct
// {
//     char *point;
//     unsigned int length;
// } sstr_t;

typedef char sstr_t;

typedef struct dict_s
{
    sstr_t key[512];
    sstr_t value[512];
    struct dict_s *next;
} dict_t;

dict_t *newDict();
void dict_free(dict_t *dict);
sstr_t *getDict(dict_t *dict, const char *key);

#endif