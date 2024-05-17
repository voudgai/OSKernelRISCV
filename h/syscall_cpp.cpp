#include "syscall_cpp.hpp"

char Console::getc()
{
    return __getc();
}

void Console::putc(char c)
{
    __putc(c);
}
