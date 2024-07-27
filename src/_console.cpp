#include "../h/_console.hpp"

extern const uint64 CONSOLE_STATUS;  // console status location, if can receive or can transmit do it
extern const uint64 CONSOLE_TX_DATA; // on this location I store character for printing
extern const uint64 CONSOLE_RX_DATA; // from this location I read character

char _console::bufferGet[_console::NUM_OF_CHARS];
char _console::bufferPrint[_console::NUM_OF_CHARS];

uint64 _console::consoleStatusAddr = CONSOLE_STATUS;
uint64 _console::consoleTransferAddr = CONSOLE_TX_DATA;
uint64 _console::consoleReceiveAddr = CONSOLE_RX_DATA;

uint64 _console::headPrint = 0;
uint64 _console::tailPrint = 0;

uint64 _console::headGet = 0;
uint64 _console::tailGet = 0;

bool _console::consoleInterrupt = false;

//_sem *_console::mutexInt;

_sem *_console::characterReadyToGet;

/*_sem *_console::semPut;
_sem *_console::semReceive;
_sem *_console::semGet;*/

void _console::putter_wrapper(void *p)
{
    static bool putterMade = false;
    if (!putterMade)
    {
        putterMade = true;
        character_putter_thread(p);
    }
}

void _console::getter_wrapper(void *p)
{
    static bool getterMade = false;
    if (!getterMade)
    {
        getterMade = true;
        character_getter_thread(p);
    }
}
void _console::PRINT_CONSOLE_IN_EMERGENCY()
{
    while (headPrint != tailPrint)
    {
        if (checkTerminalTransfer() == true)
        {
            char ch = bufferPrint[tailPrint];
            tailPrint = (tailPrint + 1) % NUM_OF_CHARS;

            putCharInTerminal(ch);
        }
    }
    plic_complete(0xa);
}
void _console::character_putter_thread(void *)
{
    init();
    while (true)
    {
        while (checkTerminalTransfer() == true &&
               headPrint != tailPrint)
        {
            char ch = bufferPrint[tailPrint];
            tailPrint = (tailPrint + 1) % NUM_OF_CHARS;

            putCharInTerminal(ch);
        }

        if (checkTerminalReceive() == false &&
            checkTerminalTransfer() == false &&
            isConsoleInterrupt())
        {
            setConsoleInterrupt(false); // if there was interrupt and the job is done,
                                        // there is nothing more to do, reset flag and signal the console
            plic_complete(0xa);
        }
        thread_dispatch();
    }
}
void _console::character_getter_thread(void *)
{
    init();
    while (true)
    {
        while (checkTerminalReceive() == true &&
               isConsoleInterrupt() &&
               headGet + 1 != tailGet)
        {
            bufferGet[headGet] = getCharFromTerminal();
            headGet = (headGet + 1) % NUM_OF_CHARS;

            sem_signal(characterReadyToGet);
        }

        if (checkTerminalReceive() == false &&
            checkTerminalTransfer() == false &&
            isConsoleInterrupt())
        {
            setConsoleInterrupt(false); // if there was interrupt and the job is done,
                                        // there is nothing more to do, reset flag and signal the console
            plic_complete(0xa);
        }
        thread_dispatch();
    }
}
