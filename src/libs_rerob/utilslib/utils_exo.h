#pragma once

#include <vector>
using namespace std;

namespace EXO_SPRING
{
    float T_quadraticSpringBooker(float theta_c, float theta_l, float ks = 0.0)
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

    float T_linearSpring(float theta_c, float theta_l, float ks = 0.0)
    {
        float X = theta_c - theta_l;
        return ks * X;
    }
};

#define N_samples_vars 10
class ActuatorVARS
{
private:
    int _idx;
    int _n_size;

public:
    double N = 150.0;
    double ks = 104.0;

    double Kv = 0;
    double Bv = 0;

    double theta_d = 0.0;
    double omega_d = 0.0;

    double Z_theta_m = 0;
    double Z_theta_l = 0;

    vector<double> theta_m;
    vector<double> theta_l;
    vector<double> omega_m;
    vector<double> omega_l;
    vector<double> tau_l;
    vector<double> tau_d;

    float (*fnc_Torque)(float theta_c,float theta_l, float ks) ;

    ActuatorVARS(int n_size = N_samples_vars)
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

    void shift()
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

    double getTheta_m(int idx = 0)
    {
        return theta_m[getIdx(idx)];
    }
    void setTheta_m(double val)
    {
        theta_m[getIdx(0)] = val - Z_theta_m;
    }
    double getTheta_l(int idx = 0)
    {
        return theta_l[getIdx(idx)];
    }
    void setTheta_l(double val)
    {
        theta_l[getIdx(0)] = val - Z_theta_l;
    }
    double getOmega_m(int idx = 0)
    {
        return omega_m[getIdx(idx)];
    }
    void setOmega_m(double val)
    {
        omega_m[getIdx(0)] = val;
    }
    double getOmega_l(int idx = 0)
    {
        return omega_l[getIdx(idx)];
    }
    void setOmega_l(double val)
    {
        omega_l[getIdx(0)] = val;
    }
    double getTau_d(int idx = 0)
    {
        return tau_d[getIdx(idx)];
    }
    void setTau_d(double val)
    {
        tau_d[getIdx(0)] = val;
    }

    double getTau_l(int idx = 0)
    {
        return tau_l[getIdx(idx)];
    }
    void setTau_l(double val)
    {
        tau_l[getIdx(0)] = val;
    }
};
