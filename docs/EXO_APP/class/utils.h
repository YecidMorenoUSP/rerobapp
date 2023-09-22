#pragma once

#include <time.h>
#include <winsock2.h>
#include <windows.h>
#include <vector>




#ifdef _WIN32
//LARGE_INTEGER tick_after, tick_before, TICKS_PER_SECOND;
LARGE_INTEGER tick_after_exo, tick_before_exo;
LARGE_INTEGER tick_after_ard, tick_before_ard;
#endif
#ifdef _WIN32
  LARGE_INTEGER tick_after, tick_before, TICKS_PER_SECOND;
  long long int ticksSampleTime;
#endif
class loop_timers
{
public:
  double SAMPLE_TIME;
  int samples_per_second;
  //clock_t initial_time;
  //clock_t final_time;
  long long int initial_time;
  long long int final_time;
  double t;
  long long int tempo;
  long long int tempo2;
  long long int time_now;
  long long int time_0;

#ifdef _WIN32
  LARGE_INTEGER tick_after, tick_before, TICKS_PER_SECOND;
  long long int ticksSampleTime;
#endif

  loop_timers(double s_time)
  {
    this->SAMPLE_TIME = s_time;
#ifdef _WIN32
    QueryPerformanceFrequency(&TICKS_PER_SECOND);
    this->ticksSampleTime = TICKS_PER_SECOND.QuadPart * SAMPLE_TIME;
#endif
    this->samples_per_second = (int)(1 / SAMPLE_TIME);
    this->tempo = clock();
    this->tempo2 = clock();
    this->time_0 = clock();
  };

  void start_timer()
  {
#ifdef _WIN32

    QueryPerformanceCounter(&tick_before);
    this->initial_time = tick_before.QuadPart;
    this->final_time = tick_before.QuadPart + 1 * this->ticksSampleTime;

    /*
    this->initial_time = clock();
    this->final_time = clock() + this->SAMPLE_TIME * CLOCKS_PER_SEC;
    */
#endif

#ifdef linux
    this->initial_time = clock();
    this->final_time = clock() + this->SAMPLE_TIME * CLOCKS_PER_SEC;
#endif
  };

  void wait_final_time()
  {
#ifdef _WIN32

    QueryPerformanceCounter(&tick_after);
    while (this->final_time > this->tick_after.QuadPart)
      QueryPerformanceCounter(&tick_after);

    this->t = ((double)(this->tick_after.QuadPart - this->initial_time)) / TICKS_PER_SECOND.QuadPart;

    /*
    this->time_now = clock();
    while (this->final_time > this->time_now)
    this->time_now = clock();

    this->t = this->time_now - this->initial_time;
    */
#endif
#ifdef linux
    this->time_now = clock();
    while (this->final_time > this->time_now)
      this->time_now = clock();

    this->t = this->time_now - this->initial_time;
#endif
    this->tempo2 = clock() - this->tempo;
  };

  clock_t get_current_time()
  {
    this->time_now = clock();
    return this->time_now;
  };

 double get_current_time_f()
  {   
    return ((double)(clock() - this->time_0)) / CLOCKS_PER_SEC;
  };

   double get_delta_time()
  {   
     QueryPerformanceCounter(&tick_after);
      return ((double)(tick_after.QuadPart - this->initial_time)) / TICKS_PER_SECOND.QuadPart;
  };
};

void datalog_ZERO( std::vector< std::vector<double> >  & V , int r, int c){
    V =  std::vector< std::vector<double> >(r, std::vector<double>(c));
    for(int _r = 0 ; _r < r ; _r ++)
      for(int _c = 0 ; _c < c ; _c ++)
        V[_r][_c] = 0.0;
}

#define name_lastdate "datos/datos_atimx_%m%d%Y_%H%M%S.dat"
#define name_datos    "datos/last_data_atimx.dat"
void saveDatalog(std::string name,std::vector< std::vector<double> >  & V , int r, int c){

  
  time_t ttt;
  time(&ttt);
  char fecha[50];
  struct tm *tmp = localtime(&ttt);
  strftime(fecha, sizeof(fecha), "%m%d%Y_%H%M%S.dat", tmp);

  char name1[80];
  char name2[80];
  sprintf(name1,"datos/datos_%s_%s",name.c_str(),fecha);
  sprintf(name2,"datos/last_data_%s.dat",name.c_str());

  FILE *pFile_atimx = fopen(name2, "w");
  FILE *pFile_ld_atimx = fopen(name1, "w");

  for (int i_dt = 0; i_dt < r-1; i_dt++)
  {
    for (int i_datalogs = 0; i_datalogs < c; i_datalogs++)
    {
      fprintf(pFile_atimx, "%10.16e\t", V[i_dt][i_datalogs]);
      fprintf(pFile_ld_atimx, "%10.16e\t", V[i_dt][i_datalogs]);
    }
    fprintf(pFile_atimx, "\n");
    fprintf(pFile_ld_atimx, "\n");
  }

  fclose(pFile_atimx);
  fclose(pFile_ld_atimx);

}

#define ItALL for(ThreadExo * cur_ : threadsExo._all_ExoClass)

void esperar_n_seg(int sec)
{
    long endwait = clock() + sec * CLOCKS_PER_SEC;
    while (clock() < endwait)
    {
    }
}