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

// void saveFile();

void timer_isr(int sig, siginfo_t *p, void *p2)
{
    using namespace STATUS;
    TIME::total++;
    TIME::seconds = TIME::total * CONFIG::ts_s;

    if (TIME::seconds >= CONFIG::durationTest)
    {
        RUNNING = true;
        spinlock_Control.Unlock();
        spinlock_Sync.Unlock();
    }

    TIME::control++;
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
    STATUS::RUNNING = true;
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
    if (res = pthread_join(threadSync, nullptr) != 0)
    {
        if (!(res == EPERM || res == 0))
        {
            PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "Wait for: void *threadSync %d", res);
        }
    }
    if (res = pthread_join(threadControl, nullptr) != 0)
    {
        if (!(res == EPERM || res == 0))
        {
            PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "Wait for: void *threadControl %d", res);
        }
    }
}

void my_handler(int s)
{
    closeCallBack();
    PRINT_LOG(1, PRINT_GREEN "Ctrl+C Captured" PRINT_RESET "  %.4fseconds", STATUS::TIME::seconds);
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

class File_threadControl : public RerobAppDataLog
{

public:
    vector<float> time;
    vector<float> knee_pos_in;
    vector<float> knee_pos_out;
    vector<float> knee_vel_in;
    vector<float> knee_vel_out;

    File_threadControl()
    {
    }

    void setSize(long size)
    {
        _size = size + 100;
        time.reserve(_size);
        knee_pos_in.reserve(_size);
        knee_pos_out.reserve(_size);
        knee_vel_in.reserve(_size);
        knee_vel_out.reserve(_size);
    }

    void saveHeader(FILE *file)
    {
        fprintf(file, "time\tknee_pos_in\tknee_pos_out\tknee_vel_in\tknee_vel_out\n");
    }

    void saveLine(FILE *file, long idx)
    {
        fprintf(file, "%.4f\t%.4f\t%.4f\t%.4f\t%.4f\n", time[idx], knee_pos_in[idx], knee_pos_out[idx], knee_vel_in[idx], knee_vel_out[idx]);
    }
};
