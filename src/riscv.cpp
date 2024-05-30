
#include "../h/riscv.hpp"
#include "../lib/console.h"
#include "../h/memoryAllocator.hpp"
#include "../h/syscall_cpp.hpp"
#include "../h/killQEMU.hpp"

extern void killQEMU();

uint64 Riscv::SYS_TIME = 0;

class Console;

inline void checkELSE()
{
    killQEMU();
    volatile int waiter = 1;
    while (waiter)
        ;
};

void Riscv::popSppSpieChangeMod()
{
    mc_sstatus(Riscv::SSTATUS_SPP);
    __asm__ volatile("csrw sepc, ra");
    __asm__ volatile("sret");
}

/*enum Registri{zero = 0,ra = 1,sp = 2,s0 = 8,s1,a0 = 10,a1,a2,a3,a4,a5};*/

inline void mem_alloc_wrapper();
inline void mem_free_wrapper();
inline void thread_create_wrapper();
inline void sem_open_wrapper();
inline void sem_close_wrapper();
inline void sem_wait_wrapper();
inline void sem_signal_wrapper();
inline void sem_timedwait_wrapper();
inline void sem_trywait_wrapper();
inline void time_sleep_wrapper();

void Riscv::handleSupervisorTrap() // CALLED FOR TRAP HANDLING
{
    uint64 scause = r_scause();
    uint64 volatile sepc = Riscv::r_sepc();
    uint64 volatile sstatus = Riscv::r_sstatus();
    if (scause == Riscv::ECALL_U || scause == Riscv::ECALL_S)
    {
        sepc += 4;

        uint64 volatile callCode;
        __asm__ volatile("ld %[code], 10 * 8(fp)" : [code] "=r"(callCode));
        int volatile result;
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
            _thread::timeSliceCounter = 0;
            _thread::exit();
            result = 0;
            __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
            break;
        case Riscv::THREAD_DISPATCH:
            _thread::timeSliceCounter = 0;
            _thread::dispatch();
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
            time_t timeAsleep;
            __asm__ volatile("ld %[time], 11 * 8(fp)" : [time] "=r"(timeAsleep));

            _thread::putThreadToSleep(timeAsleep);

            result = 1;
            __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
            break;
        case Riscv::GETC:
            result = _console::getCharFromBuffer();

            while (result == 255)
            {
                _thread::putThreadToSleep(4);
                result = _console::getCharFromBuffer();
            }

            __asm__ volatile("sb %[result], 10 * 8(fp)" : : [result] "r"(result));
            break;
        case Riscv::PUTC:
            char c;
            __asm__ volatile("ld %[chr], 11 * 8(fp)" : [chr] "=r"(c));
            _console::putCharInBuffer(c);
            break;
        default:
            char errorText[] = {'\n', 'U', 'n', 'k', 'n', 'o', 'w', 'n', ' ', 'E', 'C', 'A', 'L', 'L', ' ', 'T', 'r', 'a', 'p', 'C', 'o', 'd', 'e', '!', '\n', '\0'};
            Riscv::error_printer(errorText);
            killQEMU();
            volatile int waiter = 1;
            while (waiter)
                ;
        }
    }
    else if (scause == Riscv::TIMER)
    {
        Riscv::mc_sip(Riscv::SIP_SSIP);

        { // release timedwait threads from all semaphores
            Riscv::incSysTime();
            for (uint64 i = 0; i < _sem::numOfAllSemaphores; i++)
            {
                _sem *curSemaphore = _sem::allSemaphores.removeFirst();
                curSemaphore->unblockTimesUp();
                _sem::allSemaphores.addLast(curSemaphore);
            }
        }

        { // preemption if needed
            _thread::incTimeSliceCounter();
            if (_thread::getTimeSliceCounter() >= _thread::running->getTimeSlice())
            {
                _thread::resetTimeSliceCounter();
                _thread::dispatch();
            }
        }

        { // wake-up asleep threads if needed
            _thread::wakeAsleepThreads();
        }
    }
    else if (scause == Riscv::CONSOLE)
    {
        int intNumber = plic_claim();
        if (intNumber == 0xa)
        {
            _console::setConsoleInterrupt(true);
            /*char errorText[] = {'\n', 'C', 'o', 'n', 's', 'o', 'l', 'e', '!','\n', '\0'};*/
        }
    }
    else if (scause == Riscv::ILLEGAL_INSTRUCTION)
    {
        char errorText[] = {'\n', 'I', 'l', 'l', 'e', 'g', 'a', 'l', ' ', 'i', 'n', 's', 't', 'r', 'u', 'c', 't', 'i', 'o', 'n', '!', '\n', '\0'};
        Riscv::error_printer(errorText);
        killQEMU();
        volatile int waiter = 1;
        while (waiter)
            ;
        //_thread::exit();
    }
    else if (scause == Riscv::ILLEGAL_RD_ADDR)
    {
        char errorText[] = {'\n', 'I', 'l', 'l', 'e', 'g', 'a', 'l', ' ', 'a', 'd', 'd', 'r', 'e', 's', 's', '!', '\n', '\0'};
        Riscv::error_printer(errorText);
        killQEMU();
        volatile int waiter = 1;
        while (waiter)
            ;
    }
    else
    {
        char errorText[] = {'\n', 'U', 'n', 'k', 'n', 'o', 'w', 'n', ' ', 'I', 'N', 'T', 'R', '!', '\n', '\0'};
        Riscv::error_printer(errorText);
        killQEMU();
        volatile int waiter = 1;
        while (waiter)
            ;
    }
    Riscv::w_sstatus(sstatus);
    Riscv::w_sepc(sepc);
}

inline void mem_alloc_wrapper()
{
    uint64 numOfBlocks;
    __asm__ volatile("ld %[num], 11 * 8(fp)" : [num] "=r"(numOfBlocks));
    void *volatile result = memoryAllocator::_kmalloc(numOfBlocks);

    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
} //__asm__ volatile("mv %0, a1" : "=r" (numOfBlocks));
//__asm__ volatile("mv a0, %0" :: "r" (result));

inline void mem_free_wrapper()
{
    void *ptr;
    __asm__ volatile("ld %[ptr], 11 * 8(fp)" : [ptr] "=r"(ptr));
    int volatile result = memoryAllocator::_kmfree(ptr);

    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
} //__asm__ volatile("mv %0, a1" : "=r" (ptr));

inline void thread_create_wrapper()
{
    using Body = void (*)(void *);

    _thread **volatile handle;
    Body volatile body;
    void *volatile arg;
    uint64 *volatile stack_space;
    int result;

    // ucitati sacuvane registre iz memorije jer menja vrednosti a4
    __asm__ volatile("ld %[t], 11 * 8(fp)" : [t] "=r"(handle));
    __asm__ volatile("ld %[body], 12 * 8(fp)" : [body] "=r"(body));
    __asm__ volatile("ld %[arg], 13 * 8(fp)" : [arg] "=r"(arg));
    __asm__ volatile("ld %[stack], 14 * 8(fp)" : [stack] "=r"(stack_space));
    *handle = _thread::createThread(body, arg, stack_space);

    result = (*handle != nullptr) ? 0 : -1;
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
}
//__asm__ volatile("mv %0, a1" : "=r" (handle));    __asm__ volatile("mv %0, a2" : "=r" (body));    __asm__ volatile("mv %0, a3" : "=r" (arg));
//__asm__ volatile("mv %0, a4" : "=r" (stack_space)); NE RADI JER CODE KORISTI a4 REGISTAR I MENJA MI VREDNOSTI

inline void sem_open_wrapper()
{
    _sem **handle;
    uint64 init;

    __asm__ volatile("ld %[handle], 11 * 8(fp)" : [handle] "=r"(handle));
    __asm__ volatile("ld %[init], 12 * 8(fp)" : [init] "=r"(init));
    *handle = new _sem(init);

    int result = (*handle == nullptr) ? -1 : 0;
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
} //__asm__  volatile("mv a0, %[a]"::[a]"r"(result));

inline void sem_close_wrapper()
{
    _sem *handle;

    __asm__ volatile("ld %[handle], 11 * 8(fp)" : [handle] "=r"(handle));
    int result = (handle == nullptr) ? -1 : 0;
    delete handle;

    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
}

inline void sem_wait_wrapper()
{
    _sem *handle;
    __asm__ volatile("ld %[handle], 11 * 8 (fp)" : [handle] "=r"(handle));

    int result;
    result = (handle == 0) ? -1 : 0;
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));

    if (result < 0)
        return;
    handle->wait();
}

inline void sem_signal_wrapper()
{
    _sem *handle;
    __asm__ volatile("ld %[handle], 11 * 8(fp)" : [handle] "=r"(handle));

    int result = (handle == nullptr) ? -1 : 0;
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));

    if (handle == nullptr)
        return;
    handle->signal();
}

inline void sem_timedwait_wrapper()
{
    _sem *handle;
    uint64 timeout;
    __asm__ volatile("ld %[handle], 11 * 8(fp)" : [handle] "=r"(handle));
    __asm__ volatile("ld %[timeout], 12 * 8(fp)" : [timeout] "=r"(timeout));

    int volatile result = (handle == nullptr) ? -1 : 0;
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));

    if (handle == nullptr)
        return;
    handle->timedWait(timeout);
}

inline void sem_trywait_wrapper()
{
    _sem *handle;
    __asm__ volatile("ld %[handle], 11 * 8(fp)" : [handle] "=r"(handle));

    int result = (handle == nullptr) ? -1 : 0;
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));

    if (handle == nullptr)
        return;

    result = handle->tryWait();
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
}

/*inline void checkCONSOLE()
{
}*/
