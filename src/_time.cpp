#include "../h/_time.hpp"

uint64 _time::SYS_TIME = 0;               // time counter since starting of system
uint64 _time::runningThread_CPU_time = 0; // how long is running thread running
_list<_thread> _time::listAsleepThreads;  // list of asleep threads
uint64 _time::numOfThreadsAsleep = 0;     // number of asleep threads

void _time::timer_interrupt_handler()
{
    _riscV::mc_sip(_riscV::SIP_SSIP); // reset register for interrupt pending

    inc_sys_time();               // inc system time
    inc_runningThread_CPU_time(); // inc running thread running time
    wake_asleep_threads_up();     // wake-up asleep threads if needed (timed_wait + time_sleep)

    // _contextSwitch::preemption();
    if (get_runningThread_CPU_time() >= _thread::running->getTimeSlice())
        _context::dispatch(); // preemption if needed
}

void _time::put_runningThread_to_sleep(uint64 timeSleeping)
{
    put_thread_to_sleep(_thread::get_runningThread(), timeSleeping);
}

void _time::put_thread_to_sleep(_thread *thr, uint64 timeSleeping)
{
    if (_thread::isThreadValid(thr) == false)
        return;

    thr->set_threadState(_thread::SUSPENDED);                // set threadState to SUSPENDED
    thr->set_timeForWakingUp(get_sys_time() + timeSleeping); // set time for waking up
    numOfThreadsAsleep++;                                    // inc number of asleep threads

    listAsleepThreads.insert_sorted(thr, smallerSleepTime, nullptr); // insert sorted by ascending sleep time
    _context::dispatch();                                            // change context
    // _contextSwitch::dispatch()

    thr->set_threadState(_thread::READY); // we returned to this threads context, set its state to READY
    thr->set_timeForWakingUp(0);          // reset timeForWakingUp
    numOfThreadsAsleep--;                 // decrease num of asleep threads since this one just woke up
}

void _time::wake_asleep_threads_up()
{
    listAsleepThreads.foreachWhile(wakeThreadUp, nullptr, shouldWakeUpThread, nullptr);
}

bool _time::shouldWakeUpThread(_thread *thr, void *systemTimePtr)
{
    return (thr->get_timeForWakingUp() <= get_sys_time());
}

void _time::wakeThreadUp(_thread *thr, void *ptr)
{
    if (!thr || _thread::isThreadValid(thr) == false)
        return; // if not valid thread return

    if (thr->get_threadsSemStatus() == _sem::TIMEDWAITING)
    {
        if (thr->get_mySem() != nullptr)
            (thr->get_mySem())->unblockedByTime(thr); // if thr was on timedWait unblock it properly
        else
            ; // exception, how can it be on timedWait without sem
    }
    else if (listAsleepThreads.removeSpec(thr) == true)
    {
        Scheduler::put(thr); // if it was just sleeping, put it back in scheduler
                             // threadState will change when thr becomes running
    }
}
