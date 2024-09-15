
#include "../h/_riscV.hpp"
#include "../lib/console.h"
#include "../h/_memoryAllocator.hpp"
#include "../h/syscall_cpp.hpp"
class Console;
uint64 _riscV::SYS_TIME = 0;

void _riscV::handleSupervisorTrap(uint64 a0, uint64 a1, uint64 a2, uint64 a3, uint64 a4, uint64 a5)
// called from supervisorTrap() for ECall handling, a0-a5 are registers
{
    uint64 volatile scause = r_scause();
    uint64 volatile sepc = r_sepc();
    uint64 volatile sstatus = r_sstatus();

    if (scause == ECALL_S && _thread::running != nullptr && _thread::running->body != nullptr) // only main has body == nullptr
    {
        priority_print("Nested system calls at: 0x");
        priority_print_int(sepc, 16, 0);
        priority_print(" !\n");
    }

    if (scause == ECALL_U || scause == ECALL_S)
    {
        sepc += 4;

        uint64 ecallCode = a0;
        switch (ecallCode)
        {
        case MALLOC:
            a0 = mem_alloc_wrapper(a1);
            break;
        case MFREE:
            a0 = mem_free_wrapper((void *)a1);
            break;
        case THREAD_CREATE:
            a0 = thread_create_wrapper((_thread **)a1, (Body)a2, (void *)a3, (uint64 *)a4);
            break;
        case THREAD_EXIT:
            a0 = thread_exit_wrapper();
            break;
        case THREAD_DISPATCH:
            a0 = thread_dispatch_wrapper();
            break;
        case SEM_OPEN:
            a0 = sem_open_wrapper((_sem **)a1, a2);
            break;
        case SEM_CLOSE:
            a0 = sem_close_wrapper((_sem *)a1);
            break;
        case SEM_WAIT:
            a0 = sem_wait_wrapper((_sem *)a1);
            break;
        case SEM_SIGNAL:
            a0 = sem_signal_wrapper((_sem *)a1);
            break;
        case SEM_TIMEDWAIT:
            a0 = sem_timedwait_wrapper((_sem *)a1, a2);
            break;
        case SEM_TRYWAIT:
            a0 = sem_trywait_wrapper((_sem *)a1);
            break;
        case TIME_SLEEP:
            a0 = time_sleep_wrapper(a1);
            break;
        case GETC:
            a0 = getc_wrapper();
            break;
        case PUTC:
            a0 = putc_wrapper((char)a1);
            break;
        default:
            priority_print("ECALLs TrapCode : ");
            priority_print_int(ecallCode);
            error_msg_terminate(" - Unknown ECALL TrapCode!\n", sepc - 4);
            a0 = -1;
            break;
        }
        __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(a0));
    }
    else if (scause == TIMER)
    {
        mc_sip(SIP_SSIP); // reset register for interrupt pending

        incSysTime(); // inc system time

        _thread::incTimeSliceCounter(); // inc running thread running time

        _thread::wakeAsleepThreads(); // wake-up asleep threads if needed (timed_wait + time_sleep)

        if (_thread::getTimeSliceCounter() >= _thread::running->getTimeSlice())
            _thread::dispatch(); // preemption if needed
    }
    else if (scause == CONSOLE)
    {
        int intNumber = plic_claim(); // get code of terminal that caused intr
        if (intNumber == 0xa)
            _console::setConsoleInterrupt(true); // if its from console
        else
            plic_complete(intNumber);
    }

    else if (scause == ILLEGAL_INSTRUCTION)
    {
        error_msg_terminate("Illegal instruction!\n\0", sepc);
    }
    else if (scause == ILLEGAL_RD_ADDR)
    {
        error_msg_terminate("Illegal address for reading!\n\0", sepc);
    }
    else if (scause == ILLEGAL_WR_ADDR)
    {
        error_msg_terminate("Illegal address for writing!\n\0", sepc);
    }
    else
    {
        priority_print("Scause = \n\0");
        priority_print_int(scause, 2);
        error_msg_terminate("Unknown interrupt\n\0", sepc);
    }
    w_sstatus(sstatus);
    w_sepc(sepc);
}

void _riscV::popSppSpieChangeMod()
{
    mc_sstatus(SSTATUS_SPP);           // set previous privilege to 0 (user regime)
    __asm__ volatile("csrw sepc, ra"); // set sepc to return adress
    __asm__ volatile("sret");          // call return function from S mode
}
