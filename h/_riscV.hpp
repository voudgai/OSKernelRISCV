
#ifndef OS1_VEZBE07__riscV_CONTEXT_SWITCH_2_INTERRUPT__riscV_HPP
#define OS1_VEZBE07__riscV_CONTEXT_SWITCH_2_INTERRUPT__riscV_HPP

#include "../lib/hw.h"
#include "_thread.hpp"
#include "_sem.hpp"
#include "_console.hpp"
#include "_time.hpp"
#include "_SModePrinter.hpp"

class _riscV
{
public:
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

    // supervisor trap
    static void supervisorTrap();

    // push and pop x3..x31 registers onto stack
    static void pushRegisters();
    static void popRegisters();

    // kill emulator, kill system
    static void killQEMU();

private:
    // supervisor trap handler
    static void handleSupervisorTrap(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);

    // print stack trace
    inline static void print_stack_trace(uint depth, uint64 sepc);
    static constexpr uint STACK_TRACE_DEPTH = 2;

    // print error message and terminate kernel
    inline static void error_msg_terminate(const char *s, volatile uint64 sepc);

    using Body = void (*)(void *);
};

inline uint64 _riscV::r_scause()
{
    uint64 volatile scause;
    __asm__ volatile("csrr %[scause], scause" : [scause] "=r"(scause));
    return scause;
}

inline void _riscV::w_scause(uint64 scause)
{
    __asm__ volatile("csrw scause, %[scause]" : : [scause] "r"(scause));
}

inline uint64 _riscV::r_sepc()
{
    uint64 volatile sepc;
    __asm__ volatile("csrr %[sepc], sepc" : [sepc] "=r"(sepc));
    return sepc;
}

inline void _riscV::w_sepc(uint64 sepc)
{
    __asm__ volatile("csrw sepc, %[sepc]" : : [sepc] "r"(sepc));
}

inline uint64 _riscV::r_stvec()
{
    uint64 volatile stvec;
    __asm__ volatile("csrr %[stvec], stvec" : [stvec] "=r"(stvec));
    return stvec;
}

inline void _riscV::w_stvec(uint64 stvec)
{
    __asm__ volatile("csrw stvec, %[stvec]" : : [stvec] "r"(stvec));
}

inline uint64 _riscV::r_stval()
{
    uint64 volatile stval;
    __asm__ volatile("csrr %[stval], stval" : [stval] "=r"(stval));
    return stval;
}

inline void _riscV::w_stval(uint64 stval)
{
    __asm__ volatile("csrw stval, %[stval]" : : [stval] "r"(stval));
}

inline void _riscV::ms_sip(uint64 mask)
{
    __asm__ volatile("csrs sip, %[mask]" : : [mask] "r"(mask));
}

inline void _riscV::mc_sip(uint64 mask)
{
    __asm__ volatile("csrc sip, %[mask]" : : [mask] "r"(mask));
}

inline uint64 _riscV::r_sip()
{
    uint64 volatile sip;
    __asm__ volatile("csrr %[sip], sip" : [sip] "=r"(sip));
    return sip;
}

inline void _riscV::w_sip(uint64 sip)
{
    __asm__ volatile("csrw sip, %[sip]" : : [sip] "r"(sip));
}

inline void _riscV::ms_sstatus(uint64 mask)
{
    __asm__ volatile("csrs sstatus, %[mask]" : : [mask] "r"(mask));
}

inline void _riscV::mc_sstatus(uint64 mask)
{
    __asm__ volatile("csrc sstatus, %[mask]" : : [mask] "r"(mask));
}

inline uint64 _riscV::r_sstatus()
{
    uint64 volatile sstatus;
    __asm__ volatile("csrr %[sstatus], sstatus" : [sstatus] "=r"(sstatus));
    return sstatus;
}

inline void _riscV::w_sstatus(uint64 sstatus)
{
    __asm__ volatile("csrw sstatus, %[sstatus]" : : [sstatus] "r"(sstatus));
}

inline void _riscV::print_stack_trace(uint depth, uint64 sepc)
{
    static int recursionCounter = 0;
    if (recursionCounter++ > 3)
        return;

    volatile void *framePointer;
    uint64 returnAddress = 1;

    // Dobijanje trenutnog frame pointer-a
    __asm__ volatile("mv %0, fp" : "=r"(framePointer));

    _SModePrinter::priority_print("-\n-Stack trace :\n ");

    _SModePrinter::priority_print(" Current SEPC: 0x");
    _SModePrinter::priority_print_int(sepc, 16, 0);
    _SModePrinter::priority_print("\n");

    for (uint i = 0; i < depth && returnAddress; i++)
    {
        // Dobijanje povratne adrese
        __asm__ volatile("ld %0, 8(%1)" : "=r"(returnAddress) : "r"(framePointer));

        _SModePrinter::priority_print("   Call address: 0x");
        _SModePrinter::priority_print_int(returnAddress, 16, 0);
        _SModePrinter::priority_print(" (-4)\n");

        // Dobijanje sledećeg frame pointer-a
        __asm__ volatile("ld %0, 0(%1)" : "=r"(framePointer) : "r"(framePointer));
    }
    _SModePrinter::priority_print("---------------------------------------\n");

    recursionCounter--;
}

inline void _riscV::error_msg_terminate(const char *s, uint64 sepc)
{
    _SModePrinter::priority_print(s);
    print_stack_trace(STACK_TRACE_DEPTH, sepc);
    killQEMU();
}

inline void _riscV::killQEMU()
{
    _console::empty_console_print_all();

    _SModePrinter::priority_print("\nKernel finished! :)\n\n");

    __asm__(
        "li t0, 0x5555\n"   // Učitajte 32-bitnu vrednost 0x5555 u registar t0
        "li t1, 0x100000\n" // Učitajte vrednost adrese 0x100000 u registar t1
        "sw t0, (t1)\n"     // Upisujemo vrednost iz t0 (0x5555) na adresu (t1) (0x100000)
    );

    volatile int waiter = 1;
    while (waiter)
        ;
}
#endif // OS1_VEZBE07__riscV_CONTEXT_SWITCH_2_INTERRUPT__riscV_HPP