#include "utils_printlog.h"

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
