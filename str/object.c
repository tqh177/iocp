#include "object.h"
#include "chain.h"
#include "mymalloc.h"
#include <stdlib.h>
#include <string.h>

static void _obj2chain1(object *obj, chain_t *s);
static void _obj2chain(object *obj, chain_t *s);

// 创建新的对象
object *newObject()
{
    object *obj = mymalloc(sizeof(object));
    memset(obj, 0, sizeof(object));
    return obj;
}

// 判断是否有key,有则推入缓冲区
static void _obj2chain1(object *obj, chain_t *s)
{
    if (obj == NULL)
    {
        return;
    }
    else if (obj->parent->type == OBJECT_OBJECT)
    {
        chain_push(s, "\"", 1);
        chain_push(s, obj->key, strlen(obj->key));
        chain_push(s, "\":", 2);
    }
    _obj2chain(obj, s);
    if (obj->brother)
    {
        chain_push(s, ",", 1);
        _obj2chain1(obj->brother, s);
    }
}
// 根据对象类型转json
static void _obj2chain(object *obj, chain_t *s)
{
    switch (obj->type)
    {
    case OBJECT_OBJECT:
        chain_push(s, "{", 1);
        _obj2chain1(obj->child, s);
        chain_push(s, "}", 1);
        break;
    case OBJECT_ARRAY:
        chain_push(s, "[", 1);
        _obj2chain1(obj->child, s);
        chain_push(s, "]", 1);
        break;
    case OBJECT_STRING:
        chain_push(s, "\"", 1);
        chain_push(s, obj->value, strlen(obj->value));
        chain_push(s, "\"", 1);
        break;
    case OBJECT_BOOL:
    {
        char *c = obj->i ? "true" : "false";
        chain_push(s, c, strlen(c));
        break;
    }
    case OBJECT_NUMBER:
    {
        char str[20];
        ltoa(obj->i, str, 10);
        chain_push(s, str, strlen(str));
        break;
    }
    case OBJECT_NULL:
        chain_push(s, "null", 4);
        break;
    default:
        break;
    }
}

char *json_encode(object *obj)
{
    chain_t *s = newChain();
    char *str;
    _obj2chain(obj, s);
    str = chain2string(s);
    chain_free(s);
    return str;
}

void objectfree(object *obj)
{
    if (obj)
    {
        if (obj->parent->type == OBJECT_OBJECT)
        {
            myfree(obj->key);
        }
        switch (obj->type)
        {
        case OBJECT_OBJECT:
        case OBJECT_ARRAY:
            objectfree(obj->child);
            break;
        case OBJECT_STRING:
            myfree(obj->value);
            break;
        default:
            break;
        }
        objectfree(obj->brother);
        myfree(obj);
    }
}
// 跳过空白符
static char *skipSpace(char *str)
{
    while (*str && strchr(" \r\n\t", *str))
    {
        str++;
    }
    return str;
}
// 获取字符串
static char *getStr(char *str, char **str1)
{
    int i = 1;
    if (*str == '\'')
    {
        int flag = 0;
        while (str[i] != '\'' || flag)
        {
            if (str[i] == '\\')
            {
                flag = !flag;
            }
            else
            {
                flag = 0;
            }
            i++;
        }
        if (str1)
        {
            *str1 = mymalloc(sizeof(char) * i);
            strncpy(*str1, str + 1, i - 1);
            (*str1)[i - 1] = 0;
        }
        str = &(str[i + 1]);
    }
    else
    {
        return NULL;
    }
    return str;
}

object *getObjectValue1(object *obj, char *s)
{
    if (obj == NULL)
        return NULL;
    s = skipSpace(s);
    if (*s == '[')
    {
        char *key;
        if (s && *(++s) == '\'' && obj->type == OBJECT_OBJECT)
        {
            s = getStr(s, &key);
            s = skipSpace(s);
            if (*s == ']')
            {
                obj = obj->child;
                while (obj)
                {
                    if (strcmp(obj->key, key) == 0)
                    {
                        return getObjectValue(obj, s + 1);
                    }
                    else
                    {
                        obj = obj->brother;
                    }
                }
            }
        }
        else if (s && strchr("0123456789", *s) && obj->type == OBJECT_ARRAY)
        {
            int i = 0, flag = 1, j;
            obj = obj->child;
            if (obj == NULL)
                return NULL;
            do
            {
                i *= flag;
                i += *s - '0';
                flag *= 10;
            } while (strchr("0123456789", *s));
            for (j = 0; j < i; j++)
            {
                obj = obj->brother;
            }
            return getObjectValue(obj, s);
        }
    }
    else if (*s == '\0')
    {
        return obj;
    }
    return NULL;
}

object *getObjectValue(object *obj, char *s)
{
    if (obj == NULL)
        return NULL;
    char temp[125];
    int i = 0;
    for (i = 0; *s && *s != '.'; i++, s++)
    {
        if (i >= sizeof(temp))
        {
            return NULL;
        }
        temp[i] = *s;
    }
    temp[i] = 0;
    switch (obj->type)
    {
    case OBJECT_OBJECT:
        obj = obj->child;
        while (strcmp(obj->key, temp) != 0)
        {
            obj = obj->brother;
            if (obj == NULL)
                return NULL;
        }
        return *s ? getObjectValue(obj, s + 1) : obj;
        break;
    case OBJECT_ARRAY:
        obj = obj->child;
        char c[10];
        itoa(obj->i, c, 10);
        while (strcmp(c, temp) != 0)
        {
            obj = obj->brother;
            if (obj == NULL)
                return NULL;
        }
        return *s ? getObjectValue(obj, s + 1) : obj;
        break;
    default:
        break;
    }
    return NULL;
}