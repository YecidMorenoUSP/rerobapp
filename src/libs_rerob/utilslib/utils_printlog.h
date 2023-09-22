#pragma once

#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#ifndef PRINT_LEVEL
    #define PRINT_LEVEL 5
#endif

#define PRINT_RED   "\033[1;31m"
#define PRINT_GREEN "\033[1;32m"
#define PRINT_YELLOW "\033[1;33m"
#define PRINT_BLUE "\033[1;34m"
#define PRINT_MAGENTA "\033[1;35m"
#define PRINT_RESET "\033[1;0m"

int PRINT_LOG(int lvl, const char *fmt, ...)
{
    if(lvl > PRINT_LEVEL || lvl < 0) return 0;

    char time_buf[20];
    
    sprintf(time_buf,"%ld",time(NULL));
 
    va_list args1;
    va_start(args1, fmt);
    va_list args2;
    va_copy(args2, args1);

    char buf[1+vsnprintf(NULL, 0, fmt, args1)];
    va_end(args1);
    vsnprintf(buf, sizeof buf, fmt, args2);
    va_end(args2);

    return printf("\r " PRINT_MAGENTA "[%s][%d]" PRINT_RESET ": %s\n" PRINT_RESET ,time_buf,lvl, buf);
}
