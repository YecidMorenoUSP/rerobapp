#pragma once

#include "defines.h"

// #define _count_k 1
// #define _ts_s 0.001
// #define _ts_m ((long)(_ts_s * 1000L))
// #define _ts_u _ts_m * 1000L
// #define _ts_n _ts_u * 1000L

// #define T_total 10

namespace CONFIG
{
    long ts_us = 1000L;
    long ts_ns = ts_us * 1000L;
    long ts_ms = (long)(ts_us / 1000L);
    float ts_s = (float)(ts_ms / 1000.0);

    float durationTest = 10;

    void setDuration(float __duration = 10)
    {
        durationTest = __duration;
    }

    void setTimeSample_us(long __ts_us = 1000L)
    {
        ts_us = __ts_us;
        ts_ns = ts_us * 1000L;
        ts_ms = (long)(ts_us / 1000L);
        ts_s = (float)(ts_ms / 1000.0);

        PRINT_LOG(1, PRINT_GREEN "CONFIG::ts_us -> %ld" PRINT_RESET, CONFIG::ts_us);
        PRINT_LOG(1, PRINT_GREEN "CONFIG::ts_ns -> %ld" PRINT_RESET, CONFIG::ts_ns);
        PRINT_LOG(1, PRINT_GREEN "CONFIG::ts_ms -> %ld" PRINT_RESET, CONFIG::ts_ms);
        PRINT_LOG(1, PRINT_GREEN "CONFIG::ts_s -> %f" PRINT_RESET, CONFIG::ts_s);
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

timer_t timerId;

SpinLock spinlock_Control;

pthread_t threadControl;