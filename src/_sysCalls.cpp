#include "../h/_sysCalls.hpp"

uint64 _sysCallsHandler::ecall_handler(uint64 ecallCode, uint64 a1, uint64 a2, uint64 a3, uint64 a4, uint64 a5)
{
    uint64 a0 = -55;
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
        _SModePrinter::priority_print("ECALLs TrapCode : ");
        _SModePrinter::priority_print_int(ecallCode);
        _SModePrinter::priority_print(" unknown ecallCode\n");
        a0 = -55;
        break;
    }
    return a0;
}