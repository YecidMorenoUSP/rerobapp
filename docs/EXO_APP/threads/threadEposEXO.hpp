#pragma once

#if !defined(EposCommandLibraryDefinitions)
    #include<Definitions.h>
#endif


#include <imgui.h>
#include <imgui_plot2.hpp>

#include <thread_exo.h>
#include <string>
#include <thread>
#include <mutex>

#include <atomic>

#include <chrono>
using namespace std::chrono;

#define EPOS4Config4 (char*)"EPOS4",(char*)"MAXON SERIAL V2",(char*)"USB",(char*)"USB1"
#define EPOS4Config1 (char*)"EPOS4",(char*)"MAXON SERIAL V2",(char*)"USB",(char*)"USB0"
#define ID_ZERO  0

typedef struct{

    long  getPosm;
    long  getPosl;
    long  setVelm;
 
} fromEposEXO;





class ThreadEposEXO: public ThreadType{
    private:
        

        DWORD errorCode;
        HANDLE hEncoder ;
        HANDLE hActuator;

        long posEncoder_ZERO;
        long posActuator_ZERO;

        fromEposEXO data;

    

        HANDLE init_Encoder(){
            HANDLE epos = VCS_OpenDevice(EPOS4Config4,&errorCode);
            if(VCS_ClearFault(epos,ID_ZERO,&errorCode))
                return epos;
            return NULL;
        }

        HANDLE init_Actuator(){
            HANDLE epos = VCS_OpenDevice(EPOS4Config1,&errorCode);
            if(epos!= nullptr){
                if(VCS_ClearFault(epos,ID_ZERO,&errorCode))
                if(VCS_SetProtocolStackSettings(epos,1000000,50,&errorCode)>0)
                if(VCS_ActivateVelocityMode(epos,ID_ZERO,&errorCode))
                if(VCS_SetMaxProfileVelocity(epos,ID_ZERO,5000,&errorCode)){
                    return epos;
                }
            }
            return NULL;              
        }

        void destroy_Connection(HANDLE h){
            VCS_CloseDevice(h,&errorCode);
        }        

    public:       


        std::atomic<bool> Sync;
        std::atomic<bool> AsyncRunning;
        std::atomic<bool> hasError;
        std::thread AsyncEncoder;
        std::thread AsyncActuator;

        
        std::mutex _mtx_sync;
        

        void AsyncEncoder_fnc(){
            long getPosl;
            long posZero;
            long count = 0;

            if(!VCS_GetPositionIs(hEncoder,ID_ZERO,&posZero,&errorCode)){
                    hasError = true;
                    return;
            }

            while (AsyncRunning == true)
            {
                
                if(!VCS_GetPositionIs(hEncoder,ID_ZERO,&getPosl,&errorCode)){
                    hasError = true;
                    Sync = true;
                    break;
                }
                _mtx.lock();
                    data.getPosl = getPosl-posZero;
                _mtx.unlock();

                count++;
                
                Sync = true;
                    while(Sync){}
                Sync = true;
                
            }

            std::cout<<"\nCount Encoder "<<count<<" ";
     
            
        }
        void AsyncActuator_fnc(){
            long getPosm;
            long setVelm;
            long posZero;
            long count = 0;

            if(!VCS_GetPositionIs(hActuator,ID_ZERO,&posZero,&errorCode)){
                 hasError = true;
            }

            VCS_SetState(hActuator,ID_ZERO,ST_ENABLED,&errorCode);

            while (AsyncRunning == true)
            {   
                
                _mtx.lock();
                    setVelm = data.setVelm;
                _mtx.unlock();


                if(!VCS_GetPositionIs(hActuator,ID_ZERO,&getPosm,&errorCode) ||
                    !VCS_SetVelocityMust(hActuator,ID_ZERO,setVelm,&errorCode)){
                       hasError = true;
                        Sync = false;
                        break;
                   }

                _mtx.lock();
                    data.getPosm = getPosm-posZero;
                _mtx.unlock();
                
                count ++;
               Sync = false;
                    while(!Sync){}
                Sync = false;
            }
            
            std::cout<<"\nCount Actuator "<<count<<" ";
            
            VCS_SetState(hActuator,ID_ZERO,ST_DISABLED,&errorCode);
            VCS_SetVelocityMust(hActuator,ID_ZERO,0,&errorCode);

        }
        
        ThreadEposEXO(){
            data = fromEposEXO{0};
        }

 
        void _setup(){
            
            data.setVelm = 0;
            

            AsyncRunning = false;

            hEncoder = init_Encoder();
            hActuator = init_Actuator();

            if(hEncoder != nullptr){GUI.addLOG("\nEncoder OK");}
            else{throw "Epos Enconder Failed";}

            if(hActuator != nullptr ){GUI.addLOG("\nActuator OK");}
            else{throw "Epos Actuator Failed";}

            AsyncRunning = true;
            hasError = false;


            AsyncEncoder  = std::thread( &ThreadEposEXO::AsyncEncoder_fnc , this);
            AsyncActuator = std::thread( &ThreadEposEXO::AsyncActuator_fnc , this);
            
           
        }

        void _cleanup(){
            AsyncRunning = false;
            // join threads
            
            if(AsyncEncoder.joinable())
                AsyncEncoder.join();

            if(AsyncActuator.joinable())
            AsyncActuator.join();

            destroy_Connection(hEncoder);
            destroy_Connection(hActuator);     
        }

        void _loop(){
            if(hasError) throw "EposExo, algun error";
            
            _mtx.lock();   
                _datalog[time_index][0] = timer->get_current_time_f();
                _datalog[time_index][1] = data.setVelm;
                _datalog[time_index][2] = data.getPosm;
                _datalog[time_index][3] = data.getPosl;
            _mtx.unlock();
        }

        bool showGraph = false;
        void _updateGUI(){
            ImGui::Begin(_name.c_str());
                if(isAlive)
                    ImGui::Text("EposEXO : RUNNING");
                else
                    ImGui::Text("EposEXO : PAUSED");
               
                

            ImGui::End();
        }

        void setVelm(long vel){
            _mtx.lock();
                data.setVelm = vel;
            _mtx.unlock();
        }
    
        void getData(void * _data){
            _mtx.lock();
                memcpy(_data,&data,sizeof(fromEposEXO));
            _mtx.unlock();
        }
};
