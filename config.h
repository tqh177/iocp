#ifndef CONFIG_H
#define CONFIG_H

#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

typedef struct stringArray
{
    char *value;
    struct stringArray *next;
} stringArray;

typedef struct mime
{
    char *key;
    char *value;
    struct mime *next;
} mime;

typedef struct config
{
    stringArray *index;
    mime *mime;
    BOOL gzip;
    unsigned int gzip_min_length;
    unsigned short port;
    char rootpath[126];
    char host[126];
    char ip[16];
    char gzip_path[256];
    stringArray *gzip_file;
    stringArray *header;
    char page_404[16];
} config;

config *getConfig();
#endif