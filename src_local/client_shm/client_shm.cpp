#include <iostream>
#include<unistd.h>  

#define PRINT_LEVEL 5
#include <utils_printlog.h>
#include <utils_shm.h>

int main(){

    void* shm_ptr = shm_create(SHM_VARS_STREAM_KEY,SHM_VARS_STREAM_SIZE);    
    shm_struct_vars_stream* var_stream = (shm_struct_vars_stream*) shm_ptr;

    while (1)
    {   
        printf("T: %d\n",var_stream->key);
        printf("Ts: %f\n",var_stream->t_s);
        usleep(1000000);
    }
    
    shmdt(shm_ptr);

    return 0;
}