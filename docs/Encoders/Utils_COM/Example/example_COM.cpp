#include <iostream>
#include <time.h>

// #define DEBUG_COM
#include "../Utils_COM.h"


typedef struct{
    float S1;
    float S2;
}ToRec;

typedef struct{
        char type [8];
}ToSend;

int main(){

    for(int KK = 0 ; KK < 2 ; KK ++){

    COM * esp32 = new COM();

    esp32->LOCAL.buffRec_SIZE  = sizeof(ToRec);
    esp32->LOCAL.buffSend_SIZE = sizeof(ToSend);
    esp32->LOCAL.Parameter.BaudRate = 1000000;
    esp32->LOCAL.Parameter.ByteSize = 8;
    esp32->LOCAL.Parameter.StopBits = ONESTOPBIT;
    esp32->LOCAL.Parameter.Parity = NOPARITY;
    esp32->LOCAL.Parameter.fDtrControl = DTR_CONTROL_ENABLE;
    
    sprintf(esp32->LOCAL.portName,"\\\\.\\COM15");

    ToSend * toSend = (ToSend *) esp32->LOCAL.buffSend;
    ToRec  * toRec  = (ToRec  *) esp32->LOCAL.buffRec;


    esp32->openCOM();

    sprintf(toSend->type,COM_BEGIN_TX);
    esp32->sendCOM();

    esp32->runLoop();

    clock_t start = clock();
    
    double elapsed;

    while ( elapsed = (double(clock() - start) / CLOCKS_PER_SEC ) < 2) {
        Sleep(10);
        printf("\nS1  : %f",toRec->S1);
        printf("\nS2  : %f",toRec->S2);
    }
    
    elapsed = double(clock() - start) / CLOCKS_PER_SEC;

    sprintf(toSend->type,COM_END_TX);
    esp32->sendCOM();   

    esp32->closeCOM();
    
    
    printf("\n\nEllapsep time %f",elapsed);
    Sleep(1000);

    }
    return 0;
}