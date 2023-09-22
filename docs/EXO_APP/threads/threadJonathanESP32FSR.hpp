#pragma once

#include <thread_exo.h>

#include <LowPassFilter2p.h>

#include <utils_plot.hpp>

#include "../libs_exo/UtilsCOM.hpp"

#define  N_SAMPLES_ESP32_FSR 4

typedef struct{
    float FSR[8];
    float phi;
    } fromJonathanESP32FSR;



class threadJonathanESP32FSR : public ThreadType {
        fromJonathanESP32FSR data;
        COM * arduino;
        typedef struct {
            float FSR[8];
            float phi;
        }pkg_1_t;
        pkg_1_t pkg_1 = { 0 };

        typedef struct {
            float FSR[8];
            float phi;
        }VARS;

    //private:
    //    fromXsensLeo dataXsensLeo;

    public:       
        
        VARS vars[N_SAMPLES_ESP32_FSR];

        void vars_shift() {
            for (int idx = (N_SAMPLES_ESP32_FSR - 1); idx > 0; idx--) {
                vars[idx] = vars[idx - 1];
            }
        }

        std::atomic<bool> showGraph;
        std::vector<PlotWindow> pw;

        threadJonathanESP32FSR(){
            arduino = new COM();
            pw.push_back(PlotWindow("Time [ s ]","FSR [ U ]", "ESP32 FSR"));
            showGraph = false;
        }

      
        void _setup(){
            strcpy(arduino->LOCAL.portName, "\\\\.\\COM8"); //COM8
            arduino->LOCAL.Parameter.BaudRate = 115200;
            arduino->LOCAL.Parameter.ByteSize = 8;
            arduino->LOCAL.Parameter.StopBits = ONESTOPBIT;
            arduino->LOCAL.Parameter.Parity = NOPARITY;
            arduino->LOCAL.Parameter.fDtrControl = DTR_CONTROL_ENABLE;
            
            if(arduino->openCOM()<0)throw "ERROR Arduino (Init)";
            pw[0].SCOPE.x_axis.minimum = 0.0f;
            pw[0].SCOPE.x_axis.maximum = T_exec;
            pw[0].clearItems();
            pw[0].addItem("FSR 1");
            pw[0].addItem("FSR 2");
            pw[0].addItem("FSR 3");
            pw[0].addItem("FSR 4");
            pw[0].addItem("FSR 5");
            pw[0].addItem("FSR 6");
            pw[0].addItem("FSR 7");
            pw[0].addItem("FSR 8");
            pw[0].addItem("phi");
            pw[0].SCOPE.y_axis.minimum = -500;
            pw[0].SCOPE.y_axis.maximum = 1000;

        }

        void _cleanup(){
            
            //arduino->sendCOM((void*)"t", 1);
            //Sleep(100);
            arduino->closeCOM();
        }

        void _firstLoop() {            
            Sleep(3000);
            arduino->sendCOM((void*)"p", 1);
            arduino->recvCOM(&pkg_1, sizeof(pkg_1));
            _mtx.lock();
                memcpy(&data, &pkg_1, sizeof(pkg_1));
            _mtx.unlock();
            /*arduino->sendCOM((void*)"r", 1);
            */
            Sleep(100);
        }

        void _loop(){

            //XSENSLeo->getData(&dataXsensLeo);

            arduino->sendCOM((void*)"p", 1);
            arduino->recvCOM(&pkg_1, sizeof(pkg_1));
            
            vars[0].FSR[0] = pkg_1.FSR[0];
            vars[0].FSR[1] = pkg_1.FSR[1];
            vars[0].FSR[2] = pkg_1.FSR[2];
            vars[0].FSR[3] = pkg_1.FSR[3];
            vars[0].FSR[4] = pkg_1.FSR[4];
            vars[0].FSR[5] = pkg_1.FSR[5];
            vars[0].FSR[6] = pkg_1.FSR[6];
            vars[0].FSR[7] = pkg_1.FSR[7];
            vars[0].phi = pkg_1.phi;
            //vars[0].FSR[7] = dataXsensLeo.data[0];
            //vars[0].phi = dataXsensLeo.data[3];
           

            _mtx.lock();
                
                memcpy(&data, &pkg_1, sizeof(pkg_1));

                _datalog[time_index][0] = timer->get_current_time_f();
                _datalog[time_index][1] = pkg_1.FSR[0];
                _datalog[time_index][2] = pkg_1.FSR[1];
                _datalog[time_index][3] = pkg_1.FSR[2];
                _datalog[time_index][4] = pkg_1.FSR[3];
                _datalog[time_index][5] = pkg_1.FSR[4];
                _datalog[time_index][6] = pkg_1.FSR[5];
                _datalog[time_index][7] = pkg_1.FSR[6];
                _datalog[time_index][8] = pkg_1.FSR[7];
                _datalog[time_index][9] = pkg_1.phi;
                //_datalog[time_index][8] = dataXsensLeo.data[0];
                //_datalog[time_index][9] = dataXsensLeo.data[3];
                pw[0].items[0].data.push_back(ImVec2(timer->get_current_time_f(), pkg_1.FSR[0]));
                pw[0].items[1].data.push_back(ImVec2(timer->get_current_time_f(), pkg_1.FSR[1]));
                pw[0].items[2].data.push_back(ImVec2(timer->get_current_time_f(), pkg_1.FSR[2]));
                pw[0].items[3].data.push_back(ImVec2(timer->get_current_time_f(), pkg_1.FSR[3]));
                pw[0].items[4].data.push_back(ImVec2(timer->get_current_time_f(), pkg_1.FSR[4]));
                pw[0].items[5].data.push_back(ImVec2(timer->get_current_time_f(), pkg_1.FSR[5]));
                pw[0].items[6].data.push_back(ImVec2(timer->get_current_time_f(), pkg_1.FSR[6]));
                pw[0].items[7].data.push_back(ImVec2(timer->get_current_time_f(), pkg_1.FSR[7]));
                pw[0].items[8].data.push_back(ImVec2(timer->get_current_time_f(), pkg_1.phi));
                //pw[0].items[7].data.push_back(ImVec2(timer->get_current_time_f(), dataXsensLeo.data[0]));
                //pw[0].items[8].data.push_back(ImVec2(timer->get_current_time_f(), dataXsensLeo.data[3]));
                
            _mtx.unlock();

            vars_shift();

        }

        //bool cbPlot = false;
        bool cbPlot = true;
        void _updateGUI() {
            ImGui::Begin(_name.c_str());
            
            ImGui::Checkbox("Graph", &cbPlot);
            showGraph = cbPlot;
            if (showGraph) {
                for (int idx = 0; idx < pw.size(); idx++) {
                    pw[idx].showNewWindow();
                }
            }

            ImGui::End();
        }


        void getData(void * _data){
            _mtx.lock();
                memcpy(_data,&data,sizeof(fromJonathanESP32FSR));
            _mtx.unlock();
        }
};
