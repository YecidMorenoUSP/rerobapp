
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

File_threadControl file_Hip_R;

File_threadControl file_Hip_L;

void *_threadSync(void *arg)
{
    using namespace STATUS;
    using namespace VARS_EXO;

    long k = 0;

    long can_sz = can._queued_frames.size();

    void *shm_ptr = shm_create(SHM_VARS_STREAM_KEY, SHM_VARS_STREAM_SIZE);
    shm_struct_vars_stream *var_stream = (shm_struct_vars_stream *)shm_ptr;
    var_stream->key = SHM_VARS_STREAM_KEY;

    void *shm_ptr_2 = shm_create(SHM_VARS_IN_KEY, SHM_VARS_IN_SIZE);
    VARS_EXO::UDP_IN = (shm_struct_vars_in *)shm_ptr_2;
    memset(VARS_EXO::UDP_IN,0,sizeof(shm_struct_vars_in));
    shm_struct_vars_in *var_in = (shm_struct_vars_in *)shm_ptr_2;
    var_in->key = SHM_VARS_IN_KEY;

    auto us0 = std::chrono::high_resolution_clock::now();

    auto us1 = std::chrono::high_resolution_clock::now();

    long ms = 0;

    while (STATUS::RUNNING == false)
    {

        spinlock_Sync.Lock();

        if (STATUS::RUNNING)
            break;

        k = STATUS::TIME::control;
        can_sz = can._queued_frames.size();

        if (k % 100 == 0)
        {

            PRINT_LOG(2, "*[%3.4f][_threadSync] SZ_CAN: %2ld  Kv:%.4f pos_d:%.4f",
                      TIME::seconds,
                      can_sz,
                      VARS_EXO::UDP_IN->KV,
                      VARS_EXO::UDP_IN->pos_d);
        }

        if (k % 10 == 0)
        {
            // VARS_EXO::Kv = var_in->KV;

            var_stream->t_s = TIME::seconds;

            var_stream->exo_knee_rigth_pos_in = KR.getTheta_m(-1);
            var_stream->exo_knee_rigth_pos_out = KR.getTheta_l(-1);
            var_stream->exo_knee_rigth_vel_in = KR.getOmega_m(-1);
            var_stream->exo_knee_rigth_vel_out = KR.getOmega_m(-1);

            var_stream->exo_knee_left_pos_in = KL.getTheta_m(-1);
            var_stream->exo_knee_left_pos_out = KL.getTheta_l(-1);
            var_stream->exo_knee_left_vel_in = KL.getOmega_m(-1);
            var_stream->exo_knee_left_vel_out = KL.getOmega_m(-1);

            var_stream->exo_hip_rigth_pos_in = HR.getTheta_m(-1);
            var_stream->exo_hip_rigth_pos_out = HR.getTheta_l(-1);
            var_stream->exo_hip_rigth_vel_in = HR.getOmega_m(-1);
            var_stream->exo_hip_rigth_vel_out = HR.getOmega_m(-1);

            var_stream->exo_hip_left_pos_in = HL.getTheta_m(-1);
            var_stream->exo_hip_left_pos_out = HL.getTheta_l(-1);
            var_stream->exo_hip_left_vel_in = HL.getOmega_m(-1);
            var_stream->exo_hip_left_vel_out = HL.getOmega_m(-1);

            var_stream->exo_hip_rigth_pos_out = HR.getTheta_l(-1);
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

        EXO::R::Hip::Motor.startMotors();
        can_tic_toc_l;
        EXO::R::Hip::Motor.setVelocityMode();
        can_tic_toc_l;

        EXO::L::Hip::Motor.startMotors();
        can_tic_toc_l;
        EXO::L::Hip::Motor.setVelocityMode();
        can_tic_toc_l;

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

    using namespace VARS_EXO;

    // ActuatorVARS KR;
    KR.N = 150.0;
    KR.ks = 104.0;
    KR.fnc_Torque = &EXO_SPRING::T_linearSpring;

    // ActuatorVARS KL;
    KL.N = 150.0;
    KL.ks = 300;
    KL.fnc_Torque = &EXO_SPRING::T_linearSpring;

    // ActuatorVARS HR;
    HR.N = 101.0;
    HR.ks = 266;
    HR.fnc_Torque = &EXO_SPRING::T_linearSpring;

    // ActuatorVARS HL;
    HL.N = 101.0;
    HL.ks = 0.0;
    HL.fnc_Torque = &EXO_SPRING::T_quadraticSpringBooker;

    while (STATUS::RUNNING == false)
    {

        // PRINT_LOG(4,PRINT_BLUE "_threadControl: Alive" PRINT_RESET);

        spinlock_Control.Lock();

        static double dtK;

        dtK = duration_cast<microseconds>(high_resolution_clock::now() - us0).count() / 1000000.0;

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

        KL.setTheta_m(EXO::L::Knee::Motor.getPosition());
        KL.setTheta_l(EXO::L::Knee::Encoder.getPosition());

        HR.setTheta_m(EXO::R::Hip::Motor.getPosition());
        HL.setTheta_m(EXO::L::Hip::Motor.getPosition());

        if (TIME::seconds < 1.0)
        {

            HR.Z_theta_m = EXO::R::Hip::Motor.getPosition();
            HR.Z_theta_l = HR.getTheta_l();

            HL.Z_theta_m = EXO::L::Hip::Motor.getPosition();
            HL.Z_theta_l = HL.getTheta_l();

            KR.Z_theta_m = EXO::R::Knee::Motor.getPosition();
            KR.Z_theta_l = EXO::R::Knee::Encoder.getPosition();

            KL.Z_theta_m = EXO::L::Knee::Motor.getPosition();
            KL.Z_theta_l = EXO::L::Knee::Encoder.getPosition();
        }
        else
        {
            static bool floop = true;
            if (floop)
            {
                PRINT_LOG(1, PRINT_GREEN "[OK] READY" PRINT_RESET);
                floop = false;
            }

            static double omega = 0;
            static int MAX_VEL = 3000;
            static double pos_d = 0;
            pos_d = 0.3 * (1 - cos(2.0 * M_PI * .2 * TIME::seconds));

            if (TIME::seconds > 35.0)
            {
                pos_d = 0;
            }

            /******************************************/
            /**************      HL      **************/
            /******************************************/
            KR.Kv = VARS_EXO::UDP_IN->KV;
            KR.Bv = 0;
            KR.theta_d = VARS_EXO::UDP_IN->pos_d;
            KR.omega_d = 0;
            KR.precalculate();
            omega = CTRL::PID(KR, 350, 35, 3);
            if (omega >= MAX_VEL)
                omega = MAX_VEL;
            else if (omega <= -MAX_VEL)
                omega = -MAX_VEL;

            static LowPassFilter lpf_w_kr(1000, CONFIG::ts_s);
            lpf_w_kr.update(omega);
            KR.setOmega_m(lpf_w_kr.getOutput());

            /******************************************/
            /**************      KL      **************/
            /******************************************/
            KL.Kv = VARS_EXO::UDP_IN->KV;
            KL.Bv = 0;
            KL.theta_d = VARS_EXO::UDP_IN->pos_d;
            KL.omega_d = 0;
            KL.precalculate();
            // omega = CTRL::PID(KL, 300, 10, 1);
            omega = CTRL::PID(KL, 400, 40, 1);
            if (omega >= MAX_VEL)
                omega = MAX_VEL;
            else if (omega <= -MAX_VEL)
                omega = -MAX_VEL;

            static LowPassFilter lpf_w_kl(1000, CONFIG::ts_s);
            lpf_w_kl.update(omega);
            KL.setOmega_m(lpf_w_kl.getOutput());

            
            /******************************************/
            /**************      HL      **************/
            /******************************************/
            HL.Kv = VARS_EXO::UDP_IN->KV;
            HL.Bv = 0.0;
            HL.theta_d = VARS_EXO::UDP_IN->pos_d;
            HL.omega_d = 0;
            HL.precalculate();
            // omega = CTRL::PID(HL, 300, 10, 1);
            omega = CTRL::PID(HL, 170, 1, 0);
            if (omega >= MAX_VEL)
                omega = MAX_VEL;
            else if (omega <= -MAX_VEL)
                omega = -MAX_VEL;

            static LowPassFilter lpf_w_hl(1000, CONFIG::ts_s);
            lpf_w_hl.update(omega);
            HL.setOmega_m(lpf_w_hl.getOutput());


            /******************************************/
            /**************      HR      **************/
            /******************************************/
            HR.Kv = VARS_EXO::UDP_IN->KV;
            HR.Bv = 0;
            HR.theta_d = VARS_EXO::UDP_IN->pos_d;
            HR.omega_d = 0;
            HR.precalculate();
            // omega = CTRL::PID(HR, 300, 10, 1);
            omega = CTRL::PID(HR, 120, 1, 0);
            if (omega >= MAX_VEL)
                omega = MAX_VEL;
            else if (omega <= -MAX_VEL)
                omega = -MAX_VEL;

            static LowPassFilter lpf_w_hr(1000, CONFIG::ts_s);
            lpf_w_hr.update(omega);
            HR.setOmega_m(lpf_w_hr.getOutput());


            EXO::R::Knee::Motor.setVelocity(KR.getOmega_m(0));
            EXO::L::Knee::Motor.setVelocity(KL.getOmega_m(0));
            
            EXO::R::Hip::Motor.setVelocity(0);
            EXO::L::Hip::Motor.setVelocity(0);
            EXO::R::Hip::Motor.setVelocity(HR.getOmega_m(0));
            EXO::L::Hip::Motor.setVelocity(HL.getOmega_m(0)); 

             
        }

        us1 = high_resolution_clock::now();
        file_Knee_R.time[k] = duration_cast<microseconds>(us1 - usinit).count() / 1000000.0;
        file_Knee_R.pos_in[k] = KR.getTheta_m();
        file_Knee_R.pos_out[k] = KR.getTheta_l();
        file_Knee_R.vel_in[k] = EXO::R::Knee::Motor.getVelocity();
        file_Knee_R.vel_out[k] = EXO::R::Knee::Encoder.getVelocity();
        file_Knee_R.vel_d[k] = KR.getOmega_m(0);
        file_Knee_R.tau_d[k] = KR.getTau_d(0);
        file_Knee_R.tau_l[k] = KR.getTau_l(0);

        file_Knee_L.time[k]   =  TIME::seconds;
        file_Knee_L.pos_in[k] =  KL.getTheta_m();
        file_Knee_L.pos_out[k]=  KL.getTheta_l();
        file_Knee_L.vel_in[k] =  EXO::L::Knee::Motor.getVelocity();
        file_Knee_L.vel_out[k]=  EXO::L::Knee::Encoder.getVelocity();
        file_Knee_L.vel_d[k] =   KL.getOmega_m(0);
        file_Knee_L.tau_d[k] =   KL.getTau_d(0);
        file_Knee_L.tau_l[k] =   KL.getTau_l(0);

        file_Hip_R.time[k] = TIME::seconds;
        file_Hip_R.pos_in[k] = HR.getTheta_m();
        file_Hip_R.pos_out[k] = HR.getTheta_l();
        file_Hip_R.vel_in[k] = HR.getOmega_m();
        file_Hip_R.vel_out[k] = HR.getOmega_l();
        file_Hip_R.vel_d[k] = HR.getOmega_m(0);
        file_Hip_R.tau_d[k] = HR.getTau_d(0);
        file_Hip_R.tau_l[k] = HR.getTau_l(0);

        file_Hip_L.time[k] = TIME::seconds;
        file_Hip_L.pos_in[k] = HL.getTheta_m();
        file_Hip_L.pos_out[k] = HL.getTheta_l();
        file_Hip_L.vel_in[k] = HL.getOmega_m();
        file_Hip_L.vel_out[k] = HL.getOmega_l();
        file_Hip_L.vel_d[k] = HL.getOmega_m(0);
        file_Hip_L.tau_d[k] = HL.getTau_d(0);
        file_Hip_L.tau_l[k] = HL.getTau_l(0);

        HR.shift();
        HL.shift();
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
        EXO::R::Hip::Motor.stopMotors();
        can_tic_toc_l;

        EXO::L::Hip::Motor.stopMotors();
        can_tic_toc_l;

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

    PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "SAVING file_Hip_R K:%d", k);
    file_Hip_R.saveToFileName("file_Hip_R", k);

    PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "SAVING file_Hip_L K:%d", k);
    file_Hip_L.saveToFileName("file_Hip_L", k);

    PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "Finish: void *_threadControl");
    return nullptr;
}

void *_threadSerial(void *arg)
{
    using namespace STATUS;
    using namespace VARS_EXO;

    SerialNode S;
    int res = S.openPort("/dev/ttyUSB0", B921600);

    if (res < 0)
    {
        PRINT_LOG(5, PRINT_RED "/dev/ttyUSB0: Error" PRINT_RESET);
        STATUS::RUNNING = true;
        return nullptr;
    }

    TimerObj t0;
    S.writePort((char *)"r", 1);
    char response[128];
    int spot;

    while (STATUS::RUNNING == false)
    {
        spinlock_Sync.Lock();
        if (STATUS::RUNNING)
            break;
        
        char response[1024];
        S.writePort((char *)"p", 1);
        memset(response,'\0',1024);
        spot = S.readPort(response, 10);
        
        

        if ( response[0] == '@' && response[9] == '\n' )
        {
            t0.timeAt();    
            static float TimeRead = 0;   
            TimeRead = (float)(t0.get<milliseconds>()/1000.0f);
            t0.init();

            float *V = (float *)&(response[1]);
            static LowPassFilter V0(1000, CONFIG::ts_s);
            static LowPassFilter V1(1000, CONFIG::ts_s);
            
            V0.update(V[0],TimeRead,500);
            V1.update(V[1],TimeRead,500);

            HR.setTheta_l(-V0.getOutput());
            HL.setTheta_l(-V1.getOutput());
        }else{
            PRINT_LOG(5, PRINT_RED " [%3.4f] [spot %d] /dev/ttyUSB0: Read Failure" PRINT_RESET,TIME::seconds,spot);
        }
    }

    PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "Finish: void *_threadSerial");
    return nullptr;
}

int main()
{

    EXO::R::Hip::Motor.encoder_Q = 12800.0;

    EXO::L::Hip::Motor.encoder_Q = 12800.0;

    EXO::R::Knee::Motor.encoder_Q = 4096.0;
    EXO::R::Knee::Encoder.encoder_Q = -2000.0;

    EXO::L::Knee::Motor.encoder_Q = 4096.0;
    EXO::L::Knee::Encoder.encoder_Q = -2048.0;

    CONFIG::setDuration(60);
    CONFIG::setTimeSample_us(1000L);
    CONFIG::setTimeSample_us(5000L);

    file_Hip_L.setSize((long)(CONFIG::durationTest * (1.0 / CONFIG::ts_s)));
    file_Hip_R.setSize((long)(CONFIG::durationTest * (1.0 / CONFIG::ts_s)));
    file_Knee_R.setSize((long)(CONFIG::durationTest * (1.0 / CONFIG::ts_s)));
    file_Knee_L.setSize((long)(CONFIG::durationTest * (1.0 / CONFIG::ts_s)));

    createSystemInterrupt();

    if (createTimer(CONFIG::ts_ns) != 0)
    {
        PRINT_LOG(1, PRINT_RED "[FAIL]" PRINT_RESET " createTimer()");
        return EXIT_FAILURE;
    }

    can.connect();

    EXO::net.setNode(EXO::R::Hip::Motor);
    EXO::net.setNode(EXO::L::Hip::Motor);
    EXO::net.setNode(EXO::R::Knee::Encoder);
    EXO::net.setNode(EXO::R::Knee::Motor);
    EXO::net.setNode(EXO::L::Knee::Encoder);
    EXO::net.setNode(EXO::L::Knee::Motor);

    EXO::net.init();
    EXO::net.sync(false);

    EXO::R::Hip::Motor.stopMotors();
    EXO::R::Hip::Motor.startMotors();

    EXO::L::Hip::Motor.stopMotors();
    EXO::L::Hip::Motor.startMotors();

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

    pthread_attr_t attr3;
    struct sched_param params3;
    pthread_attr_init(&attr3);
    pthread_attr_setschedpolicy(&attr3, SCHED_FIFO);
    params3.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_attr_setschedparam(&attr3, &params3);

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

    if (pthread_create(&threadSerial, &attr3, _threadSerial, nullptr) != 0)
    {
        PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "Creating *threadSerial : %d", errno); // std::cerr;

        return 1;
    }

    pthread_attr_destroy(&attr1);
    pthread_attr_destroy(&attr2);
    pthread_attr_destroy(&attr3);

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

    if (pthread_join(threadSerial, nullptr) != 0)
    {
        PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "Wait for: void *threadSerial");
        // return 1;
    }

    closeCallBack();

    return EXIT_SUCCESS;
}
