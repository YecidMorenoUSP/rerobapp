#pragma once

#include "defines.h"
#include "variables.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <signal.h>
#include <string.h>
#include <utils_spinlock.h>
#include <utils_printlog.h>

using namespace std;
using namespace std::chrono;


void timer_isr(int sig, siginfo_t *p, void *p2)
{
    _TIME_STATUS.total++;
    _TIME_STATUS.seconds = _TIME_STATUS.total * CONFIG::ts_s;

    if (_TIME_STATUS.seconds >= CONFIG::durationTest)
    {
        _TIME_STATUS.RUNNING = false;
        spinlock_Control.Unlock();
        spinlock_Sync.Unlock();
    }

    _TIME_STATUS.control++;
    spinlock_Control.Unlock();
    spinlock_Sync.Unlock();
}

int createTimer(long tv_nsec)
{
    struct sigevent sev;
    struct itimerspec its;

    std::memset(&sev, 0, sizeof(sev));
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;

    if (timer_create(CLOCK_REALTIME, &sev, &timerId) == -1)
    {
        PRINT_LOG(2, PRINT_RED "[FAIL]" PRINT_RESET "timer_create");
        perror("");
        return EXIT_FAILURE;
    }

    its.it_value.tv_sec = 1;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = tv_nsec;

    if (timer_settime(timerId, 0, &its, NULL) == -1)
    {
        PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "timer_settime");
        perror("");
        return EXIT_FAILURE;
    }

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_isr;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) == -1)
    {
        PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "timer_sigaction");
        perror("");
        return EXIT_FAILURE;
    }

    return 0;
}

void closeCallBack()
{
    _TIME_STATUS.RUNNING = false;
    can.stopThread();

    for (int i = 0; i < 2; i++)
    {
        EXO::R::Knee::Motor.stopMotors();
        can_tic_toc_l;
        EXO::R::Knee::Encoder.stopMotors();
        can_tic_toc_l;
    }

    can.disconnect();

    timer_delete(timerId);

    int res;

    if (res = pthread_join(threadControl, nullptr) != 0)
    {
        if (!(res == EPERM || res == 0))
        {
            PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "Wait for: void *threadControl %d", res);
        }
    }
    EXOTAO_STOP_THREADS();
}

void my_handler(int s)
{
    closeCallBack();

    can.printDic();

    PRINT_LOG(1, PRINT_GREEN "Ctrl+C Captured" PRINT_RESET "  %.4fseconds", _TIME_STATUS.seconds);
    exit(1);
}

void createSystemInterrupt()
{
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
}
