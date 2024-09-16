//
// Created by os on 5/15/24.
//
#include "../h/_sem.hpp"
#include "../h/_riscV.hpp"

_list<_sem> _sem::allSemaphores;
uint64 _sem::numOfAllSemaphores = 0;

int _sem::generateWAITResponses()
{
    /* generates response from thread information after waking up */

    threadsSemStatus status = _thread::get_runningThread()->get_threadsSemStatus();
    _thread::get_runningThread()->set_threadsSemStatus(NON_WAITING);

    if (status == NON_WAITING)
        return _sem::SEM_DIDNTWAIT; // it did not wait

    if (status == WAITING || status == TIMEDWAITING)
        return _sem::SEM_ERROR; // how can it be still waiting if its unblocked

    if (status == SEM_DELETED)
        return _sem::SEM_DEAD; // semaphore was deleted and thats how all threads got released

    if (status == REGULARLY_WAITED)
        return _sem::SEM_OKAYWAITED; // thread was unblocked by signal

    if (status == TIMEOUT)
        return _sem::SEM_TIMEOUT; // thread was unblocked by time

    return _sem::SEM_UNEXPECTED; // should never happen
}

int _sem::wait()
{
    --this->val;
    if (this->val >= 0)
        return SEM_DIDNTWAIT;

    block();

    return generateWAITResponses();
}

int _sem::timedWait(uint64 maxTimeSleeping)
{
    if (--this->val >= 0)
        return SEM_DIDNTWAIT;

    timedBlock(maxTimeSleeping);

    return generateWAITResponses();
}

void _sem::block()
{
    _thread *old = _thread::get_runningThread();

    queueBlocked.addLast(old);
    old->set_threadsSemStatus(WAITING);
    old->set_mySem(this);

    _thread::dispatch();

    old->set_mySem(nullptr);
}

void _sem::timedBlock(uint64 maxTimeSleeping)
{
    uint64 timeForRelease = maxTimeSleeping + _time::get_sys_time();
    _thread *old = _thread::get_runningThread();

    old->set_threadsSemStatus(TIMEDWAITING);
    old->set_timeForWakingUp(timeForRelease);

    queueBlocked.addLast(old);
    _time::insert_ordered_in_list_of_sleeping_threads(old);

    numOfTimedWaiting++;
    old->set_mySem(this);

    _thread::dispatch();
    // thread returns from being blocked here:
    // if it was returned from sem destructor
    // or from timer or from unblock,
    // its semStatus will be changed from TIMEDWAITING

    old->set_mySem(nullptr);
    numOfTimedWaiting--;

    old->set_timeForWakingUp(0);
}

int _sem::signal()
{
    ++this->val;
    if (this->val > 0)
        return 0; //  returns 0 if theres nobody to activate
    unblock(REGULARLY_WAITED);
    return 1; // returns 1 if theres somebody
}

void _sem::unblock(threadsSemStatus unblockingCause)
{
    _thread *old = queueBlocked.removeFirst();

    if (old->get_threadsSemStatus() == TIMEDWAITING)
        _time::remove_thread_from_sleep(old);

    old->set_mySem(nullptr);
    old->set_threadsSemStatus(unblockingCause);
    Scheduler::put(old);
}

void _sem::unblockedByTime(_thread *old) // called by function which wakes up threads in _thread class
{
    if (old->get_threadsSemStatus() != TIMEDWAITING)
        return; // how can it be unblocked by time if its not timed waiting

    val++; // this is difference from normal unblocking; we need to increase val

    _time::remove_thread_from_sleep(old);
    queueBlocked.removeSpec(old);

    old->set_mySem(nullptr); // this would be done anyways after returning from dispatch
    old->set_threadsSemStatus(TIMEOUT);
    Scheduler::put(old);
}

void _sem::unblockAll(threadsSemStatus unblockingCause = SEM_DELETED)
{
    for (; val < 0; val++)
        unblock(unblockingCause);
}

int _sem::tryWait()
{
    enum Try_Wait
    {
        WOULD_WAIT = 0,
        WOULD_NOT_WAIT = 1
    };

    if (this->val-- < 0)
        return WOULD_WAIT;
    else
        return WOULD_NOT_WAIT;
}
