#ifndef UTILS_PRINTLOG_H
#define UTILS_PRINTLOG_H

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

int PRINT_LOG(int lvl, const char *fmt, ...);

#endif