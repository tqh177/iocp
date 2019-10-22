#ifndef _IOCP_H_
#define _IOCP_H_

#include "client.h"
#include "../until/pool.h"
#include <winsock2.h>
#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

enum OP_TYPE_E
{
    OP_ACCPET,
    OP_RECV,
    OP_WRITE
};


#define MAX_BUFF_SIZE (1024 * 4)
typedef struct
{
    LPWSAOVERLAPPED Overlapped;
    enum OP_TYPE_E type;
    SOCKET clientfd;
    http_t http;
    // »º³åÇø
    WSABUF wsabuf;
    BYTE Buffer[MAX_BUFF_SIZE];
    pool_t *pool;
}port_info_t;


BOOL initWsa();
void startServer(char *ip, unsigned short port);
#endif