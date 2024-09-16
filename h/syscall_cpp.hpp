#ifndef _s_call_cpp
#define _s_call_cpp

#include "syscall_c.h"

void *operator new(size_t);
void operator delete(void *);
class Thread
{
public:
    Thread(void (*body)(void *), void *arg);
    virtual ~Thread();
    int start();
    static void dispatch();
    static int sleep(time_t);

protected:
    Thread();
    virtual void run() {}

private:
    thread_t myHandle;
    void (*body)(void *);
    void *arg;
    static void run_wrapper(void *);
};
class Semaphore
{
public:
    Semaphore(unsigned init = 1);
    virtual ~Semaphore();
    int wait();
    int signal();
    int timedWait(time_t);
    int tryWait();

private:
    sem_t myHandle;
};

class PeriodicThread : public Thread
{
public:
    void terminate();

protected:
    PeriodicThread(time_t period);
    virtual void periodicActivation() {}

private:
    static void periodicActivation_wrapper(void *);
    time_t period;
};

class Console
{
public:
    static char getc();
    static void putc(char);
};
#endif