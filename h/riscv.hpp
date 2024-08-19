
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

    // // read any register
    // inline static uint64 read_register(const char *reg_name);
    // inline static void write_register(const char *reg_name, uint64 value);
    // inline static uint64 read_memory_with_index(uint64 *base, uint64 index, uint8 wordSizeInBytes)
    // {
    //     uint64_t value;
    //     asm volatile("ld %0, %1(%2)"
    //                  : "=r"(value)
    //                  : "r"(index * wordSizeInBytes), "r"(base));
    //     return value;
    // }
    // inline static void write_memory_with_index(uint64 *base, uint64 index, , uint8 wordSizeInBytes, uint64 value)
    // {
    //     asm volatile("sd %0, %1(%2)"
    //                  :
    //                  : "r"(value), "r"(index * wordSizeInBytes), "r"(base));
    // }

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

        GET_THREAD_ID = 0x51,
        MODIFICATION = 0x61
    };
    // supervisor trap
    static void supervisorTrap();

    // push and pop x3..x31 registers onto stack
    static void pushRegisters();
    static void popRegisters();

    static void killQEMU();

private:
    static uint64 SYS_TIME;
    static inline void incSysTime(uint64 i = 1) { SYS_TIME += i; }

    // supervisor trap handler
    static void handleSupervisorTrap();

    // error printer in S mode
    inline static void error_printer(const char *s);
    inline static void error_printInt(int xx, int base = 10, int sgn = 0);
    inline static void error_print_stack_trace(uint depth);
    static constexpr uint STACK_TRACE_DEPTH = 1;

    static inline void mem_alloc_wrapper();
    static inline void mem_free_wrapper();
    static inline void thread_create_wrapper();
    static inline void thread_exit_wrapper();
    static inline void thread_dispatch_wrapper();
    static inline void sem_open_wrapper();
    static inline void sem_close_wrapper();
    static inline void sem_wait_wrapper();
    static inline void sem_signal_wrapper();
    static inline void sem_timedwait_wrapper();
    static inline void sem_trywait_wrapper();
    static inline void time_sleep_wrapper();
    static inline void putc_wrapper();
    static inline void getc_wrapper();

    static inline void modification_wrapper();

    static inline void default_case_wrapper(int sepc);
};

// inline uint64 Riscv::read_register(const char *reg_name)
// {
//     volatile uint64 value;
//     asm volatile("mv %0, %1" : "=r"(value) : "r"(reg_name));
//     return value;
// }

// inline void Riscv::write_register(const char *reg_name, uint64 value)
// {
//     asm volatile("mv %0, %1" : "=r"(reg_name) : "r"(value));
// }

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

inline void Riscv::error_printer(const char *s)
{
    int i = 0;
    while (s[i] != '\0')
    {
        if (_console::checkTerminalTransfer() == true /*&& _console::isConsoleInterrupt()*/)
        {
            _console::putCharInTerminal(s[i]);
            i++;
        }
    }

    plic_complete(0xa);
}

inline void Riscv::error_printInt(int xx, int base, int sgn)
{
    char digits[] = "0123456789abcdef";
    char buf[16];
    int i, neg;
    uint x;

    neg = 0;
    if (sgn && xx < 0)
    {
        neg = 1;
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

inline void Riscv::error_print_stack_trace(uint depth)
{
    static int recursionCounter = 0;
    if (recursionCounter++ > 3)
        return;

    volatile void *framePointer;
    uint64 returnAddress = 1;

    // Dobijanje trenutnog frame pointer-a
    __asm__ volatile("mv %0, fp" : "=r"(framePointer));

    error_printer("-\n-Stack trace :\n ");

    for (uint i = 0; i < depth && returnAddress; i++)
    {
        // Dobijanje povratne adrese
        __asm__ volatile("ld %0, 8(%1)" : "=r"(returnAddress) : "r"(framePointer));

        error_printer("   Call address: ");
        error_printInt(returnAddress, 16, 0);
        error_printer(" (-4)\n");

        // Dobijanje sledećeg frame pointer-a
        __asm__ volatile("ld %0, 0(%1)" : "=r"(framePointer) : "r"(framePointer));
    }
    error_printer("---------------------------------------\n");

    recursionCounter--;
}

inline void Riscv::mem_alloc_wrapper()
{
    uint64 numOfBlocks;
    __asm__ volatile("ld %[num], 11 * 8(fp)" : [num] "=r"(numOfBlocks));
    void *volatile result = memoryAllocator::_kmalloc(numOfBlocks);

    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
}

inline void Riscv::mem_free_wrapper()
{
    void *ptr;
    __asm__ volatile("ld %[ptr], 11 * 8(fp)" : [ptr] "=r"(ptr));
    int volatile result = memoryAllocator::_kmfree(ptr);

    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
}

inline void Riscv::thread_create_wrapper()
{
    using Body = void (*)(void *);

    _thread **volatile handle;
    Body volatile body;
    void *volatile arg;
    uint64 *volatile stack_space;
    int volatile result;

    // ucitati sacuvane registre iz memorije jer menja vrednosti a4
    __asm__ volatile("ld %[t], 11 * 8(fp)" : [t] "=r"(handle));
    __asm__ volatile("ld %[body], 12 * 8(fp)" : [body] "=r"(body));
    __asm__ volatile("ld %[arg], 13 * 8(fp)" : [arg] "=r"(arg));
    __asm__ volatile("ld %[stack], 14 * 8(fp)" : [stack] "=r"(stack_space));
    *handle = _thread::createThread(body, arg, stack_space);

    result = (*handle != nullptr) ? 0 : -1;
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
}
//__asm__ volatile("mv %0, a1" : "=r" (handle));

inline void Riscv::thread_exit_wrapper()
{
    _thread::exit();
    int volatile result = 0;
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
}

inline void Riscv::sem_open_wrapper()
{
    _sem **handle;
    uint64 init;

    __asm__ volatile("ld %[handle], 11 * 8(fp)" : [handle] "=r"(handle));
    __asm__ volatile("ld %[init], 12 * 8(fp)" : [init] "=r"(init));
    *handle = new _sem(init);

    int volatile result = (*handle == nullptr) ? -1 : 0;
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
} //__asm__  volatile("mv a0, %[a]"::[a]"r"(result));

inline void Riscv::sem_close_wrapper()
{
    _sem *handle;

    __asm__ volatile("ld %[handle], 11 * 8(fp)" : [handle] "=r"(handle));
    int volatile result = (handle == nullptr) ? -1 : 0;
    delete handle;

    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
}

inline void Riscv::sem_wait_wrapper()
{
    _sem *handle;
    __asm__ volatile("ld %[handle], 11 * 8 (fp)" : [handle] "=r"(handle));

    int volatile result = (handle == 0) ? -1 : 0;

    if (result >= 0)
        result = handle->wait();
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
}

inline void Riscv::sem_signal_wrapper()
{
    _sem *handle;
    __asm__ volatile("ld %[handle], 11 * 8(fp)" : [handle] "=r"(handle));

    int volatile result = (handle == nullptr) ? -1 : 0;

    if (handle != nullptr)
        result = handle->signal(); // 1 if signaled any thread, 0 if didnt

    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
    return;
}

inline void Riscv::sem_timedwait_wrapper()
{
    _sem *handle;
    uint64 timeout;
    __asm__ volatile("ld %[handle], 11 * 8(fp)" : [handle] "=r"(handle));
    __asm__ volatile("ld %[timeout], 12 * 8(fp)" : [timeout] "=r"(timeout));

    int volatile result = (handle == nullptr) ? -1 : 0;

    if (result >= 0)
        result = handle->timedWait(timeout);
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
}

inline void Riscv::sem_trywait_wrapper()
{
    _sem *handle;
    __asm__ volatile("ld %[handle], 11 * 8(fp)" : [handle] "=r"(handle));

    int volatile result = (handle == nullptr) ? -1 : 0;
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));

    if (handle == nullptr)
        return;

    result = handle->tryWait();
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
}

inline void Riscv::thread_dispatch_wrapper()
{
    int volatile result = 0;
    _thread::dispatch();

    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
}
inline void Riscv::time_sleep_wrapper()
{

    time_t timeForSleeping;
    __asm__ volatile("ld %[time], 11 * 8(fp)" : [time] "=r"(timeForSleeping));

    _thread::putThreadToSleep(timeForSleeping);

    int volatile result = 0;
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
}

inline void Riscv::putc_wrapper()
{
    char c;
    __asm__ volatile("ld %[chr], 11 * 8(fp)" : [chr] "=r"(c));
    _console::putCharInBuffer(c);

    int volatile result = 0;
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
}

inline void Riscv::getc_wrapper()
{
    int volatile result = _console::getCharFromBuffer();

    __asm__ volatile("sb %[result], 10 * 8(fp)" : : [result] "r"(result));
}

inline void Riscv::modification_wrapper()
{
    uint64 param1;
    uint64 param2;
    uint64 param3;
    uint64 param4;
    uint64 param5;
    __asm__ volatile("ld %[param], 11 * 8(fp)" : [param] "=r"(param1));
    __asm__ volatile("ld %[param], 12 * 8(fp)" : [param] "=r"(param2));
    __asm__ volatile("ld %[param], 13 * 8(fp)" : [param] "=r"(param3));
    __asm__ volatile("ld %[param], 14 * 8(fp)" : [param] "=r"(param4));
    __asm__ volatile("ld %[param], 15 * 8(fp)" : [param] "=r"(param5));

    uint64 volatile result = 0;

    __asm__ volatile("sb %[result], 10 * 8(fp)" : : [result] "r"(result));
}

inline void Riscv::default_case_wrapper(int sepc)
{
    Riscv::error_printer("Unknown ECALL TrapCode!\n");
    error_printInt(sepc, 16);
    error_print_stack_trace(STACK_TRACE_DEPTH);
    killQEMU();
    volatile int waiter = 1;
    while (waiter)
        ;
}

inline void Riscv::killQEMU()
{
    _console::PRINT_CONSOLE_IN_EMERGENCY();

    error_printer("\nKernel finished! :)\n\n");

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
