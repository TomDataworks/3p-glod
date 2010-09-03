#ifndef NAT_TIMER_H
#define NAT_TIMER_H
/***************************************************************************
 * Timer Code
 ***************************************************************************/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef _WIN32
#define TIMER_DATA LARGE_INTEGER
#else
#define TIMER_DATA double
#endif

inline void Timer_Start(TIMER_DATA* data) {
#ifdef _WIN32
    QueryPerformanceCounter(data);
#else
  struct timeval stamp;
  struct timezone zone;
  gettimeofday(&stamp, &zone);
  
  *data = (double) stamp.tv_sec + (((double)stamp.tv_usec) / 1000000.0f);
#endif
}

inline double Timer_Elapsed(TIMER_DATA* data) {
#ifdef _WIN32
    LARGE_INTEGER inow;
    LARGE_INTEGER ifreq;
    QueryPerformanceCounter(&inow);
    QueryPerformanceFrequency(&ifreq);
    double e = (double) ((inow.QuadPart - data->QuadPart));
    double f = (double) ifreq.QuadPart;
    return e / f;

#else
  struct timeval stamp;
  struct timezone zone;
  double end_stamp;

  gettimeofday(&stamp, &zone);
  end_stamp = (double) stamp.tv_sec + (((double)stamp.tv_usec) / 1000000.0f);

  return end_stamp - *data;
#endif
}

inline double Timer_Reset(TIMER_DATA* data) {
    double elapsed = Timer_Elapsed(data);
    Timer_Start(data);
    return elapsed;
}

/***************************************************************************
 * Timer Code
 ***************************************************************************/
class natFPS {
private:
    TIMER_DATA counter;
    int tick;
    int bucket_size;
    double last_fps;
public:
    natFPS() {
        Timer_Start(&counter);
        bucket_size = 0;
        tick = 0;
    }

    natFPS(int bs) {
        Timer_Start(&counter);
        bucket_size = bs;
        tick = 0;
    }

    double GetFPS() { return fps() ; }
    
    double fps() {
        tick++;
        if(tick == bucket_size) {
            last_fps = ((double)bucket_size) / Timer_Reset(&counter);
            tick = 0;
        }
        return last_fps;
    }
};

#endif
