#include "inc_encoder.h"

Inc_encoder::Inc_encoder(gpio_num_t a,gpio_num_t b,float max_counts){
    this->A = a;
    this->B = b;
    this->global_counts_m1 = 0;
    this->max_counts = max_counts;

    init_isr();
}

esp_err_t Inc_encoder::init_isr(){
    gpio_config_t pgpio_a;
    pgpio_a.pin_bit_mask = (1ULL << (int)A );      // enable pin
    pgpio_a.mode = GPIO_MODE_INPUT;                 // set input mode
    pgpio_a.pull_up_en = GPIO_PULLUP_DISABLE;       // not used
    pgpio_a.pull_down_en = GPIO_PULLDOWN_DISABLE;   // not used
    pgpio_a.intr_type = GPIO_INTR_ANYEDGE;          // set interrupt type
    gpio_config(&pgpio_a);
    
    gpio_config_t pgpio_b;
    pgpio_b.pin_bit_mask = (1ULL << (int)B );          
    pgpio_b.mode = GPIO_MODE_INPUT;               
    pgpio_b.pull_up_en = GPIO_PULLUP_DISABLE;       
    pgpio_b.pull_down_en = GPIO_PULLDOWN_DISABLE;   
    pgpio_b.intr_type = GPIO_INTR_ANYEDGE;      
    gpio_config(&pgpio_b);
    
    // gpio_install_isr_service(0);

    gpio_isr_handler_add(A,Inc_encoder::callback_ISR,this);
    gpio_isr_handler_add(B,Inc_encoder::callback_ISR,this);

    // gpio_install_isr_service(1);


    global_m1a_state = gpio_get_level(A);
    global_m1b_state = gpio_get_level(B);  

    return ESP_OK;
}

void Inc_encoder::fnc_ISR(void * args){
    global_m1a_state = gpio_get_level(A);
    global_m1b_state = gpio_get_level(B);  
    
    bool forward_m1 = global_m1a_state ^ global_last_m1b_state; // clockwise direction
    bool backward_m1 = global_m1b_state ^ global_last_m1a_state; // counter clockwise direction

        if(forward_m1 == 1){      
            global_counts_m1 += 1;          
        }
        else if(backward_m1 == 1){ 
            global_counts_m1 -= 1;
        }

    global_last_m1a_state = global_m1a_state;
    global_last_m1b_state = global_m1b_state;
}

float Inc_encoder::getAngle(){
    angle = global_counts_m1/max_counts * 2.0*M_PI;
    return angle-angle_0;
}

void Inc_encoder::setZero(){
    angle_0 = angle;
}

void Inc_encoder::callback_ISR(void * args){
    ((Inc_encoder*)args)->fnc_ISR(NULL);
}

void Inc_encoder::activateGlobalInterruptions(){
    gpio_install_isr_service(0);
}