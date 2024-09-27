#include "../h/_context.hpp"
#include "../h/_thread.hpp"
#include "../h/_riscV.hpp"

void _context::contextSwitch(_context *oldContext, _context *runningContext)
{
    __asm__ volatile("sd ra, %[currRA]" : : [currRA] "m"(oldContext->ra));
    __asm__ volatile("sd sp, %[currSP]" : : [currSP] "m"(oldContext->sp));

    __asm__ volatile("ld ra, %[newRA]" : [newRA] "=m"(runningContext->ra));
    __asm__ volatile("ld sp, %[newSP]" : [newSP] "=m"(runningContext->sp));
}

void _context::exit()
{
    if (_thread::running->parentThread)
        _thread::running->parentThread->numOfChildren--;
    _thread::running->setFinished();
    dispatch(); // in dispatch(), if thread is finished, it frees its memory
}

void _context::dispatch() // in dispatch(), if thread is finished, it frees its memory
{
    _time::reset_runningThread_CPU_time();
    _thread *old = _thread::running;
    if (old->readyToRun())
    {
        old->threadState = _thread::READY;
        Scheduler::put(old);
    }

    if (old->isFinished())
    {
        delete old;
    }

    Scheduler::queueThreads.foreachWhile(deleteThread_inDispatch, nullptr, threadDEAD, nullptr);
    _thread::running = Scheduler::get();
    _thread::running->threadState = _thread::RUNNING;

    _riscV::pushRegisters();
    contextSwitch(&(old->context), &(_thread::running->context)); // yes, old thread context memory may be freed,
                                                                  // but we are still in supervisor mode so nobody will get chance to use it
    _riscV::popRegisters();
}

bool _context::threadDEAD(_thread *thr, void *ptr)
{
    return !thr || !_thread::isThreadValid(thr) || thr->isFinished();
}

void _context::deleteThread_inDispatch(_thread *thr, void *systemTimePtr)
{
    if (thr == nullptr)
        return;

    if (Scheduler::queueThreads.removeSpec(thr) == true &&
        _thread::isThreadValid(thr) &&
        thr->isFinished())
    {
        delete thr;
    }
}

int _context::subtleKill(_thread *threadToBeKilled)
{
    if (!threadToBeKilled || !_thread::isThreadValid(threadToBeKilled))
        return _thread::THREAD_IS_INVALID_ERR;
    if (!_thread::running)
        return -1;
    if (_thread::running != threadToBeKilled->parentThread)
        return -2;
    if (_thread::running == threadToBeKilled)
        return -3;

    threadToBeKilled->setFinished(); // scheduler will destroy it
    return 0;
}