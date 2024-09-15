/*

#ifndef TIME__H
#define TIME__H

class _time
{
public:
    // Time from starting of system control
    static inline void inc_SYS_TIME(uint64 i = 1) { SYS_TIME += i; }
    static inline void get_SYS_TIME() { return SYS_TIME; }

    // Handler for interrupts by timer
    static void timer_interrupt_handler();

    // CPU usage by threads control
    static inline uint64 get_runningThread_CPU_time() { return runningThread_CPU_time; }
    static inline void inc_runningThread_CPU_time(uint64 i = 1) { runningThread_CPU_time += i; }
    static inline void reset_runningThread_CPU_time() { runningThread_CPU_time = 0; }

    // for thread sleeping
    static void put_runningThread_to_sleep(uint64 timeSleeping);
    static void put_thread_to_sleep(_thread *thr, uint64 timeSleeping);
    static void wake_asleep_threads_up();

private:
    static uint64 SYS_TIME;               // time counter since starting of system
    static uint64 runningThread_CPU_time; // how long is running thread running

    static _list<_thread> listAsleepThreads; // list of asleep threads
    static uint64 numOfThreadsAsleep;        // number of asleep threads

    // could check if they are valid, but will check that in Scheduler anyways :
    static bool smallerSleepTime(_thread *thrA, _thread *thrB, void *aux) { return (thrA->timeForWakingUp < thrB->timeForWakingUp); } // used for inserting sorted in listAsleepThreads

    // while shouldWakeUpThread do wakeThreadUp
    static bool shouldWakeUpThread(_thread *thr, void *systemTimePtr); // used for foreachWhile in listAsleepThreads
    static void wakeThreadUp(_thread *thr, void *ptr);                 // used for foreachWhile in listAsleepThreads
};

#endif
*/