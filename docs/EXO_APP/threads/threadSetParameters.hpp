#pragma once

#include <thread_exo.h>

#include <LowPassFilter2p.h>

#include <utils_plot.hpp>







class ThreadSetParameters : public ThreadType{
    private:
        fromSetParameters data;
        fromPID_4J dataPID_4J;
        
    public:       

        std::atomic<bool> showGraph;
        std::atomic<float> Freq;
        std::atomic<float> Amp;
        std::atomic<float> Off; 
        std::atomic<float> Phase;
        std::atomic<bool> Inv;
        std::vector<PlotWindow> pw;

        ThreadSetParameters(){
         
            pw.push_back(PlotWindow("Time [ s ]","Angle [ Rad ]", "Joint values"));
            showGraph = false;
        }

      
        void _setup(){
           
            pw[0].SCOPE.x_axis.minimum = 0.0f;
            pw[0].SCOPE.x_axis.maximum = T_exec;
            pw[0].clearItems();
            pw[0].addItem("ref");
            pw[0].addItem("position");
            pw[0].SCOPE.y_axis.minimum = -M_PI/2.0;
            pw[0].SCOPE.y_axis.maximum = M_PI/2.0;
            pw[0].items[0].size = 4;
            pw[0].items[1].size = 2;
            pw[0].items[0].color = ImVec4(1.00000000000, 0.92156862745, 0.23137254902,1.0);
            pw[0].items[1].color = ImVec4(1.00000000000, 1.00000000000, 1.00000000000,1.0);

            _mtx.lock();
                data.Kv = ui_kv;
                data.Bv = ui_bv;
            _mtx.unlock();


        }

        void _cleanup(){
           
        }

        void _firstLoop() {            
           
        }
        float line1 = 0;
        double t0 = 0;
        void _loop(){
            if (time_index == 0) {
                t0 = timer->get_current_time_f();
            }


            PID_4J->getData(&dataPID_4J);

            line1 = Off + Amp*(-cos(Phase*2.0 * M_PI + 2.0 * M_PI * Freq * (timer->get_current_time_f() - t0))+1)/2.0;
            _mtx.lock();
            data.line1 = line1;
            if (Inv)pw[0].items[0].data.push_back(ImVec2(timer->get_current_time_f(), -line1));
            else pw[0].items[0].data.push_back(ImVec2(timer->get_current_time_f(), line1));
            pw[0].items[1].data.push_back(ImVec2(timer->get_current_time_f(), dataPID_4J.angle_knee_r));
                _datalog[time_index][0] = timer->get_current_time_f();
                _datalog[time_index][1] = line1;
                _datalog[time_index][2] = dataPID_4J.angle_knee_r;
            _mtx.unlock();

        }

        bool cbPlot = false;
        float ui_freq = 0.1;
        float ui_amp = -M_PI/2.0;
        float ui_kv = 0;
        float ui_bv = 0;
        float ui_off = 0;
        float ui_phase = 0;
        bool  ui_inv = false;
        void _updateGUI() {
            ImGui::Begin(_name.c_str());
            
            ImGui::Checkbox("Graph_parameters", &cbPlot);
            ImGui::InputFloat("Freq#parameters", &ui_freq, 0.1, 1, "%.2f", 0);
            ImGui::InputFloat("Amp#parameters",&ui_amp,0.1,1,"%.2f",0);
            ImGui::InputFloat("Off#parameters", &ui_off, 0.1, 1, "%.2f", 0);
            ImGui::InputFloat("Phase#parameters", &ui_phase, 0.1, 1, "%.2f", 0);
            ImGui::Checkbox("Invert#parameters", &ui_inv);
            ImGui::InputFloat("Kv#parameters", &ui_kv, 0.1, 1, "%.2f", 0);
            ImGui::InputFloat("Bv#parameters", &ui_bv, 0.1, 1, "%.2f", 0);
            

            showGraph = cbPlot;
            Freq = ui_freq;
            Amp = ui_amp;
            Inv = ui_inv;
            Off = ui_off;
            Phase = ui_phase;

            if (showGraph) {
                for (int idx = 0; idx < pw.size(); idx++) {
                    pw[idx].showNewWindow();
                }
            }

            ImGui::End();
        }


        void getData(void * _data){
            _mtx.lock();
                memcpy(_data,&data,sizeof(fromSetParameters));
            _mtx.unlock();
        }
};
