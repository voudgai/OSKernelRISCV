#include "../h/syscall_cpp.hpp"

Thread::Thread(void (*body)(void *), void *arg)
{
    this->arg = arg;
    this->body = body;
    this->myHandle = nullptr;
}

Thread::~Thread() {}
int Thread::start()
{
    if (myHandle != nullptr)
        return -1; // already called start for this thread
    if (body != nullptr)
    {
        return thread_create(&myHandle, body, arg);
    }
    else
    {
        return thread_create(&myHandle, Thread::run_wrapper, this);
    }
}

void Thread::dispatch()
{
    thread_dispatch();
}

int Thread::sleep(time_t timeForSleeping)
{
    return time_sleep(timeForSleeping);
}

Thread::Thread()
{
    myHandle = nullptr;
    body = nullptr;
    arg = nullptr;
}

void Thread::run_wrapper(void *ptr)
{
    if (ptr == nullptr)
        return;
    Thread *pthis = (Thread *)ptr;
    if (pthis == nullptr)
        return;
    pthis->run();
}
