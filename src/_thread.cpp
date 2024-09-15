//
// Created by os on 5/14/24.
//
#include "../h/_thread.hpp"
#include "../h/riscv.hpp"
#include "../h/syscall_c.h"

_thread *_thread::running = nullptr;
uint64 _thread::timeSliceCounter = 0;
_list<_thread> _thread::listAsleepThreads;
uint64 _thread::numOfThreadsAsleep = 0;
uint64 _thread::ID = 0;
uint64 _thread::numOfSystemThreads = 0;

void _thread::priority_print(const char *s)
{
    _console::empty_console_print_all();
    Riscv::priority_print("T_");
    Riscv::priority_print_int(myID);
    Riscv::priority_print(" : ");
    Riscv::priority_print(s);
}

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
    if (running->parentThread)
        running->parentThread->numOfChildren--;
    running->setFinished(true);
    dispatch(); // in dispatch(), if thread is finished, it frees its memory
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool _thread::readyToRun() { return threadState == READY || threadState == RUNNING; }
//{ return !this->finished && !this->sleeping && !(this->semStatus == _sem::WAITING || this->semStatus == _sem::TIMEDWAITING); }

void _thread::dispatch() // in dispatch(), if thread is finished, it frees its memory
{
    resetTimeSliceCounter();
    _thread *old = running;
    if (old->readyToRun())
    {
        old->threadState = READY;
        Scheduler::put(old);
    }

    if (old->isFinished())
    {
        delete old;
    }

    Scheduler::queueThreads.foreachWhile(deleteThread_inDispatch, nullptr, threadDEAD, nullptr);
    running = Scheduler::get();
    running->threadState = RUNNING;

    Riscv::pushRegisters();
    contextSwitch(&old->context, &running->context); // yes, old thread context memory may be freed,
                                                     // but we are still in supervisor mode so nobody will get chance to use it
    Riscv::popRegisters();
}

bool _thread::threadDEAD(_thread *thr, void *ptr)
{
    return !thr || !isThreadValid(thr) || thr->isFinished();
}

void _thread::disableThread()
{
    if (!isThreadValid(this))
        return; // already disabled
    myMagicNumber = THREAD_DUMP_MAGIC_NUMBER;
    memoryAllocator::_kmfree(stack);
}

void _thread::deleteThread_inDispatch(_thread *thr, void *systemTimePtr)
{
    if (thr == nullptr)
        return;

    if (Scheduler::queueThreads.removeSpec(thr) == true &&
        isThreadValid(thr) &&
        thr->isFinished())
    {
        delete thr;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

void _thread::putThreadToSleep(uint64 timeForSleeping)
{
    running->threadState = SUSPENDED;
    running->timeForWakingUp = Riscv::getSystemTime() + timeForSleeping;
    numOfThreadsAsleep++;

    listAsleepThreads.insert_sorted(running, smallerSleepTime, nullptr);
    dispatch();

    running->threadState = READY;
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

    if (thr->getThreadsSemStatus() == _sem::TIMEDWAITING)
    {
        if (thr->mySem != nullptr)
            (thr->mySem)->unblockedByTime(thr);
    }
    else if (listAsleepThreads.removeSpec(thr) == true)
    {
        Scheduler::put(thr);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------

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