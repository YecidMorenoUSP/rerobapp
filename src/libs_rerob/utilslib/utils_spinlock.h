#ifndef UTILS_SPINLOCK_H
#define UTILS_SPINLOCK_H

#include <thread>
#include <chrono>
#include <pthread.h>
#include <atomic>

class SpinLock
{
public:
    void Lock();

    void Unlock();

private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

#endif