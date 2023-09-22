#pragma once

#include <imgui.h>

#include <thread_exo.h>
#include <string>

typedef struct{
} fromEXOSquatDavid;

class ThreadEXOSquatDavid: public ThreadType{
    private:
        fromEXOSquatDavid data;

    public:       
        
        ThreadEXOSquatDavid(){
            
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
                ImGui::Text("Hello from : EXOSquatDavid");
                if (ImGui::Button("Click-me")) {
                    std::cout << "Sai daqui...";
                }
            ImGui::End();
        }

        void getData(void * _data){
            _mtx.lock();
                memcpy(_data,&data,sizeof(fromEXOSquatDavid));
            _mtx.unlock();
        }
};
