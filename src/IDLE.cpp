#include "../h/syscall_c.h"

extern void mainCopeTest(void *p);
void funcWrapper(void *);
extern void userMain();
extern void mainCopeTest(void *p);

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
    ::userMain();
    // mainCopeTest(ptr);
    finished = true;
}
