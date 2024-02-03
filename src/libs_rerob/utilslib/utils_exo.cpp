
#include "utils_exo.h"

#include <vector>
#include <math.h>

using namespace std;

float EXO_SPRING::T_quadraticSpringBooker(float theta_c, float theta_l, float ks)
{
    float X = theta_c - theta_l;
    X = -X;
    return -(-321500 * pow(X, 5) +
             14990 * pow(X, 4) +
             10620 * pow(X, 3) -
             ((1963 * pow(X, 2)) / (10)) +
             ((7919 * X) / (100)));

    if (X > 0)
    {

        return (52.19 * X + 907.8 * X * X); // f
    }
    else
    {
        X = -X;
        return -(71.5 * X + 825.8 * X * X);
    }
}

float EXO_SPRING::T_linearSpring(float theta_c, float theta_l, float ks)
{
    float X = theta_c - theta_l;
    return ks * X;
}

ActuatorVARS::ActuatorVARS(int n_size)
{
    _n_size = n_size;
    _idx = _n_size - 1;
    theta_m.assign(n_size, 0);
    theta_l.assign(n_size, 0);
    omega_m.assign(n_size, 0);
    omega_l.assign(n_size, 0);
    tau_l.assign(n_size, 0);
    tau_d.assign(n_size, 0);
}

void ActuatorVARS::shift()
{
    if ((_idx + 1) == _n_size)
        _idx = 0;
    else
        _idx++;

    setTheta_m(getTheta_m(-1));
    setTheta_l(getTheta_l(-1));
    setOmega_m(getOmega_m(-1));
    setOmega_l(getOmega_l(-1));
    setTau_d(getTau_d(-1));
    setTau_l(getTau_l(-1));
}
