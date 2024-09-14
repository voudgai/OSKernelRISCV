
#include "../h/riscv.hpp"
#include "../lib/console.h"
#include "../h/memoryAllocator.hpp"
#include "../h/syscall_cpp.hpp"

uint64 Riscv::SYS_TIME = 0;

class Console;

void Riscv::popSppSpieChangeMod()
{
    mc_sstatus(SSTATUS_SPP);           // set previous privilege to 0 (user regime)
    __asm__ volatile("csrw sepc, ra"); // set sepc to return adress
    __asm__ volatile("sret");          // call return function from S mode
}

void Riscv::handleSupervisorTrap(uint64 a0, uint64 a1, uint64 a2, uint64 a3, uint64 a4, uint64 a5) // CALLED FOR TRAP HANDLING
{
    uint64 scause = r_scause();
    uint64 volatile sepc = r_sepc();
    uint64 volatile sstatus = r_sstatus();
    if (scause == ECALL_U || scause == ECALL_S)
    {
        if (scause == ECALL_S && _thread::running != nullptr && _thread::running->body != nullptr)
        {
            priority_print("Nested system calls at: 0x");
            priority_print_int(sepc, 16, 0);
            priority_print(" !\n");

            print_stack_trace(STACK_TRACE_DEPTH, sepc);
        }

        sepc += 4;

        uint64 volatile ecallCode;
        __asm__ volatile("ld %[code], 10 * 8(fp)" : [code] "=r"(ecallCode));

        switch (ecallCode)
        {
        case MALLOC:
            mem_alloc_wrapper();
            break;
        case MFREE:
            mem_free_wrapper();
            break;
        case THREAD_CREATE:
            thread_create_wrapper();
            break;
        case THREAD_EXIT:
            thread_exit_wrapper();
            break;
        case THREAD_DISPATCH:
            thread_dispatch_wrapper();
            break;
        case SEM_OPEN:
            sem_open_wrapper();
            break;
        case SEM_CLOSE:
            sem_close_wrapper();
            break;
        case SEM_WAIT:
            sem_wait_wrapper();
            break;
        case SEM_SIGNAL:
            sem_signal_wrapper();
            break;
        case SEM_TIMEDWAIT:
            sem_timedwait_wrapper();
            break;
        case SEM_TRYWAIT:
            sem_trywait_wrapper();
            break;
        case TIME_SLEEP:
            time_sleep_wrapper();
            break;
        case GETC:
            getc_wrapper();
            break;
        case PUTC:
            putc_wrapper();
            break;
        default:
            priority_print("ECALLs TrapCode : ");
            priority_print_int(ecallCode);
            error_msg_terminate(" - Unknown ECALL TrapCode!\n", sepc - 4);
        }
    }
    else if (scause == TIMER)
    {
        mc_sip(SIP_SSIP);

        incSysTime();
        _thread::incTimeSliceCounter();

        { // wake-up asleep threads if needed (timed_wait + time_sleep)
            _thread::wakeAsleepThreads();
        }

        { // preemption if needed
            if (_thread::getTimeSliceCounter() >= _thread::running->getTimeSlice())
            {
                _thread::dispatch();
            }
        }
    }
    else if (scause == CONSOLE)
    {
        int intNumber = plic_claim();
        if (intNumber == 0xa)
        {
            _console::setConsoleInterrupt(true);
        }
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
