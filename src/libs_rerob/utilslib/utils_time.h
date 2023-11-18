#pragma once

#include <chrono>

#define ChronoTimePoint std::chrono::_V2::system_clock::time_point

using namespace std;

class TimerObj
{

private:
    ChronoTimePoint us0,us1;

public:
    TimerObj();
    ~TimerObj();
    void init()
    {
        us0 = std::chrono::high_resolution_clock::now();
    }
    void timeAt(){
        

        us1 = std::chrono::high_resolution_clock::now();
        
    }

    template <typename T>
    unsigned long get(){
        using namespace chrono;
        return duration_cast<T>(us1 - us0).count();
    }
};

TimerObj::TimerObj()
{
    init();
}

TimerObj::~TimerObj()
{
}
