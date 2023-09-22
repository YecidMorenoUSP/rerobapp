#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <string>
#include <utils.h>

enum{
    T_ERROR = 0,
    T_SUCCESS
};

class ThreadExo;
class ThreadType;
class _Threads;


class _Threads{
    private:
        
        std::vector<std::thread *> _threads;
        
    public:
        std::vector<ThreadExo*> _threadsExoClass;
        std::vector<std::atomic<bool>*> aborts;
        std::vector<std::atomic<bool>*> flags;        
        std::vector<ThreadExo*> _all_ExoClass;

        std::atomic<uint32_t> A_aborts;
        std::atomic<uint32_t> A_flags;

        void clear(){_threadsExoClass.clear();aborts.clear();flags.clear();}
        void add(ThreadExo * threadExo);
        void run(float time);
        void join();
        bool RUNNING();
        void updateGUI();

}threadsExo;

class ThreadType{
    public:
        std::string _name;
        float _Ts;
        float T_exec;
        int time_index = 0;
        int _n_datalogs;
        std::mutex _mtx;
        ThreadExo * parent;
        float * v_datalogs;
        void run();
        virtual void _setup(){};
        virtual void _firstLoop() {};
        virtual void _loop() {};
        virtual void _exit(){};
        virtual void _cleanup(){};
        virtual void _updateGUI(){};
        virtual void getData(void * _data){};
        std::atomic<bool> isAlive;
        void abort();
        int waitAllThreads();
        void releaseAllThreads();
        void lockAllThreads();
        loop_timers * timer;
        std::vector< std::vector<double> > _datalog;
        static std::atomic<bool> isSound;
};

class ThreadExo{
            
    public:
        int ixd;
        bool toRUNfromGUI;
        ThreadType * threadType_;

        ThreadExo(ThreadType * type,std::string __name__,float __sample_time__, int __n_datalogs__ ):threadType_(type){
            toRUNfromGUI = false;  
            threadType_->parent = this;
            threadType_->_name = __name__;
            threadType_->_Ts = __sample_time__;
            threadType_->_n_datalogs = __n_datalogs__;
            threadsExo._all_ExoClass.push_back(this);  
        };

        void setTime(float t){
            threadType_->T_exec = t;
        }

        void run(){
            this->threadType_->run();
        }

        void updateGUI(){
             this->threadType_->_updateGUI();
        }

        void getData(void * _data){
            threadType_->getData(_data);
        }

        bool RUNNING(){
            return this->threadType_->isAlive;
        }

        std::string getName(){
            return this->threadType_->_name;
        }
};

#include "thread_exo.hpp"



