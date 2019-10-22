#include <stdio.h>
#include <io.h>
#include <string.h>
#include "func.h"
#include "zlib.h"
#include "../config.h"

#define CHUNK 16384
//将一个字节的前4位与后4为进行异或运算
#define WORD(a, b) ((((a) >> 4) ^ ((a)&0x0f)) ^ (((b) >> 4) ^ ((b)&0x0f)))
// 将source_file压缩存为dest_file,level为压缩水平
int file2gzip(char *source_file, char *dest_file, int level)
{
    z_stream strm;
    FILE *source = fopen(source_file, "rb");
    FILE *dest = fopen(dest_file, "wb");
    do
    {
        if (source == NULL || dest == NULL)
        {
            break;
        }
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        if (deflateInit2(&strm, level, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY))
        {
            break;
        }
        int flush;
        int have;
        unsigned char in[CHUNK];
        unsigned char out[CHUNK];
        do
        {
            strm.avail_in = fread(in, sizeof(char), CHUNK, source);
            strm.next_in = in;
            flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
            do
            {
                strm.avail_out = CHUNK;
                strm.next_out = out;
                deflate(&strm, flush);
                have = CHUNK - strm.avail_out;
                fwrite(out, 1, have, dest);
            } while (0 == strm.avail_out);
        } while (Z_FINISH != flush);
    } while (0);
    fclose(source);
    fclose(dest);
    deflateEnd(&strm);
    return TRUE;
}
// 判断是否时目录
static BOOL isDir(char *path)
{
    return !strcmp(strrchr(path, '/'), "/");
}
// 计算str的hash并存入dest数组16位
void hashPath(char *str, char dest[16])
{
    int i;
    static char words[16] = "0123456789ABCDEF";
    memset(dest, 0, sizeof(char) * 16);
    while (*str)
    {
        for (i = 0; i < 16; i++)
        {
            if (*str == 0)
            {
                dest[i] = words[WORD(dest[i], '=')];
            }
            else
            {
                dest[i] = words[WORD(dest[i], *str)];
                dest[i] = words[WORD(dest[i], dest[16-i])];
                str++;
            }
        }
    }
}
// 判断str是否在parr数组中
BOOL strInArr(stringArray *parr, char *str)
{
    while (parr)
    {
        if (strcmp(parr->value, str) == 0)
        {
            return TRUE;
        }
        parr = parr->next;
    }
    return FALSE;
}
// 根据扩展名返回mime类型
char *getmime(char *ext, mime *m)
{
    if (ext == NULL)
    {
        ext = "*";
    }
    while (m)
    {
        if (strcmp(ext, m->key) == 0)
        {
            return m->value;
        }
        m = m->next;
    }
    return NULL;
}
//尝试索引文件是否存在,存在则存入path
FILE *tryIndex(stringArray *index, char *path)
{
    if (isDir(path) == FALSE)
    {
        return NULL;
    }
    config *con = getConfig();
    FILE *fp;
    int from = strlen(path);
    char tpath[216];
    strcpy(tpath, path);
    while (index)
    {
        strcpy(tpath + from, index->value);
        fp = fopen(tpath, "rb");
        if (fp)
        {
            strcpy(path, tpath);
            return fp;
        }
        else
            index = index->next;
    }
    return NULL;
}
void my_system(const char *cmd, char result[])
{
	char buf[1024] = {0};
    FILE *fp = NULL;
    result[0] = 0;
    if( (fp = popen(cmd, "r")) == NULL ) {
        printf("popen error!\n");
        return;
    }
    while (fgets(buf, sizeof(buf), fp)) {
        strcat(result, buf);
    }
    pclose(fp);
}