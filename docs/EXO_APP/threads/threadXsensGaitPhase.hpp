#pragma once

#include <imgui.h>

#include <thread_exo.h>
#include <string>
#include <iomanip>
#include <random>

typedef struct{
    int gait_phase;
} fromXsensGaitPhase;

class ThreadXsensGaitPhase: public ThreadType{
    private:
        fromXsensGaitPhase data;

    public:       
        
        ThreadXsensGaitPhase(){
            
        }

        int delay = 0;
        int S = 0;
        int mode = 0;
        double prom = 0, prom_1 = 0;
        double th_mode_1 = 0;
        int count;
        arma::mat data_10;
        std::atomic<bool> isRandom;
        bool random = false;

        void _setup(){

            if (XsensRead->toRUNfromGUI == false && isRandom == false)
                throw std::runtime_error("Precisa iniciar o thread XsensRead");

            ((ThreadXsensRead*)XsensRead->threadType_)->sensorsSxens[2].required = true;
            S = 5;
            mode = S;
            data_10.zeros(10, 1);
            prom = 0;
            prom_1 = 0;
            th_mode_1 = 0;
            count = 0;
            isRandom = random;
            data.gait_phase = 5;
        }
        
        int gen_number(int m , int M)
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(m, M);

            return dis(gen);
        }

        void _cleanup(){
            // desligar todo
            std::cout << "\n\nTotal de pasos dados  : " << count;
        }

        fromXsensRead imu_shared_data;
        float imus_data[N_DATA_IMU];
        void _loop(){
                
            using namespace arma;

            XsensRead->getData(&imu_shared_data);
            memcpy(imus_data, imu_shared_data.imus_data, sizeof(imus_data));

            data_10(time_index % 10, 0) = -imus_data[12];
            prom = arma::as_scalar(arma::mean(arma::mean(data_10)));


            if (delay > 0) {       
                delay--;
            }
            else {

                switch (S) {
                case 5:
                    if ((prom > -1 && prom < -0.40f) && (prom_1 > -1) && (prom_1 < -0.40f) && (prom_1 < prom)) {
                        th_mode_1 = prom;
                        S = 10;
                    }
                    break;

                case 10:
                    if ((prom > -1 && prom < -0.40f) && (prom_1 > -1) && (prom_1 < -0.40f) && (prom_1 > prom)) {
                        S = 1;
                        count++;
                    }
                    break;

                case 1:
                    if ((prom<-1 && prom > -4) && (prom_1<-1 && prom_1 > -4) && (prom > prom_1)) {
                        S = 2;
                    }
                    break;

                case 2:
                    if ((prom_1 <= 0) && (prom >= 0)) {
                        S = 3;
                    }
                    break;

                case 3:
                    if ((prom > 1 && prom < 5) && (prom_1 > 1 && prom_1 < 4) && (prom_1 > prom)) {
                        S = 4;
                    }
                    break;

                case 4:
                    if ((prom_1 >= 0) && (prom <= 0)) {
                        S = 5;
                        delay = 50;
                    }
                    break;

                }
            }          
            
            
            if (!isRandom) {
                if (S >= 1 && S <= 5) mode = S;
                data.gait_phase = mode;
            }                
            else {
                if (delay == 0) {
                    mode = gen_number(1, 5);
                    data.gait_phase = mode;
                    delay = gen_number(50,200);
                }
                
            }
                
            prom_1 = prom;

            _mtx.lock();   
                _datalog[time_index][0] = timer->get_current_time_f();
                _datalog[time_index][1] = mode;
                _datalog[time_index][2] = S;
                _datalog[time_index][3] = prom;
            _mtx.unlock();
        }
        
        void _updateGUI(){
            ImGui::Begin(_name.c_str());
                ImGui::Text("Hello from : XsensGaitPhase");
                ImGui::Checkbox("Random ", &random);
                isRandom = random;
            ImGui::End();
        }

        void getData(void * _data){
            _mtx.lock();
                memcpy(_data,&data,sizeof(fromXsensGaitPhase));
            _mtx.unlock();
        }
};
