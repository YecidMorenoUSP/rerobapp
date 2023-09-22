#pragma once

#include <thread>
#include <chrono>
#include <pthread.h>
#include <atomic>

class SpinLock
{
public:
    void Lock()
    {
        while (flag.test_and_set(std::memory_order_acquire))
        {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }

    void Unlock()
    {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};