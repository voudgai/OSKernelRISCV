class _thread;
#include "../lib/hw.h"

#ifndef _CONTEXT__HPP
#define _CONTEXT__HPP

class _context // could probably define it inside of _thread class, since its only usage is there
{
public:        // made public for ease of use. Its kernel code, shouldnt malfunction
    uint64 ra; // return address of thread owner, for context switch
    uint64 sp; // stack_pointer of thread owner, for context switch

    _context(uint64 _ra, uint64 _sp) : ra(_ra), sp(_sp) {}
    void setRa(uint64 _ra) { ra = _ra; }
    void setSp(uint64 _sp) { sp = _sp; }

    // static methods
public:
    static void contextSwitch(_context *oldContext, _context *runningContext); // saves ra and sp to oldCont. and loads into ra and sp from newCont.
    static void dispatch();                                                    // changes running thread if needed, changes context

    static void exit(); // ends running thread and calls dispatch

    static int subtleKill(_thread *threadToBeKilled); // kill a thread

private:
    // while threadDEAD do deleteThread_inDispatch
    static bool threadDEAD(_thread *thr, void *ptr);              // used for foreachWhile in queueThreads in dispatch(), Scheduling
    static void deleteThread_inDispatch(_thread *thr, void *ptr); // used for foreachWhile in queueThreads in dispatch(), Scheduling
};

#endif