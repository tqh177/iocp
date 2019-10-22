#include <string.h>
#include "mymalloc.h"
#include "object.h"
typedef int BOOL;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
static char *skipSpace(char *str);
static char *getStr(char *str, char **str1);
static char *parse(object *obj, char *str);
static char *parse_object(object *obj, char *str);
static char *parse_array(object *obj, char *str);
static char *getNumber(char *str, long *i);
// 跳过空白符
static char *skipSpace(char *str)
{
    while (strchr(" \r\n\t", *str))
    {
        str++;
    }
    return str;
}

// 获取字符串
static char *getStr(char *str, char **str1)
{
    int i = 1;
    if (*str == '"')
    {
        int flag = 0;
        while (str[i] != '"' || flag)
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
#if 0
// 返回引号中字符串的长度，错误返回-1
int getstr(char *source, char *dest)
{
    int len = 0;
    if (*source == '"')
    {
        BOOL flag = FALSE;
        source++;
        if (dest)
        {
            for (len = 0; source[len] != '"' || source[len - 1] == '\\'; len++)
            {
                if (source[len] == 0)
                {
                    return -1;
                }
                dest[len] = source[len];
            }
        }
        else
        {
            for (len = 0; source[len] != '"' || source[len - 1] == '\\'; len++)
            {
                if (source[len] == 0)
                {
                    return -1;
                }
            }
        }
    }
    else
    {
        return -1;
    }
    return len;
}
#endif
static char *parse(object *obj, char *str)
{
    if (*str == '{')
    {
        if (obj)
        {
            obj->type = OBJECT_OBJECT;
        }
        str = skipSpace(str + 1);
        str = parse_object(obj, str);
        if (str == NULL || *str != '}')
        {
            return NULL;
        }
        str++;
    }
    else if (*str == '[')
    {
        if (obj)
        {
            obj->type = OBJECT_ARRAY;
        }
        str = skipSpace(str + 1);
        str = parse_array(obj, str);
        if (str == NULL || *str != ']')
        {
            return NULL;
        }
        str++;
    }
    else if (*str == '"')
    {
        if (obj)
        {
            obj->type = OBJECT_STRING;
            str = getStr(str, &(obj->value));
            // int i = getstr(str, NULL);
            // if (i == -1)
            //     return NULL;
            // str++;
            // obj->value = mymalloc(i + 1);
            // strncpy(obj->value, str, i);
            // str += i + 1;
        }
        else
        {
            str = getStr(str, NULL);
        }
        if (str == NULL)
        {
            return NULL;
        }
    }
    else if (memcmp(str, "true", 4) == 0)
    {
        if (obj)
        {
            obj->type = OBJECT_BOOL;
            obj->i = 1;
        }
        str += 4;
    }
    else if (memcmp(str, "false", 5) == 0)
    {
        if (obj)
        {
            obj->type = OBJECT_BOOL;
            obj->i = 0;
        }
        str += 5;
    }
    else if (memcmp(str, "null", 4) == 0)
    {
        if (obj)
        {
            obj->type = OBJECT_NULL;
            obj->child = NULL;
        }
        str += 4;
    }
    else if (strchr("-0123456789", *str))
    {
        if (obj)
        {
            str = getNumber(str, &obj->i);
            obj->type = OBJECT_NUMBER;
        }
        else
        {
            str = getNumber(str, NULL);
        }
    }
    else
    {
        return NULL;
    }

    return str;
}

static char *parse_object(object *obj, char *str)
{
    if (*str == '}')
    {
        return str;
    }
    if (obj)
    {
        obj->child = newObject();
        obj->child->parent = obj;
        obj = obj->child;
    }
    // 循环查找brother节点
    while (1)
    {
        str = skipSpace(str);
        if (obj)
        {
            str = getStr(str, &(obj->key));
        }
        else
        {
            str = getStr(str, NULL);
        }

        if (str == NULL)
        {
            return NULL;
        }
        str = skipSpace(str);
        if (*str == ':')
        {
            str = skipSpace(str + 1);
            str = parse(obj, str);
            if (str == NULL)
            {
                return NULL;
            }
        }
        else
        {
            return NULL;
        }
        str = skipSpace(str);
        if (*str == ',')
        {
            if (obj)
            {
                obj->brother = newObject();
                obj->brother->parent = obj->parent;
                obj = obj->brother;
            }
            str++;
        }
        else
        {
            break;
        }
    }
    return str;
}

static char *parse_array(object *obj, char *str)
{
    int index;
    if (*str == ']')
    {
        return str;
    }
    index = 0;
    if (obj)
    {
        obj->child = newObject();
        obj->child->parent = obj;
        obj = obj->child;
        obj->index = 0;
    }
    // 循环查找brother节点
    while (1)
    {
        index++;
        str = skipSpace(str);
        str = parse(obj, str);
        if (str == NULL)
        {
            return NULL;
        }
        str = skipSpace(str);
        if (*str == ',')
        {
            if (obj)
            {
                obj->brother = newObject();
                obj->brother->parent = obj->parent;
                obj = obj->brother;
                obj->index = index;
            }
            str++;
        }
        else
        {
            break;
        }
    }
    return str;
}
// 解析数字
static char *getNumber(char *str, long *i)
{
    long x = 0;
    int flag = 1;
    if (*str == '-')
    {
        flag = -1;
        str++;
    }
    if (strchr("0123456789", *str))
    {
        do
        {
            x = x * 10 + *str - '0';
            str++;
        } while (strchr("0123456789", *str));
        if (i)
        {
            *i = flag * x;
        }
        return str;
    }
    return NULL;
}
// json语法检查成功返回TRUE,失败返回FALSE
BOOL json_lint(char *str)
{
    return parse(NULL, str) && TRUE;
}
// json解析
object *json(char *str)
{
    object *obj;
    str = skipSpace(str);
    if (!json_lint(str))
        return NULL;
    obj = newObject();
    str = parse(obj, str);
    return obj;
}
