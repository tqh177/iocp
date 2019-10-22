#include "iocp.h"
#include "../config.h"
#include "../str/dict.h"
#include "../until/func.h"
#include "../until/log.h"
#include <stdio.h>
#include <sys/stat.h>
#include <winsock2.h>
#include <mswsock.h>
#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "mswsock.lib")

#define MAX_BODY_LENGTH (1024 * 1024 * 2)
extern void pushHttp(http_t *http, char *buf, size_t len);
extern void init_port_info(port_info_t *portInfo);
extern void free_port_info(port_info_t *portInfo);
static void handle(port_info_t *portInfo);
static BOOL postRecv(port_info_t *portInfo);
static BOOL postSend(port_info_t *portInfo);
BOOL postAccept(SOCKET servfd, HANDLE cp);
port_info_t *_portInfo;
// 通知内核接收连接
BOOL postAccept(SOCKET servfd, HANDLE cp)
{
    port_info_t *portInfo;
    struct sockaddr_in clientaddr;
    int length = sizeof(clientaddr);
    DWORD dwBytes = 0;
    memset(&clientaddr, 0, sizeof(clientaddr));
    pool_t *pool = newPool(sizeof(port_info_t) + sizeof(WSAOVERLAPPED));
    // 分配内存并初始化
    portInfo = (port_info_t *)pool->Buf;
    _portInfo = portInfo;
    portInfo->Overlapped = (LPWSAOVERLAPPED)((char *)portInfo + sizeof(port_info_t));
    portInfo->pool = pool;
    ZeroMemory(portInfo->Overlapped, sizeof(WSAOVERLAPPED));
    init_port_info(portInfo);
    portInfo->clientfd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (portInfo->clientfd == SOCKET_ERROR)
    {
        log("socket error.");
        return FALSE;
    }
    if (AcceptEx(servfd, portInfo->clientfd, portInfo->Buffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, portInfo->Overlapped) == FALSE && WSAGetLastError() != WSA_IO_PENDING)
    {
        log("Code:[%ld];acceptex err", GetLastError());
        return FALSE;
    }
    if (CreateIoCompletionPort((HANDLE)portInfo->clientfd, cp, (ULONG_PTR)portInfo, 0) != cp)
    {
        DWORD ecode = GetLastError();
        free_port_info(portInfo);
        portInfo = NULL;
        log("add iocp error");
        return FALSE;
        // success = FALSE;
    }
    log("open clientfd[%d]", portInfo->clientfd);
    return TRUE;
}
// 通知内核接收数据
static BOOL postRecv(port_info_t *portInfo)
{
    DWORD dwRecvNumBytes = 0, dwFlags = 0;
    LONG e = 0;
    portInfo->type = OP_RECV;
    if (WSARecv(portInfo->clientfd, &(portInfo->wsabuf), 1, &dwRecvNumBytes, &dwFlags, portInfo->Overlapped, NULL) == SOCKET_ERROR && ERROR_IO_PENDING != (e = (LONG)WSAGetLastError()))
    {
        return FALSE;
    }
    if (e == ERROR_IO_PENDING)
    {
        log("recv clientfd[%d] pending", portInfo->clientfd);
    }
    else
    {
        log("recv clientfd[%d]", portInfo->clientfd);
    }
    return TRUE;
}
// 通知内核发送数据
static BOOL postSend(port_info_t *portInfo)
{
    DWORD dwRecvNumBytes = 0;
    LONG e = 0;
    portInfo->type = OP_WRITE;
    if (WSASend(portInfo->clientfd, &(portInfo->wsabuf), 1, &dwRecvNumBytes, 0, portInfo->Overlapped, NULL) == SOCKET_ERROR && ERROR_IO_PENDING != (e = (LONG)WSAGetLastError()))
    {
        return FALSE;
    }
    if (e == ERROR_IO_PENDING)
    {
        log("send clientfd[%d] pending", portInfo->clientfd);
    }
    else
    {
        log("send clientfd[%d]", portInfo->clientfd);
    }
    return TRUE;
}
unsigned Worker(SOCKET servfd, HANDLE cp);
unsigned Worker1(BYTE *pArguent)
{
    SOCKET servfd = *(SOCKET *)pArguent;
    HANDLE cp = *(HANDLE *)(pArguent + sizeof(SOCKET));
    return Worker(servfd, cp);
}
unsigned Worker(SOCKET servfd, HANDLE cp)
{
    DWORD dwIoSize = 0;
    LPOVERLAPPED lpOverlapped = NULL;
    BOOL success;
    port_info_t *portInfo = NULL;
    while (TRUE)
    {
        success = GetQueuedCompletionStatus(cp, &dwIoSize, (PULONG_PTR)&portInfo, &lpOverlapped, INFINITE);
        if (!success)
        {
            log("Code:%ld;GetQueuedCompletionStatus error", GetLastError());
        }
        else if (portInfo == NULL)
        {
            struct sockaddr_in *local_addr = NULL;
            struct sockaddr_in *remote_addr = NULL;
            int local_addr_len = sizeof(SOCKADDR_IN);
            int remote_addr_len = sizeof(SOCKADDR_IN);
            portInfo = _portInfo;
            GetAcceptExSockaddrs(portInfo->Buffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (struct sockaddr **)&local_addr, &local_addr_len, (struct sockaddr **)&remote_addr, &remote_addr_len);
            struct in_addr addr;
            memcpy(&addr, &remote_addr->sin_addr.S_un.S_addr, 4); //复制4个字节大小
            // addr.S_un.S_addr = remote_addr->sin_addr.S_un.S_addr;
            log("recv from %s:%d", inet_ntoa(addr), ntohs(remote_addr->sin_port));
            success = postRecv(portInfo);
            success = success ? postAccept(servfd, cp) : success;
        }
        else if (dwIoSize == 0)
        {
            success = FALSE;
        }
        else
        {
            switch (portInfo->type)
            {
            case OP_RECV:
                pushHttp(&portInfo->http, portInfo->Buffer, dwIoSize);
                if (portInfo->http.flag == 5)
                {
                    handle(portInfo);
                    portInfo->type = OP_WRITE;
                    log("recv clientfd[%d] completely", portInfo->clientfd);
                    success = postSend(portInfo);
                }
                else if (portInfo->http.flag == -1)
                {
                    log("http header err");
                    success = FALSE;
                }
                else
                {
                    success = postRecv(portInfo);
                }
                break;
            case OP_WRITE:
                if (portInfo->wsabuf.len == dwIoSize)
                {
                    portInfo->http.body.content = chain_shift(portInfo->http.body.content);
                    if (portInfo->http.body.content == NULL)
                    {
                        portInfo->type = OP_RECV;
                        portInfo->http.flag = 0;
                        portInfo->http.header.method = 0;
                        portInfo->http.header.protocol = 0;
                        portInfo->http.header.version = 0;
                        portInfo->http.header.dict = NULL;
                        portInfo->http.header.length = 0;
                        portInfo->wsabuf.buf = portInfo->Buffer;
                        portInfo->wsabuf.len = sizeof(portInfo->Buffer);
                        log("send clientfd[%d] completely", portInfo->clientfd);
                        success = postRecv(portInfo);
                        break;
                    }
                    else
                    {
                        portInfo->wsabuf.buf = portInfo->http.body.content->byte;
                        portInfo->wsabuf.len = portInfo->http.body.content->length;
                    }
                }
                success = postSend(portInfo);
                break;
            default:
                break;
            }
        }
        if (!success)
        {
            free_port_info(portInfo);
        }
    }
    return 0;
}
// 接收完客户端数据后的处理函数，主要负责将预发送的数据存入内存
static void handle(port_info_t *portInfo)
{
    chain_t *s = newChain();
    char path[256];
    FILE *fp;
    config *con = getConfig();
    http_header_t header = portInfo->http.header;
    SOCKET sockClient = portInfo->clientfd;
    int i, total = 0;
    char sendBuf[2048]; //发送至客户端的字符串
    char *header_buf = NULL;
    char *mime;
    strcpy(path, con->rootpath);
    log("requestPath:%s", header.path);
    if ((fp = fopen(strcat(path, header.path), "rb")) || (fp = tryIndex(con->index, path)))
    { //200 OK
        char *ext;
        header_buf = strcpy(portInfo->Buffer, "HTTP/1.1 200 ok\r\nContent-type: %s\r\nContent-Length: %d\r\n");
        ext = strrchr(path, '.');
        if (ext != NULL)
        {
            struct stat s;
            stat(path, &s);
            ext++;
            // 判断是否需要gzip压缩
            if (con->gzip && s.st_size > con->gzip_min_length && strInArr(con->gzip_file, ext))
            {
                char gz_path[256];
                char hash_str[17];
                int Modified = s.st_mtime;
                strcpy(gz_path, con->gzip_path);
                hashPath(path, hash_str);
                hash_str[16] = 0;
                strcat(strcat(gz_path, hash_str), ".gz");
                fclose(fp);
                // 搜索gzip缓存，无则创建
                if ((fp = fopen(gz_path, "rb")) == NULL || Modified > (stat(gz_path, &s), s.st_mtime))
                {
                    file2gzip(path, gz_path, 9);
                    fp = fopen(gz_path, "rb");
                }
                // 添加gzip编码http头
                strcat(header_buf, "Content-Encoding: gzip\r\n");
            }
        }
        mime = getmime(ext, con->mime);
        if (mime == NULL)
        {
            mime = "text/html; charset=UTF-8";
        }
    }
    else
    { //404 NOT FOUND
        header_buf = strcpy(portInfo->Buffer, "HTTP/1.1 404 NOTFOUND\r\nContent-type: %s\r\nContent-Length: %d\r\n");
        mime = "text/html; charset=UTF-8";
        fp = fopen(strcat(strcpy(path, con->rootpath), con->page_404), "rb");
    }
    do
    {
        i = fread(sendBuf, sizeof(BYTE), sizeof(sendBuf), fp);
        total += i;
        chain_push(s, sendBuf, i);
        if (total > MAX_BODY_LENGTH)
            break;
    } while (i == sizeof(sendBuf));
    fclose(fp);
    stringArray *parr = con->header;
    while (parr)
    {
        strcat(header_buf, parr->value);
        parr = parr->next;
    }
    i = sprintf(sendBuf, header_buf, mime, total);
    strcat(sendBuf, "\r\n");
    s = chain_unshift(s, sendBuf, i + 2);
    chain_free(portInfo->http.body.content);
    // portInfo->http.body.content = NULL;

    portInfo->http.body.content = s;
    dict_free(portInfo->http.header.dict);
    portInfo->http.header.dict = NULL;

    portInfo->wsabuf.buf = portInfo->http.body.content->byte;
    portInfo->wsabuf.len = portInfo->http.body.content->length;
    log("need send bytes=%d", chain_len(portInfo->http.body.content->next));
}
