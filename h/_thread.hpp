//
// Created by marko on 20.4.22..
//

#ifndef OS1_VEZBE07__riscV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP
#define OS1_VEZBE07__riscV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP

#include "../lib/hw.h"
#include "_scheduler.hpp"
#include "_memoryAllocator.hpp"
#include "_sem.hpp"

class _sem;
class _riscV;
class _time;
// Thread Control Block
class _thread
{

    friend class _sysCallsHandler;
    friend class _time;

    void priority_print(const char *s);
    using Body = void (*)(void *);

public:
    // possible thread states
    enum threadStateTypes : uint8
    {
        RUNNING,
        READY,
        SUSPENDED,
        FINISHED
    };

    // check if thread is valid, checks if its nullptr and if its magic number has changed
    static inline bool isThreadValid(const _thread *thr) { return (thr && THREAD_MAGIC_NUMBER == thr->myMagicNumber); }

    void *operator new(size_t n) { return _memoryAllocator::_kmalloc((_memoryAllocator::SIZE_HEADER + n + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE); }
    void *operator new[](size_t n) { return _memoryAllocator::_kmalloc((_memoryAllocator::SIZE_HEADER + n + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE); }

    void operator delete(void *p) noexcept { _memoryAllocator::_kmfree(p); }
    void operator delete[](void *p) noexcept { _memoryAllocator::_kmfree(p); }

    ~_thread() { disableThread(); }

    // check if thread is FINISHED or set its state to FINISHED
    inline bool isFinished() const { return isThreadValid(this) == false || threadState == FINISHED; }
    inline void setFinished();
    inline bool readyToRun() { return threadState == READY || threadState == RUNNING; }

    // check if thread is somehow related to some semaphore (waiting,etc...), and set its status
    inline _sem::threadsSemStatus get_threadsSemStatus() const { return semStatus; }
    inline void set_threadsSemStatus(_sem::threadsSemStatus status);

    // change threadState of particular thread
    inline static int set_runningThread_state(threadStateTypes newState);
    inline int set_threadState(threadStateTypes newState);

    // set timeForWakingUp of particular thread
    inline static int set_runningThread_timeForWakingUp(uint64 time);
    inline int set_timeForWakingUp(uint64 time);

    // get timeForWakingUp of particular thread
    inline static uint64 get_runningThread_timeForWakingUp() { return running ? running->timeForWakingUp : 0; }
    inline uint64 get_timeForWakingUp() { return this->timeForWakingUp; }

    // get pointer to running thread
    static _thread *get_runningThread() { return running; }

    // get dedicated time slice of particular thread
    inline uint64 getTimeSlice() const { return timeSlice; }

    // create a thread
    static _thread *createThread(Body body, void *arg, uint64 *stack_space);

    // kill a thread
    static int subtleKill(_thread *threadToBeKilled);

    // set or get semaphore which is running thread on
    inline static _sem *get_runningThread_sem() { return running ? running->mySem : nullptr; }
    inline static void set_runningThread_sem(_sem *sem);

    // set or get semaphore which is this thread on
    inline _sem *get_mySem() { return this->mySem; }
    inline void set_mySem(_sem *sem);

    inline void *get_bodyPtr() { return (void *)(this->body); }

private:
    static _thread *running;
    _thread(Body body, void *arg, uint64 *stack_space) : body(body),
                                                         arg(arg),
                                                         stack(stack_space != nullptr ? (stack_space - STACK_SIZE) : nullptr),
                                                         context({(uint64)&threadWrapper,
                                                                  stack != nullptr ? (uint64)stack_space : 0}),
                                                         timeSlice(TIME_SLICE),
                                                         parentThread(running)
    {
        if (body)
            Scheduler::put(this); // only main has body == nullptr

        if (parentThread)
            parentThread->numOfChildren++; // if has parent, increase parents number of children

        if (running == nullptr && body == nullptr)
        {
            threadState = RUNNING; // we first make thread for main, so it will take the first running spot
            running = this;
        }

        myID = ++ID;                         // self explainatory
        myMagicNumber = THREAD_MAGIC_NUMBER; // sets magic number, changes it in destructor

        if (body == nullptr || (parentThread && parentThread->body == nullptr))
            numOfSystemThreads++; // if thread is main or its made by main (main has body == nullptr) then its system thread
    }

    struct Context
    {
        uint64 ra; // return address for this thread for context switch
        uint64 sp; // stack_pointer for this thread for context switch
    };

    Body body = nullptr;     // body of thread, function that thread will process
    void *arg = nullptr;     // argument for threads body
    uint64 *stack = nullptr; // pointer to stack allocated for this thread
    Context context;         // context of this thread, valueable for context switching
    uint64 timeSlice = 0;    // how much time this thread gets on CPU before preemption

    threadStateTypes threadState = READY; // threadState, says a lot about thread

    uint64 myID;              // id of this thread
    uint64 myMagicNumber = 0; // magic number, used to check if any pointer is actually a thread

    _thread *parentThread = nullptr; // pointer to parent thread
    uint64 numOfChildren = 0;        // number of threads this thread made and are still not dead

    _sem::threadsSemStatus semStatus = _sem::NON_WAITING; // semaphore status for this thread
    _sem *mySem = nullptr;                                // semaphore on which this thread is waiting

    uint64 timeForWakingUp = 0; // for thread_sleep and timed_wait

    void disableThread();

    static void threadWrapper();
    static void contextSwitch(Context *oldContext, Context *runningContext);
    static void dispatch();
    static void exit();

    static uint64 constexpr STACK_SIZE = DEFAULT_STACK_SIZE / sizeof(uint64);
    static uint64 constexpr TIME_SLICE = DEFAULT_TIME_SLICE;

    static uint64 ID;
    static uint64 numOfSystemThreads;

    static constexpr uint64 THREAD_MAGIC_NUMBER = 0xf832d45809acdeef;
    static constexpr uint64 THREAD_DUMP_MAGIC_NUMBER = 0xacdcacdcacdcacdc;
    static constexpr int THREAD_IS_INVALID_ERR = -10;

    static bool threadDEAD(_thread *thr, void *ptr);              // used for foreachWhile in queueThreads in dispatch(), Scheduling
    static void deleteThread_inDispatch(_thread *thr, void *ptr); // used for foreachWhile in queueThreads in dispatch(), Scheduling
    // while threadDEAD do deleteThread_inDispatch
};

inline void _thread::setFinished()
{
    if (isThreadValid(this) == false)
        return;
    threadState = FINISHED;
}

inline void _thread::set_threadsSemStatus(_sem::threadsSemStatus status)
{
    if (isThreadValid(this) == false || this->isFinished())
        return; // if thread is already finished or invalid just return

    semStatus = status; // set new semStatus

    if (semStatus == _sem::WAITING || semStatus == _sem::TIMEDWAITING)
    {
        threadState = SUSPENDED; // if we set new status to some kind of wait thread is SUSPENDED
        return;                  // and we return since updating threadState is over
    }

    if (running == this)
        threadState = RUNNING; // if thread we are doing this to is currently running
    else
        threadState = READY; // else
}

inline int _thread::set_runningThread_state(threadStateTypes newState)
{
    if (isThreadValid(running) == false)
        return -1;
    running->threadState = newState;
    return 0;
}

inline int _thread::set_threadState(threadStateTypes newState)
{
    if (isThreadValid(this) == false)
        return -1;
    this->threadState = newState;
    return 0;
}

inline int _thread::set_runningThread_timeForWakingUp(uint64 time)
{
    if (isThreadValid(running) == false)
        return -1;
    running->timeForWakingUp = time;
    return 0;
}

inline int _thread::set_timeForWakingUp(uint64 time)
{
    if (isThreadValid(this) == false)
        return -1;
    this->timeForWakingUp = time;
    return 0;
}

inline void _thread::set_runningThread_sem(_sem *sem)
{
    if (running)
        running->set_mySem(sem);
}

inline void _thread::set_mySem(_sem *sem)
{
    if (isThreadValid(this) == false)
        return;
    this->mySem = sem;
}
#endif // OS1_VEZBE07__riscV_CONTEXT_SWITCH_2_INTERRUPT_TCB_HPP