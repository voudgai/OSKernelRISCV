//
// Created by os on 5/15/24.
//

#ifndef PROJECT_FOR_REAL__SEM_HPP
#define PROJECT_FOR_REAL__SEM_HPP

#include "_memoryAllocator.hpp"
#include "_list.hpp"

class _thread;
class _time;
class _sem
{
public:
    friend class _sysCallsHandler;
    friend class _time;
    void *operator new(size_t n) { return _memoryAllocator::_kmalloc((_memoryAllocator::SIZE_HEADER + n + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE); }
    void *operator new[](size_t n) { return _memoryAllocator::_kmalloc((_memoryAllocator::SIZE_HEADER + n + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE); }

    void operator delete(void *p) noexcept
    {
        // ((_sem *)(p))->~_sem();
        _memoryAllocator::_kmfree(p);
    }
    void operator delete[](void *p) noexcept { _memoryAllocator::_kmfree(p); }

    explicit _sem(uint64 N = 1) : val(N)
    {
        allSemaphores.addLast(this);
        numOfAllSemaphores++;
    };

    ~_sem()
    {
        unblockAll(SEM_DELETED);
        allSemaphores.removeSpec(this);
        numOfAllSemaphores--;
    }
    int wait();                           // blocks running thread if val is negative after decreasing
    int timedWait(uint64 timeForRelease); // ovaj timeForRelease ce biti systime + koliko ceka maksimalno
    int tryWait();                        // decreases val and checks if it would wait
    int signal();                         // deblocks one thread if val is zero or negative after increasing

    uint64 value() const { return val; }

    enum threadsSemStatus // possible status for threads
    {
        NON_WAITING,      // thread is not blocked on semaphore
        WAITING,          // thread is on semaphore by .wait()
        TIMEDWAITING,     // thread is on semaphore by .timedWait(time)
        REGULARLY_WAITED, // thread just finished waiting by .signal()
        TIMEOUT,          // thread was timedWaiting and time went out so it got woken up (unblockedByTime())
        SEM_DELETED       // thread was waiting on sem which got deleted
    };

    enum semWaitAnswers // possible answers for ALL wait functions
    {
        SEM_DIDNTWAIT = 1,
        SEM_OKAYWAITED = 0,
        SEM_DEAD = -1,
        SEM_TIMEOUT = -2,
        HANDLE_NULL = -3,
        SEM_UNEXPECTED = -4,
        SEM_ERROR = -5,
    };

protected:
    void block();                                                      // blocks a thread which is regularly waiting
    void timedBlock(uint64 timeSleepingAtMost);                        // blocks a thread which is time_waiting
    void unblock(threadsSemStatus unblockingCause = REGULARLY_WAITED); // unblocks a thread from either one of the waits, and sets response
    void unblockedByTime(_thread *old);                                // called from _thread when there is thread to be woken up which is timed_blocked
    void unblockAll(threadsSemStatus unblockingStatus);                // unblocks all threads waiting on this semaphore
    // void unblockTimesUp();

    int generateWAITResponses(); // depending on threads state when returning from block,
                                 // generates return values for wait functions

private:
    volatile long int val;
    _list<_thread> queueBlocked; // queue of ALL blocked threads

    volatile uint64 numOfTimedWaiting = 0; // just for statistics

    static _list<_sem> allSemaphores; // UNUSED
    static uint64 numOfAllSemaphores; // UNUSED
};

#endif // PROJECT_FOR_REAL__SEM_HPP
