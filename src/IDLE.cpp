#include "../h/syscall_c.h"

extern void mainCopeTest(void *p);
void funcWrapper(void *);
extern void userMain();
extern void mainCopeTest(void *p);
extern int modifikacijaPredrok();
extern int modifikacijaJun();
extern int modifikacijaAvg2023();

bool finished = false;

void idleThread(void *ptr)
{
    _thread *userMain_thread;
    thread_create(&userMain_thread, funcWrapper, nullptr);

    while (!finished)
    {
        thread_dispatch();
    }
}
void funcWrapper(void *ptr)
{
    //::modifikacijaAvg2023();
    //::modifikacijaJun();
    //::modifikacijaPredrok();
    ::userMain();
    // mainCopeTest(ptr);
    finished = true;
}
