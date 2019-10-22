#include "chain.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"


static inline chain_t *_chain_end(chain_t *s);
static inline size_t _chain_fill(chain_t *s, BYTE bytes[], size_t bytes_len);

chain_t *_chain_end(chain_t *s)
{
    while (s->next)
    {
        s = s->next;
    }
    return s;
}

size_t _chain_fill(chain_t *s, BYTE bytes[], size_t bytes_len)
{
    if (s->length == MAX_CHAIN_LEN)
    {
        return 0;
    }
    else
    {
        size_t len = MAX_CHAIN_LEN - s->length;
        if (bytes_len < len)
        {
            len = bytes_len;
        }
        memcpy(s->byte + s->length, bytes, len);
        s->length += len;
        return len;
    }
}

inline chain_t *newChain()
{
    chain_t *s = mymalloc(sizeof(chain_t));
    s->length = 0;
    s->next = NULL;
    // memset(s, 0, sizeof(chain_t));
    return s;
}

void chain_push(chain_t *s, BYTE bytes[], size_t bytes_len)
{
    size_t len;
    s = _chain_end(s);
    // len = _chain_fill(s, bytes, bytes_len);
    // while (bytes_len -= len)
    // {
    //     bytes += len;
    //     s->next = newChain();
    //     s = s->next;
    //     len = _chain_fill(s, bytes, bytes_len);
    // }
    do
    {
        len = _chain_fill(s, bytes, bytes_len);
        bytes_len -= len;
        if (bytes_len <= 0)
            break;
        bytes += len;
        s->next = newChain();
        s = s->next;
    } while (1);
}
chain_t *chain_unshift(chain_t *s, BYTE bytes[], size_t bytes_len)
{
    chain_t *s1 = newChain();
    chain_push(s1, bytes, bytes_len);
    chain_connect(s1, s);
    return s1;
}
chain_t *chain_shift(chain_t *s)
{
    chain_t *next = s->next;
    s->next = NULL;
    chain_free(s);
    return next;
}
void chain_free(chain_t *s)
{
    if (s)
    {
        chain_free(s->next);
        myfree(s);
    }
}
size_t chain_len(chain_t *s)
{
    size_t len = s->length;
    while (s->next)
    {
        s = s->next;
        len += s->length;
    }
    return len;
}
void chain_connect(chain_t *s, chain_t *s1)
{
    s = _chain_end(s);
    s->next = s1;
}
char *chain2string(chain_t *s)
{
    size_t len = chain_len(s);
    char *str = mymalloc(sizeof(char) * (len + 1));
    char *hstr = str;
    str[len] = 0;
    do
    {
        memcpy(str, s->byte, s->length);
        str += s->length;
        s = s->next;
    } while (s);
    return hstr;
}
