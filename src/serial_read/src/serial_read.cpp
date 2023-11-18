#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <seriallib/seriallib.h>
#include <utils_time.h>

using namespace std;

int main()
{
    using namespace chrono;
    SerialNode S;

    int res = S.openPort("/dev/ttyUSB0",B921600);

    if (res < 0)
    {
        printf("USB: Error");
        return 1;
    }

    printf("USB: %d\n", res);

    

    TimerObj t0,tinit;
    tinit.init();

    S.writePort((char *)"r", 1);
    int spot;
    for (int count = 0; count < 10; count++)
    {

        t0.init();
        char response[1024];
        S.writePort((char *)"p", 1);
        memset(response,'\0',1024);
        spot = S.readPort(response, 10);
        t0.timeAt();


        printf("Tus: %ld  - Response[%d]: %s\n", t0.get<microseconds>(), spot, response);

        if (response[0] == '@' && response[spot - 1] == '\n')
        {
            float *V = (float *)&(response[1]);
            printf("V[R]: %.6f\n", V[0]);
            printf("V[L]: %.6f\n", V[1]);
        }
    }

    tinit.timeAt();

    printf("Tms: %ld  -   Tmean:  %d \n", tinit.get<milliseconds>() , tinit.get<microseconds>()/10);

    return 0;
}