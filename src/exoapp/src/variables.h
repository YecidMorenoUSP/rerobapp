#pragma once

#include "defines.h"

extern CanNetwork can;

namespace EXO
{
    EposNetwork net(can);

    namespace R
    {
        namespace Knee
        {
            EposNode Motor(1, can);
            EposNode Encoder(2, can);
        };
        namespace Hip
        {
            EposNode Motor(5, can);
        };
    };

    namespace L
    {
        namespace Knee
        {
            EposNode Motor(3, can);
            EposNode Encoder(4, can);
        };
        namespace Hip
        {
            EposNode Motor(6, can);
        };
    };
}

namespace VARS_EXO{
    ActuatorVARS KR;
    ActuatorVARS KL;
    ActuatorVARS HL;
    ActuatorVARS HR;
    shm_struct_vars_in * UDP_IN;
};

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

        // printf("ts_us : %ld \n "
        //        "ts_ns : %ld \n "
        //        "ts_ms : %ld \n "
        //        "ts_s : %f \n ",
        //        ts_us, ts_ns, ts_ms, ts_s);
    }

};

namespace STATUS
{
    atomic<bool> RUNNING(false);
    namespace TIME
    {
        long control = 0;
        long total = 0;
        volatile float seconds;
    };
};

timer_t timerId;

SpinLock spinlock_Control;
SpinLock spinlock_Sync;

pthread_t threadSync, threadControl, threadSerial;