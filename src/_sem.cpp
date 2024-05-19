//
// Created by os on 5/15/24.
//
#include "../h/_sem.hpp"
#include "../h/riscv.hpp"

List<_sem> _sem::allSemaphores;
uint64 _sem::numOfAllSemaphores = 0;

void _sem::wait()
{
    --this->val;
    if (this->val >= 0)
        return;
    else
        block();
    return;
}

void _sem::timedWait(uint64 timeForRelease)
{
    if (--this->val >= 0)
        return;
    // lock();
    timedBlock(timeForRelease);
    // unlock();
}

void _sem::signal()
{
    ++this->val;
    if (this->val > 0)
        return;
    // lock();
    unblock();
}

void _sem::block()
{
    _thread *old = _thread::running;
    queueBlocked.addLast(old);
    // promenicemo u destruktoru semafora ako se obrise u medjuvremenu
    old->setWaitingStatus(_thread::WAITING);
    _thread::dispatch();

    // ovde se po pravilu dolazi iz prekidne rutine
    // pa ne moramo cuvati registre opet
    // jer su vec sacuvani pri ulasku u prekidnu
    // za stari, a za novi su svakako sacuvani
}

void _sem::timedBlock(uint64 timeSleepingAtMost)
{
    uint64 timeForRelease = timeSleepingAtMost + Riscv::getSystemTime();
    numOfTimedWaiting++;
    _thread *old = _thread::running;
    queueTimedBlock.addLast(old);
    old->timedWait_semTimeRelease = timeForRelease;
    // promenicemo u destruktoru semafora ako se obrise u medjuvremenu
    old->waitResponse = _thread::TIMEDWAITING;
    _thread::dispatch();
}

void _sem::unblock()
{
    _thread *old;
    if (numOfTimedWaiting > 0)
    {
        old = queueTimedBlock.removeFirst();
        old->timedWait_semTimeRelease = 0;
        numOfTimedWaiting--;
    }
    else
    {
        old = queueBlocked.removeFirst();
    }
    old->waitResponse = _thread::REGULARLY_WAITED;
    Scheduler::put(old);
    _thread::dispatch();
}

void _sem::unblockAll_CLOSING()
{
    while (queueTimedBlock.peekFirst() != nullptr)
    {
        _thread *old = queueBlocked.removeFirst();
        old->waitResponse = _thread::SEM_DELETED;
        old->timedWait_semTimeRelease = 0;
        Scheduler::put(old);
        numOfTimedWaiting--;
    }
    while (queueBlocked.peekFirst() != nullptr)
    {
        _thread *old = queueBlocked.removeFirst();
        old->waitResponse = _thread::SEM_DELETED;
        Scheduler::put(old);
    }
}

void _sem::unblockTimesUp()
{
    uint64 systemTime = Riscv::getSystemTime();
    uint64 N = numOfTimedWaiting;
    for (uint64 i = 0; i < N; i++)
    {
        _thread *old = queueTimedBlock.removeFirst();
        if (old->timedWait_semTimeRelease <= systemTime)
        {
            old->waitResponse = _thread::TIMEOUT;
            old->timedWait_semTimeRelease = 0;
            Scheduler::put(old);
            numOfTimedWaiting--;
        }
        else
        {
            queueTimedBlock.addLast(old);
        }
    }
}

int _sem::tryWait()
{
    enum Try_Wait
    {
        WOULD_WAIT = 0,
        WOULD_NOT_WAIT = 1
    };

    this->val--;
    if (val < 0)
        return WOULD_WAIT;
    else
        return WOULD_NOT_WAIT;
}
