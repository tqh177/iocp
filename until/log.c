#include "log.h"
#include <stdio.h>
#include <stdarg.h>

void _log(char *file, int line,char* format, ...)
{
    va_list aptr;
    va_start(aptr, format);
    printf("File:%s;Line:%d;\t", file, line);
    vfprintf(stdout, format, aptr);
    putchar('\n');
    va_end(aptr);
}