//
// Created by marko on 20.4.22..
//

#ifndef OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP
#define OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP

#include "../lib/hw.h"
#include "scheduler.hpp"
#include "memoryAllocator.hpp"
#include "_sem.hpp"

class _sem;
class Riscv;
// Thread Control Block
class _thread
{
    void priority_print(const char *s);
    using Body = void (*)(void *);

public:
    static inline bool isThreadValid(const _thread *thr) { return (thr && THREAD_MAGIC_NUMBER == thr->myMagicNumber); }

    void *operator new(size_t n) { return memoryAllocator::_kmalloc((memoryAllocator::SIZE_HEADER + n + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE); }
    void *operator new[](size_t n) { return memoryAllocator::_kmalloc((memoryAllocator::SIZE_HEADER + n + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE); }

    void operator delete(void *p) noexcept { memoryAllocator::_kmfree(p); }
    void operator delete[](void *p) noexcept { memoryAllocator::_kmfree(p); }

    friend class Riscv;
    friend class _sem;

    ~_thread()
    {
        priority_print("~dst\n");
        // memoryAllocator::_kmfree((void *)stack); //doing it in disableThread()
        disableThread();
    }

    inline bool isFinished() const { return isThreadValid(this) == false || threadState == FINISHED; }
    inline void setFinished(bool value) { threadState = (value) ? FINISHED : threadState; }

    inline _sem::threadsSemStatus getThreadsSemStatus() const { return semStatus; }
    inline void setThreadsSemStatus(_sem::threadsSemStatus status)
    {
        semStatus = status;
        if (semStatus == _sem::WAITING || semStatus == _sem::TIMEDWAITING)
            threadState = SUSPENDED;
        else if (isFinished() == false)
        {
            if (running == this)
                threadState = RUNNING;
            else
                threadState = READY;
        }
    }

    inline uint64 getTimeSlice() const { return timeSlice; }

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
                                                         parentThread(running)
    {
        if (body)
            Scheduler::put(this);

        if (parentThread)
            parentThread->numOfChildren++;

        if (running == nullptr && body == nullptr)
        {
            threadState = RUNNING;
            running = this;
        }

        myID = ++ID;
        myMagicNumber = THREAD_MAGIC_NUMBER;

        if (body == nullptr || (parentThread && parentThread->body == nullptr)) // either its main or its made by main (main has body == nullptr)
        {
            numOfSystemThreads++;
            // priority_print(" one more kernel thread.\n\n");
        }
        // else{// priority_print(" one more user thread.\n\n");}
    }

    enum threadState : uint8
    {
        RUNNING,
        READY,
        SUSPENDED,
        FINISHED
    };

    struct Context
    {
        uint64 ra; // return address for this thread for context switch
        uint64 sp; // stack_pointer for this thread for context switch
    };

    Body body = nullptr;     // body of thread, function that thread will process
    void *arg = nullptr;     // argument for threads body
    uint64 *stack = nullptr; // pointer to stack allocated for this thread
    Context context;         // context of this thread, valueable for context switching
    uint64 timeSlice = 0;    // how much time is this thread on CPU already

    threadState threadState = READY;

    uint64 myID;          // id of this thread
    uint64 myMagicNumber; // magic number, used to check if any pointer is actually a thread

    _thread *parentThread = nullptr; // pointer to parent thread
    uint64 numOfChildren = 0;        // number of threads this thread made

    _sem::threadsSemStatus semStatus = _sem::NON_WAITING; // semaphore status for this thread
    _sem *mySem = nullptr;                                // semaphore on which this thread is waiting

    uint64 timeForWakingUp = 0; // for thread_sleep and timed_wait

    bool readyToRun();
    void disableThread();

    static void putThreadToSleep(uint64 timeAsleep);
    static void wakeAsleepThreads();
    static List<_thread> listAsleepThreads;
    static uint64 numOfThreadsAsleep;

    static void threadWrapper();
    static void contextSwitch(Context *oldContext, Context *runningContext);
    static void dispatch();
    static void exit();

    static uint64 timeSliceCounter;
    static inline uint64 getTimeSliceCounter() { return timeSliceCounter; }
    static inline void incTimeSliceCounter(uint64 i = 1) { timeSliceCounter += i; }
    static inline void resetTimeSliceCounter() { timeSliceCounter = 0; }

    static uint64 constexpr STACK_SIZE = DEFAULT_STACK_SIZE / sizeof(uint64);
    static uint64 constexpr TIME_SLICE = DEFAULT_TIME_SLICE;

    static uint64 ID;
    static uint64 numOfSystemThreads;

    static constexpr uint64 THREAD_MAGIC_NUMBER = 0xf832d45809acdeef;
    static constexpr uint64 THREAD_DUMP_MAGIC_NUMBER = 0xacdcacdcacdcacdc;
    static constexpr int THREAD_IS_INVALID_ERR = -10;

    // could check if they are valid, but will check that in Scheduler anyways :
    static bool smallerSleepTime(_thread *thrA, _thread *thrB, void *aux) { return (thrA->timeForWakingUp < thrB->timeForWakingUp); } // used for inserting sorted in listAsleepThreads

    static bool shouldWakeUpThread(_thread *thr, void *systemTimePtr); // used for foreachWhile in listAsleepThreads
    static void wakeThreadUp(_thread *thr, void *ptr);                 // used for foreachWhile in listAsleepThreads
    // while shouldWakeUpThread do wakeThreadUp

    static bool threadDEAD(_thread *thr, void *ptr);              // used for foreachWhile in queueThreads in dispatch(), Scheduling
    static void deleteThread_inDispatch(_thread *thr, void *ptr); // used for foreachWhile in queueThreads in dispatch(), Scheduling
    // while threadDEAD do deleteThread_inDispatch
};

#endif // OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP