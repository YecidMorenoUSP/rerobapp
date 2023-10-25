#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "candefines.h"
#include "cannetwork.hpp"

class EposNode
{
private:
    int8_t _id;
    int32_t _controlWord;
    CanNetwork *_can;

    can_frame_s frame;

    can_frame_s _PDO_TX_1 = {0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    can_frame_s _PDO_RX_1 = {0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    can_frame_s _PDO_TX_2 = {0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    can_frame_s _PDO_RX_2 = {0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    can_frame_s _SDO_TX_1 = {0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    can_frame_s _SDO_RX_1 = {0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    PDO_TX_1_type * PDO_TX_1_data;
    PDO_RX_1_type * PDO_RX_1_data;
    PDO_TX_2_type * PDO_TX_2_data;
    PDO_RX_2_type * PDO_RX_2_data;

    void _init(int8_t id)
    {
        _id = id;

        _PDO_TX_1.can_id = 0x180 + _id;
        _PDO_RX_1.can_id = 0x200 + _id;
        _PDO_TX_2.can_id = 0x280 + _id;
        _PDO_RX_2.can_id = 0x300 + _id;
        _SDO_TX_1.can_id = 0x580 + _id;
        _SDO_RX_1.can_id = 0x600 + _id;   

        PDO_TX_1_data = (PDO_TX_1_type*) &_PDO_TX_1.data;  
        PDO_RX_1_data = (PDO_RX_1_type*) &_PDO_RX_1.data;  
        PDO_TX_2_data = (PDO_TX_2_type*) &_PDO_TX_2.data;     
        PDO_RX_2_data = (PDO_RX_2_type*) &_PDO_RX_2.data;     
    }

public:
    
    int32_t position;
    int16_t A1,A2;

    EposNode(){};
    EposNode(int id, CanNetwork &can);
    ~EposNode();

    void setVelocityMode(){
        PDO_RX_2_data->controlWord = 0;
        PDO_RX_2_data->ddd = 0;
        PDO_RX_2_data->velocity_d = 0;
        
        PDO_RX_2_data->controlWord =  0xFA<<8 | 0x7F;

        // _PDO_RX_1.data[4]=0xFA;
        // _PDO_RX_1.data[5]=0x7F;

        _can->sendFrame(_PDO_RX_2);
    }

    void setVelocity(int16_t velocity){
        PDO_RX_2_data->controlWord = 0;
        PDO_RX_2_data->ddd = 0;
        PDO_RX_2_data->velocity_d = velocity;
        
        PDO_RX_2_data->controlWord =  0xFA<<8 | 0x7F;

        _can->sendFrame(_PDO_RX_2);
    }

    void stopMotors(){
        _PDO_RX_1.data[6] = 0x06;
        _can->sendFrame(_PDO_RX_1);
    }

    void startMotors(){
  
    }

    void readPDO_TX_all(){
        _can->getDicKey(_PDO_TX_1.can_id,_PDO_TX_1);
        _can->getDicKey(_PDO_TX_2.can_id,_PDO_TX_2);        
    }

    int32_t getPosition(){
        return PDO_TX_1_data->position;
    }

    int16_t getVelocity(){
        return PDO_TX_2_data->velocity;
    }

    void faultReset()
    {
        PDO_RX_1_type * frame = (PDO_RX_1_type*) &_PDO_RX_1.data;

        _PDO_RX_1.data[6] = 0x80;
        _can->sendFrame(_PDO_RX_1);
        
        _PDO_RX_1.data[6] = 0x06;
        _can->sendFrame(_PDO_RX_1);

        _PDO_RX_1.data[6] = 0x07;
        _can->sendFrame(_PDO_RX_1);

        _PDO_RX_1.data[6] = 0x0F;
        _can->sendFrame(_PDO_RX_1);
    }
};
