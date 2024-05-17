
#include "../h/riscv.hpp"
#include "../lib/console.h"
#include "../h/memoryAllocator.hpp"
#include "../h/syscall_cpp.hpp"
uint64 Riscv::SYS_TIME = 0;

class Console;

inline void checkECALL(uint64 scause);
// inline void checkTIMER(uint64 scause);
inline void checkCONSOLE(uint64 scause);
inline void checkELSE(uint64 scause) {};

void Riscv::popSppSpieChangeMod()
{
    mc_sstatus(Riscv::SSTATUS_SPP);
    __asm__ volatile("csrw sepc, ra");
    __asm__ volatile("sret");
}

/*enum Registri{zero = 0,ra = 1,sp = 2,s0 = 8,s1,a0 = 10,a1,a2,a3,a4,a5};
inline void write_register_fp(Registri reg, uint64 val){
    int volatile pomeraj = reg * 8;
    uint64 volatile adresa;
    // adresa = pomeraj + frame pointer
    __asm__ volatile("add %[adr], %[pom], fp" :[adr] "=r"(adresa): [pom] "r"(pomeraj));
    __asm__ volatile("sd %[val], (%[adr])" :: [val] "r"(val), [adr] "r"(adresa));
}*/
/*inline uint64 read_register_fp(uint64 reg) {
    uint64 volatile value;
    __asm__ volatile("ld %[value], 8*%[reg](fp)" : [value] "=r"(value) : [reg] "n"(reg));
    return value;

}*/

inline void mem_alloc_wrapper();
inline void mem_free_wrapper();
inline void thread_create_wrapper();
inline void sem_open_wrapper();
inline void sem_close_wrapper();
inline void sem_wait_wrapper();
inline void sem_signal_wrapper();
inline void sem_timedwait_wrapper(uint64 SYSTIME);
inline void sem_trywait_wrapper();

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
            sem_timedwait_wrapper(Riscv::SYS_TIME);
            break;
        case Riscv::SEM_TRYWAIT:
            break;
        case Riscv::TIME_SLEEP:
            break;
        case Riscv::GETC:
            result = Console::getc();
            __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));
            break;
        case Riscv::PUTC:
            char c;
            __asm__ volatile("ld %[chr], 11 * 8(fp)" : [chr] "=r"(c));
            Console::putc(c);
            break;
        default:
            __putc('G');
            thread_exit();
            while (1)
                ;
            break;
        }
    }
    else if (scause == Riscv::TIMER)
    {
        Riscv::mc_sip(Riscv::SIP_SSIP);
    }
    else if (scause == Riscv::CONSOLE)
    {
        checkCONSOLE(scause);
    }
    else if (scause == Riscv::ILLEGAL_INSTRUCTION)
    {
        __putc('E');
        __putc('R');
        __putc('R');
        __putc('\n');
        _thread::exit();
    }
    else
    {
        __putc('N');
        __putc('M');
        __putc('P');
        __putc('\n');
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

inline void sem_timedwait_wrapper(uint64 SYSTIME)
{
    _sem *handle;
    uint64 timeout;
    __asm__ volatile("ld %[handle], 11 * 8(fp)" : [handle] "=r"(handle));
    __asm__ volatile("ld %[timeout], 12 * 8(fp)" : [timeout] "=r"(timeout));

    int volatile result = (handle == nullptr) ? -1 : 0;
    __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(result));

    if (handle == nullptr)
        return;
    handle->timedWait(SYSTIME + timeout);
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
/*inline void checkTIMER (uint64 scause) {

    // interrupt: yes; cause code: supervisor software interrupt (CLINT; machine timer interrupt)
    Riscv::mc_sip(SIP_SSIP);
    Riscv::SYS_TIME++;
    Thread::timeSliceCounter++;
    if ( Thread::timeSliceCounter >= Thread::running->getTimeSlice())
    {
        preemption_wrapper();
    }

}*/
/*inline void Riscv::dispatch_wrapper()
{
    _thread::timeSliceCounter = 0;
    _thread::dispatch();
}

inline void Riscv::yield_wrapper()
{
    uint64 volatile sepc = Riscv::r_sepc();
    uint64 volatile sstatus = Riscv::r_sstatus();

    Riscv::dispatch_wrapper();

    Riscv::w_sstatus(sstatus);
    Riscv::w_sepc(sepc);
}
*/
inline void checkCONSOLE(uint64 scause)
{
    // interrupt: yes; cause code: supervisor external interrupt (PLIC; could be keyboard)
    console_handler();
}