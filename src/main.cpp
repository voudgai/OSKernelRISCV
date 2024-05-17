//
// Created by os on 5/10/24.
//
#include "../h/print.hpp"
#include "../h/riscv.hpp"
#include "../h/memoryAllocator.hpp"
#include "../h/syscall_c.hpp"
#include "../h/_thread.hpp"
#include "../h/workers.hpp"
//#include "../test/userMain.cpp"

void workerBodyAWrapper(void* ptr);
void workerBodyBWrapper(void* ptr);
void workerBodyCWrapper(void* ptr);
void workerBodyDWrapper(void* ptr);

_sem* semaphoreAB;

int main(){
    Riscv::w_stvec((uint64) &Riscv::supervisorTrap);
    Riscv::mc_sstatus(Riscv::SSTATUS_SIE);

    memoryAllocator::printMemory();
    int* a = (int*)mem_alloc(sizeof(int));
    //memoryAllocator::printMemory();
    int* b = (int*)mem_alloc(sizeof(int));
    //memoryAllocator::printMemory();

    *a = 5;
    *b = 10;
    printInteger(*a);
    printString("\n");
    printInteger(*b);
    printString("\n");

    mem_free(a);
    //memoryAllocator::printMemory();
    mem_free(b);
    //memoryAllocator::printMemory();
    /*printInteger(aa);
    printInteger(bb);*/


    _thread *threads[5];

    thread_create(&threads[0],nullptr,nullptr);
    _thread::running = threads[0];

    sem_open(&semaphoreAB, 1);

    thread_create(&threads[1],workerBodyAWrapper,semaphoreAB);

    /*printString("ThreadA created\n");
    printInteger((uint64) &threads[1]);
    printString(" - &threadA\n");
    threads[1]->printThread();
    printInteger((uint64) workerBodyAWrapper);
    printString("\n");*/

    thread_create(&threads[2],workerBodyBWrapper,semaphoreAB);

    /*thread_create(&threads[3],workerBodyCWrapper,nullptr);

    thread_create(&threads[4],workerBodyDWrapper,nullptr);*/

    uint64 volatile i = 1;
    while (!(threads[1]->isFinished() &&
             threads[2]->isFinished() /*&&
             threads[3]->isFinished() &&
             threads[4]->isFinished()*/))
    {
        i++;
        if(i % 100000 == 0){
            printString("MAIN: DISPATCH MAIN\n\n");
            thread_dispatch();
            printString("MAIN: BACK TO MAIN\n");
        }

    }

    for (auto &thread: threads)
    {
        delete thread;
    }
    sem_close(semaphoreAB);
    memoryAllocator::printMemory();
    printString("Finished\n");


    return 0;
}

void workerBodyAWrapper(void* ptr){
    workerBodyA((_sem*) ptr);
}
void workerBodyBWrapper(void* ptr){
    workerBodyB((_sem*) ptr);
}
void workerBodyCWrapper(void* ptr){
    workerBodyC(); 
};
void workerBodyDWrapper(void* ptr){
    workerBodyD();
}