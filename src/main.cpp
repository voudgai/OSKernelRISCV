//
// Created by os on 5/10/24.
//
#include "../h/print.hpp"
#include "../h/riscv.hpp"
#include "../h/memoryAllocator.hpp"
#include "../h/syscall_c.h"
#include "../h/_thread.hpp"

void funcWrapper(void *);
// void character_putter_wrapper(void *);
// void character_getter_wrapper(void *);

extern void idleThread(void *p);
/*
 * POKRETANJE DEBUGERA:
 * gore desno naci prozorcic levo od zelenog play buttona, otvoriti padajuci meni i izabrati edit configurations.
 * otici na plusic u gornjem levom uglu.
 * izabrati remote debug i kada se otvori preimenovati ga slobodno.
 * za debuger izabrate /bin/gdb-multiarch
 * za target remote args staviti localhost:26000
 * za symbol file izabrati kernel fajl
 * za project root izabrati project file
 * */
int main()
{
    Riscv::w_stvec((uint64)&Riscv::supervisorTrap);

    _thread *main_thread;
    _thread *putc_thread;
    _thread *getc_thread;
    _thread *idle_thread;

    // body for main() must be nullptr !
    thread_create(&main_thread, nullptr, nullptr);

    thread_create(&putc_thread, _console::putter_wrapper, nullptr);
    thread_create(&getc_thread, _console::getter_wrapper, nullptr);
    thread_create(&idle_thread, idleThread, nullptr);

    //_thread::setMaximumThreads(5 + 3, 50, 10); // avg2023

    while (!idle_thread->isFinished() || _console::isThereAnythingToPrint())
    {
        thread_dispatch();
    }

    _thread::subtleKill(putc_thread);
    _thread::subtleKill(getc_thread);

    thread_dispatch();

    Riscv::killQEMU();
    return 0;
}