
#include "defines.h"
#include "variables.h"
#include "exo_control.hpp"

#include "utils_exo.h"
#include "utils_control.h"

#include <LowPassFilter/LowPassFilter.hpp>

using namespace std;
using namespace std::chrono;

CanNetwork can((char *)"can1");

File_threadControl file_Knee_R;

File_threadControl file_Knee_L;

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

            PRINT_LOG(2, "*[%3.4f][_threadSync] SZ_CAN: %2ld  Encoder: %5.4f Motor: %5.4f",
                      TIME::seconds,
                      can_sz,
                      EXO::R::Knee::Encoder.getPosition(),
                      EXO::R::Knee::Motor.getPosition());

            PRINT_LOG(2, "  [%3.4f][_threadSync] SZ_CAN: %2ld  Encoder: %5.4f Motor: %5.4f",
                      TIME::seconds,
                      can_sz,
                      EXO::L::Knee::Encoder.getPosition(),
                      EXO::L::Knee::Motor.getPosition());
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

        EXO::L::Knee::Motor.startMotors();
        can_tic_toc_l;
        EXO::L::Knee::Encoder.startMotors();
        can_tic_toc_l;
        EXO::L::Knee::Encoder.stopMotors();
        can_tic_toc_l;
        EXO::L::Knee::Motor.setVelocityMode();
        can_tic_toc_l;
    }

    auto usinit = high_resolution_clock::now();
    auto us0 = high_resolution_clock::now();
    auto us1 = high_resolution_clock::now();

    long dt_ms;

    ActuatorVARS KR;
    KR.N = 150.0;
    KR.ks = 104.0;
    

    ActuatorVARS KL;
    KL.N = 150.0;
    KL.ks = 300;

    while (STATUS::RUNNING == false)
    {

        // PRINT_LOG(4,PRINT_BLUE "_threadControl: Alive" PRINT_RESET);
        
        spinlock_Control.Lock();

        static double dtK;
        
        dtK = duration_cast<microseconds>(high_resolution_clock::now() - us0).count()/1000000.0;

        us0 = high_resolution_clock::now();

        if (STATUS::RUNNING)
            break;

        EXO::net.sync(true);
        EXO::net.readPDO_TX_all();

        k = TIME::control;
                
        static LowPassFilter lpf_m(50, CONFIG::ts_s);
        static LowPassFilter lpf_l(50, CONFIG::ts_s);

        lpf_m.update(EXO::R::Knee::Motor.getPosition());
        lpf_l.update(EXO::R::Knee::Encoder.getPosition());

        KR.setTheta_m(lpf_m.getOutput());
        KR.setTheta_l(lpf_l.getOutput());
        // KR.setOmega_l(- EXO::R::Knee::Encoder.getVelocity());

        // KR.setTheta_m(EXO::R::Knee::Motor.getPosition());
        // KR.setTheta_l(EXO::R::Knee::Encoder.getPosition());

        
        KL.setTheta_m(EXO::L::Knee::Motor.getPosition());
        KL.setTheta_l(EXO::L::Knee::Encoder.getPosition());


        if (TIME::seconds < 1.0){

            KR.Z_theta_m = EXO::R::Knee::Motor.getPosition();
            KR.Z_theta_l = EXO::R::Knee::Encoder.getPosition();
        }
        else
        {
            static bool floop = true;
            if(floop){
                PRINT_LOG(1, PRINT_GREEN "[OK] READY" PRINT_RESET);
                floop = false;
            }
            

            static double omega = 0;
            static int MAX_VEL = 5000;
            static double pos_d = 0;
            pos_d = .5*sin(2.0 * M_PI * .2 * TIME::seconds);

            KR.Kv = 10;
            KR.Bv = 0.01;
            KR.theta_d = pos_d;
            KR.omega_d = 0;
            KR.precalculate();
            omega = CTRL::PID(KR, 350, 35, 3);
            if (omega >= MAX_VEL)
                omega = MAX_VEL;
            else if (omega <= -MAX_VEL)
                omega = -MAX_VEL;

            static LowPassFilter lpf_w(1000, CONFIG::ts_s);
            lpf_w.update(omega);
            KR.setOmega_m(lpf_w.getOutput());


            KL.Kv = 20;
            KL.theta_d = 0.01;
            KL.theta_d = pos_d;
            KL.precalculate();
            omega = CTRL::PID(KL, 150, 10, 1);
            if (omega >= MAX_VEL)
                omega = MAX_VEL;
            else if (omega <= -MAX_VEL)
                omega = -MAX_VEL;
            
            static LowPassFilter lpf_wl(1000, CONFIG::ts_s);
            lpf_wl.update(omega);
            KL.setOmega_m(lpf_wl.getOutput());
            

            

            EXO::R::Knee::Motor.setVelocity(KR.getOmega_m(0));            
            EXO::L::Knee::Motor.setVelocity(KL.getOmega_m(0));

            
            // EXO::R::Knee::Motor.setVelocity(10);
            // EXO::L::Knee::Motor.setVelocity(100);
            
            
        }

        us1 = high_resolution_clock::now();
        file_Knee_R.time[k] = duration_cast<microseconds>(us1 - usinit).count()/1000000.0;
        file_Knee_R.knee_pos_in[k] = KR.getTheta_m();
        file_Knee_R.knee_pos_out[k] = KR.getTheta_l();
        file_Knee_R.knee_vel_in[k] = EXO::R::Knee::Motor.getVelocity();
        file_Knee_R.knee_vel_out[k] = EXO::R::Knee::Encoder.getVelocity();
        file_Knee_R.knee_vel_d[k] = KR.getOmega_m(0);
        file_Knee_R.knee_tau_d[k] = KR.getTau_d(0);
        file_Knee_R.knee_tau_l[k] = KR.getTau_l(0);

        file_Knee_L.time[k] = TIME::seconds;
        file_Knee_L.knee_pos_in[k] = EXO::L::Knee::Motor.getPosition();
        file_Knee_L.knee_pos_out[k] = EXO::L::Knee::Encoder.getPosition();
        file_Knee_L.knee_vel_in[k] = EXO::L::Knee::Motor.getVelocity();
        file_Knee_L.knee_vel_out[k] = EXO::L::Knee::Encoder.getVelocity();
        file_Knee_L.knee_vel_d[k] = KL.getOmega_m(0);
        file_Knee_L.knee_tau_d[k] = KL.getTau_d(0);
        file_Knee_L.knee_tau_l[k] = KL.getTau_l(0);
        
        KR.shift();
        KL.shift();

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

        EXO::L::Knee::Motor.stopMotors();
        can_tic_toc_l;
        EXO::L::Knee::Encoder.stopMotors();
        can_tic_toc_l;
    }

    PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "SAVING file_Knee_R K:%d", k);
    file_Knee_R.saveToFileName("file_Knee_R", k);

    PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "SAVING file_Knee_L K:%d", k);
    file_Knee_L.saveToFileName("file_Knee_L", k);

    PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "Finish: void *_threadControl");
    return nullptr;
}

int main()
{

    EXO::R::Knee::Motor.encoder_Q = 4096.0;
    EXO::R::Knee::Encoder.encoder_Q = -2000.0;

    EXO::L::Knee::Motor.encoder_Q = 4096.0;
    EXO::L::Knee::Encoder.encoder_Q = -2048.0;

    CONFIG::setDuration(20);
    CONFIG::setTimeSample_us(1000L);
    CONFIG::setTimeSample_us(5000L);


    file_Knee_R.setSize((long)(CONFIG::durationTest * (1.0 / CONFIG::ts_s)));

    file_Knee_L.setSize((long)(CONFIG::durationTest * (1.0 / CONFIG::ts_s)));


    createSystemInterrupt();

    if (createTimer(CONFIG::ts_ns) != 0)
    {
        PRINT_LOG(1, PRINT_RED "[FAIL]" PRINT_RESET " createTimer()");
        return EXIT_FAILURE;
    }

    can.connect();

    EXO::net.setNode(EXO::R::Knee::Encoder);
    EXO::net.setNode(EXO::R::Knee::Motor);
    EXO::net.setNode(EXO::L::Knee::Encoder);
    EXO::net.setNode(EXO::L::Knee::Motor);

    EXO::net.init();
    EXO::net.sync(false);

    EXO::R::Knee::Motor.stopMotors();
    EXO::R::Knee::Encoder.stopMotors();

    EXO::R::Knee::Motor.startMotors();
    EXO::R::Knee::Encoder.startMotors();

    EXO::L::Knee::Motor.stopMotors();
    EXO::L::Knee::Encoder.stopMotors();

    EXO::L::Knee::Motor.startMotors();
    EXO::L::Knee::Encoder.startMotors();

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
