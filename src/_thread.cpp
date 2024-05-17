//
// Created by os on 5/14/24.
//
#include "../h/_thread.hpp"
#include "../h/riscv.hpp"
#include "../h/syscall_c.hpp"


_thread *_thread::running = nullptr;

uint64 _thread::timeSliceCounter = 0;

_thread *_thread::createThread(Body body, void* arg, uint64* stack_space)
{
    return new _thread(body, arg, stack_space);
}

void _thread::dispatch()
{
    _thread *old = running;
    if (!old->isFinished() && old->getWaitingStatus() != WAITING && old->getWaitingStatus() != TIMEDWAITING)
        { Scheduler::put(old); }
    running = Scheduler::get();

    _thread::contextSwitch(&old->context, &running->context);
}

void _thread::exit()
{
    running->setFinished(true);
    dispatch();
}

void _thread::threadWrapper()
{
    //printString("THREAD WRAPPER\n");
    Riscv::popSppSpieChangeMod();
    running->body(running->arg);
    thread_exit();
}

void _thread::printThread() {
    printString("Ovo je thread sa funkcijom koja pocinje na mestu ");
    printInteger((uint64) body);
    printString( " a stek je na ");
    printInteger((uint64)stack);
    printString("\n");
}

