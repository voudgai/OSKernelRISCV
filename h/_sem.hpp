//
// Created by os on 5/15/24.
//

#ifndef PROJECT_FOR_REAL__SEM_HPP
#define PROJECT_FOR_REAL__SEM_HPP

#include "_thread.hpp"
#include "memoryAllocator.hpp"
#include "list.hpp"


class _sem{
public:
    friend class Riscv;
    void *operator new(size_t n){ return memoryAllocator::_kmalloc((memoryAllocator::SIZE_HEADER + n + MEM_BLOCK_SIZE -1) / MEM_BLOCK_SIZE); }
    void *operator new[](size_t n){ return memoryAllocator::_kmalloc((memoryAllocator::SIZE_HEADER + n + MEM_BLOCK_SIZE -1) / MEM_BLOCK_SIZE); }

    void operator delete(void *p) noexcept{ memoryAllocator::_kmfree(p); }
    void operator delete[](void *p) noexcept{ memoryAllocator::_kmfree(p);}

    explicit _sem(uint64 N = 1) : val(N){
    };

    ~_sem(){
        unblockAll_CLOSING();
    }
    void wait();
    void timedWait(uint64 timeForRelease); // ovaj timeForRelease ce biti systime + koliko ceka maksimalno
    int tryWait();
    void signal();

    uint64 value() const{return val;}
protected:
    void block();
    void timedBlock(uint64 timeForRelease);
    void unblock();
    void unblockAll_CLOSING();
    void unblockTimesUp(uint64 systemTime);
private:
    long int val;
    uint64 numOfTimedWaiting = 0;
    List <_thread> queueBlocked;
    List <_thread> queueTimedBlock;

};

#endif //PROJECT_FOR_REAL__SEM_HPP
