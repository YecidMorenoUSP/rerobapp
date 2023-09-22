#pragma once

#include <string.h>
#include <mutex>
#include <atomic>

static class GUI_c{
    public:
    std::mutex _mtx;
    std::string LOG;
    std::atomic<bool> FORCE_STOP;
    GUI_c(){
        LOG = "Ready . . . \n";
        FORCE_STOP = false;
    }

    void addLOG(std::string txt){
        _mtx.lock();
        LOG += txt;
        _mtx.unlock();
    }

} GUI;