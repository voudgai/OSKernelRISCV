//
// Created by os on 5/15/24.
//
#include "../h/_sem.hpp"
#include "../h/riscv.hpp"

List<_sem> _sem::allSemaphores;
uint64 _sem::numOfAllSemaphores = 0;

int _sem::generateWAITResponses(_thread::semResponses response)
{
    /* generates response from thread information after waking up */

    if (response == _thread::NON_WAITING)
        return _sem::SEMDIDNTWAIT;

    if (response == _thread::WAITING)
        return _sem::SEMERROR;

    if (response == _thread::SEM_DELETED)
        return _sem::SEMDEAD;

    if (response == _thread::REGULARLY_WAITED)
        return _sem::SEMOKAY;

    return _sem::SEMUNEXPECTED; // should never happen
}

int _sem::wait()
{
    --this->val;
    if (this->val >= 0)
        return _sem::SEMDIDNTWAIT;

    block();

    _thread::semResponses response = _thread::running->getThreadsSemStatus();
    _thread::running->setThreadsSemStatus(_thread::NON_WAITING);

    return generateWAITResponses(response);
}

int _sem::timedWait(uint64 timeSleepingMost)
{
    if (--this->val >= 0)
        return _sem::SEMDIDNTWAIT;

    timedBlock(timeSleepingMost);

    _thread::semResponses response = _thread::running->getThreadsSemStatus();
    _thread::running->setThreadsSemStatus(_thread::NON_WAITING);

    return generateWAITResponses(response);
}

void _sem::block()
{
    _thread *old = _thread::running;

    queueBlocked.addLast(old);
    old->setThreadsSemStatus(_thread::WAITING); // promenicemo u destruktoru semafora ako se obrise u medjuvremenu

    old->mySem = this;
    _thread::dispatch();
    old->mySem = nullptr;
}

void _sem::timedBlock(uint64 timeSleepingAtMost)
{
    uint64 timeForRelease = timeSleepingAtMost + Riscv::getSystemTime();

    _thread *old = _thread::running;
    old->semStatus = _thread::TIMEDWAITING;

    queueBlocked.addLast(old);
    old->timeForWakingUp = timeForRelease;
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
        return 0; // 0 ako nikoga nije probudio
    unblock();
    return 1; // ako jeste
}

void _sem::unblock(_thread::semResponses unblockingCause)
{
    _thread *old = queueBlocked.removeFirst();

    if (old->getThreadsSemStatus() == _thread::TIMEDWAITING)
        _thread::listAsleepThreads.removeSpec(old);

    old->mySem = nullptr;
    old->setThreadsSemStatus(unblockingCause);
    Scheduler::put(old);
}

void _sem::unblockedByTime(_thread *old)
{
    if (old->getThreadsSemStatus() != _thread::TIMEDWAITING)
        return;

    val++; // this is difference from normal unblocking; we need to increase val

    _thread::listAsleepThreads.removeSpec(old);
    _sem::queueBlocked.removeSpec(old);

    // old->mySem = nullptr; // this would be done anyways after returning from dispatch
    old->setThreadsSemStatus(_thread::TIMEOUT);
    Scheduler::put(old);
}

void _sem::unblockAll_CLOSING()
{
    for (; val < 0; val++)
    {
        unblock(_thread::SEM_DELETED);
    }
}

// void _sem::unblockTimesUp()
// {
//     // TODO
//     //------------------------------------------------------------------------//------------------------------------------------------------------------

//     uint64 systemTime = Riscv::getSystemTime();
//     uint64 N = numOfTimedWaiting;
//     for (uint64 i = 0; i < N; i++)
//     {
//         _thread *old = queueTimedBlock.removeFirst();
//         if (old->timedWait_semTimeRelease <= systemTime)
//         {
//             old->semStatus = _thread::TIMEOUT;
//             old->timedWait_semTimeRelease = 0;
//             Scheduler::put(old);
//             numOfTimedWaiting--;
//             val++;
//         }
//         else
//         {
//             queueTimedBlock.addLast(old);
//         }
//     }
// }

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
