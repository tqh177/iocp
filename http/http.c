#include "../str/chain.h"
#include "../str/json.h"
#include "../str/dict.h"
#include "../sock/client.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

static void pushHeader(http_t *http, char *buf, size_t len);
static void pushBody(http_t *http, char *buf, size_t len);
static BOOL parseHeader(http_t *h, char *header);
static char *parseHTTP(http_t *http, char *header);
static inline char *_parseHeader(http_t *http, char *header);

void pushHttp(http_t *http, char *buf, size_t len)
{
    if (http->flag == 4)
    {
        pushBody(http, buf, len);
    }
    else
    {
        pushHeader(http, buf, len);
    }
}

static void pushHeader(http_t *http, char *buf, size_t len)
{
    size_t i;
    char c[] = "\r\n\r\n";
    for (i = 0; i < len; i++)
    {
        http->header.content[http->header.length++] = buf[i];
        if (buf[i] == c[http->flag])
        {
            http->flag++;
            if (http->flag == 4)
            {
                buf += i;
                len -= i;
                if (parseHeader(http, http->header.content) == FALSE)
                {
                    http->flag = -1;
                    return;
                }
                char *sstr;
                size_t contentLength;
                char c[10];
                c[0] = 0;
                sstr = getDict(http->header.dict, "Content-Length");
                if (sstr == NULL || strcat(c, sstr) == NULL)
                {
                    contentLength = 0;
                    http->flag = 5;
                }
                else
                {
                    contentLength = atol(c);
                    http->body.content = newChain();
                    pushBody(http, buf, len);
                }
                break;
            }
        }
        else
        {
            http->flag = 0;
        }
    }
}

static void pushBody(http_t *http, char *buf, size_t len)
{
    chain_push(http->body.content, buf, len);
    len = chain_len(http->body.content);
    if (len == http->body.length)
    {
        http->flag = 5;
    }
}

// ½âÎöheader
static BOOL parseHeader(http_t *h, char *header)
{
    header = parseHTTP(h, header);
    header = _parseHeader(h, header);
    if (header == NULL)
        return FALSE;
    return TRUE;
}

static char *parseHTTP(http_t *http, char *header)
{
    if (memcmp("GET", header, 3) == 0)
    {
        header += 3;
        http->header.method = METHOD_GET;
    }
    else if (memcmp("POST", header, 4) == 0)
    {
        header += 4;
        http->header.method = METHOD_POST;
    }
    else if (memcmp("HEAD", header, 4) == 0)
    {
        header += 4;
        http->header.method = METHOD_HEAD;
    }
    else if (memcmp("OPTION", header, 6) == 0)
    {
        header += 6;
        http->header.method = METHOD_OPTION;
    }

    if (memcmp(" /", header++, 2) == 0)
    {
        int i;
        for (i = 0; *header != '?' && *header != ' ' && i < sizeof(http->header.path); i++)
        {
            http->header.path[i] = *header;
            header++;
        }
        http->header.path[i] = 0;
        if (*header++ == '?')
        {
            char *p = memccpy(http->header.query, header, ' ', sizeof(http->header.query) - 1);
            if (p)
            {
                *(p - 1) = 0;
                header += p - http->header.query;
            }
            else
            {
                return NULL;
            }
        }
        else
        {
            http->header.query[0] = 0;
        }
        if (memcmp("HTTP/", header, 5) == 0)
        {
            http->header.protocol = PROTOCOL_HTTP;
            header += 5;
            http->header.version = ((*header - '0') << 8) | ((*(header + 2) - '0'));
            return header + 5;
        }
        return NULL;
    }
    return NULL;
}
// ½âÎöheader×Öµä
static inline char *_parseHeader(http_t *http, char *header)
{
    dict_t h = {0}, *dheader = &h;
    char key[1024];
    char value[1024];
    char *p;
    while (header)
    {
        p = memccpy(key, header, ':', sizeof(key));
        *(p - 1) = 0;
        header += p - key;
        header++;
        p = memccpy(value, header, '\r', sizeof(value));
        *(p - 1) = 0;
        header += p - value;
        header++;
        if (http)
        {
            dheader->next = newDict();
            dheader = dheader->next;
            strcpy(dheader->key, key);
            strcpy(dheader->value, value);
        }
        if (memcmp("\r\n", header, 2) == 0)
        {
            break;
        }
    }
    if (header && http)
    {
        http->header.dict = h.next;
    }
    return header;
}