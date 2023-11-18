#pragma once

/*
    UtilsCOM.h para propositos generales
    
    #define DEBUG_COM -> Para mostras los logs en la consola

    By Yecid Moreno : 2021
    git: https://github.com/YecidMorenoUSP

    Baseado de:
        SerialPort.h

    Requerimientos:
        1. [X]
*/


#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <mutex> 
#include <thread>
#include <time.h>




// #define DEBUG_COM
void  COM_DEBUG_LOG( char const* const _Format,...){
        #if defined(DEBUG_COM)
            printf(_Format);
        #endif // DEBUG_COM    
}

#define ARDUINO_WAIT_TIME 500
#define COM_BUF_SIZE 256

#define COM_BEGIN_TX "[BEGIN]\0"
#define COM_ALIVE_TX "[ALIVE]\0"
#define COM_END_TX   "[END]\0"

enum COM_ERROR{
        COM_NO_ERROR          = 0x01000000,
        COM_ERROR             = 0x00100000,
};

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

        std::mutex mtx;
        std::thread comThread_;

    }LOCAL;

    int openCOM(){

        LOCAL.CONECTED = false;

        LOCAL.handler = CreateFile(LOCAL.portName,
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);

        if (LOCAL.handler == INVALID_HANDLE_VALUE){
            COM_DEBUG_LOG("ERROR: Handle was not attached. Reason: %s not available\n", LOCAL.portName);
            return COM_ERROR;
        }
        else {
            DCB dcbSerialParameters = {0};

            if (!GetCommState(LOCAL.handler, &dcbSerialParameters)) {
                printf("failed to get current serial parameters");
                return COM_ERROR;
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
                    return COM_ERROR;
                }
                else {
                    LOCAL.CONECTED = true;
                    COM_DEBUG_LOG("\nCONNECTED");
                    PurgeComm(LOCAL.handler, PURGE_RXCLEAR | PURGE_TXCLEAR);
                    Sleep(ARDUINO_WAIT_TIME);
                    return COM_NO_ERROR;
                }
            }
        }
        return COM_NO_ERROR;
    }

    
    int closeCOM(){
        if (LOCAL.CONECTED){
            LOCAL.CONECTED = false;
            CloseHandle(LOCAL.handler);
        }
        COM_DEBUG_LOG("\nDISCONNECTED");
        return COM_NO_ERROR;
    }

    int sendCOM(){
        DWORD bytesSend;

        if (!WriteFile(LOCAL.handler, (void*) LOCAL.buffSend, LOCAL.buffSend_SIZE, &bytesSend, 0)){
            ClearCommError(LOCAL.handler, &LOCAL.errors, &LOCAL.status);
            COM_DEBUG_LOG("\nERROR SEND");
            return COM_ERROR;
        }
        COM_DEBUG_LOG("\nENVIADO");
        return COM_NO_ERROR;
    }

    int recvCOM(){

         DWORD bytesRead;
        
        unsigned int toRead = 0;

        while(true){
            ClearCommError(LOCAL.handler, &LOCAL.errors, &LOCAL.status);
            if (LOCAL.status.cbInQue >= LOCAL.buffRec_SIZE){
                toRead = LOCAL.buffRec_SIZE;
                if (ReadFile(LOCAL.handler,LOCAL.buffRec, toRead, &bytesRead, NULL)) return bytesRead;
                COM_DEBUG_LOG( "error code: %d\n", GetLastError() );
            }
            
            COM_DEBUG_LOG("\ntoRead : %d -----------------------------------------------",toRead);
            if(!this->LOCAL.CONECTED) break;
            Sleep(1);
            // if(LOCAL.status.cbInQue == 0) break;

           
        }
        
        return COM_NO_ERROR;
    }

    void loopCom(){

        clock_t start = clock();
        
        while(true){
            if(!this->LOCAL.CONECTED) break;

            if( (double(clock() - start) / CLOCKS_PER_SEC ) >= 0.2f ){
                sprintf(LOCAL.buffSend,COM_ALIVE_TX);
                this->sendCOM();
                start = clock();
            }
            
            this->recvCOM();
        }
        
    }

    void waitJoin(){
        if(LOCAL.comThread_.joinable())LOCAL.comThread_.join();
    }

    void runLoop(){
        waitJoin();
        LOCAL.comThread_ =  std::thread(&loopCom,this);
    }

};

// #include "Utils_COM.cpp"