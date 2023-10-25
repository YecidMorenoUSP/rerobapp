#pragma once


#include "utils_exo.h"
using namespace std;

namespace CTRL{

    double PID(ActuatorVARS joint, double kp, double ki, double kd, double dt){
        double erro_0 = joint.tau_d[joint.getIdx(0)]-joint.tau_l[joint.getIdx(0)];
        double erro_1 = joint.tau_d[joint.getIdx(-1)]-joint.tau_l[joint.getIdx(-1)];
        double erro_2 = joint.tau_d[joint.getIdx(-2)]-joint.tau_l[joint.getIdx(-2)];

        // PRINT_LOG(2,PRINT_MAGENTA " 0:%2d   -1:%2d   -2:%2d   " PRINT_RESET,joint.getIdx(0),joint.getIdx(-1),joint.getIdx(-2));

        return(joint.omega_m[joint.getIdx(-1)] + kp*(erro_0 - erro_1)
                + ki*dt*erro_0
                + (kd/dt)* (erro_0 - 2.0 * erro_1 + erro_2));
    }

    double PID(ActuatorVARS joint, double kp, double ki, double kd){
        return PID(joint, kp, ki,kd,CONFIG::ts_s);
    }
    
};