#pragma once

/*
    UtilsCOM.h para propositos generales
    
    #define DEBUG_COM -> Para mostras los logs en la consola

    By Yecid Moreno : 2021
    git: https://github.com/YecidMorenoUSP

    Verifique que los bufer satisfacen el tamanho de acuerdo a base 2, 2^n

    Requerimientos:
        1. [X]
*/


#define COM_BEGIN_TX "[BEGIN]\0"
#define COM_ALIVE_TX "[ALIVE]\0"
#define COM_END_TX   "[END]\0"

extern void COM_Callback(void) __attribute__((weak));

template <typename T, typename... Rest>
void  COM_DEBUG_LOG( T _Format, Rest... rest) {
#if defined(DEBUG_COM)
  Serial.printf(_Format, rest...);
#endif // DEBUG_COM    
}

bool
isCommand (const char *p1, const char *p2)
{
  const unsigned char *s1 = (const unsigned char *) p1;
  const unsigned char *s2 = (const unsigned char *) p2;
  unsigned char c1, c2;
  do
  {
    c1 = (unsigned char) * s1++;
    c2 = (unsigned char) * s2++;
    if (c2 == '\0')
      return true;
  }
  while (c1 == c2);
  return false;
}

#define COM_BUF_SIZE 256

namespace COM {
enum COM_ERROR {
  COM_NO_ERROR          = 0x01000000,
  COM_ERROR             = 0x00100000,
};

bool isInit = false;

typedef struct {

  long int baudRate = 115200;

  uint8_t * buffRec = new uint8_t[COM_BUF_SIZE];
  uint8_t * buffSend = new uint8_t[COM_BUF_SIZE];

  int buffRec_SIZE     =  COM_BUF_SIZE;
  int buffSend_SIZE    =  COM_BUF_SIZE;

} COM_Object;

COM_Object * comData;


static int initCOM(COM_Object * obj) {
  comData = obj;
  Serial.begin(comData->baudRate);
  isInit = true;
  return COM_NO_ERROR;
}

static void Update() {

  if (!isInit) {
    COM_DEBUG_LOG("No iniciado aún . . .");
    return;
  }

  while (Serial.available() >= (comData->buffRec_SIZE)) {

    Serial.readBytes(comData->buffRec, (comData->buffRec_SIZE));

    if (COM_Callback) {
      COM_Callback();
    } else {
      Serial.printf("\n Read : %s", comData->buffRec);
    }
  }
}

template <typename T, typename P>
static void ReadBytesSync(T  _buffer,P _size){
    while (Serial.available() < _size);
    Serial.readBytes((uint8_t*)_buffer, _size);
}

};
