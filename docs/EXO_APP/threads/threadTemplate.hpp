#pragma once

#include <imgui.h>

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
} from__NAME__;

class Thread__NAME__: public ThreadType{
    private:
        from__NAME__ data;

    public:       
        
        Thread__NAME__(){
            
        }

        void _setup(){
            //throw "ERROR";
        }

        void _cleanup(){
            // desligar todo
        }

        void _loop(){
            
            // for(int i = 0 ; i < 10 ; i++)
            // printf("\t ATI: %d" , i);
            
            _mtx.lock();   
                // setdata 
            _mtx.unlock();
        }

        void _updateGUI(){
            ImGui::Begin(_name.c_str());
                ImGui::Text("Hello from : __NAME__");
            ImGui::End();
        }

        void getData(void * _data){
            _mtx.lock();
                memcpy(_data,&data,sizeof(from__NAME__));
            _mtx.unlock();
        }
};
