#pragma once

#include <string.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <stdio.h>
#include <unistd.h>

#include "candefines.h"
#include <unordered_map>
#include <mutex>
#include <queue>

#include <atomic>

using namespace std;

int printFrame(can_frame_s frame);

class CanNetwork
{
private:
    char _iface[10];
    int _sout, _sin;
    struct sockaddr_can _addr;
    struct sockaddr_can _addr_in;
    struct ifreq _ifr;
    struct ifreq _ifr_in;
    std::unordered_map<int, can_frame_s> _dic;

    mutex _mtx_dic;
    mutex _mtx_write;
    mutex _mtx_write_queue;

    atomic<bool> END;

    pthread_t threadSendCan;

    long _TS_U = 500000;

public:
    std::queue<can_frame_s> _queued_frames;

    CanNetwork(char *iface);
    CanNetwork();

    int connect();
    int sendFrameAsync(can_frame_s frame);
    int sendFrame(can_frame_s frame, bool async = false);
    int disconnect();
    int recFrame(can_frame_s &frame);

    void setIface(char *iface);
    void setPeriod(long ts_us);
    int writeAsync();
    void getDicKey(int key, can_frame_s &frame);
    void fillDic();
    void *_threadSendCan(void *arg);
    static void *_threadSendCanHelper(void *ctx);
    int stopThread();
    int startThread();

    void printDic()
    {
        for (auto it = _dic.cbegin(); it != _dic.cend(); ++it)
        {
            printf("%d\n",it->first);
            printFrame(it->second);
        }
    }

    ~CanNetwork();
};
