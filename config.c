
#include "config.h"
#include "str/json.h"
#include "str/object.h"
#include <stdio.h>
#include <sys/stat.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

static config con = {0};

extern config *getConfig()
{
    object w;
    FILE *fp;
    if (*con.rootpath)
    {
        return &con;
    }
    fp = fopen("config.json", "r");
    if (fp)
    {
        struct stat st;
        char *str, buf[2048];
        int len;
        object *window;
        object *temp;
        stringArray arr, *parr = &arr;
        mime mime, *pmime = &mime;
        stat("config.json", &st);
        str = malloc(sizeof(unsigned char) * st.st_size);
        str[0] = 0;
        do
        {
            len = fread(buf, sizeof(char), sizeof(buf), fp);
            strncat(str, buf, len);
        } while (len == sizeof(buf));
        fclose(fp);
        window = json(str);
        temp = getObjectValue(window, "rootpath");
        assert(temp && temp->type == OBJECT_STRING);
        strcpy(con.rootpath, temp->value);
        temp = getObjectValue(window, "index");
        assert(temp && temp->type == OBJECT_ARRAY);
        temp = temp->child;
        while (temp && temp->type == OBJECT_STRING)
        {
            parr->next = malloc(sizeof(stringArray));
            parr->next->value = malloc(sizeof(char) * strlen(temp->value) + 1);
            strcpy(parr->next->value, temp->value);
            parr = parr->next;
            parr->next = NULL;
            temp = temp->brother;
        }
        con.index = arr.next;
        arr.next = NULL;
        parr = &arr;
        temp = getObjectValue(window, "mime");
        assert(temp && temp->type == OBJECT_OBJECT);
        temp = temp->child;
        while (temp)
        {
            pmime->next = malloc(sizeof(mime));
            pmime->next->key = malloc(sizeof(char) * strlen(temp->key) + 1);
            pmime->next->value = malloc(sizeof(char) * strlen(temp->value) + 1);
            strcpy(pmime->next->key, temp->key);
            strcpy(pmime->next->value, temp->value);
            pmime = pmime->next;
            pmime->next = NULL;
            temp = temp->brother;
        }
        con.mime = mime.next;
        temp = getObjectValue(window, "ip");
        assert(temp);
        strcpy(con.ip, temp->value);
        temp = getObjectValue(window, "port");
        assert(temp);
        con.port = temp->i;
        temp = getObjectValue(window, "gzip");
        assert(temp && temp->type == OBJECT_BOOL);
        con.gzip = temp->i;
        temp = getObjectValue(window, "gzip_min_length");
        assert(temp && temp->type == OBJECT_NUMBER);
        con.gzip_min_length = temp->i;
        temp = getObjectValue(window, "gzip_path");
        assert(temp && temp->type == OBJECT_STRING && strlen(temp->value) < 256);
        strcpy(con.gzip_path, temp->value);
        char path[256];
        FILE *fp = fopen(strcat(strcpy(path, con.gzip_path), "lock"), "w+");
        if (fp == NULL)
        {
            printf("%s目录不存在", con.gzip_path);
            exit(EXIT_FAILURE);
        }
        temp = getObjectValue(window, "gzip_file");
        assert(temp && temp->type == OBJECT_ARRAY);
        temp = temp->child;
        while (temp && temp->type == OBJECT_STRING)
        {
            parr->next = malloc(sizeof(stringArray));
            parr->next->value = malloc(sizeof(char) * strlen(temp->value) + 1);
            strcpy(parr->next->value, temp->value);
            parr = parr->next;
            parr->next = NULL;
            temp = temp->brother;
        }
        con.gzip_file = arr.next;
        arr.next = NULL;
        parr = &arr;
        // temp = getObjectValue(window, "CGI");
        // assert(temp && temp->type == OBJECT_OBJECT);
        // temp = getObjectValue(temp, "cmd");
        temp = getObjectValue(window, "header");
        if (temp)
        {
            assert(temp->type == OBJECT_ARRAY);
            temp = temp->child;
            while (temp && temp->type == OBJECT_STRING)
            {
                parr->next = malloc(sizeof(stringArray));
                parr->next->value = malloc(sizeof(char) * strlen(temp->value) + 3);
                strcpy(parr->next->value, temp->value);
                strcat(parr->next->value, "\r\n");
                parr = parr->next;
                parr->next = NULL;
                temp = temp->brother;
            }
            con.header = arr.next;
            arr.next = NULL;
            parr = &arr;
        }
        temp = getObjectValue(window, "page_404");
        assert(temp && temp->type == OBJECT_STRING);
        strcpy(con.page_404, temp->value);
        objectfree(window->child);
        free(window);
        puts("读取配置文件成功");
        return (&con);
    }
    else
    {
        puts("config.json文件不存在");
        exit(-1);
    }
}