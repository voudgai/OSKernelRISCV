//
// Created by os on 5/14/24.
//
#include "../h/_thread.hpp"
#include "../h/_riscV.hpp"
#include "../h/syscall_c.h"

_thread *_thread::running = nullptr;
uint64 _thread::ID = 0;
uint64 _thread::numOfSystemThreads = 0;

_thread *_thread::createThread(Body body, void *arg, uint64 *stack_space)
{
    return new _thread(body, arg, stack_space);
}

void _thread::threadWrapper()
{
    _riscV::popSppSpieChangeMod();
    running->body(running->arg);
    thread_exit();
}

void _thread::exit()
{
    if (running->parentThread)
        running->parentThread->numOfChildren--;
    running->setFinished();
    dispatch(); // in dispatch(), if thread is finished, it frees its memory
}

void _thread::dispatch() // in dispatch(), if thread is finished, it frees its memory
{
    _time::reset_runningThread_CPU_time();
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

    _riscV::pushRegisters();
    contextSwitch(&old->context, &running->context); // yes, old thread context memory may be freed,
                                                     // but we are still in supervisor mode so nobody will get chance to use it
    _riscV::popRegisters();
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
    _memoryAllocator::_kmfree(stack);
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

    threadToBeKilled->setFinished(); // scheduler will destroy it
    return 0;
}