//
// Created by os on 5/14/24.
//
#include "../h/_thread.hpp"
#include "../h/riscv.hpp"
#include "../h/syscall_c.h"

_thread *_thread::running = nullptr;
uint64 _thread::timeSliceCounter = 0;

List<_thread> _thread::listAsleepThreads;
uint64 _thread::numOfThreadsAsleep = 0;

uint64 _thread::ID = 0;

uint64 _thread::activeThreadsCounter = 0;
uint64 _thread::maxThreadsCounter = 0;
_sem *_thread::semThreadsCounter = nullptr;
List<_thread> _thread::listNotStartedThreads;

_thread *_thread::createThread(Body body, void *arg, uint64 *stack_space)
{
    return new _thread(body, arg, stack_space);
}

int _thread::subtleKill(_thread *threadToBeKilled)
{
    if (!_thread::running || !threadToBeKilled)
        return -1;
    if (_thread::running != threadToBeKilled->parentThread)
        return -2;
    if (_thread::running == threadToBeKilled)
        return -3;

    threadToBeKilled->setFinished(true); // scheduler will destroy it
    return 0;
}
int _thread::joinAll(uint64 sleepingAtMost)
{
    // we activate it either thru _thread::exit() or wake it up thru _thread::wakeAsleepThreads()
    joinedChildrenWAITING = true;
    if (numOfChildren > 0)
    {
        _thread::running->timeForWakingUp = Riscv::getSystemTime() + sleepingAtMost;
        numOfThreadsAsleep++;
        listAsleepThreads.addLast(this);

        dispatch();
    }
    joinedChildrenWAITING = false;
    if (listAsleepThreads.removeSpec(this) == 0) // if it was woken up, it wont be found here so it will not decrease value,
                                                 // however if it was activated by last son thread,
                                                 // then it will reduce the value of numSleeping
    {
        numOfThreadsAsleep--;
        return 1; // 1 for i was activated by my son-thread
    }

    return 0; // 0 if was woken up
}
int _thread::setMaximumThreads(uint64 num_of_threads, uint64 max_time, uint64 interval_time)
{
    // blocks them in thread wrapper if needed
    // unblocks in postponedThreadsActivatorThread
    static int working = 0;
    if (working)
        return -1;
    working = 1;
    maxThreadsCounter = num_of_threads;
    semThreadsCounter = new _sem(maxThreadsCounter);

    uint64 *info = new uint64[4];
    info[0] = num_of_threads;
    info[1] = max_time;
    info[2] = interval_time;
    info[3] = (uint64)&working;

    _thread::createThread(postponedThreadsActivatorThread, info, &((new uint64[512])[512]));
    return 0;
}
void _thread::postponedThreadsActivatorThread(void *ptr) // used for setMaximumThreads() method, ONLY there
{
    uint64 *info = (uint64 *)ptr;
    time_sleep(info[1]);
    printString("Waiting done... Starting threads...\n");

    maxThreadsCounter = 0;
    _sem *semToClose = semThreadsCounter;
    semThreadsCounter = nullptr;

    time_sleep(info[2]);
    while (sem_signal(semToClose))
    {
        printString("Interval time elapsed\n");
        activeThreadsCounter++;
        time_sleep(info[2]);
    }
    sem_close(semToClose);

    *((uint64 *)(info[3])) = 0;
}

void _thread::dispatch() // in dispatch(), if thread is finished, it frees its memory
{
    _thread *old = running;
    if (!old->isFinished() &&
        !old->isSleeping() &&
        !old->isJoined() &&
        old->getWaitingStatus() != WAITING &&
        old->getWaitingStatus() != TIMEDWAITING)
    {
        Scheduler::put(old);
    }
    if (old->isFinished())
    {
        activeThreadsCounter--;
        memoryAllocator::_kmfree(old);
    }
    running = Scheduler::get();
    while (running->isFinished() == true) // for the case we subtleKill() the thread which was still in Scheduler
    {
        activeThreadsCounter--;
        memoryAllocator::_kmfree(old);
        running = Scheduler::get();
    }
    _thread::contextSwitch(&old->context, &running->context);
}

void _thread::exit()
{
    _thread *parent = running->parentThread;
    if (parent != nullptr)
    {
        parent->numOfChildren--;
        if (parent->numOfChildren == 0 && parent->isJoined())
        {
            Scheduler::put(parent);
        }
    }

    running->setFinished(true);
    dispatch(); // in dispatch(), if thread is finished, it frees its memory
}

void _thread::putThreadToSleep(uint64 timeAsleep)
{
    _thread::running->sleeping = true;
    _thread::running->timeForWakingUp = Riscv::getSystemTime() + timeAsleep;

    numOfThreadsAsleep++;
    listAsleepThreads.addLast(_thread::running);
    dispatch();
}

void _thread::wakeAsleepThreads()
{
    uint64 systemTime = Riscv::getSystemTime();
    uint64 N = numOfThreadsAsleep;
    for (uint64 i = 0; i < N; i++)
    {
        _thread *old = listAsleepThreads.removeFirst();
        if (old->timeForWakingUp <= systemTime)
        {
            old->sleeping = false;
            old->timeForWakingUp = 0;
            numOfThreadsAsleep--;

            Scheduler::put(old);
        }
        else
        {
            listAsleepThreads.addLast(old);
        }
    }
}

void _thread::threadWrapper()
{

    {
        if (maxThreadsCounter > 0 && semThreadsCounter != nullptr)
            semThreadsCounter->wait();
        activeThreadsCounter++;
    }
    Riscv::popSppSpieChangeMod();

    running->body(running->arg);
    thread_exit();
}
