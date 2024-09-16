#ifndef PRIORITY__PRINTER__HPP
#define PRIORITY__PRINTER__HPP

#include "_console.hpp"

class _console;

class _SModePrinter
{
public:
    // priority printer, only for S mode
    inline static void priority_print(const char *s);
    inline static void priority_print_int(int xx, int base = 10, int sgn = 0);

    // change letters colour for terminal
    inline static void changeTerminalToRed();
    inline static void changeTerminalToDef();
};

inline void _SModePrinter::priority_print(const char *s)
{
    changeTerminalToRed();
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
    changeTerminalToDef();
}

inline void _SModePrinter::priority_print_int(int xx, int base, int sgn)
{
    changeTerminalToRed();

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

    changeTerminalToDef();
}

inline void _SModePrinter::changeTerminalToDef()
{

    static const char *changeColor = "\033[0m";

    int k = 0;
    while (changeColor[k] != '\0')
    {
        if (_console::transferReady())
        {
            _console::putCharInTerminal(changeColor[k]);
            k++;
        }
    }
    plic_complete(0xa);
}

inline void _SModePrinter::changeTerminalToRed()
{
    static const char *changeColor = "\033[31m";

    int k = 0;
    while (changeColor[k] != '\0')
    {
        if (_console::transferReady())
        {
            _console::putCharInTerminal(changeColor[k]);
            k++;
        }
    }
    plic_complete(0xa);
}

#endif