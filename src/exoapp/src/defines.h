#pragma once

// // #define _count_k 1
// // #define _ts_s 0.001
// #define _ts_m ((long)(_ts_s * 1000L))
// #define _ts_u _ts_m * 1000L
// #define _ts_n _ts_u * 1000L

// #define _count_k 1
// #define _ts_s 0.001
// #define _ts_m ((long)(0.001 * 1000L))
// #define _ts_n (((long)(0.001 * 1000L)) * 1000L) * 1000L

// #define T_total 10

#define EPOS_NET_SIZE 10

#define PRINT_LEVEL 5

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <condition_variable>

#include <cannetwork.hpp>
#include <eposnetwork.hpp>
#include <eposnode.hpp>

#include <math.h>


#include <sys/stat.h>
#include <sys/types.h>

#include <utils_shm.h>
#include <utils_spinlock.h>
#include <utils_printlog.h>
#include <utils_files.h>
