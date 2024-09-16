
#include "../h/_riscV.hpp"
#include "../lib/console.h"
#include "../h/_memoryAllocator.hpp"
#include "../h/_sysCalls.hpp"

class Console;

void _riscV::handleSupervisorTrap(uint64 a0, uint64 a1, uint64 a2, uint64 a3, uint64 a4, uint64 a5)
// called from supervisorTrap() for ECall handling, a0-a5 are registers
{
    uint64 volatile scause = r_scause();
    uint64 volatile sepc = r_sepc();
    uint64 volatile sstatus = r_sstatus();

    if (scause == ECALL_S)
        _sysCallsHandler::checkForNestedSysCalls(sepc);

    if (scause == ECALL_U || scause == ECALL_S)
    {
        sepc += 4; // increase sepc because we want to return to next instruction, not the one that did ecall

        a0 = _sysCallsHandler::ecall_handler(a0, a1, a2, a3, a4, a5);      // call ecall handler
        __asm__ volatile("sd %[result], 10 * 8(fp)" : : [result] "r"(a0)); // write result in a0 on stack so process restores it when sret happens
    }
    else if (scause == TIMER)
        _time::timer_interrupt_handler();
    else if (scause == CONSOLE)
        _console::console_interrupt_handler();
    else if (scause == ILLEGAL_INSTRUCTION)
        error_msg_terminate("Illegal instruction!\n\0", sepc);
    else if (scause == ILLEGAL_RD_ADDR)
        error_msg_terminate("Illegal address for reading!\n\0", sepc);
    else if (scause == ILLEGAL_WR_ADDR)
        error_msg_terminate("Illegal address for writing!\n\0", sepc);
    else
    {
        _SModePrinter::priority_print("Scause = ");
        _SModePrinter::priority_print_int(scause, 2);
        error_msg_terminate("\nUnknown interrupt\n\0", sepc);
    }
    w_sstatus(sstatus);
    w_sepc(sepc);
}

void _riscV::popSppSpieChangeMod()
{
    mc_sstatus(SSTATUS_SPP);           // set previous privilege to 0 (user regime)
    __asm__ volatile("csrw sepc, ra"); // set sepc to return adress
    __asm__ volatile("sret");          // call return function from S mode, returns to ra
}
