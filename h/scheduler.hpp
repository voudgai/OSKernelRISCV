#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

//#include "syscall_c.hpp"
#include "list.hpp"

class _thread;
class Scheduler{
public:
    static void put(_thread*);
    static _thread* get();
private:
    static List<_thread> queueThreads;
};

#endif