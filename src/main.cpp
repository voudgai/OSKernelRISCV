#include "../h/_riscV.hpp"
#include "../h/_memoryAllocator.hpp"
#include "../h/syscall_c.h"
#include "../h/_thread.hpp"

extern void idleThread(void *p);

/*
 * POKRETANJE DEBUGERA u CLion-u:
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
    _riscV::w_stvec((uint64)&_riscV::supervisorTrap); // setting up function to call upon interruption

    _thread *main_thread, *putc_thread, *getc_thread, *idle_thread;

    thread_create(&main_thread, nullptr, nullptr); // body for main() must be nullptr !
    thread_create(&putc_thread, _console::putter_wrapper, nullptr);
    thread_create(&getc_thread, _console::getter_wrapper, nullptr);
    thread_create(&idle_thread, idleThread, nullptr);

    while (!idle_thread->isFinished() || _console::isThereAnythingToPrint())
    {
        thread_dispatch();
    }

    _thread::subtleKill(putc_thread);
    _thread::subtleKill(getc_thread);

    thread_dispatch();

    _riscV::killQEMU();
    return 0;
}