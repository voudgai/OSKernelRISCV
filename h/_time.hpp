#ifndef TIME__H
#define TIME__H

#include "_thread.hpp"
#include "_sem.hpp"
#include "_riscV.hpp"

class _sem;
class _thread;
class _time
{
public:
    // Handler for interrupts by timer
    static void timer_interrupt_handler();

    // Time from starting of system control
    static inline uint64 get_sys_time() { return SYS_TIME; }

    // CPU usage by threads control
    static inline uint64 get_runningThread_CPU_time() { return runningThread_CPU_time; }
    static inline void reset_runningThread_CPU_time() { runningThread_CPU_time = 0; }

    // for thread sleeping
    static void put_runningThread_to_sleep(uint64 timeSleeping);
    static void put_thread_to_sleep(_thread *thr, uint64 timeSleeping);

    // for timedWait
    static inline bool remove_thread_from_sleep(_thread *thr) { return listAsleepThreads.removeSpec(thr); } // returns true if found and removed it
    static inline bool insert_ordered_in_list_of_sleeping_threads(_thread *thr);                            // insert sorted by ascending sleep time

private:
    // time control - incrementation of timers happens ONLY in timer_interrupt_handler()
    static inline void inc_sys_time(uint64 i = 1) { SYS_TIME += i; }
    static inline void inc_runningThread_CPU_time(uint64 i = 1) { runningThread_CPU_time += i; }

    // could check if threads in listAsleep are valid, but will check that in Scheduler anyways :
    static void wake_asleep_threads_up();
    static bool smallerSleepTime(_thread *thrA, _thread *thrB, void *aux) { return (thrA->get_timeForWakingUp() < thrB->get_timeForWakingUp()); } // used for inserting sorted in listAsleepThreads

    // foreach thread wakeThreadUp while shouldWakeUpThread (used for foreachWhile in listAsleepThreads)
    static bool shouldWakeUpThread(_thread *thr, void *systemTimePtr);
    static void wakeThreadUp(_thread *thr, void *ptr);

    // class fields
    static uint64 SYS_TIME;               // time counter since starting of system
    static uint64 runningThread_CPU_time; // how long is running thread running

    static _list<_thread> listAsleepThreads; // list of asleep threads
    static uint64 numOfThreadsAsleep;        // number of asleep threads
};
inline bool _time::insert_ordered_in_list_of_sleeping_threads(_thread *thr)
{
    if (_thread::isThreadValid(thr) == false)
        return false;
    listAsleepThreads.insert_sorted(thr, smallerSleepTime, nullptr);
    return true;
}
#endif