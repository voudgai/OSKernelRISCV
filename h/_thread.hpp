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

    void operator delete(void *p) noexcept { memoryAllocator::_kmfree(p); }
    void operator delete[](void *p) noexcept { memoryAllocator::_kmfree(p); }

    friend class Riscv;
    friend class _sem;

    ~_thread() { delete[] stack; }

    bool isFinished() const { return finished; }
    void setFinished(bool value) { finished = value; }

    bool isSleeping() const { return sleeping; }
    bool isJoined() const { return joinedChildrenWAITING; }

    semResponses getWaitingStatus() const { return waitResponse; }
    void setWaitingStatus(semResponses response) { waitResponse = response; }

    uint64 getTimeSlice() const { return timeSlice; }

    using Body = void (*)(void *);

    static _thread *createThread(Body body, void *arg, uint64 *stack_space);

    static _thread *running;

    // some modifications
    static int subtleKill(_thread *threadToBeKilled);
    int joinAll(uint64 sleepingAtMost);
    static void postponedThreadsActivatorThread(void *ptr);
    static int setMaximumThreads(uint64 num_of_threads, uint64 max_time, uint64 interval_time);

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

    uint64 numOfChildren = 0;
    bool joinedChildrenWAITING = false;

    semResponses waitResponse = NON_WAITING;
    uint64 timedWait_semTimeRelease = 0; // for timedWait(), latest time semaphore must release this thread,
                                         // this value doesnt mean much now, but only when timedwait is called

    bool sleeping = false; // for thread_sleep
    uint64 timeForWakingUp = 0;

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

    static uint64 activeThreadsCounter;
    static uint64 maxThreadsCounter;
    static _sem *semThreadsCounter;
    static List<_thread> listNotStartedThreads; // threads which are made after reaching maxThreadsCounter
};

#endif // OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP