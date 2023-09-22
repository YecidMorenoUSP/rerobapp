
#include "defines.h"
#include "variables.h"
#include "exo_control.hpp"

using namespace std;
using namespace std::chrono;

CanNetwork can(r "can0");

File_threadControl file_threadControl;

void *_threadSync(void *arg)
{
    using namespace STATUS;

    long k = 0;

    long can_sz = can._queued_frames.size();

    void *shm_ptr = shm_create(SHM_VARS_STREAM_KEY, SHM_VARS_STREAM_SIZE);
    shm_struct_vars_stream *var_stream = (shm_struct_vars_stream *)shm_ptr;
    var_stream->key = SHM_VARS_STREAM_KEY;

    auto us0 = std::chrono::high_resolution_clock::now();

    auto us1 = std::chrono::high_resolution_clock::now();

    long ms = 0;

    while (STATUS::RUNNING == false)
    {

        spinlock_Sync.Lock();
        fflush(stdout);

        if (STATUS::RUNNING)
            break;

        k = STATUS::TIME::control;
        can_sz = can._queued_frames.size();

        if (k % 1000 == 0)
        {

            PRINT_LOG(2, "[%3.4f][_threadSync] SZ_CAN: %2ld  pos_3:%10ld pos_4: %10ld",
                      TIME::seconds,
                      can_sz,
                      EXO::R::Knee::Encoder.getPosition(),
                      EXO::R::Knee::Motor.getPosition());
        }

        if (k % 20 == 0)
        {
            var_stream->t_s = TIME::seconds;
            var_stream->exo_knee_rigth_pos_in = EXO::R::Knee::Encoder.getPosition();
            var_stream->exo_knee_rigth_pos_out = EXO::R::Knee::Motor.getPosition();
            var_stream->exo_knee_rigth_vel_in = EXO::R::Knee::Encoder.getVelocity();
            var_stream->exo_knee_rigth_vel_out = EXO::R::Knee::Motor.getVelocity();
        }
    }
    shmdt(shm_ptr);
    PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "Finish: void *_threadSync");
    return nullptr;
    
}

void *_threadControl(void *arg)
{
    using namespace STATUS;

    long k = 0;

    for (int i = 0; i < 2; i++)
    {
        EXO::R::Knee::Motor.startMotors();
        can_tic_toc_l;
        EXO::R::Knee::Encoder.startMotors();
        can_tic_toc_l;
        EXO::R::Knee::Encoder.stopMotors();
        can_tic_toc_l;
        EXO::R::Knee::Motor.setVelocityMode();
        can_tic_toc_l;
    }

    auto us0 = high_resolution_clock::now();
    auto us1 = high_resolution_clock::now();
    long dt_ms;

    while (STATUS::RUNNING == false)
    {

        spinlock_Control.Lock();

        us0 = high_resolution_clock::now();

        if (STATUS::RUNNING)
            break;

        EXO::net.sync(true);
        EXO::net.readPDO_TX_all();

        k = TIME::control;

        static bool step1 = false;

        if (TIME::seconds < 40.0 && !step1)
        {
            EXO::R::Knee::Motor.setVelocity(1000 * sin(2.0 * M_PI * (EXO::R::Knee::Encoder.getPosition() / 2048.0) * TIME::seconds));
        }

        if (TIME::seconds >= 40.0 && !step1)
        {
            EXO::R::Knee::Motor.stopMotors();
            step1 = true;
        }

        file_threadControl.time[k] = TIME::seconds;
        file_threadControl.knee_pos_in[k] = EXO::R::Knee::Motor.getPosition();
        file_threadControl.knee_pos_out[k] = EXO::R::Knee::Encoder.getPosition();
        file_threadControl.knee_vel_in[k] = EXO::R::Knee::Motor.getVelocity();
        file_threadControl.knee_vel_out[k] = EXO::R::Knee::Encoder.getVelocity();

        us1 = high_resolution_clock::now();
        dt_ms = duration_cast<milliseconds>(us1 - us0).count();
        if (dt_ms >= 1000)
        {
            PRINT_LOG(5, PRINT_RED "[TimeOut] _threadControl %ldms\n", dt_ms);
        }
    }

    for (int i = 0; i < 2; i++)
    {
        EXO::R::Knee::Motor.stopMotors();
        can_tic_toc_l;
        EXO::R::Knee::Encoder.stopMotors();
        can_tic_toc_l;
    }

    PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "SAVING file_threadControl K:%d",k);
    file_threadControl.saveToFileName("_threadControl",k);

    PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "Finish: void *_threadControl");
    return nullptr;
}

int main()
{

    CONFIG::setDuration(10);
    CONFIG::setFrequencySample(1000L);

    file_threadControl.setSize((long)(CONFIG::durationTest * (1.0 / CONFIG::ts_s)));

    createSystemInterrupt();

    if (createTimer(CONFIG::ts_ns) != 0)
    {
        PRINT_LOG(1, PRINT_RED "[FAIL]" PRINT_RESET " createTimer()");
        return EXIT_FAILURE;
    }

    can.connect();

    EXO::net.setNode(3, EXO::R::Knee::Encoder);
    EXO::net.setNode(4, EXO::R::Knee::Motor);

    EXO::net.init();
    EXO::net.sync(false);

    EXO::R::Knee::Motor.stopMotors();
    EXO::R::Knee::Encoder.stopMotors();

    EXO::R::Knee::Motor.startMotors();
    EXO::R::Knee::Encoder.startMotors();

    EXO::net.sync(false);
    

    pthread_attr_t attr1;
    pthread_attr_init(&attr1);
    struct sched_param params1;
    params1.sched_priority = 10;
    pthread_attr_setschedpolicy(&attr1, SCHED_FIFO);
    pthread_attr_setschedparam(&attr1, &params1);

    pthread_attr_t attr2;
    struct sched_param params2;

    pthread_attr_init(&attr2);
    pthread_attr_setschedpolicy(&attr2, SCHED_FIFO);
    params2.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_attr_setschedparam(&attr2, &params2);

    can.setPeriod(CONFIG::ts_us);
    can.startThread();

    if (pthread_create(&threadControl, &attr2, _threadControl, nullptr) != 0)
    {
        PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "Creating *threadControl : %d", errno); // std::cerr;

        return 1;
    }

    if (pthread_create(&threadSync, &attr1, _threadSync, nullptr) != 0)
    {
        PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "Creating *threadSync : %d", errno); // std::cerr;

        return 1;
    }    

    pthread_attr_destroy(&attr1);
    pthread_attr_destroy(&attr2);

    if (pthread_join(threadSync, nullptr) != 0)
    {
        PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "Wait for: void *threadSync");
        // return 1;
    }
    if (pthread_join(threadControl, nullptr) != 0)
    {
        PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "Wait for: void *threadControl");
        // return 1;
    }

    closeCallBack();

    return EXIT_SUCCESS;
}
