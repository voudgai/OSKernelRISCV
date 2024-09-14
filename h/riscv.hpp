
#ifndef OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_RISCV_HPP
#define OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_RISCV_HPP

#include "../lib/hw.h"
#include "_thread.hpp"
#include "_sem.hpp"
#include "_console.hpp"

class Riscv
{
public:
    static inline uint64 getSystemTime() { return SYS_TIME; }

    // pop sstatus.spp and sstatus.spie bits (has to be a non inline function)
    static void popSppSpieChangeMod();

    // read register scause
    static uint64 r_scause();

    // write register scause
    static void w_scause(uint64 scause);

    // read register sepc
    static uint64 r_sepc();

    // write register sepc
    static void w_sepc(uint64 sepc);

    // read register stvec
    static uint64 r_stvec();

    // write register stvec
    static void w_stvec(uint64 stvec);

    // read register stval
    static uint64 r_stval();

    // write register stval
    static void w_stval(uint64 stval);

    enum BitMaskSip
    {
        SIP_SSIP = (1 << 1),
        SIP_STIP = (1 << 5),
        SIP_SEIP = (1 << 9),
    };

    // mask set register sip
    static void ms_sip(uint64 mask);

    // mask clear register sip
    static void mc_sip(uint64 mask);

    // read register sip
    static uint64 r_sip();

    // write register sip
    static void w_sip(uint64 sip);

    enum BitMaskSstatus
    {
        SSTATUS_SIE = (1 << 1),
        SSTATUS_SPIE = (1 << 5),
        SSTATUS_SPP = (1 << 8),
    };

    // mask set register sstatus
    static void ms_sstatus(uint64 mask);

    // mask clear register sstatus
    static void mc_sstatus(uint64 mask);

    // read register sstatus
    static uint64 r_sstatus();

    // write register sstatus
    static void w_sstatus(uint64 sstatus);

    enum trapType : unsigned long
    {
        ECALL_U = 0x0000000000000008UL,
        ECALL_S = 0x0000000000000009UL,
        TIMER = 0x8000000000000001UL,
        CONSOLE = 0x8000000000000009UL,
        ILLEGAL_INSTRUCTION = 0x0000000000000002UL,
        ILLEGAL_WR_ADDR = 0x0000000000000007UL,
        ILLEGAL_RD_ADDR = 0x0000000000000005UL,
    };

    enum trapEcallCause
    {
        MALLOC = 0x01,
        MFREE = 0x02,

        THREAD_CREATE = 0x11,
        THREAD_EXIT = 0x12,
        THREAD_DISPATCH = 0x13,

        SEM_OPEN = 0x21,
        SEM_CLOSE = 0x22,
        SEM_WAIT = 0x23,
        SEM_SIGNAL = 0x24,
        SEM_TIMEDWAIT = 0x25,
        SEM_TRYWAIT = 0x26,

        TIME_SLEEP = 0x31,

        GETC = 0x41,
        PUTC = 0x42,

        GET_THREAD_ID = 0x51
    };

    // supervisor trap
    static void supervisorTrap();

    // push and pop x3..x31 registers onto stack
    static void pushRegisters();
    static void popRegisters();

    static void killQEMU();

private:
    // supervisor trap handler
    static void handleSupervisorTrap(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
    static uint64 SYS_TIME;
    static inline void incSysTime(uint64 i = 1) { SYS_TIME += i; }

    // error printer in Supervisor mode
    inline static void priority_print(const char *s);
    inline static void priority_print_int(int xx, int base = 10, int sgn = 0);
    inline static void print_stack_trace(uint depth, uint64 sepc);
    inline static void error_msg_terminate(const char *s, volatile uint64 sepc);
    static constexpr uint STACK_TRACE_DEPTH = 1;

    using Body = void (*)(void *);

    static inline uint64 mem_alloc_wrapper(uint64 numOfBlocks);
    static inline uint64 mem_free_wrapper(void *ptr);
    static inline uint64 thread_create_wrapper(_thread **handle, Body body, void *arg, uint64 *stack_space);
    static inline uint64 thread_exit_wrapper();
    static inline uint64 thread_dispatch_wrapper();
    static inline uint64 sem_open_wrapper(_sem **handle, uint64 val);
    static inline uint64 sem_close_wrapper(_sem *sem);
    static inline uint64 sem_wait_wrapper(_sem *sem);
    static inline uint64 sem_signal_wrapper(_sem *sem);
    static inline uint64 sem_timedwait_wrapper(_sem *sem, uint64 maxTime);
    static inline uint64 sem_trywait_wrapper(_sem *sem);
    static inline uint64 time_sleep_wrapper(uint64 timeForSleep);
    static inline uint64 putc_wrapper(char c);
    static inline uint64 getc_wrapper();
};

inline uint64 Riscv::r_scause()
{
    uint64 volatile scause;
    __asm__ volatile("csrr %[scause], scause" : [scause] "=r"(scause));
    return scause;
}

inline void Riscv::w_scause(uint64 scause)
{
    __asm__ volatile("csrw scause, %[scause]" : : [scause] "r"(scause));
}

inline uint64 Riscv::r_sepc()
{
    uint64 volatile sepc;
    __asm__ volatile("csrr %[sepc], sepc" : [sepc] "=r"(sepc));
    return sepc;
}

inline void Riscv::w_sepc(uint64 sepc)
{
    __asm__ volatile("csrw sepc, %[sepc]" : : [sepc] "r"(sepc));
}

inline uint64 Riscv::r_stvec()
{
    uint64 volatile stvec;
    __asm__ volatile("csrr %[stvec], stvec" : [stvec] "=r"(stvec));
    return stvec;
}

inline void Riscv::w_stvec(uint64 stvec)
{
    __asm__ volatile("csrw stvec, %[stvec]" : : [stvec] "r"(stvec));
}

inline uint64 Riscv::r_stval()
{
    uint64 volatile stval;
    __asm__ volatile("csrr %[stval], stval" : [stval] "=r"(stval));
    return stval;
}

inline void Riscv::w_stval(uint64 stval)
{
    __asm__ volatile("csrw stval, %[stval]" : : [stval] "r"(stval));
}

inline void Riscv::ms_sip(uint64 mask)
{
    __asm__ volatile("csrs sip, %[mask]" : : [mask] "r"(mask));
}

inline void Riscv::mc_sip(uint64 mask)
{
    __asm__ volatile("csrc sip, %[mask]" : : [mask] "r"(mask));
}

inline uint64 Riscv::r_sip()
{
    uint64 volatile sip;
    __asm__ volatile("csrr %[sip], sip" : [sip] "=r"(sip));
    return sip;
}

inline void Riscv::w_sip(uint64 sip)
{
    __asm__ volatile("csrw sip, %[sip]" : : [sip] "r"(sip));
}

inline void Riscv::ms_sstatus(uint64 mask)
{
    __asm__ volatile("csrs sstatus, %[mask]" : : [mask] "r"(mask));
}

inline void Riscv::mc_sstatus(uint64 mask)
{
    __asm__ volatile("csrc sstatus, %[mask]" : : [mask] "r"(mask));
}

inline uint64 Riscv::r_sstatus()
{
    uint64 volatile sstatus;
    __asm__ volatile("csrr %[sstatus], sstatus" : [sstatus] "=r"(sstatus));
    return sstatus;
}

inline void Riscv::w_sstatus(uint64 sstatus)
{
    __asm__ volatile("csrw sstatus, %[sstatus]" : : [sstatus] "r"(sstatus));
}

inline void Riscv::priority_print(const char *s)
{
    int i = 0;
    while (s[i] != '\0')
    {
        if (_console::transferReady())
        {
            _console::putCharInTerminal(s[i]);
            i++;
        }
    }

    plic_complete(0xa);
}

inline void Riscv::priority_print_int(int xx, int base, int sgn)
{
    char digits[] = "0123456789abcdef";
    char buf[16];
    int i;
    bool neg;
    uint x;

    neg = false;
    if (sgn && xx < 0)
    {
        neg = true;
        x = -xx;
    }
    else
    {
        x = xx;
    }

    i = 0;
    do
    {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);

    if (neg)
        buf[i++] = '-';

    while (--i >= 0)
        _console::putCharInTerminal(buf[i]);

    plic_complete(0xa);
}

inline void Riscv::print_stack_trace(uint depth, uint64 sepc)
{
    static int recursionCounter = 0;
    if (recursionCounter++ > 3)
        return;

    volatile void *framePointer;
    uint64 returnAddress = 1;

    // Dobijanje trenutnog frame pointer-a
    __asm__ volatile("mv %0, fp" : "=r"(framePointer));

    priority_print("-\n-Stack trace :\n ");

    priority_print(" Current SEPC: 0x");
    priority_print_int(sepc, 16, 0);
    priority_print("\n");

    for (uint i = 0; i < depth && returnAddress; i++)
    {
        // Dobijanje povratne adrese
        __asm__ volatile("ld %0, 8(%1)" : "=r"(returnAddress) : "r"(framePointer));

        priority_print("   Call address: 0x");
        priority_print_int(returnAddress, 16, 0);
        priority_print(" (-4)\n");

        // Dobijanje sledećeg frame pointer-a
        __asm__ volatile("ld %0, 0(%1)" : "=r"(framePointer) : "r"(framePointer));
    }
    priority_print("---------------------------------------\n");

    recursionCounter--;
}

inline void Riscv::error_msg_terminate(const char *s, uint64 sepc)
{
    priority_print(s);
    print_stack_trace(STACK_TRACE_DEPTH, sepc);
    killQEMU();
}

inline uint64 Riscv::mem_alloc_wrapper(uint64 numOfBlocks)
{
    return (uint64)memoryAllocator::_kmalloc(numOfBlocks);
}

inline uint64 Riscv::mem_free_wrapper(void *ptr)
{
    return memoryAllocator::_kmfree(ptr);
}

inline uint64 Riscv::thread_create_wrapper(_thread **handle, Body body, void *arg, uint64 *stack_space)
{
    *handle = _thread::createThread(body, arg, stack_space);
    return (*handle != nullptr) ? 0 : -1;
}
//__asm__ volatile("mv %0, a1" : "=r" (handle));

inline uint64 Riscv::thread_exit_wrapper()
{
    _thread::exit();
    return -1;
}

inline uint64 Riscv::thread_dispatch_wrapper()
{
    _thread::dispatch();
    return 0;
}

inline uint64 Riscv::sem_open_wrapper(_sem **handle, uint64 val)
{
    *handle = new _sem(val);
    return (*handle == nullptr) ? -1 : 0;
} //__asm__  volatile("mv a0, %[a]"::[a]"r"(result));

inline uint64 Riscv::sem_close_wrapper(_sem *sem)
{
    if (!sem)
        return -1;

    delete sem;
    return 0;
}

inline uint64 Riscv::sem_wait_wrapper(_sem *sem)
{
    if (!sem)
        return -1;

    return sem->wait();
}

inline uint64 Riscv::sem_signal_wrapper(_sem *sem)
{
    if (!sem)
        return -1;

    return sem->signal(); // 1 if signaled any thread, 0 if didnt
}

inline uint64 Riscv::sem_timedwait_wrapper(_sem *sem, uint64 maxTime)
{
    if (!sem)
        return -1;

    return sem->timedWait(maxTime);
}

inline uint64 Riscv::sem_trywait_wrapper(_sem *sem)
{
    if (!sem)
        return -1;

    return sem->tryWait();
}

inline uint64 Riscv::time_sleep_wrapper(uint64 timeForSleeping)
{

    if (timeForSleeping > ((uint64)1 << 63))
        timeForSleeping = timeForSleeping >> 1;

    _thread::putThreadToSleep(timeForSleeping);
    return 0;
}

inline uint64 Riscv::getc_wrapper()
{
    return (uint64)_console::getCharFromBuffer();
}

inline uint64 Riscv::putc_wrapper(char c)
{
    return _console::putCharInBuffer(c);
}

inline void Riscv::killQEMU()
{
    _console::empty_console_print_all();

    priority_print("\nKernel finished! :)\n\n");

    __asm__(
        "li t0, 0x5555\n"   // Učitajte 32-bitnu vrednost 0x5555 u registar t0
        "li t1, 0x100000\n" // Učitajte vrednost adrese 0x100000 u registar t1
        "sw t0, (t1)\n"     // Upisujemo vrednost iz t0 (0x5555) na adresu (t1) (0x100000)
    );

    volatile int waiter = 1;
    while (waiter)
        ;
}
#endif // OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_RISCV_HPP