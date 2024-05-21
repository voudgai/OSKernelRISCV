//
// Created by os on 5/10/24.
//
#include "../h/print.hpp"
#include "../h/riscv.hpp"
#include "../h/memoryAllocator.hpp"
#include "../h/syscall_c.h"
#include "../h/_thread.hpp"
#include "../h/killQEMU.hpp"

void funcWrapper(void *);
// void character_putter_wrapper(void *);
// void character_getter_wrapper(void *);

extern void killQEMU();
extern void userMain();
extern void mainCopeTest(void *p);

extern void character_putter_thread(void *);
extern void character_getter_thread(void *);

int main()
{
    Riscv::w_stvec((uint64)&Riscv::supervisorTrap);
    // Riscv::mc_sstatus(Riscv::SSTATUS_SIE); // should i remove this?

    _thread *main_thread;
    _thread *userMain_thread;
    _thread *putc_thread;
    _thread *getc_thread;

    // body for main() must be nullptr !
    thread_create(&main_thread, nullptr, nullptr);

    thread_create(&putc_thread, character_putter_thread, nullptr);
    thread_create(&getc_thread, character_getter_thread, nullptr);

    thread_create(&userMain_thread, funcWrapper, nullptr);
    // thread_create(&userMain_thread, mainCopeTest, nullptr);

    while (!userMain_thread->isFinished() || _console::isThereAnythingToPrint())
    {
        thread_dispatch();
    }
    _thread::subtleKill(putc_thread);
    _thread::subtleKill(getc_thread);
    thread_dispatch();

    delete putc_thread;
    delete getc_thread;
    delete userMain_thread;

    killQEMU();

    return 0;
}
void funcWrapper(void *)
{
    userMain();
}
