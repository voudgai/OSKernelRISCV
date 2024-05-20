
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
    };
    // supervisor trap
    static void supervisorTrap();

private:
    static uint64 SYS_TIME;
    static inline void incSysTime(uint64 i = 1) { SYS_TIME += i; }

    // supervisor trap handler
    static void handleSupervisorTrap();

    // error printer in S mode
    inline static void error_printer(char s[]);
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

inline void Riscv::error_printer(char s[])
{
    while (*s != '\0')
    {
        if (_console::checkTerminalTransfer() == true /*&& _console::isConsoleInterrupt()*/)
        {
            char ch = *s;
            s++;
            _console::putCharInTerminal(ch);
        }
    }
    char end[] = {'F', 'A', 'T', 'A', 'L', '!', '\n', '\0'};
    int i = 0;
    while (i <= 6)
    {
        if (_console::checkTerminalTransfer() == true /*&& _console::isConsoleInterrupt()*/)
        {
            char ch = end[i];
            i++;
            _console::putCharInTerminal(ch);
        }
    }
    plic_complete(0xa);
}

#endif // OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_RISCV_HPP
