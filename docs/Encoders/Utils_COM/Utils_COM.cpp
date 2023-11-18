#include "Utils_COM.h"

#define COM_BEGIN_TX "[BEGIN]\0"
#define COM_ALIVE_TX "[ALIVE]\0"
#define COM_END_TX   "[END]\0"


namespace COM{

    static int openCOM(COM_Object * comData){

        comData->CONECTED = false;

        comData->handler = CreateFile(comData->portName,
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);

        if (comData->handler == INVALID_HANDLE_VALUE){
            COM_DEBUG_LOG("ERROR: Handle was not attached. Reason: %s not available\n", comData->portName);
            return COM_ERROR;
        }
        else {
            DCB dcbSerialParameters = {0};

            if (!GetCommState(comData->handler, &dcbSerialParameters)) {
                printf("failed to get current serial parameters");
                return COM_ERROR;
            }
            else {
                dcbSerialParameters.BaudRate = comData->Parameter.BaudRate;
                dcbSerialParameters.ByteSize = comData->Parameter.ByteSize;
                dcbSerialParameters.StopBits = comData->Parameter.StopBits;
                dcbSerialParameters.Parity   = comData->Parameter.Parity;
                dcbSerialParameters.fDtrControl = comData->Parameter.fDtrControl;

                if (!SetCommState(comData->handler, &dcbSerialParameters))
                {
                    COM_DEBUG_LOG("ALERT: could not set Serial port parameters\n");
                    return COM_ERROR;
                }
                else {
                    comData->CONECTED = true;
                    COM_DEBUG_LOG("\nCONNECTED");
                    PurgeComm(comData->handler, PURGE_RXCLEAR | PURGE_TXCLEAR);
                    Sleep(ARDUINO_WAIT_TIME);
                    return COM_NO_ERROR;
                }
            }
        }
        return COM_NO_ERROR;
    }

    static int closeCOM(COM_Object * comData){
        if (comData->CONECTED){
            comData->CONECTED = false;
            CloseHandle(comData->handler);
        }
        COM_DEBUG_LOG("\nDISCONNECTED");
        return COM_NO_ERROR;
    }

    static int sendCOM(COM_Object * comData){
        DWORD bytesSend;

        if (!WriteFile(comData->handler, (void*) comData->buffSend, comData->buffSend_SIZE, &bytesSend, 0)){
            ClearCommError(comData->handler, &comData->errors, &comData->status);
            COM_DEBUG_LOG("\nERROR SEND");
            return COM_ERROR;
        }
        COM_DEBUG_LOG("\nENVIADO");
        return COM_NO_ERROR;
    }

    static int recvCOM(COM_Object * comData){

         DWORD bytesRead;
        
        unsigned int toRead = 0;

        while(true){
            ClearCommError(comData->handler, &comData->errors, &comData->status);
            if (comData->status.cbInQue >= comData->buffRec_SIZE){
                toRead = comData->buffRec_SIZE;
                if (ReadFile(comData->handler,comData->buffRec, toRead, &bytesRead, NULL)) return bytesRead;
                COM_DEBUG_LOG( "error code: %d\n", GetLastError() );
            }
            
            COM_DEBUG_LOG("\ntoRead : %d -----------------------------------------------",toRead);
            Sleep(1);
            // if(comData->status.cbInQue == 0) break;
           
        }
        
        return COM_NO_ERROR;
    }


};