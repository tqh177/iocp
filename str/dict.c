#include "dict.h"
#include "mymalloc.h"
#include <string.h>

dict_t *newDict()
{
    dict_t *h = mymalloc(sizeof(dict_t));
    memset(h, 0, sizeof(dict_t));
    return h;
}

void dict_free(dict_t *dict)
{
    if (dict)
    {
        dict_free(dict->next);
        myfree(dict);
    }
}
// 获取键值对
sstr_t *getDict(dict_t *dict, const char *key)
{
    while (dict)
    {
        if (strcmp(dict->key, key) == 0)
        {
            return dict->value;
        }
        dict = dict->next;
    }
    return NULL;
}