

#include "defines.h"
#include "variables.h"
#include "exo_control.hpp"

#include "utils_exo.h"
#include "utils_control.h"

TIME_STATUS _TIME_STATUS;

using namespace std;
using namespace std::chrono;

void *__threadControl(void *arg)
{

    EXOTAO_LOAD_CONFIGURATION();
    EXOTAO_ENABLE();

    long k = 0;
    double delta_t;
    auto us0 = high_resolution_clock::now();

    while (_TIME_STATUS.RUNNING)
    {
        EXOTAO_READ_CAN_BUFFER();

        spinlock_Control.Lock();

        delta_t = duration_cast<microseconds>(high_resolution_clock::now() - us0).count() / 1000000.0;
        us0 = high_resolution_clock::now();

        if (_TIME_STATUS.RUNNING == false)
            break;

        k = _TIME_STATUS.control;

        EXOTAO_READ_VARIABLES();

        if (_TIME_STATUS.seconds < 1.0)
        {
            EXOTAO_CALIBRATE();
        }
        else
        {
            // Your controller here !!!!
        }

        EXOTAO_ADD_LOG_SAMPLE(k);
    }

    EXOTAO_DISABLE();
    EXOTAO_SAVE_LOGS(_TIME_STATUS.control);

    return nullptr;
}

void *_threadControl(void *arg)
{

    EXOTAO_LOAD_CONFIGURATION();
    EXOTAO_ENABLE();

    long k = 0;
    double delta_t;
    auto us0 = high_resolution_clock::now();

    using namespace VARS_EXO;

    while (_TIME_STATUS.RUNNING)
    {

        EXOTAO_READ_CAN_BUFFER();

        spinlock_Control.Lock();

        delta_t = duration_cast<microseconds>(high_resolution_clock::now() - us0).count() / 1000000.0;
        us0 = high_resolution_clock::now();

        if (_TIME_STATUS.RUNNING == false)
            break;

        k = _TIME_STATUS.control;

        EXOTAO_READ_VARIABLES();

        static LowPassFilter lpf_m(50, CONFIG::ts_s);
        static LowPassFilter lpf_l(50, CONFIG::ts_s);
        lpf_m.update(EXO::R::Knee::Motor.getPosition(), delta_t, 50);
        lpf_l.update(EXO::R::Knee::Encoder.getPosition(), delta_t, 50);
        KR.setTheta_m(lpf_m.getOutput());
        KR.setTheta_l(lpf_l.getOutput());

        if (_TIME_STATUS.seconds < 1.0)
        {
            EXOTAO_CALIBRATE();
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
            pos_d = 0.3 * (1 - cos(2.0 * M_PI * .2 * _TIME_STATUS.seconds));

            if (_TIME_STATUS.seconds > 35.0)
            {
                pos_d = 0;
            }

            /******************************************/
            /**************      HL      **************/
            /******************************************/
            KR.Kv = VARS_EXO::UDP_IN->KV;
            KR.Bv = 0;
            KR.theta_d = VARS_EXO::UDP_IN->KR_pos_d;
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
            KL.theta_d = VARS_EXO::UDP_IN->KL_pos_d;
            KL.omega_d = 0;
            KL.precalculate();
            omega = CTRL::PID(KL, 300, 10, 1);
            // omega = CTRL::PID(KL, 400, 40, 1);
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
            HL.theta_d = VARS_EXO::UDP_IN->HL_pos_d;
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
            HR.theta_d = VARS_EXO::UDP_IN->HR_pos_d;
            HR.omega_d = 0;
            HR.precalculate();
            // omega = CTRL::PID(HR, 300, 10, 1);
            omega = CTRL::PID(HR, 110, 1, 0);
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
            // EXO::R::Hip::Motor.setVelocity(HR.getOmega_m(0));
            // EXO::L::Hip::Motor.setVelocity(HL.getOmega_m(0));
        }

        EXOTAO_ADD_LOG_SAMPLE(k);

        if (delta_t >= CONFIG::ts_s)
        {
            // PRINT_LOG(5, PRINT_RED "[TimeOut] _threadControl %.8f s\n", delta_t);
        }
    }

    EXOTAO_DISABLE();
    EXOTAO_SAVE_LOGS(_TIME_STATUS.control);

    PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "Finish: void *_threadControl");

    return nullptr;
}

int main()
{

    CONFIG::setDuration(60);
    CONFIG::setTimeSample_us(2000L);

    EXOTAO_INIT_LOG_FILES((long)(CONFIG::durationTest * (1.0 / CONFIG::ts_s)));

    createSystemInterrupt();

    if (createTimer(CONFIG::ts_ns) != 0)
    {
        PRINT_LOG(1, PRINT_RED "[FAIL]" PRINT_RESET " createTimer()");
        return EXIT_FAILURE;
    }

    pthread_attr_t attr1;
    struct sched_param params2;
    pthread_attr_init(&attr1);
    pthread_attr_setschedpolicy(&attr1, SCHED_FIFO);
    params2.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_attr_setschedparam(&attr1, &params2);

    _TIME_STATUS.RUNNING = true;

    if (pthread_create(&threadControl, &attr1, _threadControl, nullptr) != 0)
    {
        PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "Creating *threadControl : %d", errno); // std::cerr;
        return 1;
    }

    if (EXOTAO_INIT_THREADS() != 0)
        return 1;

    pthread_attr_destroy(&attr1);

    if (pthread_join(threadControl, nullptr) != 0)
    {
        PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "Wait for: void *threadControl");
    }

    EXOTAO_STOP_THREADS();

    closeCallBack();

    return EXIT_SUCCESS;
}
