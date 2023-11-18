[![YecidMorenoUSP - ExoControl](https://img.shields.io/static/v1?label=YecidMorenoUSP&message=MY_LIBS&color=blue&logo=github)](https://github.com/YecidMorenoUSP/MY_LIBS)
[![alt](https://img.shields.io/github/license/YecidMorenoUSP/MY_LIBS?color=blue)](LICENSE.md)
![Windows](https://img.shields.io/badge/Windows-x64%20\|%20x86-blue?style=flat&logo=windows)

## UTILS_COM

### How Use? Arduino & ESP32
1. Cargar la biblioteca
   ```c
   #include <Utils_COM_esp32.h>
   ```
2. Crear las plantillas para los buffer de entrada y salida
   ```c
   typedef struct {
   int P1;
   int P2;
   } ToSend;

   typedef struct {
      char Type [8];
    } ToRec;
   ```
3. Declarar variables y punteros
   ```c
   ToSend * toSend;
   ToRec  * toRecv;

   COM::COM_Object comData;
   ```
4. Configurar e iniciar la conexion
   ```c
   comData.baudRate = 1000000;
   comData.buffRec_SIZE  = sizeof(ToRec);
   comData.buffSend_SIZE = sizeof(ToSend);

   memmove(&toSend , &comData.buffSend ,  sizeof(ToSend));
   memmove(&toRecv , &comData.buffRec  ,  sizeof(ToRec ))

   COM::initCOM(&comData);
   ```
5. Recibir buffer: Es importante a la hora de recibir un dato ejecutar previamente la sentencia `COM::Update();`, para hacer unas verificaciones previas. 
   ```c
   void loop() {
      COM::Update();
      /*  ... YOUR CODE ...*/
   }
   ```
   Una vez todo esté en orden listo para recibir se ejecuta el metodo 
   ```c
   void COM_Callback(){ . . .}
   ```
   Y todas las semas instrucciones tendrán la siguiente estructura:
   ```c
   if (isCommand(toRecv->Type, COM_BEGIN_TX)) {
      toSend->P1 = 0;
      toSend->P2 = 0;
   }
   ```
6. Enviar buffer
   ```c
   toSend->P1  = millis();
   toSend->P2  += 1;
   Serial.write(comData.buffSend, sizeof(ToSend));
   ```
### How Use? Desktop
1. Cargar la biblioteca
   ```c++
   #include <Utils_COM.h>
   ```
2. Crear las plantillas para los buffer de entrada y salida
   ```c
   typedef struct {
   int P1;
   int P2;
   } ToRec;

   typedef struct {
      char Type [8];
    } ToSend;
   ```
3. Configurar la interfaz Serial: 
   ```c++
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
   ```
4. Conectarse a un dispositivo
   ```c++
   esp32->openCOM();
   esp32->runLoop();
   ```
5. Enviar un buffer
   ```c++
   sprintf(toSend->type,COM_BEGIN_TX);
   esp32->sendCOM();
   ```
6. Recibir un buffer
   ```c++
   printf("\nS1  : %f",toRec->P1);
   printf("\nS2  : %f",toRec->P2);
   ```
7. Cerrar conexion   
   ```c++
   esp32->closeCOM();
   ```
### Commands

| Command         |Value  | Info
| ---         |--- |---
|COM_BEGIN_TX | [BEGIN]\0  | Iniciar la transferencia de datos
|COM_ALIVE_TX | [ALIVE]\0  | Mantener activa la comunicacion por un lapso de tiempo
|COM_END_TX   | [END]\0  | Finalizar el intercambio de informacion

### How Install? Arduino & ESP32
1. Intalar `Utils_COM.zip` desde el IDE de Arduino.
2. Ver los ejemplos que estan disponibles desde la pestanha **Archivo > Ejemplos > Esp32_Utils_COM**

### How Install? Desktop
1. Guardar la carpeta `Utils_COM` en una ubicacion conocida
2. *Compilar*: con el parametro **-I** adicione la ruta donde está ubicada la carpeta de la bibiloteca.
    ```PS
    g++ main.cpp -o main.exe -I "${DEV2}/MY_LIBS/Utils_COM"
    ```
3. *Debug*: De manera opcional adicione la definición **DEBUG_COM**, para visualizar en consola el estado de ejecucion.
    ```PS
    g++ main.cpp -o main.exe -I "${DEV2}/MY_LIBS/Utils_COM" -D DEBUG_COM
    ```

### Errors
|Name                  | Value       | Info
|----                  | ----        |  ----
|COM_NO_ERROR          | 0x01000000  | No existen errores
|COM_ERROR | 0x00100000  | Ha ocurrido intentando coectar, o configurar la interfaz serial

## License
Released under [MIT](LICENSE) by [@YecidMorenoUSP](https://github.com/YecidMorenoUSP).
  

