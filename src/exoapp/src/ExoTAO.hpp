#ifndef EXOTAO_HEADER
#define EXOTAO_HEADER

#include <cannetwork.hpp>
#include <eposnetwork.hpp>
#include <eposnode.hpp>
#include <utils_shm.h>
#include <utils_spinlock.h>
#include <utils_printlog.h>
#include <utils_files.h>
#include <utils_time.h>
#include <utils_exo.h>

extern CanNetwork can;
extern TIME_STATUS _TIME_STATUS;

namespace EXO
{
    extern EposNetwork net;
    namespace R
    {
        namespace Knee
        {
            extern EposNode Motor;
            extern EposNode Encoder;
        };
        namespace Hip
        {
            extern EposNode Motor;
        };
    };

    namespace L
    {
        namespace Knee
        {
            extern EposNode Motor;
            extern EposNode Encoder;
        };
        namespace Hip
        {
            extern EposNode Motor;
        };
    };
}

class File_ExoTao_Joint : public RerobAppDataLog
{

public:
    vector<float> time;
    vector<float> pos_in;
    vector<float> pos_out;
    vector<float> vel_in;
    vector<float> vel_out;
    vector<float> tau_d;
    vector<float> tau_l;
    vector<float> vel_d;

    File_ExoTao_Joint()
    {
    }

    void setSize(long size)
    {
        _size = size + 100;
        time.reserve(_size);
        pos_in.reserve(_size);
        pos_out.reserve(_size);
        vel_in.reserve(_size);
        vel_out.reserve(_size);
        vel_d.reserve(_size);
        tau_d.reserve(_size);
        tau_l.reserve(_size);
    }

    void saveHeader(FILE *file)
    {
        fprintf(file, "time\tpos_in\tpos_out\tvel_in\tvel_out\tvel_d\ttau_d\ttau_l\n");
    }

    void saveLine(FILE *file, long idx)
    {
        fprintf(file, "%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f\n", time[idx], pos_in[idx], pos_out[idx], vel_in[idx], vel_out[idx], vel_d[idx], tau_d[idx], tau_l[idx]);
    }
};

extern File_ExoTao_Joint file_Knee_R;
extern File_ExoTao_Joint file_Knee_L;
extern File_ExoTao_Joint file_Hip_R;
extern File_ExoTao_Joint file_Hip_L;

extern SpinLock spinlock_Sync;

namespace VARS_EXO
{
    extern ActuatorVARS KR;
    extern ActuatorVARS KL;
    extern ActuatorVARS HL;
    extern ActuatorVARS HR;
    extern shm_struct_vars_in *UDP_IN;
};


void EXOTAO_ENABLE(bool status = true);
inline void EXOTAO_DISABLE() { EXOTAO_ENABLE(false); };

inline void EXOTAO_CALIBRATE()
{
    using namespace VARS_EXO;

    HR.Z_theta_m = EXO::R::Hip::Motor.getPosition();
    HR.Z_theta_l = HR.getTheta_l();

    HL.Z_theta_m = EXO::L::Hip::Motor.getPosition();
    HL.Z_theta_l = HL.getTheta_l();

    KR.Z_theta_m = EXO::R::Knee::Motor.getPosition();
    KR.Z_theta_l = EXO::R::Knee::Encoder.getPosition();

    KL.Z_theta_m = EXO::L::Knee::Motor.getPosition();
    KL.Z_theta_l = EXO::L::Knee::Encoder.getPosition();
}

void EXOTAO_LOAD_CONFIGURATION();

inline void EXOTAO_READ_CAN_BUFFER() { can.fillDic(); }

inline void EXOTAO_READ_VARIABLES()
{
    using namespace VARS_EXO;

    EXO::net.sync(false);
    EXO::net.readPDO_TX_all();

    KR.setTheta_m(EXO::R::Knee::Motor.getPosition());
    KR.setTheta_l(EXO::R::Knee::Encoder.getPosition());

    KL.setTheta_m(EXO::L::Knee::Motor.getPosition());
    KL.setTheta_l(EXO::L::Knee::Encoder.getPosition());

    HR.setTheta_m(EXO::R::Hip::Motor.getPosition());
    HL.setTheta_m(EXO::L::Hip::Motor.getPosition());
}

void EXOTAO_INIT_LOG_FILES(long k);
inline void EXOTAO_ADD_LOG_SAMPLE(long k)
{
    using namespace VARS_EXO;

    if (k == -1)
        k = _TIME_STATUS.control;

    file_Knee_R.time[k] = _TIME_STATUS.seconds;
    file_Knee_R.pos_in[k] = KR.getTheta_m();
    file_Knee_R.pos_out[k] = KR.getTheta_l();
    file_Knee_R.vel_in[k] = EXO::R::Knee::Motor.getVelocity();
    file_Knee_R.vel_out[k] = EXO::R::Knee::Encoder.getVelocity();
    file_Knee_R.vel_d[k] = KR.getOmega_m(0);
    file_Knee_R.tau_d[k] = KR.getTau_d(0);
    file_Knee_R.tau_l[k] = KR.getTau_l(0);

    file_Knee_L.time[k] = _TIME_STATUS.seconds;
    file_Knee_L.pos_in[k] = KL.getTheta_m();
    file_Knee_L.pos_out[k] = KL.getTheta_l();
    file_Knee_L.vel_in[k] = EXO::L::Knee::Motor.getVelocity();
    file_Knee_L.vel_out[k] = EXO::L::Knee::Encoder.getVelocity();
    file_Knee_L.vel_d[k] = KL.getOmega_m(0);
    file_Knee_L.tau_d[k] = KL.getTau_d(0);
    file_Knee_L.tau_l[k] = KL.getTau_l(0);

    file_Hip_R.time[k] = _TIME_STATUS.seconds;
    file_Hip_R.pos_in[k] = HR.getTheta_m();
    file_Hip_R.pos_out[k] = HR.getTheta_l();
    file_Hip_R.vel_in[k] = HR.getOmega_m();
    file_Hip_R.vel_out[k] = HR.getOmega_l();
    file_Hip_R.vel_d[k] = HR.getOmega_m(0);
    file_Hip_R.tau_d[k] = HR.getTau_d(0);
    file_Hip_R.tau_l[k] = HR.getTau_l(0);

    file_Hip_L.time[k] = _TIME_STATUS.seconds;
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
}

void EXOTAO_SAVE_LOGS(long nSamples = -1);

int EXOTAO_INIT_THREADS();
int EXOTAO_STOP_THREADS();

#endif