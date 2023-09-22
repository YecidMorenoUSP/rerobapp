#pragma once

#include <imgui.h>

#include <thread_exo.h>
#include <string>
#include <AXIS.h>

#define N_SAMPLES 6

typedef struct{
    
} fromPID_Jonathan;


class ThreadPID_Jonathan : public ThreadType{
    private:
        fromPID_Jonathan data;
        fromJonathanESP32FSR ESP32FSR;

    public:       
        
        class EXO_joint {
        private:
            typedef struct {
                double theta_l;
                double theta_ld;
                double theta_c;
                double theta_m;
                double omega_m;
                double omega_l;
                double torque_d;
                double torque_l;
                double time;
            }Vars;

            typedef struct {
                double ZERO_L;       //
                double ZERO_C;       //
                double ZERO_M;       //
                int encoder_in_Q;    // adimencional
                int encoder_out_Q;   // adimencional
                AXIS* axis_m_ptr;    // 
                AXIS* axis_l_ptr;    //
                double ks;           // N*m/rad
                float (*fnc_Torque)(float theta_c,float theta_l, float ks) ;
                double Ts;           // s
                int VEL_MAX;         // rad/s
                int N;               // 
                double Kp;
                double Ki;
                double Kd;
                double Kv;
                double Bv;
                double TORQUE_MAX;
                bool active;

            }Local;    

            bool esp32_out = false;

        public:

            Local local;
            Vars vars[N_SAMPLES];

            EXO_joint() {
                ZeroMemory(vars, sizeof(Vars) * N_SAMPLES);
                local.VEL_MAX = 7000;
                local.TORQUE_MAX = 5;
                local.active = false;
            }

            void setESP32_out(bool val) {
                esp32_out = val;
            }

            void vars_shift() {
                for (int idx = (N_SAMPLES-1); idx > 0; idx--) {
                    vars[idx] = vars[idx - 1];
                }
            }

            void vars_calculate() {

            }

            void calc_torque(){
                vars[0].torque_d = local.Kv * (vars[0].theta_ld - vars[0].theta_l) + local.Bv * (0 - vars[0].omega_l);
                vars[0].torque_l = local.fnc_Torque(vars[0].theta_c,vars[0].theta_l,local.ks);

                if (vars[0].torque_d >= local.TORQUE_MAX) { vars[0].torque_d = local.TORQUE_MAX; }
                else if (vars[0].torque_d <= -local.TORQUE_MAX) { vars[0].torque_d = -local.TORQUE_MAX; }

            }

            void calc_vel() {
                double erro_0 = vars[0].torque_d - vars[0].torque_l;
                double erro_1 = vars[1].torque_d - vars[1].torque_l;
                double erro_2 = vars[2].torque_d - vars[2].torque_l;

                vars[0].omega_m = vars[1].omega_m + local.Kp * (erro_0 - erro_1)
                    + local.Ki * local.Ts * erro_0
                    + (local.Kd / local.Ts) * (erro_0 - 2.0 * erro_1 + erro_2);


            }

            void setOrigin() {
                local.ZERO_M = 0;
                local.ZERO_L = 0;
                local.ZERO_C = 0;
                updatePosEncoder();
                local.ZERO_M = vars[0].theta_m;
                local.ZERO_L = vars[0].theta_l;
                local.ZERO_C = vars[0].theta_c;
                std::cout << "\n\n Fron MAarkov : ZERO_M : " << local.ZERO_M << "     ZERO_L :   " << local.ZERO_L << "\n\n";
            }

            void updatePosEncoder() {
                //std::cout << "\nUpdate . . .";
                long raw_angle_m;
                long raw_angle_l;
                ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->getPosition(local.axis_m_ptr, &raw_angle_m);
                if(!esp32_out)((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->getPosition(local.axis_l_ptr, &raw_angle_l);

                setPosEncoder_m(raw_angle_m);
                if (!esp32_out)setPosEncoder_l(raw_angle_l);
            }

            void setPosEncoder_m(long raw_angle) {
                //raw_angle -= ZERO_M;
                vars[0].theta_c = (raw_angle * 2.0 * arma::datum::pi) / (local.encoder_in_Q * local.N);
                vars[0].theta_m = (raw_angle * 2.0 * arma::datum::pi) / (local.encoder_in_Q);

                vars[0].theta_c -= local.ZERO_C;
                vars[0].theta_m -= local.ZERO_M;

            }

            void setPosEncoder_l(long raw_angle) {
                //raw_angle -= ZERO_L;
                vars[0].theta_l = (-raw_angle * 2.0 * arma::datum::pi) / (local.encoder_out_Q);
                vars[0].theta_l -= local.ZERO_L;
            }
            void setPosEncoder_l(float raw_angle) {
                //raw_angle -= ZERO_L;
                vars[0].theta_l = -raw_angle;
                vars[0].theta_l -= local.ZERO_L;
            }
            void setVelocity() {
                if (vars[0].omega_m >= local.VEL_MAX) { vars[0].omega_m = local.VEL_MAX; }
                else if (vars[0].omega_m <= -local.VEL_MAX) { vars[0].omega_m = -local.VEL_MAX; }
                ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->setVelocity(local.axis_m_ptr, (long)vars[0].omega_m);
            }
            void setVelocity(long val) {
                ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->setVelocity(local.axis_m_ptr,val);
            }

            static float T_quadraticSpringBooker(float theta_c, float theta_l, float ks = 0.0) {
                float X = theta_c - theta_l;
                X = -X;
                return -(-321500*pow(X,5) +
                        14990 *pow(X,4) + 
                        10620 * pow(X, 3) - 
                        ((1963 * pow(X, 2)) / (10)) +
                        ((7919 * X) / (100)));
                
                if (X > 0) {
                    
                    return (52.19 * X + 907.8 * X * X); // f
                }
                else {
                    X = -X;
                    return -(71.5 * X + 825.8 * X * X);
                         
                }
                
            }

            static float T_linearSpring(float theta_c, float theta_l, float ks =0.0) {
                float X = theta_c - theta_l;
                return ks*X;
            }

        };

        EXO_joint* knee_r;
        EXO_joint * knee_l;
        EXO_joint* hip_r;
        EXO_joint* hip_l;

        ThreadPID_Jonathan(){

            knee_r = new EXO_joint();
            knee_r->local.axis_m_ptr = &((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->servo_knee_right;
            knee_r->local.axis_l_ptr = &((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->encoder_knee_right;

            knee_l = new EXO_joint();
            knee_l->local.axis_m_ptr = &((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->servo_knee_left;
            knee_l->local.axis_l_ptr = &((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->encoder_knee_left;

            
            hip_r = new EXO_joint();
            hip_r->local.axis_m_ptr = &((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->servo_hip_right;
            hip_r->setESP32_out(true);

            hip_l = new EXO_joint();
            hip_l->local.axis_m_ptr = &((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->servo_hip_left;
            hip_l->setESP32_out(true);

        }

        void _setup(){
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->useEPOS[ID_SERVO_HIP_R] = true;
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->useEPOS[ID_SERVO_HIP_L] = true;
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->useEPOS[ID_SERVO_KNEE_L] = true;
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->useEPOS[ID_SERVO_KNEE_R] = true;
            
            //throw "ERROR";
            if (EposEXOCAN1->toRUNfromGUI == false)
                throw "ERROR Markov";

            /*if (ESP32_1->toRUNfromGUI == false)
                throw "ERROR ESP32";*/

            knee_r->local.encoder_in_Q = 4096;
            knee_r->local.encoder_out_Q = 2048;
            knee_r->local.Kp = 380;// 400;//380;
            knee_r->local.Ki = 35;
            knee_r->local.Kd = 3;

            knee_r->local.Kv = 30;// 90;// 120;
            knee_r->local.Bv = 0;

            knee_r->local.Ts = (1.0 / 200.0);
            knee_r->local.VEL_MAX = 5000;
            knee_r->local.ks = 104;
            knee_r->local.fnc_Torque = &EXO_joint::T_linearSpring;
            knee_r->local.N = 150;
            knee_r->local.TORQUE_MAX = 20;





            knee_l->local.encoder_in_Q = 4096;
            knee_l->local.encoder_out_Q = 2048;
            knee_l->local.Kp = 500;
            knee_l->local.Ki = 50;
            knee_l->local.Kd = 1;
            knee_l->local.Kv = 0;
            knee_l->local.Bv = 0;
            knee_l->local.Ts = (1.0 / 200.0);
            knee_l->local.VEL_MAX = 7000;
            knee_l->local.ks = 300;
            knee_l->local.fnc_Torque = &EXO_joint::T_linearSpring;
            knee_l->local.N = 150;
            knee_l->local.TORQUE_MAX = 10;

            
        }

        void _cleanup(){
            // desligar tudo
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->Desabilita_Eixo(0);
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->setVelocityZero();

        }

        void _firstLoop() {
            
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->setVelocityMode(knee_r->local.axis_m_ptr);
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->setVelocityMode(knee_l->local.axis_m_ptr);
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->setVelocityMode(hip_r->local.axis_m_ptr);
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->setVelocityMode(hip_l->local.axis_m_ptr);
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->setVelocityZero();
            ((ThreadEposEXO_CAN*)EposEXOCAN1->threadType_)->Habilita_Eixo(2);
            
            knee_r->setVelocity(0);
            knee_r->setOrigin();

            knee_l->setVelocity(0);
            knee_l->setOrigin();

            hip_r->setVelocity(0);
            hip_r->setOrigin();

            hip_l->setVelocity(0);
            hip_l->setOrigin();
            
        }
        
        void squareShape_test(int * state, float * t_d,float t_d_A) {
            if ((time_index % (4 * 200)) == 0) {
                *t_d = - (*t_d);
                switch (*state) {
                case 1:
                    *t_d = 0;
                    break;
                case 2:
                    *t_d = t_d_A;
                    break;
                case 3:
                    *t_d = 0;
                    break;
                case 4:
                    *t_d = -t_d_A;
                    break;
                default:
                    *state = 1;
                    break;
                }
                *state = (((*state)++) % 4) + 1;
            }
        }
        void sineShape_test(float Amp, float freq, float* t_d) {
            (*t_d) = Amp * sin(2.0 * M_PI * freq * timer->get_current_time_f());
            if ( (time_index + 5*200)>=((int)(T_exec / _Ts)) )(*t_d) = 0.0f;
        }

        void _loop(){
                   
            ESP32_Jonathan->getData(&ESP32FSR);
            // aqui
            knee_r->vars[0].time = timer->get_current_time_f();
            knee_r->updatePosEncoder();

            static float theta_d_kv = 0;
            sineShape_test(0.5, 0.3, &theta_d_kv);
            knee_r->vars[0].theta_ld = theta_d_kv;



            knee_r->calc_torque();

            //static float t_d_k_r = 0; // test2 descomentar
            
            //sineShape_test(7, .5, &t_d_k_r);
            
            //knee_r->vars[0].torque_d = t_d_k_r;
            //knee_r->vars[0].torque_d = 0;
            
            knee_r->calc_vel();
            knee_r->setVelocity(); // test1 
            



            ///////////////////////////////////////////
            // //squareShape_test(&state_k_r,&t_d_k_r,4);
            // 
            // //static int state_k_r = 0;
            // left
            knee_l->updatePosEncoder();
            knee_l->calc_torque();
            static float t_d_k_l = 0;
            static int state_k_l = 0;
            //sineShape_test(5, .3, &t_d_k_l);
            //squareShape_test(&state_k_l,&t_d_k_l,15);
            //knee_l->vars[0].torque_d = t_d_k_l;
            knee_l->calc_vel();
            knee_l->setVelocity();

          

            _mtx.lock();   
            
                // setdata 
            _datalog[time_index][0] = timer->get_current_time_f();
            _datalog[time_index][0 * 10 + 1] = knee_r->vars[0].theta_ld;
            _datalog[time_index][0 * 10 + 2] = knee_r->vars[0].theta_l;
            _datalog[time_index][0 * 10 + 3] = knee_r->vars[0].theta_c;
            _datalog[time_index][0 * 10 + 4] = knee_r->vars[0].torque_d;
            _datalog[time_index][0 * 10 + 5] = knee_r->vars[0].torque_l;
            _datalog[time_index][0 * 10 + 6] = knee_r->vars[0].omega_m;
            _datalog[time_index][0 * 10 + 7] = ESP32FSR.FSR[2];
            _datalog[time_index][0 * 10 + 8] = 0.0f;
            _datalog[time_index][0 * 10 + 9] = 0.0f;
            _datalog[time_index][0 * 10 + 10] = 0.0f;

            _datalog[time_index][1 * 10 + 1] = knee_l->vars[0].theta_m;
            _datalog[time_index][1 * 10 + 2] = knee_l->vars[0].theta_l;
            _datalog[time_index][1 * 10 + 3] = knee_l->vars[0].theta_c;
            _datalog[time_index][1 * 10 + 4] = knee_l->vars[0].torque_d;
            _datalog[time_index][1 * 10 + 5] = knee_l->vars[0].torque_l;
            _datalog[time_index][1 * 10 + 6] = knee_l->vars[0].omega_m;
            _datalog[time_index][1 * 10 + 7] = 0.0f;
            _datalog[time_index][1 * 10 + 8] = 0.0f;
            _datalog[time_index][1 * 10 + 9] = 0.0f;
            _datalog[time_index][1 * 10 + 10] = 0.0f;

            _mtx.unlock();

            knee_r->vars_shift();
            //knee_l->vars_shift();


        }

        void _updateGUI(){
            ImGui::Begin(_name.c_str());
                ImGui::Text("Hello from : PID_4J");
            ImGui::End();
        }

        void getData(void * _data){
            _mtx.lock();
                memcpy(_data,&data,sizeof(fromPID_4J));
            _mtx.unlock();
        }
};
