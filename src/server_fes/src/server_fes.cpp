
#include <stdio.h>
#include <iostream>

#include "rehamovelib.c"

int main()
{

    Rehamove *FES;

    FES = open_port("/dev/ttyUSB0");

    if (FES == NULL)
    {
        close_port(FES);
        return 1;
    }

    

    printf("Hello\n");

    return 0;
}