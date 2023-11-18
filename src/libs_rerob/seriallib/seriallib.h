#pragma once

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

class SerialNode
{
private:
    bool CONNECTED = false;
    char terminator = '\n';
    timeval timeout_struct;
    timeval time_struct;
    int USB;
    fd_set in_set;

public:
    SerialNode()
    {
        CONNECTED = false;
        USB = -1;
        timeout_struct.tv_sec = 0;
        timeout_struct.tv_usec = 1000;
    }

    int openPort(const char *device = "/dev/ttyUSB0", speed_t _speed_t = B115200)
    {
        CONNECTED = false;

        USB = open(device, O_RDWR | O_NDELAY);

        struct termios tty;
        struct termios tty_old;

        memset(&tty, 0, sizeof tty);

        /* Error Handling */
        if (tcgetattr(USB, &tty) != 0)
        {
            std::cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
            return -2;
        }

        /* Save old tty parameters */
        tty_old = tty;

        /* Set Baud Rate */
        cfsetospeed(&tty, (speed_t)_speed_t); // B115200
        cfsetispeed(&tty, (speed_t)_speed_t); // B115200

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
            return -2;
        }

        CONNECTED = true;

        return USB;
    }
    ssize_t writePort(void *buff, size_t len)
    {
        return write(USB, buff, len);
    }

    int readPort(char *buff, int len)
    {
        int n = 0, spot = 0, count = 0, res = 0;

        do
        {
            FD_ZERO(&in_set);
            FD_SET(USB, &in_set);
            time_struct.tv_sec = timeout_struct.tv_sec;
            time_struct.tv_usec = timeout_struct.tv_usec;

            res = select(USB + 1, &in_set, nullptr, nullptr, &time_struct);

            if (res == 1)
            {
                n = read(USB, &buff[spot], len);
                spot += n;
            }
            else if (res == 0)
            {
                count++;
            }
            else
            {
                break;
            }

        } while (n >= 0 && count < 4 && buff[spot-1]!=terminator );
        
        buff[spot] = '\0';

        return spot;
    }
};