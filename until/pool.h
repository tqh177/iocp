#ifndef _MEMERY_H_
#define _MEMERY_H_

#include <stddef.h>

// 最大内存池容量
#define MAX_POOL_SIZE 1024 * 12
#define MAX_MEMORY_SIZE 1024 * 1024 * 2

typedef struct poo_info_s
{
    unsigned int from;
    unsigned int length;
    struct poo_info_s *next;
} pool_info_t, small_pool_info_t, big_pool_info_t;

typedef int pool_1K_info_t;
typedef unsigned char BYTE;
// 内存池结构体
typedef struct pool_s
{
    BYTE *Buf;
    struct pool_s *next;
    pool_info_t *space;
    size_t len;
} pool_t;

typedef struct pool_1K_s
{
    BYTE Buf[MAX_POOL_SIZE];
    struct pool_1K_s *next;
    pool_1K_info_t space;
}pool_1K_t;

void *Malloc(size_t size);
void Free(void *p);
pool_t *newPool(size_t pool_size);
void *pool_malloc(size_t size, pool_t *pool);
void pool_free(void *p, pool_t *pool);
void pool_clear(pool_t *pool);
#endif