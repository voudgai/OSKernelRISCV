//
// Created by marko on 20.4.22..
//

#ifndef OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP
#define OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP

#include "../lib/hw.h"
#include "scheduler.hpp"
#include "print.hpp"
#include "memoryAllocator.hpp"

// Thread Control Block
class _thread
{
public:
    static inline bool isThreadValid(_thread *thr) { return (thr && THREAD_MAGIC_NUMBER == thr->myMagicNumber); }

    enum semResponses
    {
        NON_WAITING,
        WAITING,
        TIMEDWAITING,
        TIMEOUT,
        REGULARLY_WAITED,
        SEM_DELETED
    };

    void *operator new(size_t n) { return memoryAllocator::_kmalloc((memoryAllocator::SIZE_HEADER + n + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE); }
    void *operator new[](size_t n) { return memoryAllocator::_kmalloc((memoryAllocator::SIZE_HEADER + n + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE); }

    void operator delete(void *p) noexcept
    {
        if (!p)
            return;
        static_cast<_thread *>(p)->~_thread();
        memoryAllocator::_kmfree(p);
    }

    void operator delete[](void *p) noexcept { memoryAllocator::_kmfree(p); } // should call destructor for every thread...

    friend class Riscv;
    friend class _sem;

    ~_thread()
    {
        disableThread();
    }
    void disableThread()
    {
        if (!isThreadValid(this))
            return; // already disabled
        myMagicNumber = THREAD_DUMP_MAGIC_NUMBER;
        memoryAllocator::_kmfree(stack);
    }

    inline bool isFinished() const { return finished; }
    inline void setFinished(bool value) { finished = value; }

    inline bool isSleeping() const { return sleeping; }

    inline semResponses getThreadsSemStatus() const { return semStatus; }
    inline void setThreadsSemStatus(semResponses response) { semStatus = response; }

    inline uint64 getTimeSlice() const { return timeSlice; }

    using Body = void (*)(void *);

    static _thread *createThread(Body body, void *arg, uint64 *stack_space);
    static _thread *running;

    static int subtleKill(_thread *threadToBeKilled);

private:
    _thread(Body body, void *arg, uint64 *stack_space) : body(body),
                                                         arg(arg),
                                                         stack(stack_space != nullptr ? (stack_space - STACK_SIZE) : nullptr),
                                                         context({(uint64)&threadWrapper,
                                                                  stack != nullptr ? (uint64)stack_space : 0}),
                                                         timeSlice(TIME_SLICE),
                                                         finished(false),
                                                         parentThread(running)
    {
        if (body != nullptr)
        {
            Scheduler::put(this);
        }
        if (parentThread != nullptr)
        {
            parentThread->numOfChildren++;
        }
        if (_thread::running == nullptr && body == nullptr)
        {
            _thread::running = this;
        }
        myID = ID++;
        myMagicNumber = THREAD_MAGIC_NUMBER;
    }

    struct Context
    {
        uint64 ra;
        uint64 sp;
    };

    Body body;
    void *arg;
    uint64 *stack;
    Context context;
    uint64 timeSlice;
    bool finished;
    _thread *parentThread;
    uint64 myID;
    uint64 myMagicNumber;

    uint64 numOfChildren = 0;

    _sem *mySem = nullptr;
    semResponses semStatus = NON_WAITING;
    bool sleeping = false;      // for thread_sleep, if false it can anyway be timed_waiting
    uint64 timeForWakingUp = 0; //  for thread_sleep and timed_wait

    static void putThreadToSleep(uint64 timeAsleep);
    static void wakeAsleepThreads();
    static List<_thread> listAsleepThreads;
    static uint64 numOfThreadsAsleep;

    static void threadWrapper();
    static void contextSwitch(Context *oldContext, Context *runningContext);
    static void dispatch();
    static void exit();
    bool readyToRun();

    static uint64 timeSliceCounter;
    static inline uint64 getTimeSliceCounter() { return timeSliceCounter; }
    static inline void incTimeSliceCounter(uint64 i = 1) { timeSliceCounter += i; }
    static inline void resetTimeSliceCounter() { timeSliceCounter = 0; }

    static uint64 constexpr STACK_SIZE = DEFAULT_STACK_SIZE / sizeof(uint64);
    static uint64 constexpr TIME_SLICE = DEFAULT_TIME_SLICE;

    static uint64 ID;

    static constexpr uint64 THREAD_MAGIC_NUMBER = 0xf832d45809acdeef;
    static constexpr uint64 THREAD_DUMP_MAGIC_NUMBER = 0xacdcacdcacdcacdc;
    static constexpr int THREAD_IS_INVALID_ERR = -10;

    // helper functions:
    // if (!(thrA && thrB && isThreadValid(thrA) && isThreadValid(thrB)))...
    static bool smallerSleepTime(_thread *thrA, _thread *thrB, void *aux) { return (thrA->timeForWakingUp < thrB->timeForWakingUp); } // used for inserting sorted in listAsleepThreads

    static bool shouldWakeUpThread(_thread *thr, void *systemTimePtr); // used for foreachWhile in listAsleepThreads
    static void wakeThreadUp(_thread *thr, void *ptr);                 // used for foreachWhile in listAsleepThreads

    static bool threadDEAD(_thread *thr, void *ptr);              // used for foreachWhile in queueThreads in dispatch(), Scheduling
    static void deleteThread_inDispatch(_thread *thr, void *ptr); // used for foreachWhile in queueThreads in dispatch(), Scheduling
};

#endif // OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP