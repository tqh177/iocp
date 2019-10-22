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
        char *key;// �����ֵ�ʱʹ�á�
        unsigned int index; //����������ʱʹ�á�
    };
    union
    {
        long i; // number���͡�����ʱʹ�á�
        char *value; // ֵ���ַ���ʱʹ�á�
        struct object *child; // �ӽڵ�
    };
    struct object *parent; // ���ڵ�
    struct object *brother; // �ֵܽڵ�
    int type; // ����
}object;

object *newObject();
char *json_encode(object *obj);
void objectfree(object *obj);
object *getObjectValue(object *obj, char *s);

#endif