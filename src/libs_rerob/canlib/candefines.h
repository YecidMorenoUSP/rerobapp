#pragma once

#include <unistd.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <time.h>
#include <thread>

#define r (char *)
typedef struct can_frame can_frame_s;

// #define can_tic_toc usleep(400);
#define can_tic_toc std::this_thread::sleep_for(std::chrono::microseconds(10));
#define can_tic_toc_l std::this_thread::sleep_for(std::chrono::milliseconds(5));

const can_frame_s FRAME_NMT_PREOPERATIONAL = {0x00, 0x02, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00};
const can_frame_s FRAME_NMT_RESET_COM = {0x00, 0x02, 0x00, 0x00, 0x00, 0x81, 0x00, 0x00, 0x00, 0x00};
const can_frame_s FRAME_NMT_RESET_ALL = {0x00, 0x02, 0x00, 0x00, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00};

const can_frame_s FRAME_NMT_START_1 = {0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
const can_frame_s FRAME_NMT_START_2 = {0x50, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const can_frame_s FRAME_FAULT_RESET_1 = {0x0204, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00};
const can_frame_s FRAME_FAULT_RESET_2 = {0x0204, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00};
const can_frame_s FRAME_FAULT_RESET_3 = {0x0204, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00};
const can_frame_s FRAME_FAULT_RESET_4 = {0x0204, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00};

const can_frame_s FRAME_VELOCITY_MODE = {0x0304, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFA, 0x7F, 0x00, 0x00};

const can_frame_s FRAME_VELOCITY_MODE_100 = {0x0304, 0x08, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0xFA, 0x7F, 0x00, 0x00};

const can_frame_s FRAME_NMT_STOP = {0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00};
const can_frame_s FRAME_SYNC = {0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

typedef struct
{
    int32_t position;
    int16_t current;
    int16_t statusWord;
} PDO_TX_1_type;

typedef struct
{
    int32_t position;
    int16_t current;
    int16_t statusWord;
    
} PDO_RX_1_type;

typedef struct
{
    int32_t velocity;
    int16_t A1;
    int16_t A2;
} PDO_TX_2_type;

typedef struct
{
    int32_t velocity_d;
    int16_t controlWord;
    int16_t ddd;
    
} PDO_RX_2_type;

// can_frame_s frame;
// frame.can_id = 0x00;
// frame.can_dlc = 2;
// frame.data[0] = 0x01;
// frame.data[1] = 0x00;

// PDO_, SDO, index -- EPOS2 Communication Guide.pdf -- 43
//              COB-ID
// NMT           (0)
// SYNC         (0080h)    1005h
// EMERGENCY (0081h-00FFh) 1014h
// PDO1 (tx) (0181h-01FFh) 1800h  // getPosition
// PDO1 (rx) (0201h-027Fh) 1400h
// PDO2 (tx) (0281h-02FFh) 1801h  // getVelocity
// PDO2 (rx) (0301h-037Fh) 1401h
// PDO3 (tx) (0381h-03FFh) 1802h
// PDO3 (rx) (0401h-047Fh) 1402h
// PDO4 (tx) (0481h-04FFh) 1803h
// PDO4 (rx) (0501h-057Fh) 1403h
// SDO1 (tx) (0581h-05FFh) 1200h
// SDO1 (rx) (0601h-067Fh) 1200h

// ControlWord -- EPOS2 Firmware Specification.pdf -- 193

// PC can
//   can0  RX - -  000   [8]  01 00 00 00 00 00 00 00    ->   0      1
//   can0  RX - -  050   [8]  00 00 00 00 00 00 00 00    ->   80     0
//   can0  RX - -  080   [8]  00 00 00 00 00 00 00 00    ->   128    0
//   can0  RX - -  183   [8]  00 00 00 00 00 00 40 17    ->
//   can0  RX - -  181   [8]  00 00 00 00 00 00 40 17    ->
//   can0  RX - -  281   [8]  00 00 00 00 0E 00 18 00    ->
//   can0  RX - -  282   [8]  00 00 00 00 00 00 00 00    ->
//   can0  RX - -  182   [8]  00 00 00 00 00 00 40 07    ->
//   can0  RX - -  283   [8]  00 00 00 00 0E 00 18 00    ->

// Reset falhas
//   can0  RX - -  201   [8]  00 00 00 00 00 00 80 00
//   can0  RX - -  202   [8]  00 00 00 00 00 00 80 00
//   can0  RX - -  203   [8]  00 00 00 00 00 00 80 00
//   can0  RX - -  206   [8]  00 00 00 00 00 00 80 00
//   can0  RX - -  201   [8]  00 00 00 00 00 00 00 00
//   can0  RX - -  202   [8]  00 00 00 00 00 00 00 00
//   can0  RX - -  203   [8]  00 00 00 00 00 00 00 00
//   can0  RX - -  206   [8]  00 00 00 00 00 00 00 00

// Define origem
//   can0  RX - -  080   [8]  00 00 00 00 00 00 00 00
//   can0  RX - -  183   [8]  00 00 00 00 00 00 40 17
//   can0  RX - -  181   [8]  71 0B 00 00 00 00 40 17
//   can0  RX - -  281   [8]  00 00 00 00 13 00 18 00
//   can0  RX - -  282   [8]  00 00 00 00 00 00 00 00
//   can0  RX - -  182   [8]  00 00 00 00 00 00 40 07
//   can0  RX - -  283   [8]  00 00 00 00 0E 00 13 00

// Set velocityMode
//   can0  RX - -  301   [8]  00 00 00 00 FA 7F 00 00
//   can0  RX - -  303   [8]  00 00 00 00 FA 7F 00 00

// Activate motors
//   can0  RX - -  301   [8]  00 00 00 00 FA 7F 00 00
//   can0  RX - -  303   [8]  00 00 00 00 FA 7F 00 00

// Set Velocity 100
//   can0  RX - -  301   [8]  64 00 00 00 FA 7F 00 00

// Set Velocity 0
//   can0  RX - -  601   [8]  22 60 60 00 FE 00 00 00
//   can0  RX - -  603   [8]  22 60 60 00 FE 00 00 00
//   can0  RX - -  301   [8]  00 00 00 00 FA 7F 00 00
//   can0  RX - -  303   [8]  00 00 00 00 FA 7F 00 00
//   can0  RX - -  581   [8]  60 60 60 00 00 00 00 00
//   can0  RX - -  583   [8]  60 60 60 00 00 00 00 00

// Disable motors
//   can0  RX - -  203   [8]  00 00 00 00 00 00 07 00
//   can0  RX - -  201   [8]  00 00 00 00 00 00 07 00
//   can0  RX - -  203   [8]  00 00 00 00 00 00 06 00
//   can0  RX - -  201   [8]  00 00 00 00 00 00 06 00
//   can0  RX - -  081   [8]  20 32 04 00 00 00 00 00
//   can0  RX - -  082   [8]  20 32 04 00 00 00 00 00
//   can0  RX - -  083   [8]  20 32 04 00 00 00 00 00

// #define controlword 0x0000000000000000

// void PDOsetControlWord_FaultReset(bool state){
// 	if (state) controlWord = controlWord | 128;
// 	else controlWord = controlWord & (~128);
// }

// int a = (controlword|2);