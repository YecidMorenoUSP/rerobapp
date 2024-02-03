#include "utils_shm.h"

#include <cstdint>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

void* shm_create(key_t key, size_t size){
    int shmid = shmget(key, size, 0666 | IPC_CREAT);
    return shmat(shmid, (void *)0, 0);
}