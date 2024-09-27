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

void _thread::disableThread()
{
    if (!isThreadValid(this))
        return; // already disabled
    myMagicNumber = THREAD_DUMP_MAGIC_NUMBER;
    _memoryAllocator::_kmfree(stack);
}