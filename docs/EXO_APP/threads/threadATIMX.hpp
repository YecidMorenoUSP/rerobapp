#pragma once

#include <Utils_Axia80M50.h>

#include <imgui.h>
#include <imgui_plot2.hpp>

#include <thread_exo.h>
#include <string>

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
} fromATIMX;

class ThreadATIMX: public ThreadType{
    private:
        fromATIMX data;

    public:       
        
        ThreadATIMX(){
            _loadInterface();
        }

        std::vector<ImGui::PlotItem> items; 
        ImGui::PlotInterface SCOPE;
        std::mutex _mtx_gui;

        
        void _loadInterface(){
            
            SCOPE.y_axis.minimum = -50;
            SCOPE.y_axis.maximum = 50;
            SCOPE.x_axis.minimum = 0;
            SCOPE.x_axis.maximum = 60;
            SCOPE.x_axis.label = std::string("Time [ s ]");
            SCOPE.y_axis.label = std::string("Amplitude [ u ]");
            SCOPE.title = std::string("Axia80M50 Sensor Data");
            
            ImGui::PlotItem Fx;
            Fx.label = "Fx [ N ] ";
            Fx.type = (ImGui::PlotItem::Type) ImGui::PlotItem::Line; 
            // Fx.color = ImVec4(0.06f, 0.59f, 0.87f, 1.00f);
            Fx.size = 1;
            items.push_back(Fx);

            ImGui::PlotItem Fy;
            Fy.label = "Fy [ N ] ";
            Fy.type = (ImGui::PlotItem::Type) ImGui::PlotItem::Line; 
            // Fy.color = ImVec4(0.06f, 0.59f, 0.87f, 1.00f);
            Fy.size = 1;
            items.push_back(Fy);

            ImGui::PlotItem Fz;
            Fz.label = "Fz [ N ] ";
            Fz.type = (ImGui::PlotItem::Type) ImGui::PlotItem::Line; 
            // Fz.color = ImVec4(0.06f, 0.59f, 0.87f, 1.00f);
            Fz.size = 1;
            items.push_back(Fz);

            ImGui::PlotItem Tx;
            Tx.label = "Tx [ N*m ] ";
            Tx.type = (ImGui::PlotItem::Type) ImGui::PlotItem::Line; 
            // Tx.color = ImVec4(0.06f, 0.59f, 0.87f, 1.00f);
            Tx.size = 1;
            items.push_back(Tx);

            ImGui::PlotItem Ty;
            Ty.label = "Ty [ N*m ] ";
            Ty.type = (ImGui::PlotItem::Type) ImGui::PlotItem::Line; 
            // Ty.color = ImVec4(0.06f, 0.59f, 0.87f, 1.00f);
            Ty.size = 1;
            items.push_back(Ty);

            ImGui::PlotItem Tz;
            Tz.label = "Tz [ N*m ] ";
            Tz.type = (ImGui::PlotItem::Type) ImGui::PlotItem::Line; 
            // Tz.color = ImVec4(0.06f, 0.59f, 0.87f, 1.00f);
            Tz.size = 1;
            items.push_back(Tz);
        }

        Axia80M50 * sensor;
        char _IP[64] = "192.168.0.169";
        

        void _setup(){
            //throw "ERROR";
            std::string ip = _IP;
            sensor = new Axia80M50((char *)ip.c_str());
            if(!sensor->init()) throw "ERROR Axia80M50";
            sensor->bias();
            for(int i = 0 ; i < 6 ; i++){
						items[i].data.clear();
			}

            srand(time(0));
            _mtx_gui.lock();
                SCOPE.y_axis.minimum = -20.0f;
                SCOPE.y_axis.maximum = 20.0f;
                SCOPE.x_axis.minimum = 0.0f;
                SCOPE.x_axis.maximum = T_exec;
            _mtx_gui.unlock();
        }

        void _cleanup(){
            
        }
        ValuesAxia80M50 values;
        void _loop(){
            
            sensor->peek();

            _mtx.lock();   
                values = sensor->values;
                memcpy(&data,&values,sizeof(values));
                _datalog[time_index][0] = timer->get_current_time_f();
                _datalog[time_index][1] = data.Fx;
                _datalog[time_index][2] = data.Fy;
                _datalog[time_index][3] = data.Fz;
                _datalog[time_index][4] = data.Tx;
                _datalog[time_index][5] = data.Ty;
                _datalog[time_index][6] = data.Tz;
                items[0].data.push_back( ImVec2( (float)_datalog[time_index][0] , data.Fx ) ); 
                items[1].data.push_back( ImVec2( (float)_datalog[time_index][0] , data.Fy ) ); 
                items[2].data.push_back( ImVec2( (float)_datalog[time_index][0] , data.Fz ) ); 
                items[3].data.push_back( ImVec2( (float)_datalog[time_index][0] , data.Tx ) ); 
                items[4].data.push_back( ImVec2( (float)_datalog[time_index][0] , data.Ty ) ); 
                items[5].data.push_back( ImVec2( (float)_datalog[time_index][0] , data.Tz ) );  
            _mtx.unlock();
        }

        //bool showGraph = false;
        bool showGraph = true;
        void _updateGUI(){
            ImGui::Begin(_name.c_str());
                if(isAlive)
                    ImGui::Text("ATI : RUNNING");
                else
                    ImGui::Text("ATI : PAUSED");

                
                ImGui::InputText("IP##ATI", _IP, 64);
                ImGui::SameLine(); if(ImGui::Button("TEST##ATI")){}

                ImGui::SameLine(); if(ImGui::Button("BIAS##ATI")){}

                ImGui::SameLine(); ImGui::Checkbox("GRAPH? ##ATI",&showGraph);

                if(showGraph){
                    _mtx_gui.lock();
                        ImGui::Plot("SCOPE", SCOPE, items); 
                    _mtx_gui.unlock();
                }


            ImGui::End();
        }

 
        void getData(void * _data){
            _mtx.lock();
                memcpy(_data,&data,sizeof(fromATIMX));
            _mtx.unlock();
        }
};
