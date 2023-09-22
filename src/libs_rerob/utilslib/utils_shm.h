#pragma once

#include <cstdint>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>




#define SHM_VARS_STREAM_KEY 0x1010
#define SHM_VARS_STREAM_SIZE 1024

typedef struct
{
    uint16_t key = SHM_VARS_STREAM_KEY;
    uint16_t statusWord;

    float t_s;

    float exo_hip_rigth_pos_in;
    float exo_hip_rigth_vel_in;
    float exo_hip_rigth_acc_in;
    float exo_hip_rigth_pos_out;
    float exo_hip_rigth_vel_out;
    float exo_hip_rigth_acc_out;
    
    float exo_knee_rigth_pos_in;
    float exo_knee_rigth_vel_in;
    float exo_knee_rigth_acc_in;
    float exo_knee_rigth_pos_out;
    float exo_knee_rigth_vel_out;
    float exo_knee_rigth_acc_out;

    float exo_ankle_rigth_pos_in;
    float exo_ankle_rigth_vel_in;
    float exo_ankle_rigth_acc_in;
    float exo_ankle_rigth_pos_out;
    float exo_ankle_rigth_vel_out;
    float exo_ankle_rigth_acc_out;

    float exo_hip_left_pos_in;
    float exo_hip_left_vel_in;
    float exo_hip_left_acc_in;
    float exo_hip_left_pos_out;
    float exo_hip_left_vel_out;
    float exo_hip_left_acc_out;
    
    float exo_knee_left_pos_in;
    float exo_knee_left_vel_in;
    float exo_knee_left_acc_in;
    float exo_knee_left_pos_out;
    float exo_knee_left_vel_out;
    float exo_knee_left_acc_out;

    float exo_ankle_left_pos_in;
    float exo_ankle_left_vel_in;
    float exo_ankle_left_acc_in;
    float exo_ankle_left_pos_out;
    float exo_ankle_left_vel_out;
    float exo_ankle_left_acc_out;

}shm_struct_vars_stream;



void* shm_create(key_t key, size_t size){
    int shmid = shmget(key, size, 0666 | IPC_CREAT);
    return shmat(shmid, (void *)0, 0);
}