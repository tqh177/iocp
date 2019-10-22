#include "str.h"

// 比较两个字符串是否相等
BOOL strequ(char *s1, char *s2)
{
    while (*s1 == *s2)
    {
        if (*s1 == 0)
        {
            return TRUE;
        }
        s1++;
        s2++;
    }
    return FALSE;
}

BOOL memequ(void *p1, void *p2, size_t n)
{
    char *c1 = p1, *c2 = p2;
    size_t i;
    for (i = 0; i < n; i++)
    {
        if (*c1 != *c2)
        {
            return FALSE;
        }
        c1++;
        c2++;
    }
    return TRUE;
}
