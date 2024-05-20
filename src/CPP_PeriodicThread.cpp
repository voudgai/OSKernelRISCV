#include "../h/syscall_cpp.hpp"
void PeriodicThread::terminate()
{
    this->period = 0;
}
PeriodicThread::PeriodicThread(time_t periodWaiting) : Thread(PeriodicThread::periodicActivation_wrapper, this),
                                                       period(periodWaiting)
{
    if (periodWaiting == 0)
        this->period = 1;
}

void PeriodicThread::periodicActivation_wrapper(void *ptr)
{

    if (ptr == nullptr)
        return;
    PeriodicThread *pthread = (PeriodicThread *)ptr;

    while (pthread->period > 0)
    {
        pthread->periodicActivation();
        pthread->sleep(pthread->period);
    }
}