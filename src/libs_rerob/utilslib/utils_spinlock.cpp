#include "utils_spinlock.h"

#include <thread>
#include <chrono>
#include <pthread.h>
#include <atomic>

void SpinLock::Lock()
{
    while (flag.test_and_set(std::memory_order_acquire))
    {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void SpinLock::Unlock()
{
    flag.clear(std::memory_order_release);
}
