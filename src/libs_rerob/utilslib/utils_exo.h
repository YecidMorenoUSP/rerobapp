#ifndef UTILS_EXO_H
#define UTILS_EXO_H

#include <vector>
using namespace std;

namespace EXO_SPRING
{
    float T_quadraticSpringBooker(float theta_c, float theta_l, float ks = 0.0);
    float T_linearSpring(float theta_c, float theta_l, float ks = 0.0);
};

#define N_samples_vars 10
class ActuatorVARS
{
private:
    int _idx;
    int _n_size;

public:
    double N;
    double ks;

    double Kv;
    double Bv;

    double theta_d;
    double omega_d;

    double Z_theta_m;
    double Z_theta_l;

    vector<double> theta_m;
    vector<double> theta_l;
    vector<double> omega_m;
    vector<double> omega_l;
    vector<double> tau_l;
    vector<double> tau_d;

    float (*fnc_Torque)(float theta_c,float theta_l, float ks) ;

    ActuatorVARS(int n_size = N_samples_vars);

    void shift();

    inline int getIdx(int idx)
    {
        if (idx == 0)
            return _idx;
        else
        {
            int res = _idx + idx;

            if (res >= 0)
                return (res) % _n_size;
            else
                return _n_size + ((res) % _n_size);
        }
    }

    void precalculate()
    {
        tau_l[_idx] = fnc_Torque(theta_m[_idx] / N , theta_l[_idx] , ks);//(theta_m[_idx] / N - theta_l[_idx]) * ks;
        tau_d[_idx] = Kv * (theta_d - theta_l[_idx]) + Bv * (omega_d - omega_l[_idx]);
    }

    inline double getTheta_m(int idx = 0)
    {
        return theta_m[getIdx(idx)];
    }
    inline void setTheta_m(double val)
    {
        theta_m[getIdx(0)] = val - Z_theta_m;
    }
    inline double getTheta_l(int idx = 0)
    {
        return theta_l[getIdx(idx)];
    }
    inline void setTheta_l(double val)
    {
        theta_l[getIdx(0)] = val - Z_theta_l;
    }
    inline double getOmega_m(int idx = 0)
    {
        return omega_m[getIdx(idx)];
    }
    inline void setOmega_m(double val)
    {
        omega_m[getIdx(0)] = val;
    }
    inline double getOmega_l(int idx = 0)
    {
        return omega_l[getIdx(idx)];
    }
    inline void setOmega_l(double val)
    {
        omega_l[getIdx(0)] = val;
    }
    inline double getTau_d(int idx = 0)
    {
        return tau_d[getIdx(idx)];
    }
    inline void setTau_d(double val)
    {
        tau_d[getIdx(0)] = val;
    }

    inline double getTau_l(int idx = 0)
    {
        return tau_l[getIdx(idx)];
    }
    inline void setTau_l(double val)
    {
        tau_l[getIdx(0)] = val;
    }
};


#endif