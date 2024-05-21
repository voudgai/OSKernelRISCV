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

_thread *_thread::createThread(Body body, void *arg, uint64 *stack_space)
{
    return new _thread(body, arg, stack_space);
}

int _thread::subtleKill(_thread *threadToBeKilled)
{
    if (_thread::running->body != nullptr)
        return -1; // only main function can kill other threads
    if (_thread::running == threadToBeKilled)
        return -1; // main cannot kill himself

    threadToBeKilled->setFinished(true);
    // Scheduler::queueThreads.removeSpec(threadToBeKilled);
    //  memoryAllocator::kmfree(threadToBeKilled);
    thread_dispatch();
    return 0;
}
void _thread::dispatch()
{
    _thread *old = running;
    if (!old->isFinished() &&
        !old->isSleeping() &&
        old->getWaitingStatus() != WAITING &&
        old->getWaitingStatus() != TIMEDWAITING)
    {
        Scheduler::put(old);
    }
    if (old->isFinished())
        memoryAllocator::_kmfree(old);
    running = Scheduler::get();
    while (running->isFinished() == true) // for the case we subtleKill() the thread which was still in Scheduler
    {
        running = Scheduler::get();
    }
    _thread::contextSwitch(&old->context, &running->context);
}

void _thread::exit()
{
    running->setFinished(true);
    /* in dispatch(), if thread is finished it frees its memory*/
    dispatch();
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
    Riscv::popSppSpieChangeMod();
    running->body(running->arg);
    thread_exit();
}
