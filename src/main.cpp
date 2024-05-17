//
// Created by os on 5/10/24.
//
#include "../h/print.hpp"
#include "../h/riscv.hpp"
#include "../h/memoryAllocator.hpp"
#include "../h/syscall_c.hpp"
#include "../h/_thread.hpp"

void funcWrapper(void *);

extern void userMain();
int main()
{
    Riscv::w_stvec((uint64)&Riscv::supervisorTrap);
    // Riscv::mc_sstatus(Riscv::SSTATUS_SIE); // should i remove this?

    _thread *threads[2];

    thread_create(&threads[0], nullptr, nullptr);

    thread_create(&threads[1], funcWrapper, nullptr);

    while (!(threads[1]->isFinished()))
    {
        // printString("MAIN!");
        thread_dispatch();
    }

    for (auto &thread : threads)
    {
        delete thread;
    }
    return 0;
}

void funcWrapper(void *)
{
    userMain();
}
