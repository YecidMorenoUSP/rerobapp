#pragma once

#include <thread_exo.h>

#include <LowPassFilter2p.h>

#include <utils_plot.hpp>

#include "../libs_exo/UtilsCOM.hpp"


typedef struct{
    float knee_r;
    float knee_l;
    float hip_r;
    float hip_l;
} fromESP32;



class ThreadESP32: public ThreadType{
    private:
        fromESP32 data;
        COM * arduino;
        typedef struct {
            float inc_1;
            float inc_2;
            float abs_1;
            float abs_2;
        }pkg_1_t;
        pkg_1_t pkg_1 = { 0 };
    public:       

        std::atomic<bool> showGraph;
        std::vector<PlotWindow> pw;

        ThreadESP32(){
            arduino = new COM();
            pw.push_back(PlotWindow("Time [ s ]","Angle [ Rad ]", "Joint values"));
            showGraph = false;
            //showGraph = true;
        }

      
        void _setup(){
            strcpy(arduino->LOCAL.portName, "\\\\.\\COM8"); //  COM8
            arduino->LOCAL.Parameter.BaudRate = 115200;
            arduino->LOCAL.Parameter.ByteSize = 8;
            arduino->LOCAL.Parameter.StopBits = ONESTOPBIT;
            arduino->LOCAL.Parameter.Parity = NOPARITY;
            arduino->LOCAL.Parameter.fDtrControl = DTR_CONTROL_ENABLE;
            
            if(arduino->openCOM()<0)throw "ERROR Arduino (Init)";
            pw[0].SCOPE.x_axis.minimum = 0.0f;
            pw[0].SCOPE.x_axis.maximum = T_exec;
            pw[0].clearItems();
            pw[0].addItem("inc_1");
            pw[0].addItem("inc_2");
            pw[0].addItem("abs_1");
            pw[0].addItem("abs_2");
            pw[0].SCOPE.y_axis.minimum = -2.0*M_PI;
            pw[0].SCOPE.y_axis.maximum = 2.0 * M_PI;

        }

        void _cleanup(){
            arduino->closeCOM();
        }

        void _firstLoop() {            
            Sleep(3000);
            arduino->sendCOM((void*)"r", 1);
            Sleep(100);
            arduino->sendCOM((void*)"p", 1);
            arduino->recvCOM(&pkg_1, sizeof(pkg_1));
            _mtx.lock();
                memcpy(&data, &pkg_1, sizeof(pkg_1));
            _mtx.unlock();
            Sleep(100);
        }

        void _loop(){
            arduino->sendCOM((void*)"p", 1);
            arduino->recvCOM(&pkg_1, sizeof(pkg_1));

            _mtx.lock();
                
                memcpy(&data, &pkg_1, sizeof(pkg_1));

                _datalog[time_index][0] = timer->get_current_time_f();
                _datalog[time_index][1] = pkg_1.inc_1;
                _datalog[time_index][2] = pkg_1.inc_2;
                _datalog[time_index][3] = pkg_1.abs_1;
                _datalog[time_index][4] = pkg_1.abs_2;
                pw[0].items[0].data.push_back(ImVec2(timer->get_current_time_f(), pkg_1.inc_1));
                pw[0].items[1].data.push_back(ImVec2(timer->get_current_time_f(), pkg_1.inc_2));
                pw[0].items[2].data.push_back(ImVec2(timer->get_current_time_f(), pkg_1.abs_1));
                pw[0].items[3].data.push_back(ImVec2(timer->get_current_time_f(), pkg_1.abs_2));
            _mtx.unlock();

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
                memcpy(_data,&data,sizeof(fromESP32));
            _mtx.unlock();
        }
};
