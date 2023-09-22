#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <condition_variable>

#include <cannetwork.hpp>
#include <eposnetwork.hpp>
#include <eposnode.hpp>

#define _count_k 5
#define _ts_s 0.001
#define _ts_m ((long)(_ts_s * 1000L))
#define _ts_u _ts_m * 1000L
#define _ts_n _ts_u * 1000L

#define T_total 10

using namespace std;

CanNetwork can(r "can0");
EposNode node4(4, can);
EposNode node3(3, can);
EposNetwork net(can);

timer_t timerId;

long time_k = 0;
long time_total = 0;
volatile float time_s;

mutex mtx;
condition_variable timer0;
condition_variable timerCan;
atomic<bool> END(false);

vector<long> pos3_vec;
vector<long> pos4_vec;

// Función de devolución de llamada del temporizador
void timer_isr(int sig, siginfo_t *p, void *p2)
{
    time_total++;
    time_s = time_total * _ts_s;

    if (time_s >= T_total)
        END = true;

    timerCan.notify_all();

    if (time_total % _count_k == 0)
    {
        time_k++;
        timer0.notify_all();
    }
}

int createTimer()
{
    struct sigevent sev;
    struct itimerspec its;

    // Configura la estructura sigevent para la señal SIGALRM
    std::memset(&sev, 0, sizeof(sev));
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;

    // Crea el temporizador
    if (timer_create(CLOCK_REALTIME, &sev, &timerId) == -1)
    {
        perror("timer_create");
        return EXIT_FAILURE;
    }

    // Configura el temporizador para que se active cada 1 segundo
    its.it_value.tv_sec = 1;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = _ts_n;

    // Activa el temporizador
    if (timer_settime(timerId, 0, &its, NULL) == -1)
    {
        perror("timer_settime");
        return EXIT_FAILURE;
    }

    // Configura la función de devolución de llamada para el temporizador
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_isr;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) == -1)
    {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    return 0;
}

void *_threadSendCan(void *arg)
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

        if ((t_k + 1) != time_total)
        {
            // printf("[_threadSendCan]: timeout %ld \n", t_k);
        }
        else
        {
            // printf("[_threadSendCan]: OK \n");
        }
        t_k = time_total;

        // us0 = std::chrono::high_resolution_clock::now();

        can.writeAsync();
        // net.sync();
        can.fillDic();
        node4.readPDO_TX_all();
        node3.readPDO_TX_all();

        // us1 = std::chrono::high_resolution_clock::now() - us0;

        // microseconds = std::chrono::duration_cast<std::chrono::microseconds>(us1).count();

        // if (microseconds >= _ts_u*2)
        //     printf("Time SEND :%lld \n", microseconds);
    }

    printf("[END] _threadSendCan \n");
    return nullptr;
}

void *_threadSync(void *arg)
{
    long t_k = 0;
    mutex mtx;
    long can_sz = can._queued_frames.size();
    while (END == false)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            timer0.wait(lock);
        }

        t_k = time_k;
        can_sz = can._queued_frames.size();

        if (t_k % 200 == 0)
        {
            printf("[%3.4f][_threadSync] SZ_CAN: %2ld  pos_3:%10ld pos_4: %10ld \n",
                   time_s,
                   can_sz,
                   node3.getPosition(),
                   node4.getPosition());
        }
    }
    return nullptr;
}

void *_threadControl(void *arg)
{

    long i = 0;
    long t_k = 0;

    node3.faultReset();
    node4.faultReset();

    node3.stopMotors();

    node4.setVelocityMode();
    mutex mtx;
    auto us0 = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    auto us1 = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    while (END == false)
    {
        net.sync();

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

        if (time_s < 40.0 && !step1)
        {
            i++;

            if (abs(node3.getPosition()) > 100)
            {
                node4.setVelocity(node3.getPosition() * 10);
            }
            else
            {
                node4.setVelocity(0);
            }
        }

        if (time_s >= 40.0 && !step1)
        {
            node4.stopMotors();
            step1 = true;
        }

        us1 = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch());

        pos3_vec[t_k] = node3.getPosition();
        pos4_vec[t_k] = node4.getPosition();

        // printf("pos: %d\n", node3.getPosition());
        // printf("vel: %d\n", node3.getVelocity());
        // printf("us: %lld \n\n", us1 - us0);
    }
    node4.stopMotors();
    node3.stopMotors();

    return nullptr;
}

int main()
{

    if (createTimer() != 0)
    {
        return EXIT_FAILURE;
    }

    can.connect();

    net.init();
    net.sync();

    pos3_vec.reserve(T_total * 1000 + 2 * 1000);
    pos4_vec.reserve(T_total * 1000 + 2 * 1000);

    node4.stopMotors();
    node3.stopMotors();

    pthread_t threadSync, threadControl, threadSendCan;
    int threadId;

    // Configura los atributos del hilo
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    struct sched_param params;
    params.sched_priority = 99;

    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_setschedparam(&attr, &params);

    if (pthread_create(&threadSendCan, &attr, _threadSendCan, nullptr) != 0)
    {
        std::cerr << "Error al crear el hilo." << std::endl;
        return 1;
    }

    if (pthread_create(&threadSync, &attr, _threadSync, nullptr) != 0)
    {
        std::cerr << "Error al crear el hilo." << std::endl;
        return 1;
    }

    if (pthread_create(&threadControl, &attr, _threadControl, nullptr) != 0)
    {
        std::cerr << "Error al crear el hilo." << std::endl;
        return 1;
    }

    pthread_attr_destroy(&attr);

    // Wait for n seconds

    if (pthread_join(threadSync, nullptr) != 0)
    {
        std::cerr << "Error al esperar al hilo." << std::endl;
        return 1;
    }
    if (pthread_join(threadControl, nullptr) != 0)
    {
        std::cerr << "Error al esperar al hilo." << std::endl;
        return 1;
    }
    if (pthread_join(threadSendCan, nullptr) != 0)
    {
        std::cerr << "Error al esperar al hilo." << std::endl;
        return 1;
    }

    can.disconnect();

    timer_delete(timerId);


    // scp debian@192.168.6.2:/home/debian/build/pos_vec.dat .
    FILE *f1;

    f1 = fopen("pos_vec.dat", "w+");

    if (f1 != nullptr)
    {
        for (int i = 0; i < 10 * 1000; i++)
        {
            fprintf(f1, "%10ld\t%10ld\n", pos3_vec[i], pos4_vec[i]);
        }

        fclose(f1);
    }else{
        printf("Error guardando el archivo");
    }

    return EXIT_SUCCESS;
}
