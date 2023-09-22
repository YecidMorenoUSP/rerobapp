#pragma once


#include <imgui.h>

#ifndef ARMA_INCLUDES
#include <cstdio>
#include <iostream>
#include <armadillo>
#endif


#include <thread_exo.h>
#include <string>
#include <thread>
#include <atomic>

#include <AXIS.h>



#include <utils_control.hpp>
#include <LowPassFilter2p.h>

typedef union{
    struct{
        float Fx;
        float Fy;   
        float Fz;
        float Tx;
        float Ty;   
        float Tz;
    };
    float raw[6];
} fromMarkovMao1;


class ThreadMarkovMao1: public ThreadType{
    private:
        fromMarkovMao1 data;
        fromATIMX dataATI;
        fromEposEXO dataEXO;
        fromXsensLeo dataXsensLeo;
        fromXsensGaitPhase dataXsensGaitPhase;

        class Joint_EXO {
            public:
                double Kv, Bv, torque_d, theta_c, theta_l, omega_l, omega_m, theta_m,
                    omega_ld, theta_ld, dot_torque[2] = { 0 }, torque_r[7] = { 0 }, error_theta[3] = { 0 }, error_omega[3] = { 0 }, error_theta_l[4] = { 0 },
                    int_torque_error[2] = { 0 }, error_torque[3] = { 0 }, kp, ki, kd, encoder_in_Q, encoder_out_Q,
                    N, ks, erro_0, erro_1, erro_2, u_ant, Ts, _theta_l[4] = { 0 };
                double ZERO_M, ZERO_L, ZERO_C;
                double _omega_l[4] = { 0 }, d_omega_l = 0;
                double _torque_i[4] = { 0 }, d_torque_i = 0;

                int VEL_MAX;
                arma::mat X,u,K;
                arma::mat K1,  K2 , K3 , K4 , K5;
                arma::mat Br, Jr, Kh;
                std::string dir__;
                std::atomic<bool> _atomic_trheadaux;
                std::thread _threadAux;
                AXIS* axis_m_ptr;
                AXIS* axis_l_ptr;

                enum {
                    FILTER_D_OMEGA_L,
                    FILTER_D_TORQUE_I,
                };

                LowPassFilter2pFloat filters[2];

                Joint_EXO(){

                }
                
                bool loadKs(){
            
                    if(K1.load((dir__ +"K1.dat").c_str()))
                    if(K2.load((dir__ +"K2.dat").c_str()))
                    if(K3.load((dir__ +"K3.dat").c_str()))
                    if(K4.load((dir__ +"K4.dat").c_str()))
                    if(K5.load((dir__ +"K5.dat").c_str()))
                        return true;

                    K1.clear();
                    K2.clear();
                    K3.clear();
                    K4.clear();
                    K5.clear();
                    return false;

                }

                bool loadLoopExterno() {
                    if (Br.load((dir__ + "Br.dat").c_str()))
                    if (Jr.load((dir__ + "Jr.dat").c_str()))
                    if (Kh.load((dir__ + "Kh.dat").c_str()))
                    return true;

                    Br.clear();
                    Jr.clear();
                    Kh.clear();

                    return false;


                }

                void calculate_rlqr(){
                    _atomic_trheadaux = true;
                    using namespace rlqr;
                    using namespace std;         
                    
                    arma::cube Bk;
                    Bk.load((dir__ + "B.dat").c_str());
                    arma::cube B2;
                    B2.load((dir__ + "B2.dat").c_str());
                    arma::cube Eb;
                    Eb.load((dir__ + "Eb.dat").c_str());
                    arma::cube Ef;
                    Ef.load((dir__ + "Ef.dat").c_str());
                    arma::cube Fk;
                    Fk.load((dir__ + "F.dat").c_str());
                    arma::cube G;
                    G.load((dir__ + "G.dat").c_str());
                    arma::cube Hk;
                    Hk.load((dir__ + "H.dat").c_str());
                    arma::cube M;
                    M.load((dir__ + "M.dat").c_str());
                    arma::cube P_;
                    P_.load((dir__ + "P.dat").c_str());
                    arma::mat Prob;
                    Prob.load((dir__ + "Prob.dat").c_str());
                    arma::cube Q;
                    Q.load((dir__ + "Q.dat").c_str());
                    arma::cube R;
                    R.load((dir__ + "R.dat").c_str());

                    mat L;
                    L.zeros(8,8);
                    mat K;
                    K.zeros(1,8);
                    
                    //mat K1 ,  K2 , K3 , K4 , K5;
                    
                    int Nk = 1200;//15000/4;
                    int s = 5;

                    cube P(64,Nk,5);
                    mat P_1, P_slice0_mat, P_2, P_slice1_mat, P_3, P_slice2_mat, P_4, P_slice3_mat, P_5, P_slice4_mat;
                    P_slice0_mat.zeros(64,Nk);
                    P_slice1_mat.zeros(64,Nk);
                    P_slice2_mat.zeros(64,Nk);
                    P_slice3_mat.zeros(64,Nk);
                    P_slice4_mat.zeros(64,Nk);
                    
                    P_1.zeros(64,1);
                    P_1  = P_.slice(0);

                    P_slice0_mat.submat(0,0,63,0) = P_1;
                    P.slice(0) = P_slice0_mat; 

                    P_slice1_mat.submat(0,0,63,0) = P_1;
                    P.slice(1) = P_slice1_mat; 

                    P_slice2_mat.submat(0,0,63,0) = P_1;
                    P.slice(2) = P_slice2_mat; 

                    P_slice3_mat.submat(0,0,63,0) = P_1;
                    P.slice(3) = P_slice3_mat; 

                    P_slice4_mat.submat(0,0,63,0) = P_1;
                    P.slice(4) = P_slice4_mat; 


                    field<cube> RLQR(8,1);
                    cube RLQR_Pr(64,Nk,5);
                    cube RLQR_Kr( 8,Nk,5);
                    cube RLQR_Lr(64,Nk,5);


                    mat RLQR_Pr_mat_modo1, RLQR_Pr_mat_modo2, RLQR_Pr_mat_modo3, RLQR_Pr_mat_modo4, RLQR_Pr_mat_modo5;
                    mat RLQR_Kr_mat_modo1, RLQR_Kr_mat_modo2, RLQR_Kr_mat_modo3, RLQR_Kr_mat_modo4, RLQR_Kr_mat_modo5;
                    mat RLQR_Lr_mat_modo1, RLQR_Lr_mat_modo2, RLQR_Lr_mat_modo3, RLQR_Lr_mat_modo4, RLQR_Lr_mat_modo5;

                    RLQR_Pr_mat_modo1.zeros(64,Nk);
                    RLQR_Pr_mat_modo2.zeros(64,Nk);
                    RLQR_Pr_mat_modo3.zeros(64,Nk);
                    RLQR_Pr_mat_modo4.zeros(64,Nk);
                    RLQR_Pr_mat_modo5.zeros(64,Nk);
                        
                    RLQR_Kr_mat_modo1.zeros(8,Nk);
                    RLQR_Kr_mat_modo2.zeros(8,Nk);
                    RLQR_Kr_mat_modo3.zeros(8,Nk);
                    RLQR_Kr_mat_modo4.zeros(8,Nk);
                    RLQR_Kr_mat_modo5.zeros(8,Nk);
                    
                    RLQR_Lr_mat_modo1.zeros(64,Nk);
                    RLQR_Lr_mat_modo2.zeros(64,Nk);
                    RLQR_Lr_mat_modo3.zeros(64,Nk);   
                    RLQR_Lr_mat_modo4.zeros(64,Nk);   
                    RLQR_Lr_mat_modo5.zeros(64,Nk);   

                
                    cout<<"\n Calculando RLQR";
                    RLQR = rlqr_slsm(Fk, Ef, Bk, Eb, Hk, P, Q, R, M, s, Prob, Nk);
                    cout<<"\n RLQR terminado";

                    cout<<"\n Salvando RLQR";

                    RLQR_Pr = RLQR(0,0);
                    RLQR_Kr = RLQR(1,0);
                    RLQR_Lr = RLQR(2,0);


                    RLQR_Pr_mat_modo1 = RLQR_Pr.slice(0);
                    RLQR_Pr_mat_modo1.save((dir__ +"Pr_modo1.dat").c_str(), raw_ascii);

                    RLQR_Pr_mat_modo2 = RLQR_Pr.slice(1);
                    RLQR_Pr_mat_modo2.save((dir__ +"Pr_modo2.dat").c_str(), raw_ascii);

                    RLQR_Pr_mat_modo3 = RLQR_Pr.slice(2);
                    RLQR_Pr_mat_modo3.save((dir__ +"Pr_modo3.dat").c_str(), raw_ascii);

                    RLQR_Pr_mat_modo4 = RLQR_Pr.slice(3);
                    RLQR_Pr_mat_modo4.save((dir__ +"Pr_modo4.dat").c_str(), raw_ascii);

                    RLQR_Pr_mat_modo5 = RLQR_Pr.slice(4);
                    RLQR_Pr_mat_modo5.save((dir__ +"Pr_modo5.dat").c_str(), raw_ascii);


                    RLQR_Kr_mat_modo1 = RLQR_Kr.slice(0);
                    RLQR_Kr_mat_modo1.save((dir__ +"Kr_modo1.dat").c_str(), raw_ascii);
                    
                    RLQR_Kr_mat_modo2 = RLQR_Kr.slice(1);
                    RLQR_Kr_mat_modo2.save((dir__ +"Kr_modo2.dat").c_str(), raw_ascii);
                    
                    RLQR_Kr_mat_modo3 = RLQR_Kr.slice(2);
                    RLQR_Kr_mat_modo3.save((dir__ +"Kr_modo3.dat").c_str(), raw_ascii);

                    RLQR_Kr_mat_modo4 = RLQR_Kr.slice(3);
                    RLQR_Kr_mat_modo4.save((dir__ +"Kr_modo4.dat").c_str(), raw_ascii);

                    RLQR_Kr_mat_modo5 = RLQR_Kr.slice(4);
                    RLQR_Kr_mat_modo5.save((dir__ +"Kr_modo5.dat").c_str(), raw_ascii);


                    RLQR_Lr_mat_modo1 = RLQR_Lr.slice(0);
                    RLQR_Lr_mat_modo1.save((dir__ +"Lr_modo1.dat").c_str(), raw_ascii);

                    RLQR_Lr_mat_modo2 = RLQR_Lr.slice(1);
                    RLQR_Lr_mat_modo2.save((dir__ +"Lr_modo2.dat").c_str(), raw_ascii);

                    RLQR_Lr_mat_modo3 = RLQR_Lr.slice(2);
                    RLQR_Lr_mat_modo3.save((dir__ +"Lr_modo3.dat").c_str(), raw_ascii);

                    RLQR_Lr_mat_modo4 = RLQR_Lr.slice(3);
                    RLQR_Lr_mat_modo4.save((dir__ +"Lr_modo4.dat").c_str(), raw_ascii);

                    RLQR_Lr_mat_modo5 = RLQR_Lr.slice(4);
                    RLQR_Lr_mat_modo5.save((dir__ +"Lr_modo5.dat").c_str(), raw_ascii);
                    
                    K1 = RLQR_Kr_mat_modo1.submat(0,Nk-2,7,Nk-2);
                    K2 = RLQR_Kr_mat_modo2.submat(0,Nk-2,7,Nk-2);
                    K3 = RLQR_Kr_mat_modo3.submat(0,Nk-2,7,Nk-2);
                    K4 = RLQR_Kr_mat_modo4.submat(0,Nk-2,7,Nk-2);
                    K5 = RLQR_Kr_mat_modo5.submat(0,Nk-2,7,Nk-2);


                    K1.save((dir__ +"K1.dat").c_str(), raw_ascii);
                    K2.save((dir__ +"K2.dat").c_str(), raw_ascii);
                    K3.save((dir__ +"K3.dat").c_str(), raw_ascii);
                    K4.save((dir__ +"K4.dat").c_str(), raw_ascii);
                    K5.save((dir__ +"K5.dat").c_str(), raw_ascii);

                
                    cout<<"\n Terminado salvado RLQR";
                    _atomic_trheadaux = false;
                }   

                void calculate_rlqr_async(){
                       if(_threadAux.joinable()){
                           _threadAux.join();
                        }
                        _threadAux = std::thread(&Joint_EXO::calculate_rlqr,this);
                 }


                void reset(){
                    Kv = 0;Bv = 0;torque_d = 0;theta_c = 0;theta_l = 0;omega_l = 0;theta_m = 0;omega_ld = 0;theta_ld = 0;
                    dot_torque[0] = { 0 }; torque_r[0] = { 0 }; dot_torque[2] = { 0 }; torque_r[0] = { 0 }; error_theta[0] = { 0 }; _theta_l[0] = { 0 };
                    error_omega[0] = {0};int_torque_error[0] = {0};error_torque[3] = {0};kp  = 0;ki  = 0;kd  = 0;encoder_in_Q = 0;
                    encoder_out_Q = 0; N = 0; ks = 0; erro_0 = 0, erro_1 = 0, erro_2 = 0; u_ant = 0; Ts = 0; omega_m = 0; error_theta_l[0] = 0;
                    VEL_MAX = 500; ZERO_M = 0; ZERO_L = 0;

                    /*
                        Falta resetear Vectores
                    */


                    erro_2 = 0;
                    erro_1 = 0;
                    u_ant = 0;
                    torque_r[6] =0;
                    torque_r[5] =0;
                    torque_r[4] =0;
                    torque_r[3] =0;
                    torque_r[2] =0;
                    torque_r[1] = 0;
                    torque_r[0] = 0;

                    error_theta_l[3] =0;
                    error_theta_l[2] =0;
                    error_theta_l[1] = 0;
                    error_theta_l[0] = 0;

                    _theta_l[3] =0;
                    _theta_l[2] =0;
                    _theta_l[1] = 0;
                    _theta_l[0] = 0;


                    error_theta[2] = 0;
                    error_theta[1] = 0;
                    
                    dot_torque[1] = 0;
                    dot_torque[0] = 0;
                    int_torque_error[1] = 0;
                    int_torque_error[0] = 0;

                    _omega_l[0] = 0;
                    d_omega_l = 0;
                    d_omega_l =0;

                    _omega_l[3] = 0;
                    _omega_l[2] = 0;
                    _omega_l[1] = 0;
                    _omega_l[0] = 0;

                    _torque_i[0] = 0;
                    d_torque_i =0;
                    d_torque_i =0;

                    _torque_i[3] = 0;
                    _torque_i[2] = 0;
                    _torque_i[1] = 0;
                    _torque_i[0] = 0;


                    u.zeros(1,1);
                    X.zeros(1,1);
                    for (int i = 0; i < sizeof(filters) / sizeof(LowPassFilter2pFloat); i++) {
                        filters[i].set_cutoff_frequency(1/Ts, 10);
                        filters[i].reset();
                    }
                }


                void updatePosEncoder() {
                    long raw_angle_m; 
                    long raw_angle_l;
                    ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->getPosition(axis_m_ptr, &raw_angle_m);
                    ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->getPosition(axis_l_ptr, &raw_angle_l);

                    setPosEncoder_m(raw_angle_m);
                    setPosEncoder_l(raw_angle_l);
                }

                void setOrigin() {
                    ZERO_M = 0;
                    ZERO_L = 0;
                    ZERO_C = 0;
                    updatePosEncoder();
                    ZERO_M = theta_m;
                    ZERO_L = theta_l;
                    ZERO_C = theta_c;
                    std::cout << "\n\n Fron MAarkov : ZERO_M : " << ZERO_M << "     ZERO_L :   " << ZERO_L << "\n\n";
                }

                void setPosEncoder_m(long raw_angle){
                    //raw_angle -= ZERO_M;
                    theta_c = ( raw_angle * 2.0 * arma::datum::pi) / (encoder_in_Q * N);
                    theta_m = ( raw_angle * 2.0 * arma::datum::pi) / (encoder_in_Q );

                    theta_c -= ZERO_C;
                    theta_m -= ZERO_M;

                }

                void setPosEncoder_l(long raw_angle){
                    //raw_angle -= ZERO_L;
                    theta_l = ( - raw_angle * 2.0 * arma::datum::pi) / (encoder_out_Q );
                    theta_l -= ZERO_L;

                    _theta_l[0] = theta_l;
                    
                }

                void calculate_torque_d(){
                    torque_d = Kv * (theta_ld - theta_l) + Bv * (omega_ld - omega_l);

                    torque_r[0] = ks * (theta_c - theta_l);
                    erro_0 = (torque_d - torque_r[0]);
                }

                void prepareMarkovStates() {

                    torque_r[0] = ks * (theta_c - theta_l);
                    error_theta[0] = theta_ld - theta_l;
                    error_omega[0] = omega_ld - omega_l;
                    error_torque[0] = torque_d - torque_r[0];
                    int_torque_error[0] = int_torque_error[1] + error_torque[0] * Ts;
                    dot_torque[0] = (torque_r[0] - torque_r[2]) / (2 * Ts);

                }

                void calculate_PID_signal_controle(){
                    u(0,0) = u_ant + kp * (erro_0 - erro_1) + ki * Ts * erro_0 + (kd / Ts) * (erro_0 - 2 * erro_1 + erro_2);
                    omega_m = arma::as_scalar(u);

                }

                void calculate_torque_d_PID() {
                    torque_d = Kv * (theta_ld - theta_l) + Bv * (omega_ld - omega_l) ;

                    torque_r[0] = ks * (theta_c - theta_l);
                    erro_0 = (torque_d - torque_r[0]);
                }

                void calculate_torque_d_loop_externo(int gait_phase) {
                    //torque_d = 0; -Kv * X(2, 0) - Bv * d_torque_i; (Kh(0, gait_phase - 1) * X(5, 0) + Jr(0, 0) * d_omega_l);
                    torque_d =  -Kv * X(2, 0) - Bv * d_torque_i;
                }

                void calculate_Markov_signal_controle(int gait_phase){
                    if(gait_phase == 1) { K = K1; }
                    else if(gait_phase == 2) {K = K2; }
                    else if(gait_phase == 3) {K = K3; }
                    else if(gait_phase == 4) {K = K4; }
                    else if(gait_phase == 5) {K = K5; }

                    u =  K.t() * X;

                    omega_m = arma::as_scalar(u);
                    omega_l = (_theta_l[0] - _theta_l[2])/(2*Ts); // ((omega_m / N) - (dot_torque[0] / ks));
                }

                void prepareNewLoop(){
                    erro_2 = erro_1;
                    erro_1 = erro_0;
                    u_ant = arma::as_scalar(u);
                    torque_r[6] = torque_r[5];
                    torque_r[5] = torque_r[4];
                    torque_r[4] = torque_r[3];
                    torque_r[3] = torque_r[2];
                    torque_r[2] = torque_r[1];
                    torque_r[1] = torque_r[0];

                    error_theta_l[3] = error_theta_l[2];
                    error_theta_l[2] = error_theta_l[1];
                    error_theta_l[1] = error_theta_l[0];

                    _theta_l[3] = _theta_l[2];
                    _theta_l[2] = _theta_l[1];
                    _theta_l[1] = _theta_l[0];


                    error_theta[2] = error_theta[1];
                    error_theta[1] = error_theta[0];
                    dot_torque[1] = dot_torque[0];
                    int_torque_error[1] = int_torque_error[0];

                    _omega_l[0] = omega_l;
                     d_omega_l = (_omega_l[0] - _omega_l[2]) / (2.0 * Ts);
                     d_omega_l = filters[FILTER_D_OMEGA_L].apply(d_omega_l);

                    _omega_l[3] = _omega_l[2];
                    _omega_l[2] = _omega_l[1];
                    _omega_l[1] = _omega_l[0];

                    _torque_i[0] = X(2, 0);
                    d_torque_i = (_torque_i[0] - _torque_i[2]) / (2.0 * Ts);
                    d_torque_i = filters[FILTER_D_TORQUE_I].apply(d_torque_i);

                    _torque_i[3] = _torque_i[2];
                    _torque_i[2] = _torque_i[1];
                    _torque_i[1] = _torque_i[0];
                    
                }

                void setVelocity() {
                    if (omega_m >= VEL_MAX) { omega_m = VEL_MAX; }
                    else if (omega_m <= -VEL_MAX) { omega_m = -VEL_MAX; }
                    ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->setVelocity(axis_m_ptr, (long) omega_m);
                }
 
        };

        Joint_EXO * knee_r;
        Joint_EXO * knee_l;
        

    public:       
        
        std::atomic<bool> showGraph;
        
        std::vector<PlotWindow> pw;


        std::string LOG;
        
        ThreadMarkovMao1(){

            knee_r = new Joint_EXO();
            knee_r->dir__ = "Mat2Exo/out/";
            knee_r->axis_m_ptr = &((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->servo_knee_right;
            knee_r->axis_l_ptr = &((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->encoder_knee_right;

            knee_l = new Joint_EXO();
            knee_l->axis_m_ptr = &((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->servo_knee_left;
            //knee_l->axis_l_ptr = &((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->encoder_knee_left;

            pw.push_back(PlotWindow("Units [ Nm/s ]", "Time [ s ]", "State 1"));
            pw.push_back(PlotWindow("Units [ Nm ]", "Time [ s ]", "State 2"));
            pw.push_back(PlotWindow("Units [ Nm ]", "Time [ s ]", "State 3"));
            pw.push_back(PlotWindow("Units [ rad/s ]", "Time [ s ]", "State 4"));
            pw.push_back(PlotWindow("Units [ rad ]", "Time [ s ]", "State 5"));
            pw.push_back(PlotWindow("Units [ rad/s ]", "Time [ s ]", "State 6"));
            pw.push_back(PlotWindow("Units [ rad ]", "Time [ s ]", "State 7"));
            pw.push_back(PlotWindow("Units [ int(Nm) ]", "Time [ s ]", "State 8"));
            pw.push_back(PlotWindow("Units [ rad/s ]", "Time [ s ]", "Signal Control"));

            
            

            showGraph = false;
        }

      

        std::string printK(arma::mat p){
            std::string tAut;
            char tt[20];
            for(int r = 0 ; r < p.n_rows ; r++){
                for(int c = 0 ; c < p.n_cols ; c++){
                    sprintf(tt,"%10.5e\t", p(r,c));
                    tAut += tt;    
                }
                tAut +=  "\n";
            }
            return tAut;
        }
       
        void _setup(){
            LOG = "";
            //if (EposEXOCAN1->toRUNfromGUI == false || ATIMX1->toRUNfromGUI == false || XSENSLeo->toRUNfromGUI == false)
            if (EposEXOCAN1->toRUNfromGUI == false)
                 throw "ERROR Markov";            
  

            knee_r->dir__ = "Mat2Exo/out/";
            if(!knee_r->loadKs()) throw "Error en las Ks";
            if(!knee_r->loadLoopExterno()) throw "Error en lectura loop externo";
            knee_r->reset();
            knee_r->Kv = KKVV;
            knee_r->Bv = BBVV;
            knee_r->kp = 380;
            knee_r->ki = 35;
            knee_r->kd = 3;
            knee_r->N = 150;
            knee_r->ks = 104;
            knee_r->Ts = _Ts;
            knee_r->encoder_in_Q = 4096;
            knee_r->encoder_out_Q = 2048;
            knee_r->VEL_MAX = 2500;
            knee_r->X.zeros(8, 1);
            knee_r->u.zeros(1, 1);
            knee_r->omega_m = 0;

            knee_l->reset();
            knee_l->Kv = 0;
            knee_l->Bv = 0;
            knee_l->kp = 200;
            knee_l->ki = 4;
            knee_l->kd = 0;
            knee_l->N = 150;
            knee_l->ks = 300;
            knee_l->Ts = _Ts;
            knee_l->encoder_in_Q = 4096;
            knee_l->encoder_out_Q = 2048;
            knee_l->VEL_MAX = 1000;
            knee_l->omega_m = 0;
            knee_l->u.zeros(1, 1);
            knee_l->X.zeros(8, 1);


            /*((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->reset_falhas();
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->setVelocityMode(knee_r->axis_m_ptr);
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->Habilita_Eixo(2);*/
            
            for (int idx = 0; idx < pw.size(); idx++) {
                pw[idx].SCOPE.x_axis.minimum = 0.0f;
                pw[idx].SCOPE.x_axis.maximum = T_exec;
                pw[idx].clearItems();
                pw[idx].addItem("...");
            }

            pw[0].SCOPE.y_axis.minimum = -200.0f;
            pw[0].SCOPE.y_axis.maximum =  200.0f;

            pw[1].SCOPE.y_axis.minimum = -10.0f;
            pw[1].SCOPE.y_axis.maximum =  10.0f;

            pw[2].SCOPE.y_axis.minimum = -10.0f;
            pw[2].SCOPE.y_axis.maximum =  10.0f;

            pw[3].SCOPE.y_axis.minimum = -1.0f;
            pw[3].SCOPE.y_axis.maximum =  1.0f;

            pw[4].SCOPE.y_axis.minimum = -.80f;
            pw[4].SCOPE.y_axis.maximum =  .80f;

            pw[5].SCOPE.y_axis.minimum = -1.0f;
            pw[5].SCOPE.y_axis.maximum =  1.0f;

            pw[6].SCOPE.y_axis.minimum = -.80f;
            pw[6].SCOPE.y_axis.maximum =  .80f;

            pw[7].SCOPE.y_axis.minimum = -1.0f;
            pw[7].SCOPE.y_axis.maximum =  1.0f;

            pw[8].SCOPE.y_axis.minimum = -2000.0f;
            pw[8].SCOPE.y_axis.maximum =  2000.0f;
        }

        void _cleanup(){
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->Desabilita_Eixo(0);
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->setVelocityZero();
        }
        
        void _firstLoop() {
            
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->setVelocityMode(knee_r->axis_m_ptr);
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->setVelocityMode(knee_l->axis_m_ptr);
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->Habilita_Eixo(2);
            knee_r->setOrigin();
            knee_l->setOrigin();
            
        }


        

        void _loop(){

          


            //ATIMX1->getData(&dataATI);
            //XSENSLeo->getData(&dataXsensLeo);
            //XsensGaitPhase1->getData(&dataXsensGaitPhase);

            //knee_r->updatePosEncoder();

            //int gait_phase = dataXsensGaitPhase.gait_phase;
           
            
            //if (time_index < (2 * _Ts)) knee_r->torque_d = 0;
            //else {
            //    knee_r->calculate_torque_d_loop_externo(gait_phase);
            //}
            

            knee_r->updatePosEncoder();
            knee_r->theta_ld = (sin(2.0*arma::datum::pi*1.0/5.0* timer->get_current_time_f() - arma::datum::pi / 2.0 )+1.0) * 0.5;
            knee_r->calculate_torque_d_PID();
            knee_r->calculate_PID_signal_controle();
            knee_r->setVelocity();
            knee_r->prepareNewLoop();

            /*knee_r->updatePosEncoder();

            knee_r->calculate_torque_d_PID();
            knee_r->calculate_PID_signal_controle();
            knee_r->setVelocity();
            knee_r->prepareNewLoop();*/

            //knee_r->prepareMarkovStates();

            knee_r->X(0,0) = knee_r->dot_torque[0]; 
            knee_r->X(1,0) = knee_r->torque_r[0]; 
            //knee_r->X(2,0) = -dataATI.Fx * .25;
            knee_r->X(3,0) = knee_r->omega_l; 
            knee_r->X(4,0) = knee_r->theta_l; 
            //knee_r->X(5,0) = dataXsensLeo.data[1];
            //knee_r->X(6,0) = dataXsensLeo.data[0];
            knee_r->X(7,0) = knee_r->int_torque_error[0]; 

            //knee_r->calculate_Markov_signal_controle(gait_phase);
            
            //knee_r->setVelocity();
            
            //knee_r->prepareNewLoop();

            

            //knee_l->updatePosEncoder();
            //knee_l->calculate_torque_d_PID();
            //knee_l->calculate_PID_signal_controle();
            //knee_l->setVelocity();
            //knee_l->prepareNewLoop();

            

            _mtx.lock();   
                _datalog[time_index][0] = timer->get_delta_time();
                _datalog[time_index][1] = timer->get_current_time_f();
                _datalog[time_index][2] = (float) knee_r->X(0, 0);
                _datalog[time_index][3] = (float) knee_r->X(1, 0);
                _datalog[time_index][4] = (float) knee_r->X(2, 0);
                _datalog[time_index][5] = (float) knee_r->X(3, 0);
                _datalog[time_index][6] = (float) knee_r->X(4, 0);
                _datalog[time_index][7] = (float) knee_r->X(5, 0);
                _datalog[time_index][8] = (float) knee_r->X(6, 0);
                _datalog[time_index][9] = (float) knee_r->X(7, 0);
                _datalog[time_index][10] = arma::as_scalar(knee_r->omega_m);//u;
                _datalog[time_index][11] = knee_r->torque_d;
                _datalog[time_index][12] = knee_r->theta_m;
                _datalog[time_index][13] = knee_r->d_omega_l;
                //_datalog[time_index][14] = gait_phase;
                //_datalog[time_index][15] = knee_l->theta_m;
                //_datalog[time_index][16] = knee_l->theta_l;
                //_datalog[time_index][17] = knee_l->omega_m;
                //_datalog[time_index][18] = knee_l->torque_r[0];
                //_datalog[time_index][19] = knee_l->torque_d;
                

                for (int idx = 0; idx < pw.size(); idx++) {
                    pw[idx].items[0].data.push_back(ImVec2((float)_datalog[time_index][1], (float)_datalog[time_index][2+idx]));
                }

                pw[0].items[0].data.push_back(ImVec2(timer->get_current_time_f(), knee_l->theta_m));
                pw[1].items[0].data.push_back(ImVec2(timer->get_current_time_f(), knee_l->theta_l));

            _mtx.unlock();

            
        }

        std::atomic<float> KKVV ;
        std::atomic<float> BBVV ;
        float _Kv = -1;
        float _Bv = -0.1;
        
        bool cbPlot = false;
        void _updateGUI(){
            ImGui::Begin(_name.c_str());
                if(isAlive)
                    ImGui::Text("Markov : RUNNING");
                else
                    ImGui::Text("Markov : PAUSED");              

                if(ImGui::Button(knee_r->_atomic_trheadaux? "Calculating . . ." : "Calculate rlqr_slsm")){
                    if(!knee_r->_atomic_trheadaux){
                        knee_r->calculate_rlqr_async();
                    }
                }

                if(ImGui::Button("Show K{1:5}")){
                    if(knee_r->loadKs()){                        
                        LOG = "";
                        LOG += ("K1:\n\t" +  printK(knee_r->K1.t()));
                        LOG += ("K2:\n\t" +  printK(knee_r->K2.t()));
                        LOG += ("K3:\n\t" +  printK(knee_r->K3.t()));
                        LOG += ("K4:\n\t" +  printK(knee_r->K4.t()));
                        LOG += ("K5:\n\t" +  printK(knee_r->K5.t()));
                    }else{
                        LOG += "\nNo se han podido cargar";
                    }
                    
                    // K1.raw_print

                }
                ImGui::SameLine();
                if(ImGui::Button("Clear LOG")){
                    LOG = "";
                }

                ImGui::Checkbox("Graph", &cbPlot);
                showGraph = cbPlot;
                if (showGraph) {
                    for (int idx = 0; idx < pw.size(); idx++) {
                        pw[idx].showNewWindow();
                    }
                }

                ImGui::DragFloat("Kv ##time_count", &_Kv, 0.1f, -2, -2, "%.3f Segundos");
                ImGui::DragFloat("Bv ##time_count", &_Bv, 0.1f,  -2, 2, "%.3f Segundos");

                KKVV = _Kv;
                BBVV = _Bv;

                ImGui::Text(LOG.c_str());


            ImGui::End();
        }

 
        void getData(void * _data){
            _mtx.lock();
                memcpy(_data,&data,sizeof(fromMarkovMao1));
            _mtx.unlock();
        }
};
