#ifndef OBJECT_H
#define OBJECT_H

#define OBJECT_GLOBAL   0x0000
#define OBJECT_OBJECT   0x0001
#define OBJECT_ARRAY    0x0002
#define OBJECT_STRING   0x0003
#define OBJECT_BOOL     0x0004
#define OBJECT_NUMBER   0x0005
#define OBJECT_NULL     0x0006

typedef struct object
{
    union
    {
        char *key;// 键【字典时使用】
        unsigned int index; //索引【数组时使用】
    };
    union
    {
        long i; // number类型【整型时使用】
        char *value; // 值【字符串时使用】
        struct object *child; // 子节点
    };
    struct object *parent; // 父节点
    struct object *brother; // 兄弟节点
    int type; // 类型
}object;

object *newObject();
char *json_encode(object *obj);
void objectfree(object *obj);
object *getObjectValue(object *obj, char *s);

#endif