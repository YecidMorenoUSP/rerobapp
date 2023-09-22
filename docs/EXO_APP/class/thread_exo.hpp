#include <utilsGUI.h>
#include <thread_exo.h>
#include <iostream>
#include <utils.h>

void _Threads::add(ThreadExo * threadExo){
    this->_threadsExoClass.push_back(threadExo);
    //atomic uint32
    this->aborts.push_back(new std::atomic<bool>(false));
    this->flags.push_back(new std::atomic<bool>(true));

    threadExo->ixd = this->_threadsExoClass.size() - 1 ;
}

void _Threads::run(float time){
    this->_threads.clear();

    GUI.FORCE_STOP = false;
    GUI._mtx.lock();
        GUI.LOG = "\nIniciando Hilos";
    GUI._mtx.unlock();
    
    for(auto cur_ : this->aborts){
        (*cur_) = false;
    }

    for(ThreadExo * _cur : this->_threadsExoClass){
        (*_cur).setTime(time);
       this->_threads.push_back(new std::thread(&ThreadExo::run, _cur));
    }

    GUI.addLOG("\nHilos iniciados");
}

void _Threads::join(){
    
    for(std::thread * cur : this->_threads){
       if((*cur).joinable())
            (*cur).join();
    }
    threadsExo.clear();
    ItALL{
        (cur_)->toRUNfromGUI = false;
    }
    
}

void ThreadType::abort(){
    (*threadsExo.aborts[this->parent->ixd]) = true ;
    GUI.addLOG("\n *** Aborting - " + _name + " *** ");
    std::cout<<"\n *** Aborting - "<< _name << " *** ";
}



void ThreadType::run(){
    time_index = 0;
    this->isAlive = true;
    
    int T_END = (int)(T_exec/_Ts);
    
    datalog_ZERO(_datalog,T_END,_n_datalogs);

    //Init datalogs
    try{
        _setup();
    }catch(std::exception const &ex){
        abort();
        GUI.addLOG(ex.what());
    }catch(...){
        abort();
    }
    
    if(waitAllThreads() == T_ERROR){
        try{_cleanup();}
        catch (std::exception const& ex) { GUI.addLOG(ex.what()); }
        catch(...){ GUI.addLOG("Erro em _cleanup 1"); }
        this->isAlive = false;
        return;
    }
    lockAllThreads();
    esperar_n_seg(1);
    
    try {
        _firstLoop();
    }
    catch (std::exception const& ex) {
        abort();
        GUI.addLOG(ex.what());
    }
    catch (...) {
        abort();
    }
    if (threadsExo._threadsExoClass[0]->threadType_ == this) {
        Beep(523 * 1, 500);
        Beep(523 * 2, 500);
        Beep(523 * 3, 500);
    }
    

    if (waitAllThreads() == T_ERROR) {
        try { _cleanup(); }
        catch (std::exception const& ex) { GUI.addLOG(ex.what()); }
        catch (...) { GUI.addLOG("Erro em _cleanup 1"); }
        this->isAlive = false;
        return;
    }

    timer = new loop_timers(_Ts);

    do{
        //timer->start_timer();
        auto begin_timestamp = std::chrono::steady_clock::now();
        try{
            _loop();
            time_index ++;
        }
        catch (std::exception const& ex) { break; GUI.addLOG(ex.what()); }
        catch (...) { break;  GUI.addLOG("Erro em _cleanup 2"); }

            
        //timer->wait_final_time();
        std::this_thread::sleep_until(begin_timestamp + std::chrono::microseconds(int(_Ts * 1000000)));

    }while(!GUI.FORCE_STOP && (time_index < T_END)); 

    try{_cleanup();}
    catch (std::exception const& ex) { GUI.addLOG(ex.what()); }
    catch (...) { GUI.addLOG("Erro em _cleanup 3"); }
    
    if (threadsExo._threadsExoClass[0]->threadType_ == this) {
        Beep(523 * 3, 500);
        Beep(523 * 2, 500);
        Beep(523 * 1, 500);
    }

    saveDatalog(_name,_datalog,T_END,_n_datalogs);
    
    this->isAlive = false;
    //Save datalogs
    
}

int ThreadType::waitAllThreads(){
    releaseAllThreads();
    int flags = 0;
    while (true){

        for(auto cur_ : threadsExo.aborts){
            if((*cur_) == true) return T_ERROR;
        }

        flags = 0;
        for(auto cur_ : threadsExo.flags){
            flags+=(*cur_);
        }
        if(flags == 0) return T_SUCCESS;

    }
}

void ThreadType::releaseAllThreads() {
    (*threadsExo.flags[this->parent->ixd]) = false;
}
void ThreadType::lockAllThreads() {
    (*threadsExo.flags[this->parent->ixd]) = true;
}

void _Threads::updateGUI(){
    for(ThreadExo * cur_ : this->_threadsExoClass){
         (*cur_).updateGUI();
    }
}

bool _Threads::RUNNING(){
    for(ThreadExo * cur_ : this->_threadsExoClass){
         if((*cur_).RUNNING()) return true;
    }
    return false;
}