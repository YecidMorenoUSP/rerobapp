#pragma once


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#include <AXIS.h>
#include <EPOS_NETWORK.h>
#pragma GCC diagnostic pop

#include <imgui.h>

#include <thread_exo.h>
#include <string>
#include <thread>
#include <mutex>

#include <atomic>

#include <chrono>

#define isActiveEpos(A) if(useEPOS[A] == true)

using namespace std::chrono;

#define ID_ZERO  0
#define ID_SERVO_KNEE_R  1
#define ID_SENSOR_KNEE_R 2
#define ID_SERVO_KNEE_L  3
#define ID_SERVO_HIP_R   4
#define ID_SERVO_HIP_L   5
#define ID_SENSOR_KNEE_L 6
#define tochar(A) #A
#define STR_VALUE(arg)      #arg
#define TO_CHAR(name) STR_VALUE(name)

typedef struct{

    long  getPosm;
    long  getPosl;
    long  setVelm;
 
} fromEposEXO_CAN;
    
class ThreadEposEXO_CAN: public ThreadType{
    private:
        
        fromEposEXO_CAN data;

    public:       

        std::atomic<bool> _running_aux;
        std::thread _aux_thread;

        bool useEPOS[10] = { 0 };

        char *CAN_INTERFACE = (char *)"CAN1";
        char *CAN_DATABASE = (char *)"database";
        char *CAN_CLUSTER = (char *)"NETCAN";

        char* NET_ID_SERVO_KNEE_R  = (char*)TO_CHAR(ID_SERVO_KNEE_R);
        char* NET_ID_SENSOR_KNEE_R = (char*)TO_CHAR(ID_SENSOR_KNEE_R);
        char* NET_ID_SERVO_KNEE_L  = (char*)TO_CHAR(ID_SERVO_KNEE_L);
        char* NET_ID_SENSOR_KNEE_L = (char*)TO_CHAR(ID_SENSOR_KNEE_L);
        char* NET_ID_SERVO_HIP_R   = (char*)TO_CHAR(ID_SERVO_HIP_R);
        char* NET_ID_SERVO_HIP_L   = (char*)TO_CHAR(ID_SERVO_HIP_L);

        EPOS_NETWORK epos;

        AXIS servo_knee_right   ;
        AXIS encoder_knee_right ;
        AXIS servo_knee_left    ;
        AXIS encoder_knee_left  ;
        AXIS servo_hip_right    ;
        AXIS servo_hip_left     ;        
        
        int ZERO_SENSOR_KNEE_right = 0;
        int ZERO_SERVO_KNEE_right = 0;

        int ZERO_SENSOR_KNEE_left = 0;
        int ZERO_SERVO_KNEE_left = 0;

        int ZERO_SENSOR_HIP_right = 0;
        int ZERO_SERVO_HIP_right = 0;

        int ZERO_SENSOR_HIP_left = 0;
        int ZERO_SERVO_HIP_left = 0;

        long endwait;

        void start_transmissao_rede_epos()
        {
            for (int k = 1; k <= 6; k++)
                for (int i = 1; i <= 6; i++)
                    isActiveEpos(i) epos.StartPDOS(i);  
        }

        void init_comm_eixos()
        {
            _running_aux = true;
            loop_timers lt(0.2f);

            for (int i = 0; i < 10; i++)
            {

                lt.start_timer();

                epos.sync();
                //Sincroniza as epos
                
                isActiveEpos(ID_SERVO_KNEE_R) {
                    encoder_knee_right.ReadPDO01();
                    servo_knee_right.ReadPDO01();
                }
                isActiveEpos(ID_SERVO_KNEE_L) {
                    encoder_knee_left.ReadPDO01();
                    servo_knee_left.ReadPDO01();
                }
                
                isActiveEpos(ID_SERVO_HIP_R) {
                    servo_hip_right.ReadPDO01();
                }

                isActiveEpos(ID_SERVO_HIP_L) {
                    servo_hip_left.ReadPDO01();
                }

                printf(".");
                lt.wait_final_time();
            }
            _running_aux = false;
        }

        void Habilita_Eixo(int ID)
        {
            _running_aux = true;
            if ((ID == 2) | (ID == 0))
            {
                isActiveEpos(ID_SERVO_HIP_R) {
                    servo_hip_right.PDOsetControlWord_SwitchOn(false);
                    servo_hip_right.PDOsetControlWord_EnableVoltage(true);
                    servo_hip_right.PDOsetControlWord_QuickStop(true);
                    servo_hip_right.PDOsetControlWord_EnableOperation(false);
                    servo_hip_right.WritePDO01();
                }
                isActiveEpos(ID_SERVO_HIP_L) {
                    servo_hip_left.PDOsetControlWord_SwitchOn(false);
                    servo_hip_left.PDOsetControlWord_EnableVoltage(true);
                    servo_hip_left.PDOsetControlWord_QuickStop(true);
                    servo_hip_left.PDOsetControlWord_EnableOperation(false);
                    servo_hip_left.WritePDO01();
                }
                isActiveEpos(ID_SERVO_KNEE_R) {
                    servo_knee_right.PDOsetControlWord_SwitchOn(false);
                    servo_knee_right.PDOsetControlWord_EnableVoltage(true);
                    servo_knee_right.PDOsetControlWord_QuickStop(true);
                    servo_knee_right.PDOsetControlWord_EnableOperation(false);
                    servo_knee_right.WritePDO01();
                }
                isActiveEpos(ID_SERVO_KNEE_L) {
                    servo_knee_left.PDOsetControlWord_SwitchOn(false);
                    servo_knee_left.PDOsetControlWord_EnableVoltage(true);
                    servo_knee_left.PDOsetControlWord_QuickStop(true);
                    servo_knee_left.PDOsetControlWord_EnableOperation(false);
                    servo_knee_left.WritePDO01();
                }

                printf("\nENERGIZANDO O MOTOR 2 E HABILITANDO O CONTROLE");

                endwait = clock() + 0.5 * CLOCKS_PER_SEC;
                while (clock() < endwait)
                {
                }

                isActiveEpos(ID_SERVO_HIP_R) {
                    servo_hip_right.PDOsetControlWord_SwitchOn(true);
                    servo_hip_right.PDOsetControlWord_EnableVoltage(true);
                    servo_hip_right.PDOsetControlWord_QuickStop(true);
                    servo_hip_right.PDOsetControlWord_EnableOperation(false);
                    servo_hip_right.WritePDO01();
                }
                isActiveEpos(ID_SERVO_HIP_L) {
                    servo_hip_left.PDOsetControlWord_SwitchOn(true);
                    servo_hip_left.PDOsetControlWord_EnableVoltage(true);
                    servo_hip_left.PDOsetControlWord_QuickStop(true);
                    servo_hip_left.PDOsetControlWord_EnableOperation(false);
                    servo_hip_left.WritePDO01();
                }
                
                isActiveEpos(ID_SERVO_KNEE_R) {
                    servo_knee_right.PDOsetControlWord_SwitchOn(true);
                    servo_knee_right.PDOsetControlWord_EnableVoltage(true);
                    servo_knee_right.PDOsetControlWord_QuickStop(true);
                    servo_knee_right.PDOsetControlWord_EnableOperation(false);
                    servo_knee_right.WritePDO01();
                }

                isActiveEpos(ID_SERVO_KNEE_L) {
                    servo_knee_left.PDOsetControlWord_SwitchOn(true);
                    servo_knee_left.PDOsetControlWord_EnableVoltage(true);
                    servo_knee_left.PDOsetControlWord_QuickStop(true);
                    servo_knee_left.PDOsetControlWord_EnableOperation(false);
                    servo_knee_left.WritePDO01();
                }

                endwait = clock() + 0.5 * CLOCKS_PER_SEC;
                while (clock() < endwait)
                {
                }
                isActiveEpos(ID_SERVO_HIP_R) {
                    servo_hip_right.PDOsetControlWord_SwitchOn(true);
                    servo_hip_right.PDOsetControlWord_EnableVoltage(true);
                    servo_hip_right.PDOsetControlWord_QuickStop(true);
                    servo_hip_right.PDOsetControlWord_EnableOperation(true);
                    servo_hip_right.WritePDO01();
                }

                isActiveEpos(ID_SERVO_HIP_L) {
                    servo_hip_left.PDOsetControlWord_SwitchOn(true);
                    servo_hip_left.PDOsetControlWord_EnableVoltage(true);
                    servo_hip_left.PDOsetControlWord_QuickStop(true);
                    servo_hip_left.PDOsetControlWord_EnableOperation(true);
                    servo_hip_left.WritePDO01();
                }

                isActiveEpos(ID_SERVO_KNEE_R) {
                    servo_knee_right.PDOsetControlWord_SwitchOn(true);
                    servo_knee_right.PDOsetControlWord_EnableVoltage(true);
                    servo_knee_right.PDOsetControlWord_QuickStop(true);
                    servo_knee_right.PDOsetControlWord_EnableOperation(true);
                    servo_knee_right.WritePDO01();
                }

                isActiveEpos(ID_SERVO_KNEE_L) {
                    servo_knee_left.PDOsetControlWord_SwitchOn(true);
                    servo_knee_left.PDOsetControlWord_EnableVoltage(true);
                    servo_knee_left.PDOsetControlWord_QuickStop(true);
                    servo_knee_left.PDOsetControlWord_EnableOperation(true);
                    servo_knee_left.WritePDO01();
                }

            }
            _running_aux = false;
        }

        void Desabilita_Eixo(int ID)
        {
            _running_aux = true;
            if ((ID == 2) | (ID == 0))
            {
                printf("\nDESABILITANDO O MOTOR E CONTROLE");

                isActiveEpos(ID_SERVO_HIP_R) {
                    servo_hip_right.PDOsetControlWord_SwitchOn(true);
                    servo_hip_right.PDOsetControlWord_EnableVoltage(true);
                    servo_hip_right.PDOsetControlWord_QuickStop(true);
                    servo_hip_right.PDOsetControlWord_EnableOperation(false);
                    servo_hip_right.WritePDO01();
                }
                
                isActiveEpos(ID_SERVO_HIP_L) {
                    servo_hip_left.PDOsetControlWord_SwitchOn(true);
                    servo_hip_left.PDOsetControlWord_EnableVoltage(true);
                    servo_hip_left.PDOsetControlWord_QuickStop(true);
                    servo_hip_left.PDOsetControlWord_EnableOperation(false);
                    servo_hip_left.WritePDO01();
                }

                isActiveEpos(ID_SERVO_KNEE_R) {
                    servo_knee_right.PDOsetControlWord_SwitchOn(true);
                    servo_knee_right.PDOsetControlWord_EnableVoltage(true);
                    servo_knee_right.PDOsetControlWord_QuickStop(true);
                    servo_knee_right.PDOsetControlWord_EnableOperation(false);
                    servo_knee_right.WritePDO01();
                }

                isActiveEpos(ID_SERVO_KNEE_L) {
                    servo_knee_left.PDOsetControlWord_SwitchOn(true);
                    servo_knee_left.PDOsetControlWord_EnableVoltage(true);
                    servo_knee_left.PDOsetControlWord_QuickStop(true);
                    servo_knee_left.PDOsetControlWord_EnableOperation(false);
                    servo_knee_left.WritePDO01();
                }

                endwait = clock() + 0.5 * CLOCKS_PER_SEC;
                while (clock() < endwait)
                {
                }

                isActiveEpos(ID_SERVO_HIP_R) {
                    servo_hip_right.PDOsetControlWord_SwitchOn(false);
                    servo_hip_right.PDOsetControlWord_EnableVoltage(true);
                    servo_hip_right.PDOsetControlWord_QuickStop(true);
                    servo_hip_right.PDOsetControlWord_EnableOperation(false);
                    servo_hip_right.WritePDO01();
                }

                isActiveEpos(ID_SERVO_HIP_L) {
                    servo_hip_left.PDOsetControlWord_SwitchOn(false);
                    servo_hip_left.PDOsetControlWord_EnableVoltage(true);
                    servo_hip_left.PDOsetControlWord_QuickStop(true);
                    servo_hip_left.PDOsetControlWord_EnableOperation(false);
                    servo_hip_left.WritePDO01();
                }
                isActiveEpos(ID_SERVO_KNEE_R) {
                    servo_knee_right.PDOsetControlWord_SwitchOn(false);
                    servo_knee_right.PDOsetControlWord_EnableVoltage(true);
                    servo_knee_right.PDOsetControlWord_QuickStop(true);
                    servo_knee_right.PDOsetControlWord_EnableOperation(false);
                    servo_knee_right.WritePDO01();
                }
                
                isActiveEpos(ID_SERVO_KNEE_L) {
                    servo_knee_left.PDOsetControlWord_SwitchOn(false);
                    servo_knee_left.PDOsetControlWord_EnableVoltage(true);
                    servo_knee_left.PDOsetControlWord_QuickStop(true);
                    servo_knee_left.PDOsetControlWord_EnableOperation(false);
                    servo_knee_left.WritePDO01();
                }

            }
            _running_aux = false;
        }

        void setVelocityZero() {
            isActiveEpos(ID_SERVO_KNEE_R) {
                servo_knee_right.PDOsetVelocitySetpoint(0);
                servo_knee_right.WritePDO02();
            }
            isActiveEpos(ID_SERVO_KNEE_L) {
                servo_knee_left.PDOsetVelocitySetpoint(0);
                servo_knee_left.WritePDO02();
            }

            isActiveEpos(ID_SERVO_HIP_R) {
                servo_hip_right.PDOsetVelocitySetpoint(0);
                servo_hip_right.WritePDO02();
            }
            isActiveEpos(ID_SERVO_HIP_L) {
                servo_hip_left.PDOsetVelocitySetpoint(0);
                servo_hip_left.WritePDO02();
            }
        }

        void reset_falhas()
        {
            _running_aux = true;
            //EPOS 04
            loop_timers lt(1);
            
            for (int i = 0; i < 2; i++) {
                lt.start_timer();
                printf("\nResetando as falhas. ");

                isActiveEpos(ID_SERVO_HIP_R) {
                    servo_hip_right.PDOsetControlWord_FaultReset(i == 0);
                    servo_hip_right.WritePDO01();
                    printf(" %d ",i);
                }


                isActiveEpos(ID_SERVO_HIP_L) {
                    servo_hip_left.PDOsetControlWord_FaultReset(i == 0);
                    servo_hip_left.WritePDO01();
                    printf(" %d ", i);
                }


                isActiveEpos(ID_SERVO_KNEE_R) {
                    servo_knee_right.PDOsetControlWord_FaultReset(i == 0);
                    servo_knee_right.WritePDO01();
                    printf(" %d ", i);
                }

                isActiveEpos(ID_SERVO_KNEE_R) {
                    encoder_knee_right.PDOsetControlWord_FaultReset(i == 0);
                    encoder_knee_right.WritePDO01();
                    printf(" %d ", i);
                }


                isActiveEpos(ID_SERVO_KNEE_L) {
                    servo_knee_left.PDOsetControlWord_FaultReset(i == 0);
                    servo_knee_left.WritePDO01();
                    printf(" %d ", i);
                }

                isActiveEpos(ID_SERVO_KNEE_L) {
                    encoder_knee_left.PDOsetControlWord_FaultReset(i == 0);
                    encoder_knee_left.WritePDO01();
                    printf(" %d ", i);
                }

                lt.wait_final_time();
            }

            printf("OK");
            _running_aux = false;
        }

        void define_origen()
        {
            _running_aux = true;
            int total_time = 0;
           
            epos.sync();

            printf("Definindo Origem... ");

            esperar_n_seg(1);

            printf("...");
            isActiveEpos(ID_SERVO_HIP_R) {
                servo_hip_right.ReadPDO01();
                ZERO_SERVO_HIP_right = servo_hip_right.PDOgetActualPosition();
                std::cout << "\n ZERO_SERVO_HIP_right : " << ZERO_SERVO_HIP_right;
            }

            isActiveEpos(ID_SERVO_HIP_L) {
                servo_hip_left.ReadPDO01();
                ZERO_SERVO_HIP_left = servo_hip_left.PDOgetActualPosition();
                std::cout << "\n ZERO_SERVO_HIP_left : " << ZERO_SERVO_HIP_left;
            }

            isActiveEpos(ID_SERVO_KNEE_R) {
                encoder_knee_right.ReadPDO01();
                ZERO_SENSOR_KNEE_right = -encoder_knee_right.PDOgetActualPosition();
                std::cout << "\n ZERO_SENSOR_KNEE_right : " << ZERO_SENSOR_KNEE_right;
            }

            isActiveEpos(ID_SERVO_KNEE_R) {
                servo_knee_right.ReadPDO01();
                ZERO_SERVO_KNEE_right = servo_knee_right.PDOgetActualPosition();
                std::cout << "\n ZERO_SERVO_KNEE_right : " << ZERO_SERVO_KNEE_right;
            }

            isActiveEpos(ID_SERVO_KNEE_L) {
                encoder_knee_left.ReadPDO01();
                ZERO_SENSOR_KNEE_left = -encoder_knee_left.PDOgetActualPosition();
                std::cout << "\n ZERO_SENSOR_KNEE_left : " << ZERO_SENSOR_KNEE_left;
            }
            isActiveEpos(ID_SERVO_KNEE_L) {
                servo_knee_left.ReadPDO01();
                ZERO_SERVO_KNEE_left = servo_knee_left.PDOgetActualPosition();
                std::cout << "\n ZERO_SERVO_KNEE_left : " << ZERO_SERVO_KNEE_left;
            }
           

            _running_aux = false;
        }


        ThreadEposEXO_CAN(){
            
            //init_comm_eixos();
        }

        bool init_comunication = false;

        void initAllComunications() {
            _running_aux = true;
            if (!init_comunication) {

                data = fromEposEXO_CAN{ 0 };
                std::cout << "INICIALIZANDO COMUNICACAO CANOpen COM AS EPOS" << std::endl;
                epos = EPOS_NETWORK(CAN_INTERFACE, CAN_DATABASE, CAN_CLUSTER);
                
                servo_knee_right = AXIS(CAN_INTERFACE, CAN_DATABASE, CAN_CLUSTER, NET_ID_SERVO_KNEE_R);
                encoder_knee_right = AXIS(CAN_INTERFACE, CAN_DATABASE, CAN_CLUSTER, NET_ID_SENSOR_KNEE_R);
                servo_knee_left = AXIS(CAN_INTERFACE, CAN_DATABASE, CAN_CLUSTER, NET_ID_SERVO_KNEE_L); 
                encoder_knee_left = AXIS(CAN_INTERFACE, CAN_DATABASE, CAN_CLUSTER, NET_ID_SENSOR_KNEE_L);
                servo_hip_right = AXIS(CAN_INTERFACE, CAN_DATABASE, CAN_CLUSTER, NET_ID_SERVO_HIP_R);
                servo_hip_left = AXIS(CAN_INTERFACE, CAN_DATABASE, CAN_CLUSTER, NET_ID_SERVO_HIP_L);
                
                start_transmissao_rede_epos();
                                
                init_comunication = true;

                init_comm_eixos();
            }
            
            _running_aux = false;
        }
 
        void _setup(){
            initAllComunications();
            reset_falhas();
            define_origen();
            epos.sync();
        }

        void _cleanup(){
         
            if (_aux_thread.joinable())
                _aux_thread.join();
            setVelocityZero();
            Desabilita_Eixo(0);
        }

        void _loop(){

            epos.sync();
              
            _mtx.lock();   
             
            _mtx.unlock();
        }

        void setVelocityMode(AXIS* node) {
            node->VCS_SetOperationMode(VELOCITY_MODE);
        }

        

        void setVelocity(AXIS* node, long vel) {
            _mtx.lock();
                node->PDOsetVelocitySetpoint(vel);
                node->WritePDO02();
            _mtx.unlock();
        }

        void getPosition(AXIS* node, long  * pos) {
            _mtx.lock();
                node->ReadPDO01();
                (*pos) = node->PDOgetActualPosition();
            _mtx.unlock();
        }
     

        void _updateGUI(){
            ImGui::Begin(_name.c_str());
                if(isAlive)
                    ImGui::Text("EposEXO_CAN : RUNNING");
                else
                    ImGui::Text("EposEXO_CAN : PAUSED");
               
                if (!_running_aux) {
                    static bool auxEposActive = false;
                    static char name[10];
                    for (int i = 1; i <= 6; i++) {
                        auxEposActive = useEPOS[i];
                        sprintf(name,"ID: %d", i);
                        ImGui::SameLine();
                        if (ImGui::Checkbox(name, &auxEposActive));
                        
                        useEPOS[i] = auxEposActive;
                    }

                    if (ImGui::Button("Init CAN Network")) {
                        if (_aux_thread.joinable())
                            _aux_thread.join();
                        _aux_thread = std::thread(&ThreadEposEXO_CAN::initAllComunications, this);
                    }

                    if (ImGui::Button("Reset falhas")) {
                        if (_aux_thread.joinable())
                            _aux_thread.join();
                        _aux_thread = std::thread(&ThreadEposEXO_CAN::reset_falhas, this);
                    }

                    if (ImGui::Button("Define Origen")) {
                        if (_aux_thread.joinable())
                            _aux_thread.join();
                        _aux_thread = std::thread(&ThreadEposEXO_CAN::define_origen, this);
                    }

                    if (ImGui::Button("Set Velocity Mode ")) {
                        setVelocityZero();
                    }
                    ImGui::SameLine();

                    if (ImGui::Button("Activate motors ")) {
                        Habilita_Eixo(2);
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Set Zero Velocity")) {
                        isActiveEpos(ID_SERVO_KNEE_R)setVelocityMode(&servo_knee_right);
                        isActiveEpos(ID_SERVO_KNEE_L)setVelocityMode(&servo_knee_left );
                        isActiveEpos(ID_SERVO_HIP_L)setVelocityMode(&servo_hip_left  );
                        isActiveEpos(ID_SERVO_HIP_R)setVelocityMode(&servo_hip_right);
                        setVelocityZero();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Desactivate motors ")) {
                        Desabilita_Eixo(0);
                    }
                    if (ImGui::Button("SetVelocity 100 ")) {
                        setVelocity(&servo_knee_right, 100);
                    }

                    static int velocity_J2 = 0;
                    static bool enable_velocity_J2 = false;

                    ImGui::InputInt("Velo J2",&velocity_J2);
                    ImGui::Checkbox("Enable vel J2", &enable_velocity_J2);

                    if (enable_velocity_J2) {
                        setVelocity(&servo_knee_right, velocity_J2);
                    }


                }
                

            ImGui::End();
        }

    
        void getData(void * _data){
            _mtx.lock();
                memcpy(_data,&data,sizeof(fromEposEXO_CAN));
            _mtx.unlock();
        }
};
