#ifndef JSON_H
#define JSON_H

#include "object.h"

// json字符串转对象 失败返回`NULL`而不是`null`
object *json(char *str);
// object *(*json_decode)(char *str) = json;
typedef object *(*json_decode)(char *str);
// 对象转json字符串
char *json_encode(object *obj);

#endif