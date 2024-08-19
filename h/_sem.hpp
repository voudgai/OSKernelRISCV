//
// Created by os on 5/15/24.
//

#ifndef PROJECT_FOR_REAL__SEM_HPP
#define PROJECT_FOR_REAL__SEM_HPP

#include "_thread.hpp"
#include "memoryAllocator.hpp"
#include "list.hpp"

class _sem
{
public:
    friend class Riscv;
    friend class _thread;
    void *operator new(size_t n) { return memoryAllocator::_kmalloc((memoryAllocator::SIZE_HEADER + n + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE); }
    void *operator new[](size_t n) { return memoryAllocator::_kmalloc((memoryAllocator::SIZE_HEADER + n + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE); }

    void operator delete(void *p) noexcept
    {
        ((_sem *)(p))->~_sem();
        memoryAllocator::_kmfree(p);
    }
    void operator delete[](void *p) noexcept { memoryAllocator::_kmfree(p); }

    explicit _sem(uint64 N = 1) : val(N)
    {
        allSemaphores.addLast(this);
        numOfAllSemaphores++;
    };

    ~_sem()
    {
        unblockAll_CLOSING();
        allSemaphores.removeSpec(this);
        numOfAllSemaphores--;
    }
    int wait();                           // blocks running thread if val is negative after decreasing
    int timedWait(uint64 timeForRelease); // ovaj timeForRelease ce biti systime + koliko ceka maksimalno
    int tryWait();                        // decreases val and checks if it would wait
    int signal();                         // deblocks one thread if val is zero or negative after increasing

    uint64 value() const { return val; }

    enum semWaitAnswers // possible answers for ALL wait functions
    {
        SEMDIDNTWAIT = 1,
        SEMOKAY = 0,
        SEMDEAD = -1,
        SEMTIMEOUT = -2,
        HANDLE_NULL = -3,
        SEMUNEXPECTED = -4,
        SEMERROR = -5,
    };

protected:
    void block();                                                                    // blocks a thread which is regularly waiting
    void timedBlock(uint64 timeSleepingAtMost);                                      // blocks a thread which is time_waiting
    void unblock(_thread::semResponses unblockingCause = _thread::REGULARLY_WAITED); // unblocks a thread from either one of the waits, and sets response
    void unblockedByTime(_thread *old);                                              // called from _thread when there is thread to be woken up which is timed_blocked
    void unblockAll_CLOSING();                                                       // unblocks all threads waiting here
    // void unblockTimesUp();

    int generateWAITResponses(_thread::semResponses response); // depending on threads state when returning from block,
                                                               // generates return values for wait functions

private:
    volatile long int val;
    List<_thread> queueBlocked; // queue of ALL blocked threads

    volatile uint64 numOfTimedWaiting = 0; // just for statistics

    static List<_sem> allSemaphores;  // UNUSED
    static uint64 numOfAllSemaphores; // UNUSED
};

#endif // PROJECT_FOR_REAL__SEM_HPP
