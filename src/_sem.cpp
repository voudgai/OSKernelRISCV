//
// Created by os on 5/15/24.
//
#include "../h/_sem.hpp"
#include "../h/_riscV.hpp"

_list<_sem> _sem::allSemaphores;
uint64 _sem::numOfAllSemaphores = 0;

int _sem::generateWAITResponses(threadsSemStatus status)
{
    /* generates response from thread information after waking up */

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

    threadsSemStatus status = _thread::running->getThreadsSemStatus();
    _thread::running->setThreadsSemStatus(NON_WAITING);

    return generateWAITResponses(status);
}

int _sem::timedWait(uint64 maxTimeSleeping)
{
    if (--this->val >= 0)
        return SEM_DIDNTWAIT;

    timedBlock(maxTimeSleeping);

    threadsSemStatus status = _thread::running->getThreadsSemStatus();
    _thread::running->setThreadsSemStatus(NON_WAITING);

    return generateWAITResponses(status);
}

void _sem::block()
{
    _thread *old = _thread::running;

    queueBlocked.addLast(old);
    old->setThreadsSemStatus(WAITING);

    old->mySem = this;
    _thread::dispatch();
    old->mySem = nullptr;
}

void _sem::timedBlock(uint64 maxTimeSleeping)
{
    uint64 timeForRelease = maxTimeSleeping + _riscV::getSystemTime();

    _thread *old = _thread::running;

    old->semStatus = TIMEDWAITING;
    old->timeForWakingUp = timeForRelease;

    queueBlocked.addLast(old);
    _thread::listAsleepThreads.insert_sorted(_thread::running, _thread::smallerSleepTime, nullptr);

    numOfTimedWaiting++;
    old->mySem = this;

    _thread::dispatch();
    // thread returns from being blocked here:
    // if it was returned from sem destructor
    // or from timer or from unblock,
    // its semStatus will be changed from TIMEDWAITING

    old->mySem = nullptr;
    numOfTimedWaiting--;

    old->timeForWakingUp = 0;
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

    if (old->getThreadsSemStatus() == TIMEDWAITING)
        _thread::listAsleepThreads.removeSpec(old);

    old->mySem = nullptr;
    old->setThreadsSemStatus(unblockingCause);
    Scheduler::put(old);
}

void _sem::unblockedByTime(_thread *old) // called by function which wakes up threads in _thread class
{
    if (old->getThreadsSemStatus() != TIMEDWAITING)
        return;

    val++; // this is difference from normal unblocking; we need to increase val

    _thread::listAsleepThreads.removeSpec(old);
    _sem::queueBlocked.removeSpec(old);

    old->mySem = nullptr; // this would be done anyways after returning from dispatch
    old->setThreadsSemStatus(TIMEOUT);
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
