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

using namespace std;

int main()
{

    int USB = open("/dev/ttyUSB0",  O_RDWR | O_NDELAY );

    struct termios tty;
    struct termios tty_old;

    memset(&tty, 0, sizeof tty);

    /* Error Handling */
    if (tcgetattr(USB, &tty) != 0)
    {
        std::cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
        return 1;
    }

    /* Save old tty parameters */
    tty_old = tty;

    /* Set Baud Rate */
    cfsetospeed(&tty, (speed_t)B115200);
    cfsetispeed(&tty, (speed_t)B115200);

    /* Setting other Port Stuff */
    tty.c_cflag &= ~PARENB; // Make 8n1
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cflag &= ~CRTSCTS;       // no flow control
    tty.c_cc[VMIN] = 0;            // read doesn't block
    tty.c_cc[VTIME] = 0;           // 0.5 seconds read timeout
    tty.c_cflag |= CREAD | CLOCAL; // turn on READ & ignore ctrl lines
    

    /* Make raw */
    cfmakeraw(&tty);

    /* Flush Port, then applies attributes */
    tcflush(USB, TCIFLUSH);
    if (tcsetattr(USB, TCSANOW, &tty) != 0)
    {
        std::cout << "Error " << errno << " from tcsetattr" << std::endl;
        return 1;
    }

    

   

    char key = 'p';
    write(USB, (void *)&key, 1);


   

    int n = 0, spot = 0;
    char response[1024];
    memset(response, '\0', sizeof response);


    timeval timevalStruct;
    timevalStruct.tv_sec=0;
    timevalStruct.tv_usec=1000;
    fd_set in_set;
    FD_ZERO(&in_set);
    FD_SET(USB, &in_set);
    
    int count = 0;
    do
    {   
        FD_ZERO(&in_set);
        FD_SET(USB, &in_set);
        timevalStruct.tv_sec=0;
        timevalStruct.tv_usec=1000; 
        int res = select(USB + 1, &in_set, nullptr, nullptr, &timevalStruct);
        
        if(res == 1){
            n = read(USB, &response[spot], 10);
            spot+=n;
            printf("n:%d   ",n);
            fflush(stdout);
        }else if(res == 0){
            count++;
        }else{
            break;
        }
        

       
    } while ( n >= 0 && count<4 && response[spot-1]!='\n'); //buf != '\n' &&

    printf("Response[%d]: %s\n",spot,response);
    if(response[0]=='@' && response[spot-1]=='\n'){
        float *V = (float*) &(response[1]);
        printf("V[R]: %.6f\n",V[0]);
        printf("V[L]: %.6f\n",V[1]);
    }
    

    return 0;
}