#include <iostream>
#include <unistd.h>

#define PRINT_LEVEL 5
#include <utils_printlog.h>
#include <utils_shm.h>

#include <string.h>

#include <math.h>

#include <chrono>
using namespace std::chrono;

int main()
{

    void *shm_ptr = shm_create(SHM_VARS_STREAM_KEY, SHM_VARS_STREAM_SIZE);
    shm_struct_vars_stream *var_stream = (shm_struct_vars_stream *)shm_ptr;
    var_stream->key = SHM_VARS_STREAM_KEY;
    

    static auto t0_test = high_resolution_clock::now();
    
    while (1)
    {
        static auto t0_send = high_resolution_clock::now();

        long dt = duration_cast<milliseconds>(high_resolution_clock::now() - t0_send).count();
        if (dt >= 20)
        {   
            // fflush(stdout);
            // printf(".");
            
            t0_send = high_resolution_clock::now();

            var_stream->t_s = duration_cast<milliseconds>(high_resolution_clock::now() - t0_test).count() / 1000.0f;
            
            static float df = .2;
            float f = 1.0;
            var_stream->exo_hip_rigth_pos_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_hip_rigth_vel_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_hip_rigth_acc_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_hip_rigth_pos_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_hip_rigth_vel_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_hip_rigth_acc_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_knee_rigth_pos_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_knee_rigth_vel_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_knee_rigth_acc_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_knee_rigth_pos_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_knee_rigth_vel_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_knee_rigth_acc_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_ankle_rigth_pos_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_ankle_rigth_vel_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_ankle_rigth_acc_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_ankle_rigth_pos_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_ankle_rigth_vel_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_ankle_rigth_acc_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_hip_left_pos_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_hip_left_vel_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_hip_left_acc_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_hip_left_pos_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_hip_left_vel_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_hip_left_acc_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_knee_left_pos_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_knee_left_vel_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_knee_left_acc_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_knee_left_pos_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_knee_left_vel_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_knee_left_acc_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_ankle_left_pos_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_ankle_left_vel_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_ankle_left_acc_in = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_ankle_left_pos_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_ankle_left_vel_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
            var_stream->exo_ankle_left_acc_out = sin(2 * M_PI * f * var_stream->t_s); f+=df;
 
        } 
    }

    shmdt(shm_ptr);

    return 0;
}