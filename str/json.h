#ifndef JSON_H
#define JSON_H

#include "object.h"

// json�ַ���ת���� ʧ�ܷ���`NULL`������`null`
object *json(char *str);
// object *(*json_decode)(char *str) = json;
typedef object *(*json_decode)(char *str);
// ����תjson�ַ���
char *json_encode(object *obj);

#endif