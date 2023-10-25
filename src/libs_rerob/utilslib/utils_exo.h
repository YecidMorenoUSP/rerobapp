#pragma once

#include <vector>
using namespace std;

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

    ActuatorVARS(int n_size = N_samples_vars)
    {
        _n_size = n_size;
        _idx = _n_size - 1;
        theta_m.assign(n_size,0);
        theta_l.assign(n_size,0);
        omega_m.assign(n_size,0);
        omega_l.assign(n_size,0);
        tau_l.assign(n_size,0);
        tau_d.assign(n_size,0);
    }

    void shift()
    {
        if ((_idx + 1) == _n_size)
            _idx = 0;
        else
            _idx++;
    }

    inline int getIdx(int idx)
    {
        if (idx == 0)
            return _idx;
        else{
            int res = _idx + idx;
            
            if(res>=0) return (res) % _n_size;
            else return  _n_size + ( (res) % _n_size );
        }
            
    }

    void precalculate(){
        tau_l[_idx] = (theta_m[_idx]/N - theta_l[_idx]) * ks ;
        tau_d[_idx] = Kv*(theta_d-theta_l[_idx]) + Bv*(omega_d - omega_l[_idx]);
    }

    double getTheta_m(int idx = 0)
    {
        return theta_m[getIdx(idx)];
    }
    void setTheta_m(double val)
    {
        theta_m[getIdx(0)] = val-Z_theta_m;
    }
    double getTheta_l(int idx = 0)
    {
        return theta_l[getIdx(idx)];
    }
    void setTheta_l(double val)
    {
        theta_l[getIdx(0)] = val-Z_theta_l;
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
