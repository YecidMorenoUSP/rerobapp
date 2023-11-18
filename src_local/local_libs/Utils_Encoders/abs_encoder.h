#pragma once
// documentation
// expressif-spi: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/spi_master.html

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "math.h"



#define SPI_MODE  1
#define MISO_PIN  19
#define MOSI_PIN  23
#define SCLK_PIN  18
#define CS_PIN    27
#define SPI_CLOCK 500000  // 3 MHz

#define MAX_RESOLUTION 32768

#define host VSPI_HOST


class Abs_encoder{  
/* 
@info: configurate the encoder for spi communication (chip selector pin and clock frequency)
@inputs:
-------
    - cs: chip selector pin (gpio_num_t)
    - spi_freq: clock frequency (long int)
@outputs:
--------
    - raw_data: 15-bit resolution (0-32768) 
    - angle: absolute angular position (rad)
*/    
    gpio_num_t cs;
    long int spi_freq;
    spi_device_handle_t device;
    spi_device_interface_config_t dev_config;
    spi_transaction_t transaction;
    int16_t raw_data;
    float angle_buffer[2] = {0};  
    int encoder_cycles=0;
    float angle = 0;
    float angle_aux = 0;
    float angle_0 = 0;
    bool ENABLE = false;

    public:
        Abs_encoder(gpio_num_t _cs, long int _spi_freq);
        
        void ESP_LOG();
        
        void begin();

        void read_raw();

        float read_angle();

        float getAngle();
        
        void setZero();

        int16_t get_raw_data(){return raw_data;}

}; 

void spi_setup(){
    spi_bus_config_t config;
    config.intr_flags = 0;
    config.mosi_io_num = MOSI_PIN;
    config.miso_io_num = MISO_PIN;
    config.sclk_io_num = SCLK_PIN;
    config.quadwp_io_num = -1;  // -1 not used
    config.quadhd_io_num = -1;  // -1 not used
    config.max_transfer_sz = 16;
    spi_bus_initialize(host, &config, SPI_DMA_DISABLED);
}

#include "abs_encoder.cpp"
