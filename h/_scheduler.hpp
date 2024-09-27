#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

// #include "syscall_c.hpp"
#include "_list.hpp"

class _thread;
class _context;
class Scheduler
{
    friend class _context;

public:
    static void put(_thread *);
    static _thread *get();

private:
    static _list<_thread> queueThreads;
};

#endif