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

_thread *_thread::createThread(Body body, void *arg, uint64 *stack_space)
{
    return new _thread(body, arg, stack_space);
}

void _thread::threadWrapper()
{
    Riscv::popSppSpieChangeMod();
    running->body(running->arg);
    thread_exit();
}

void _thread::exit()
{
    running->setFinished(true);
    dispatch(); // in dispatch(), if thread is finished, it frees its memory
}

bool _thread::readyToRun() { return !this->finished && !this->sleeping /*&& ! old->isJoined()  */
                                    && !(this->semStatus == WAITING || this->semStatus == TIMEDWAITING); }

void _thread::dispatch() // in dispatch(), if thread is finished, it frees its memory
{
    resetTimeSliceCounter();
    _thread *old = running;
    if (old->readyToRun())
        Scheduler::put(old);

    if (old->isFinished())
    {
        delete old;
    }

    running = Scheduler::get();
    while (running->isFinished() || !isThreadValid(running)) // for the case we subtleKill() the thread which was still in Scheduler
    {
        _thread *thrToDel = running;
        if (isThreadValid(thrToDel))
        {
            thrToDel->disableThread();
            delete thrToDel;
        }
        running = Scheduler::get();
    }
    _thread::contextSwitch(&old->context, &running->context); // yes, old thread context memory may be freed,
                                                              // but we are still in supervisor mode so nobody will get chance to use it
}

void _thread::putThreadToSleep(uint64 timeForSleeping)
{
    running->sleeping = true;
    running->timeForWakingUp = Riscv::getSystemTime() + timeForSleeping;
    numOfThreadsAsleep++;

    listAsleepThreads.insert_sorted(_thread::running, smallerSleepTime, nullptr);
    dispatch();

    running->sleeping = false;
    running->timeForWakingUp = 0;
    numOfThreadsAsleep--;
}

void _thread::wakeAsleepThreads()
{
    listAsleepThreads.foreachWhile(wakeThreadUp, nullptr, shouldWakeUpThread, nullptr);
}

bool _thread::shouldWakeUpThread(_thread *thr, void *systemTimePtr)
{
    return (thr->timeForWakingUp <= Riscv::getSystemTime());
}

void _thread::wakeThreadUp(_thread *thr, void *ptr)
{
    if (!thr || !isThreadValid(thr))
        return;

    if (thr->getThreadsSemStatus() == TIMEDWAITING)
    {
        if (thr->mySem != nullptr)
            (thr->mySem)->unblockedByTime(thr);
    }
    else if (listAsleepThreads.removeSpec(thr) == true)
    {
        Scheduler::put(thr);
    }
}

int _thread::subtleKill(_thread *threadToBeKilled)
{
    if (!threadToBeKilled || !isThreadValid(threadToBeKilled))
        return THREAD_IS_INVALID_ERR;
    if (!_thread::running)
        return -1;
    if (_thread::running != threadToBeKilled->parentThread)
        return -2;
    if (_thread::running == threadToBeKilled)
        return -3;

    threadToBeKilled->setFinished(true); // scheduler will destroy it
    return 0;
}