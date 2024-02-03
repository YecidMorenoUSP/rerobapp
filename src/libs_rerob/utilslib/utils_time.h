#ifndef UTILS_TIME_H
#define UTILS_TIME_H

#include <chrono>
#include <atomic>

#define ChronoTimePoint std::chrono::_V2::system_clock::time_point

using namespace std;

class TimerObj
{

private:
    ChronoTimePoint us0, us1;

public:
    TimerObj();
    ~TimerObj();

    void init();
    void timeAt();

    template <typename T>
    unsigned long get()
    {
        using namespace chrono;
        return duration_cast<T>(us1 - us0).count();
    }
};

// namespace STATUS
// {
//     atomic<bool> RUNNING(false);
//     namespace TIME
//     {
//         long control = 0;
//         long total = 0;
//         volatile float seconds;
//     };
// };

class TIME_STATUS
{
private:
public:
    atomic<bool> RUNNING;
    long control;
    long total;
    volatile float seconds;
    TIME_STATUS();
    ~TIME_STATUS();
};



#endif