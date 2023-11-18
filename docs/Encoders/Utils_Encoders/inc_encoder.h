#pragma once
// NodeMCU-32S: https://raw.githubusercontent.com/AchimPieters/esp32-homekit-camera/master/Images/ESP32-38%20PIN-DEVBOARD.png

// info: to read two incremental encoders
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "math.h"


class Inc_encoder{  
    private:
        gpio_num_t A;
        gpio_num_t B;
        float max_counts;
        volatile int32_t   global_counts_m1;
        uint8_t   global_m1a_state;
        uint8_t   global_m1b_state;
        uint8_t   global_last_m1a_state;
        uint8_t   global_last_m1b_state;
        float angle = 0;
        float angle_0 = 0;


    public:
        Inc_encoder(gpio_num_t a,gpio_num_t b,float max_counts);
    
        esp_err_t init_isr();

        void fnc_ISR(void * args);

        float getAngle();

        static void callback_ISR(void * args);  

        static void activateGlobalInterruptions();

        void setZero();
};

#include "inc_encoder.cpp"