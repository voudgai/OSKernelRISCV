
#include "../h/riscv.hpp"
#include "../lib/console.h"
#include "../h/memoryAllocator.hpp"
#include "../h/syscall_cpp.hpp"

extern void killQEMU();

uint64 Riscv::SYS_TIME = 0;

class Console;

void Riscv::popSppSpieChangeMod()
{
    mc_sstatus(Riscv::SSTATUS_SPP);
    __asm__ volatile("csrw sepc, ra");
    __asm__ volatile("sret");
}

void Riscv::handleSupervisorTrap() // CALLED FOR TRAP HANDLING
{
    uint64 scause = r_scause();
    uint64 volatile sepc = Riscv::r_sepc();
    uint64 volatile sstatus = Riscv::r_sstatus();
    if (scause == Riscv::ECALL_U || scause == Riscv::ECALL_S)
    {
        if (scause == Riscv::ECALL_S && _thread::running != nullptr && _thread::running->body != nullptr)
        {
            error_printer("Nested system calls at: 0x");
            error_printInt(sepc, 16, 0);
            error_printer(" !\n");

            error_print_stack_trace(STACK_TRACE_DEPTH);
        }

        sepc += 4;

        uint64 volatile callCode;
        __asm__ volatile("ld %[code], 10 * 8(fp)" : [code] "=r"(callCode));

        switch (callCode)
        {
        case Riscv::MALLOC:
            mem_alloc_wrapper();
            break;
        case Riscv::MFREE:
            mem_free_wrapper();
            break;
        case Riscv::THREAD_CREATE:
            thread_create_wrapper();
            break;
        case Riscv::THREAD_EXIT:
            thread_exit_wrapper();
            break;
        case Riscv::THREAD_DISPATCH:
            thread_dispatch_wrapper();
            break;
        case Riscv::SEM_OPEN:
            sem_open_wrapper();
            break;
        case Riscv::SEM_CLOSE:
            sem_close_wrapper();
            break;
        case Riscv::SEM_WAIT:
            sem_wait_wrapper();
            break;
        case Riscv::SEM_SIGNAL:
            sem_signal_wrapper();
            break;
        case Riscv::SEM_TIMEDWAIT:
            sem_timedwait_wrapper();
            break;
        case Riscv::SEM_TRYWAIT:
            sem_trywait_wrapper();
            break;
        case Riscv::TIME_SLEEP:
            time_sleep_wrapper();
            break;
        case Riscv::GETC:
            getc_wrapper();
            break;
        case Riscv::PUTC:
            putc_wrapper();
            break;
        default:
            default_case_wrapper(sepc - 4);
        }
    }
    else if (scause == Riscv::TIMER)
    {
        Riscv::mc_sip(Riscv::SIP_SSIP);
        Riscv::incSysTime();
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
    else if (scause == Riscv::CONSOLE)
    {
        int intNumber = plic_claim();
        if (intNumber == 0xa)
        {
            _console::setConsoleInterrupt(true);
        }
        else
            plic_complete(intNumber);
    }

    else if (scause == Riscv::ILLEGAL_INSTRUCTION)
    {
        Riscv::error_printer("Illegal instruction!\n\0");
        error_print_stack_trace(STACK_TRACE_DEPTH);
        killQEMU();
    }
    else if (scause == Riscv::ILLEGAL_RD_ADDR)
    {
        Riscv::error_printer("Illegal address for reading!\n\0");
        error_print_stack_trace(STACK_TRACE_DEPTH);
        killQEMU();
    }
    else if (scause == Riscv::ILLEGAL_WR_ADDR)
    {
        Riscv::error_printer("Illegal address for writing!\n\0");
        error_print_stack_trace(STACK_TRACE_DEPTH);
        killQEMU();
    }
    else
    {
        Riscv::error_printer("Unknown interrupt!\n\0");
        error_printInt(scause, 2);
        error_print_stack_trace(STACK_TRACE_DEPTH);
        killQEMU();
    }
    Riscv::w_sstatus(sstatus);
    Riscv::w_sepc(sepc);
}
