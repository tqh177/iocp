#include "iocp.h"
#include "../until/log.h"
#include "../until/pool.h"
#include <winsock2.h>
// #include <process.h>

void init_port_info(port_info_t *portInfo);
void free_port_info(port_info_t *portInfo);
BOOL initWsa()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        return FALSE;
    }
    if (LOBYTE(wsaData.wVersion) != 2 ||
        HIBYTE(wsaData.wVersion) != 2)
    {
        WSACleanup();
        return FALSE;
    }
    return TRUE;
}

extern unsigned Worker(SOCKET servfd, HANDLE cp);
// extern unsigned Worker1(void *p);
extern BOOL postAccept(SOCKET servfd, HANDLE cp);
void startServer(char *ip, unsigned short port)
{
    SOCKET servfd;
    servfd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (servfd < 0)
    {
        log("socket error");
        return;
    }
    log("open servfd[%d]", servfd);
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.S_un.S_addr = inet_addr(ip);
    if (servaddr.sin_addr.S_un.S_addr == INADDR_NONE)
    {
        log("invalid ip");
        return;
    }
    HANDLE cp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
    //创建消息事件处理线程
    if (CreateIoCompletionPort((HANDLE)servfd, cp, 0, 1) != cp)
    {
        log("add servfd iocp error");
        return;
    }
    if (bind(servfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        log("bind failed.");
        return;
    }
    if (listen(servfd, 10) < 0)
    {
        log("listen failed.");
        return;
    }
    if (postAccept(servfd, cp) == FALSE)
    {
        log("postAccept err");
        return;
    }
    // _beginthread(Worker,0,NULL);
    // BYTE p[16];
    // memcpy(p, &servfd, sizeof(SOCKET));
    // memcpy(p + sizeof(SOCKET), &cp, sizeof(HANDLE));
    // Worker1(p);
    Worker(servfd, cp);
}

void init_port_info(port_info_t *portInfo)
{
    portInfo->type = OP_RECV;
    portInfo->http.flag = 0;
    portInfo->http.header.method = 0;
    portInfo->http.header.protocol = 0;
    portInfo->http.header.version = 0;
    portInfo->http.header.dict = NULL;
    portInfo->http.header.length = 0;
    portInfo->http.body.content = NULL;
    portInfo->wsabuf.buf = portInfo->Buffer;
    portInfo->wsabuf.len = sizeof(portInfo->Buffer);
}
void free_port_info(port_info_t *portInfo)
{
    if (portInfo)
    {
        log("close clientfd[%d]", portInfo->clientfd);
        closesocket(portInfo->clientfd);
        chain_free(portInfo->http.body.content);
        dict_free(portInfo->http.header.dict);
        pool_clear(portInfo->pool);
    }
}
