#include "utils_time.h"

using namespace std;

void TimerObj::init()
{
    us0 = std::chrono::high_resolution_clock::now();
}
void TimerObj::timeAt()
{

    us1 = std::chrono::high_resolution_clock::now();
}

TimerObj::TimerObj()
{
    init();
}

TimerObj::~TimerObj()
{
}

TIME_STATUS::TIME_STATUS()
{
    RUNNING = false;
}

TIME_STATUS::~TIME_STATUS()
{
}

// template <typename T>
// unsigned long TimerObj::get()
// {
//     using namespace chrono;
//     return duration_cast<T>(us1 - us0).count();
// }
