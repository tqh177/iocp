#ifndef CHAIN_H
#define CHAIN_H
#include <stddef.h>
#define MAX_CHAIN_LEN 1024 * 8

typedef unsigned char BYTE;
typedef struct chain_s
{
    BYTE byte[MAX_CHAIN_LEN];
    size_t length;
    struct chain_s *next;
} chain_t;

chain_t *newChain();
void chain_push(chain_t *s, BYTE bytes[], size_t bytes_len);
chain_t *chain_unshift(chain_t *s, BYTE bytes[], size_t bytes_len);
chain_t *chain_shift(chain_t *s);
void chain_free(chain_t *s);
char *chain2string(chain_t *s);
size_t chain_len(chain_t *s);
void chain_connect(chain_t *s, chain_t *s1);

#endif