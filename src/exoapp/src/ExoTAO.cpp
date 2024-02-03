#include "defines.h"

#include "ExoTAO.hpp"

// #include "variables.h"

void *_threadSync(void *arg);
void *_threadSerial(void *arg);

File_ExoTao_Joint file_Knee_R;
File_ExoTao_Joint file_Knee_L;
File_ExoTao_Joint file_Hip_R;
File_ExoTao_Joint file_Hip_L;

CanNetwork can((char *)"can0");

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

SpinLock spinlock_Sync;
pthread_t threadSync, threadSerial;

namespace VARS_EXO
{
    ActuatorVARS KR;
    ActuatorVARS KL;
    ActuatorVARS HL;
    ActuatorVARS HR;
    shm_struct_vars_in *UDP_IN;
};

void *_threadSync(void *arg)
{
    using namespace VARS_EXO;

    long k = 0;

    long can_sz = can._queued_frames.size();

    void *shm_ptr = shm_create(SHM_VARS_STREAM_KEY, SHM_VARS_STREAM_SIZE);
    shm_struct_vars_stream *var_stream = (shm_struct_vars_stream *)shm_ptr;
    var_stream->key = SHM_VARS_STREAM_KEY;

    void *shm_ptr_2 = shm_create(SHM_VARS_IN_KEY, SHM_VARS_IN_SIZE);
    VARS_EXO::UDP_IN = (shm_struct_vars_in *)shm_ptr_2;
    memset(VARS_EXO::UDP_IN, 0, sizeof(shm_struct_vars_in));
    shm_struct_vars_in *var_in = (shm_struct_vars_in *)shm_ptr_2;
    var_in->key = SHM_VARS_IN_KEY;

    auto us0 = std::chrono::high_resolution_clock::now();

    auto us1 = std::chrono::high_resolution_clock::now();

    long ms = 0;

    while (_TIME_STATUS.RUNNING)
    {

        spinlock_Sync.Lock();

        if (_TIME_STATUS.RUNNING == false)
            break;

        k = _TIME_STATUS.control;
        can_sz = can._queued_frames.size();

        if (k % 1000 == 0)
        {
            PRINT_LOG(2, "*[%3.4f][_threadSync] SZ_CAN: %2ld  Kv:%.4f pos_d:%.4f",
                      _TIME_STATUS.seconds,
                      can_sz,
                      VARS_EXO::UDP_IN->KV,
                      VARS_EXO::UDP_IN->pos_d);
        }

        if (k % 1 == 0)
        {
            // VARS_EXO::Kv = var_in->KV;

            var_stream->t_s = _TIME_STATUS.seconds;

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

void *_threadSerial(void *arg)
{
    using namespace VARS_EXO;
    using namespace std;
    using namespace std::chrono;

    SerialNode S;
    int res = S.openPort("/dev/ttyUSB0", B921600);

    if (res < 0)
    {
        PRINT_LOG(5, PRINT_RED "/dev/ttyUSB0: Error" PRINT_RESET);
        _TIME_STATUS.RUNNING = false;
        return nullptr;
    }

    TimerObj t0;
    S.writePort((char *)"r", 1);
    char response[128];
    int spot;

    while (_TIME_STATUS.RUNNING)
    {
        spinlock_Sync.Lock();
        std::this_thread::sleep_for(microseconds(100));
        if (_TIME_STATUS.RUNNING == false)
            break;

        char response[1024];
        S.writePort((char *)"p", 1);
        memset(response, '\0', 1024);
        spot = S.readPort(response, 10);

        if (response[0] == '@' && response[9] == '\n')
        {
            t0.timeAt();
            static float TimeRead = 0;
            TimeRead = (float)(t0.get<microseconds>() / 1000000.0f);
            t0.init();

            float *V = (float *)&(response[1]);
            static LowPassFilter V0;
            static LowPassFilter V1;

            V0.update(V[0], TimeRead, 500);
            V1.update(V[1], TimeRead, 500);

            HR.setTheta_l(-V0.getOutput());
            HL.setTheta_l(-V1.getOutput());
        }
        else
        {
            PRINT_LOG(5, PRINT_RED " [%3.4f] [spot %d] /dev/ttyUSB0: Read Failure" PRINT_RESET, _TIME_STATUS.seconds, spot);
        }
    }

    PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "Finish: void *_threadSerial");
    return nullptr;
}

void EXOTAO_ENABLE(bool status)
{
    if (status == true)
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

    if (status == false)
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
}

void EXOTAO_INIT_LOG_FILES(long k)
{
    file_Hip_L.setSize(k);
    file_Hip_R.setSize(k);
    file_Knee_R.setSize(k);
    file_Knee_L.setSize(k);
}

void EXOTAO_SAVE_LOGS(long nSamples)
{
    if (nSamples == -1)
        nSamples = _TIME_STATUS.control;

    PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "SAVING file_Knee_R K:%d", nSamples);
    file_Knee_R.saveToFileName("file_Knee_R", nSamples);

    PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "SAVING file_Knee_L K:%d", nSamples);
    file_Knee_L.saveToFileName("file_Knee_L", nSamples);

    PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "SAVING file_Hip_R K:%d", nSamples);
    file_Hip_R.saveToFileName("file_Hip_R", nSamples);

    PRINT_LOG(5, PRINT_GREEN "[OK]" PRINT_RESET "SAVING file_Hip_L K:%d", nSamples);
    file_Hip_L.saveToFileName("file_Hip_L", nSamples);
}

void EXOTAO_LOAD_CONFIGURATION()
{
    can.connect();

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

    EXO::R::Hip::Motor.encoder_Q = 12800.0;
    EXO::L::Hip::Motor.encoder_Q = 12800.0;
    EXO::R::Knee::Motor.encoder_Q = 4096.0;
    EXO::R::Knee::Encoder.encoder_Q = -2000.0;
    EXO::L::Knee::Motor.encoder_Q = 4096.0;
    EXO::L::Knee::Encoder.encoder_Q = -2000.0;

    EXO::net.setNode(EXO::R::Hip::Motor);
    EXO::net.setNode(EXO::L::Hip::Motor);
    EXO::net.setNode(EXO::R::Knee::Encoder);
    EXO::net.setNode(EXO::R::Knee::Motor);
    EXO::net.setNode(EXO::L::Knee::Encoder);
    EXO::net.setNode(EXO::L::Knee::Motor);

    EXO::net.init();
    EXO::net.sync(false);

    EXO::R::Hip::Motor.asyncCan = false;
    EXO::L::Hip::Motor.asyncCan = false;
    EXO::R::Knee::Encoder.asyncCan = false;
    EXO::R::Knee::Motor.asyncCan = false;
    EXO::L::Knee::Encoder.asyncCan = false;
    EXO::L::Knee::Motor.asyncCan = false;

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

    // can.setPeriod(CONFIG::ts_us);
    // can.startThread();
}

int EXOTAO_INIT_THREADS()
{

    pthread_attr_t attr1;
    pthread_attr_init(&attr1);
    struct sched_param params1;
    params1.sched_priority = 10;
    pthread_attr_setschedpolicy(&attr1, SCHED_FIFO);
    pthread_attr_setschedparam(&attr1, &params1);

    if (pthread_create(&threadSync, &attr1, _threadSync, nullptr) != 0)
    {
        PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "Creating *threadSync : %d", errno); // std::cerr;

        return 1;
    }

    pthread_attr_t attr3;
    struct sched_param params3;
    pthread_attr_init(&attr3);
    pthread_attr_setschedpolicy(&attr3, SCHED_FIFO);
    params3.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_attr_setschedparam(&attr3, &params3);

    if (pthread_create(&threadSerial, &attr3, _threadSerial, nullptr) != 0)
    {
        PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "Creating *threadSerial : %d", errno); // std::cerr;

        return 1;
    }

    pthread_attr_destroy(&attr1);
    pthread_attr_destroy(&attr3);

    return 0;
}

int EXOTAO_STOP_THREADS()
{

    if (pthread_join(threadSync, nullptr) != 0)
    {
        PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "Wait for: void *threadSync");
    }
    if (pthread_join(threadSerial, nullptr) != 0)
    {
        PRINT_LOG(5, PRINT_RED "[FAIL]" PRINT_RESET "Wait for: void *threadSerial");
    }

    return 0;
}