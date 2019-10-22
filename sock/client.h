#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "../str/chain.h"
#include "../str/dict.h"

#define MAX_HEADER_LENGTH 1024 * 4
// 连接协议
#define PROTOCOL_HTTP 1
#define PROTOCOL_HTTPS 2
#define PROTOCOL_TCP 3
#define PROTOCOL_UDP 4
#define PROTOCOL_FTP 5

// 请求方法
#define METHOD_GET 1
#define METHOD_POST 2
#define METHOD_HEAD 3
#define METHOD_OPTION 4


typedef struct http_header_s
{
    char path[256];
    char query[256];
    // http头键值对(free)
    dict_t * dict;
    size_t length;
    int method : 8;
    int protocol : 8;
    int version : 8;
    char content[MAX_HEADER_LENGTH];
} http_header_t;

typedef struct
{
    size_t length;
    chain_t *content;
} http_body_t;

typedef struct
{
    http_body_t body;
    http_header_t header;
    // 4:分割header与body; 5:body接受完成
    int flag;
} http_t;

#endif