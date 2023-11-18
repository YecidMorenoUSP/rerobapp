#include<iostream>
#include<stdio.h>
#include <windows.h>


using namespace std;

void not_printf(_In_z_ _Printf_format_string_ char const* const _Format,...){}

#define COM_DEBUG_LOG not_printf

#define COM_BUF_SIZE 255
#define ARDUINO_WAIT_TIME 500

typedef struct{
  char status;
  float enc_1;
}pkg_1_t;

class COM{
    public:
    COM(){
    }

    struct{

        int CONECTED = false;

        char portName [20] = "COM15";

        char * buffRec  = new char[COM_BUF_SIZE];
        char * buffSend = new char[COM_BUF_SIZE];
    
        void * handler;

        int buffRec_SIZE     =  COM_BUF_SIZE;
        int buffSend_SIZE    =  COM_BUF_SIZE;

        DCB Parameter = {0};

        COMSTAT status;
        DWORD errors;

    }LOCAL;

    int openCOM(){
        LOCAL.CONECTED = false;

        LOCAL.handler = CreateFile((LPCSTR) LOCAL.portName,
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
        
        if (LOCAL.handler == INVALID_HANDLE_VALUE){
            COM_DEBUG_LOG("ERROR: Handle was not attached. Reason: %s not available\n", LOCAL.portName);
            return -1;
        }else {
            DCB dcbSerialParameters = {0};

            if (!GetCommState(LOCAL.handler, &dcbSerialParameters)) {
                COM_DEBUG_LOG("failed to get current serial parameters");
                return -1;
            }
            else {
                dcbSerialParameters.BaudRate = LOCAL.Parameter.BaudRate;
                dcbSerialParameters.ByteSize = LOCAL.Parameter.ByteSize;
                dcbSerialParameters.StopBits = LOCAL.Parameter.StopBits;
                dcbSerialParameters.Parity   = LOCAL.Parameter.Parity;
                dcbSerialParameters.fDtrControl = LOCAL.Parameter.fDtrControl;

                if (!SetCommState(LOCAL.handler, &dcbSerialParameters))
                {
                    COM_DEBUG_LOG("ALERT: could not set Serial port parameters\n");
                    return -1;
                }
                else {
                    LOCAL.CONECTED = true;
                    COM_DEBUG_LOG("\nCONNECTED");
                    PurgeComm(LOCAL.handler, PURGE_RXCLEAR | PURGE_TXCLEAR);
                    Sleep(ARDUINO_WAIT_TIME);
                    return 1;
                }
            }
        }

        return  LOCAL.CONECTED;

    }

    int closeCOM() {
        if (LOCAL.CONECTED) {
            LOCAL.CONECTED = false;
            CloseHandle(LOCAL.handler);
        }
        COM_DEBUG_LOG("\nDISCONNECTED");
        return -1;
    }

    int sendCOM(void * bytes, int length){
        DWORD bytesSend;

        if (!WriteFile(LOCAL.handler, bytes , length, &bytesSend, 0)){
            ClearCommError(LOCAL.handler, &LOCAL.errors, &LOCAL.status);
            COM_DEBUG_LOG("\nERROR SEND");
            return -1;
        }
        COM_DEBUG_LOG("\nENVIADO");
        return 1;
    }

    int recvCOM(void * bytes, int length){

        DWORD bytesRead;
        
        unsigned int toRead = 0;

        while(true){
            ClearCommError(LOCAL.handler, &LOCAL.errors, &LOCAL.status);
            if (LOCAL.status.cbInQue >= length){
                toRead = length;
                if (ReadFile(LOCAL.handler,bytes, toRead, &bytesRead, NULL)) return bytesRead;
                COM_DEBUG_LOG( "error code: %d\n", GetLastError() );
                return -1;
            }
            
            COM_DEBUG_LOG("\n T:%d  O:%d",LOCAL.status.cbInQue,toRead);
            if(!this->LOCAL.CONECTED) break;
            Sleep(1);
            // if(LOCAL.status.cbInQue == 0) break;

           
        }
        
        return -1;
    }


};

int main(){

    
    COM *arduino = new COM();
    strcpy(arduino->LOCAL.portName,"COM7");
    arduino->LOCAL.Parameter.BaudRate = 1000000;
    arduino->LOCAL.Parameter.ByteSize = 8;
    arduino->LOCAL.Parameter.StopBits = ONESTOPBIT;
    arduino->LOCAL.Parameter.Parity = NOPARITY;
    arduino->LOCAL.Parameter.fDtrControl = DTR_CONTROL_ENABLE;

    arduino->openCOM();

    Sleep(3000);

    pkg_1_t pkg_1 = {0};

    for(int idx = 0 ; idx < 10000 ; idx ++){
        arduino->sendCOM((void *)"p",1);
        arduino->recvCOM(&pkg_1,sizeof(pkg_1));
        printf("enc_1: %f\n",pkg_1.enc_1);        
    }

    arduino->closeCOM();

    return 0;
}