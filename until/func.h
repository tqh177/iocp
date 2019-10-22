#ifndef _API_H_
#define _API_H_
#include "../config.h"
#include <stdio.h>

#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

int file2gzip(char *souse_path, char *dest_path, int level);
void hashPath(char *str, char dest[16]);
BOOL strInArr(stringArray *parr, char *str);
FILE *tryIndex(stringArray *index, char *path);
char *getmime(char *ext, mime *m);
void my_system(const char *cmd, char result[]);

#endif