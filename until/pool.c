#include "pool.h"
#include <malloc.h>
#include <string.h>
pool_t *gloab_pool = NULL;
void *Malloc(size_t size)
{
#ifdef _DEBUG
    if (size > MAX_POOL_SIZE / 2)
    {
        return malloc(size);
    }
    if (gloab_pool == NULL)
    {
        gloab_pool = newPool(MAX_POOL_SIZE);
    }
    return pool_malloc(size, gloab_pool);
#else
    return malloc(size);
#endif
}

void Free(void *p)
{
#ifdef _DEBUG
    pool_free(p, gloab_pool);
#else
    free(p);
#endif
}

pool_t *newPool(size_t pool_size)
{
    pool_t *pool = malloc(sizeof(pool_t) + pool_size);
    if (pool == NULL)
        return NULL;
    pool->space = NULL;
    pool->Buf = (char *)pool + sizeof(pool_t);
    pool->len = pool_size;
    pool->next = NULL;
    return pool;
}

#pragma region
// void *pool_malloc_(size_t size, pool_t *pool)
// {
//     pool_info_t *space;
//     if (pool == NULL || pool->len < size)
//         return NULL;
//     while (1)
//     {
//         space = pool->space;
//         do
//         {
//             if (size <= space->length)
//             {
//                 void *p;
//                 p = &(pool->Buf[space->from]);
//                 space->from += size;
//                 space->length -= size;
//                 return p;
//             }
//             space = space->next;
//         } while (space);
//         if (pool->next == NULL)
//             break;
//         pool = pool->next;
//     }
//     pool->next = newPool(MAX_POOL_SIZE);
//     return pool_malloc(size, pool->next);
// }
#pragma endregion

// ���ڴ�ط����ڴ�
void *pool_malloc(size_t size, pool_t *pool)
{
    pool_info_t *space;
    int poolLen = 0;
    if (pool == NULL || pool->len < size || size == 0)
        return NULL;
    do
    {
        poolLen += pool->len;
        space = pool->space;
        if (space == NULL)
        {
            pool->space = (pool_info_t *)(pool->Buf + size);
            pool->space->from = 0;
            pool->space->length = size + sizeof(pool_info_t);
            pool->space->next = NULL;
            return pool->Buf;
        }
        while (space)
        {
            size_t from = space->from + space->length;
            size_t to = from + size + sizeof(pool_info_t);
            if (to < pool->len && (space->next == NULL || to < space->next->from))
            {
                pool_info_t *temp = (pool_info_t *)(pool->Buf + from + size);
                temp->from = from;
                temp->length = size + sizeof(pool_info_t);
                temp->next = space->next;
                space->next = temp;
                return pool->Buf + from;
            }
            space = space->next;
        }
        if (pool->next == NULL)
            break;
        pool = pool->next;
    } while (1);
    if (poolLen + MAX_POOL_SIZE < MAX_MEMORY_SIZE)
    {
        pool->next = newPool(MAX_POOL_SIZE);
        return pool_malloc(size, pool->next);
    }
    return NULL;
}

// ���ڴ���ͷű���
void pool_free(void *p, pool_t *pool)
{
    if (p == NULL && pool == NULL)
        return;
    pool_info_t *space, *pre = NULL;
    pool_t *pre_pool = pool;
    do
    {
        // ���ڵ�ǰ�ڴ����
        if ((void *)(pool->Buf) <= p && p < (void *)(pool->Buf + pool->len))
        {
            space = pool->space;
            // �����ڴ���ڴ�������
            while (space)
            {
                // ���з������
                if (pool->Buf + space->from == p)
                {
                    // ��ǰ�����ڵ�һ��
                    if (pre == NULL)
                    {
                        pool->space = space->next;
                    }
                    else
                    {
                        pre->next = space->next;
                    }
                    // ��ǰ�ڴ��Ϊ�գ��ͷ�
                    if (pool->space == NULL && pool != pre_pool)
                    {
                        pre_pool->next = pool->next;
                        pool->next = NULL;
                        pool_clear(pool);
                    }
                    return;
                }
                pre = space;
                space = space->next;
            };
            // �˴�����pΪ�Ƿ��ڴ�
            return;
        }
        pre_pool = pool;
        pool = pool->next;
    } while (pool);
    // �����ڴ����,������ֱ����malloc������ڴ�
    free(p);
}

// ����ڴ��
void pool_clear(pool_t *pool)
{
    if (pool)
    {
        pool_clear(pool->next);
        free(pool);
    }
}