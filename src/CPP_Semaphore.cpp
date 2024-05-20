#include "../h/syscall_cpp.hpp"

Semaphore::Semaphore(unsigned init)
{
    sem_open(&myHandle, init);
}
Semaphore::~Semaphore()
{
    sem_close(myHandle);
}

int Semaphore::wait()
{
    if (myHandle == nullptr)
        return -10;
    return sem_wait(myHandle);
}

int Semaphore::signal()
{
    if (myHandle == nullptr)
        return -10;
    return sem_signal(myHandle);
}

int Semaphore::timedWait(time_t timeToWait)
{
    if (myHandle == nullptr)
        return -10;
    return sem_timedwait(myHandle, timeToWait);
}

int Semaphore::tryWait()
{
    if (myHandle == nullptr)
        return -10;
    return sem_trywait(myHandle);
}
