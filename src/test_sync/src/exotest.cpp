/*Falta
    Docker compilation
*/

#include <cannetwork.hpp>
#include <eposnetwork.hpp>
#include <eposnode.hpp>

#include <atomic>

#include <thread>
#include <signal.h>
#include <csignal>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include <condition_variable>
#define _count_k 5
#define _ts_s 0.001
#define _ts_m ((int)(_ts_s*1000))
#define _ts_u _ts_m*1000

using namespace std;

mutex mtx;
condition_variable timer0;
condition_variable timerCan;
atomic<bool> END(false);

CanNetwork can(r "can0");
EposNode node4(4, can);
EposNetwork net(can);

long time_k = 0;
long time_total = 0;
float time_s;

void timer_isr(int signo)
{
    if (signo == SIGALRM)
    {
        
        time_total++;
        time_s = time_total*_ts_s;
        timerCan.notify_all();
        
        if(time_total%_count_k == 0){
            time_k++;
            timer0.notify_all();
        }
    }
}

int makeTimer()
{
    struct sigaction sa;
    sa.sa_handler = timer_isr;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGALRM, &sa, NULL) == -1)
    {
        perror("Error al registrar la ISR del temporizador");
        return 1;
    }

    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 100000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = _ts_u;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
    {
        perror("Error al iniciar el temporizador");
        return 1;
    }
    return 0;
}

void _threadSendCan()
{   
    auto us0 = std::chrono::high_resolution_clock::now();
    auto us1 = std::chrono::high_resolution_clock::now() - us0;
    long long microseconds;

    mutex mtx;
    long t_k = 0;
    while (END == false)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            timerCan.wait(lock);
        }

        us0 = std::chrono::high_resolution_clock::now();

        can.writeAsync();
        // net.sync();
        can.fillDic();
        node4.readPDO_TX_all();
        

        us1 = std::chrono::high_resolution_clock::now() - us0;

        microseconds = std::chrono::duration_cast<std::chrono::microseconds>(us1).count();

        if(microseconds>=1000) printf("Time SEND :%lld \n",microseconds);

    }

    printf("[END] _threadSendCan \n");
}

void _threadSync()
{
    long t_k = 0;
    mutex mtx;
    while (END == false)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            timer0.wait(lock);
        }

        if ((t_k + 1) != time_k)
        {
            printf("[_threadSync]: timeout %ld \n", t_k);
        }
        t_k = time_k;

        if (t_k % 200 == 0)
        {
            printf("[_threadSync] CAN size queue: %ld\n", can._queued_frames.size());
        }

        net.sync();
        
    }
}

void _threadControl()
{
    long i = 0;
    long t_k = 0;

    node4.faultReset();
    node4.setVelocityMode();
    mutex mtx;
    auto us0 = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    auto us1 = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    while (END == false)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            timer0.wait(lock);
        }

        us0 = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch());

        if ((t_k + 1) != time_k)
        {
            printf("Error, este bucle est'a tardando demasiado %ld\n", t_k);
        }
        t_k = time_k;

        static bool step1 = false;

        if (time_s < 4.0 && !step1)
        {
            i++;
            node4.setVelocity(1000 * ((i % 400) / 400.0));
        }

        if (time_s >= 4.0 && !step1)
        {
            node4.stopMotors();
            step1 = true;
        }

        us1 = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch());

        // printf("pos: %d\n", node4.getPosition());
        // printf("vel: %d\n", node4.getVelocity());
        // printf("us: %lld \n\n", us1 - us0);
    }
    node4.stopMotors();
}

int main(int argc, char **argv)
{
    makeTimer();

    can.connect();
    
    net.init();
    net.sync();

    node4.stopMotors();

    std::thread threadSync(_threadSync);
    std::thread threadControl(_threadControl);
    std::thread threadSendCan(_threadSendCan);

    float t0 = time_s;

    std::this_thread::sleep_for(std::chrono::seconds(5));

    printf("[Main] : Ellapsep timer: %f\n",time_s-t0);

    END = true;

    threadSync.join();
    threadControl.join();
    threadSendCan.join();

    can.disconnect();

    return 0;
}
