#include "cannetwork.hpp"

#include <fcntl.h>
using namespace std;

CanNetwork::CanNetwork(char *iface)
{
    setIface(iface);
}

CanNetwork::~CanNetwork()
{
    printf("CanNetwork::~CanNetwork()\n");
    stopThread();
}

CanNetwork::CanNetwork()
{
}

void CanNetwork::setIface(char *iface)
{
    strcpy(_iface, iface);
}

int CanNetwork::connect()
{
    /*                   OUT                         */
    if ((_sout = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        perror("S_OUT");
        return 1;
    }

    // --------------------------------
    int sndbuf = 1000;
    if (setsockopt(_sout, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf)) < 0)
        perror("setsockopt_OUT");
    // --------------------------------

    strcpy(_ifr.ifr_name, _iface);
    ioctl(_sout, SIOCGIFINDEX, &_ifr);

    memset(&_addr, 0, sizeof(_addr));
    _addr.can_family = AF_CAN;
    _addr.can_ifindex = _ifr.ifr_ifindex;

    if (bind(_sout, (struct sockaddr *)&_addr, sizeof(_addr)) < 0)
    {
        perror("Bind_OUT");
        return 1;
    }

    fcntl(_sout, F_SETFL, O_NONBLOCK);

    /*                   IN                        */

    if ((_sin = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        perror("S_IN");
        return 1;
    }

    strcpy(_ifr_in.ifr_name, _iface);
    ioctl(_sin, SIOCGIFINDEX, &_ifr_in);

    memset(&_addr_in, 0, sizeof(_addr_in));
    _addr_in.can_family = AF_CAN;
    _addr_in.can_ifindex = _ifr_in.ifr_ifindex;

    if (bind(_sin, (struct sockaddr *)&_addr_in, sizeof(_addr_in)) < 0)
    {
        perror("Bind_IN");
        return 1;
    }

    fcntl(_sin, F_SETFL, O_NONBLOCK);

    return 0;
}

int CanNetwork::disconnect()
{
    if (close(_sout) < 0)
    {
        perror("Close_OUT");
        return 1;
    }

    if (close(_sin) < 0)
    {
        perror("Close_OUT");
        return 1;
    }

    return 0;
}

int CanNetwork::sendFrameAsync(can_frame_s frame)
{
    {
        std::unique_lock<std::mutex> lock(_mtx_write_queue);
        _queued_frames.push(frame);
    }

    return 0;
}

int CanNetwork::sendFrame(can_frame_s frame, bool async)
{
    if (async)
    {
        return sendFrameAsync(frame);
    }

    {
        std::unique_lock<std::mutex> lock(_mtx_write);

        if (write(_sout, &frame, sizeof(can_frame_s)) != sizeof(can_frame_s))
        {
            if(END) return 0;
            if (errno != EAGAIN)
            {
                printf("[Write] Error# %d\n", errno);
                return 1;
            }
        }
    }
    can_tic_toc;
    return 0;
}

int CanNetwork::recFrame(can_frame_s &frame)
{
    int nbytes = read(_sin, &frame, sizeof(can_frame_s));

    // if (nbytes > 0) printFrame(frame);

    return nbytes;
}

void CanNetwork::setPeriod(long ts_us)
{
    _TS_U = ts_us;
}

int CanNetwork::writeAsync()
{
    can_frame_s frame;
    while (!_queued_frames.empty())
    {
        {
            std::unique_lock<std::mutex> lock(_mtx_write_queue);
            frame = _queued_frames.front();
        }
        // while (sendFrame(frame,false));
        if (sendFrame(frame, false) == 0)
        {
            _queued_frames.pop();
        }
    }
    return 0;
}

void CanNetwork::getDicKey(int key, can_frame_s &frame)
{
    std::unique_lock<std::mutex> lock(_mtx_dic);
    frame = _dic[key];
}

void CanNetwork::fillDic()
{
    can_frame_s frame;
    static int count = 0;
    int nbytes;
    {
        count = 0;
        while ((nbytes = read(_sin, &frame, sizeof(can_frame_s))) > 0)
        {
            if(count >= 2000){
                printf("CanNetwork::fillDic() BUG\n");
                break;
            }
            
            {
                std::unique_lock<std::mutex> lock(_mtx_dic);
                _dic[frame.can_id] = frame;
            }
            
            count++;
        }
    }
}

void *CanNetwork::_threadSendCan(void *arg)
{

    auto us0 = std::chrono::high_resolution_clock::now();
    auto us1 = std::chrono::high_resolution_clock::now() - us0;
    long long microseconds;

    auto duracion = std::chrono::duration_cast<std::chrono::microseconds>(us1);

    mutex mtx;
    long t_k = 0;
    while (END == false)
    {

        us0 = std::chrono::high_resolution_clock::now();

        this->writeAsync();
        this->fillDic();

        t_k = 0;
        do
        {
            us1 = std::chrono::high_resolution_clock::now() - us0;
            duracion = std::chrono::duration_cast<std::chrono::microseconds>(us1);
            t_k++;
        } while (duracion.count() < _TS_U);

        if (t_k == 1)
        {
            // printf("[_threadSendCan] timeout %lld\n", duracion.count());
        }
    }
    int ret = 0;
    printf("[END] _threadSendCan \n");
    pthread_exit(&ret);
}

void *CanNetwork::_threadSendCanHelper(void *ctx)
{
    return ((CanNetwork *)(ctx))->_threadSendCan(nullptr);
}

int CanNetwork::stopThread()
{
    END = true;
    if (pthread_join(threadSendCan, nullptr) != 0)
    {
        printf("Error al esperar al hilo threadSendCan %d.\n",errno);
    }
    return 0;
}

int CanNetwork::startThread()
{
    END = false;
    pthread_attr_t attr0;
    pthread_attr_init(&attr0);
    struct sched_param params0;
    params0.sched_priority = 99;
    pthread_attr_setschedpolicy(&attr0, SCHED_FIFO);
    pthread_attr_setschedparam(&attr0, &params0);

    if (pthread_create(&threadSendCan, &attr0, CanNetwork::_threadSendCanHelper, this) != 0)
    {
        printf("Error al crear al hilo. threadSendCan\n");
        return 1;
    }

    pthread_attr_destroy(&attr0);

    return 0;
}

int printFrame(can_frame_s frame)
{
    printf("Frame:\n"
           "  ID: 0x%x\n"
           "  SZ: %d\n"
           "  Payload: \n    ",
           frame.can_id, frame.can_dlc);

    for (int i = 0; i < frame.can_dlc; i++)
        printf("%02X ", frame.data[i]);
    printf("\n");
    return 0;
}