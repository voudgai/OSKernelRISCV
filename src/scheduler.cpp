//
// Created by os on 5/14/24.
//
#include "../h/_scheduler.hpp"

_list<_thread> Scheduler::queueThreads;

_thread *Scheduler::get()
{
    return queueThreads.removeFirst();
}

void Scheduler::put(_thread *thread)
{
    queueThreads.addLast(thread);
}
