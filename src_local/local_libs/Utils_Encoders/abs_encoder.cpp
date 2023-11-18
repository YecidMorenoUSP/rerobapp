#include "abs_encoder.h"

Abs_encoder::Abs_encoder(gpio_num_t _cs, long int _spi_freq){
    this->cs = _cs;
    this->spi_freq= _spi_freq;
}
void Abs_encoder::ESP_LOG(){
    // ESP_LOGI(TAG, "CYCLES: %d", encoder_cycles);
    // ESP_LOGI(TAG, "buffer: %d", raw_data);  
    // ESP_LOGI(TAG,"Angle: %f",angle);
}
void Abs_encoder::begin(){
    // set device configuration: 
    dev_config.command_bits = 0;
    dev_config.address_bits = 8;
    dev_config.dummy_bits = 0;
    dev_config.mode = 1;
    dev_config.duty_cycle_pos = 128;  // default 128 = 50%/50% duty
    dev_config.cs_ena_pretrans = 0;  // 0 not used
    dev_config.cs_ena_posttrans = 0;  // 0 not used
    dev_config.clock_speed_hz = this->spi_freq;
    dev_config.spics_io_num = this->cs;
    dev_config.flags = SPI_DEVICE_NO_DUMMY|SPI_DEVICE_HALFDUPLEX;  // 0 not used
    dev_config.queue_size = 1;
    dev_config.pre_cb = NULL;
    dev_config.post_cb = NULL;
    spi_bus_add_device(host, &dev_config, &device);

    // set transmission configuration
    transaction.flags = 0;
    transaction.cmd = 0;
    transaction.addr = 0x00 | 0x80 ;
    transaction.length = 2 * 8;
    transaction.rxlength = 2 * 8;
    transaction.user = NULL;
    transaction.tx_buffer = NULL;
    transaction.rx_buffer = (void*) &raw_data;

    ENABLE = true;
    
}
void Abs_encoder::read_raw(){
    spi_device_transmit(device, &transaction);
}
float Abs_encoder::read_angle(){
    if(!ENABLE) return 0.0;
    read_raw();
    raw_data = (~0x8000) & (int)raw_data; // mask to remove sign-bit
    angle_aux = raw_data*2.0*M_PI/MAX_RESOLUTION - M_PI;
    


    // add angle to buffer
   
    angle_buffer[1] = angle_buffer[0];
    angle_buffer[0] = angle_aux;
    

    // compute cycles
    if( raw_data > 3.0*MAX_RESOLUTION/4.0 || raw_data < MAX_RESOLUTION/4.0  ){
        if (angle_buffer[0] > 0 && angle_buffer[1] < 0 )  encoder_cycles--;
        if (angle_buffer[0] < 0 && angle_buffer[1] > 0 )   encoder_cycles++;  
    }
    angle_aux = angle_aux + (float)encoder_cycles*(2.0*M_PI);
    
    // if(abs(angle - angle_aux) >= .2){
    //     angle = angle;
    // }else{
    //     angle = angle_aux;
    // }  

    angle = angle_aux; 

    return angle;
}

void Abs_encoder::setZero(){
    angle_0 = angle;
}

float Abs_encoder::getAngle(){
    return angle-angle_0;
}