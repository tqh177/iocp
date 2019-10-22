#ifndef _LOG_H_
#define _LOG_H_

#ifdef _DEBUG
extern void _log(char *file, int line,char* format, ...);
#define log(format, ...) _log(__FILE__, __LINE__, format, ##__VA_ARGS__)
#else
#define log(...)
#endif

#endif