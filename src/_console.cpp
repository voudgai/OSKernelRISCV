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

_sem *_console::semTransfer;
_sem *_console::semPut;
_sem *_console::semReceive;
_sem *_console::semGet;

void _console::init()
{
    static bool initialized = false;
    if (initialized)
        return;

    consoleStatusAddr = CONSOLE_STATUS;
    consoleTransferAddr = CONSOLE_TX_DATA;
    consoleReceiveAddr = CONSOLE_RX_DATA;
    headPrint = 0;
    tailPrint = 0;

    headGet = 0;
    tailGet = 0;

    semTransfer = new _sem(0);
    semPut = new _sem(NUM_OF_CHARS);
    semReceive = new _sem(NUM_OF_CHARS);
    semGet = new _sem(0);

    initialized = true;
}

void character_putter_thread(void *)
{
    _console::init();
    while (true)
    {
        sem_wait(_console::semTransfer);
        if (_console::checkTerminalTransfer() == true && _console::isConsoleInterrupt())
        {
            char ch = _console::bufferPrint[_console::tailPrint];
            _console::tailPrint = (_console::tailPrint + 1) % _console::NUM_OF_CHARS;

            _console::putCharInTerminal(ch);

            _console::setConsoleInterrupt(false);
            plic_complete(0xa);
            sem_signal(_console::semPut);
        }
        else
        {
            sem_signal(_console::semTransfer);
            thread_dispatch();
        }
    }
}
void character_getter_thread(void *)
{
    _console::init();
    while (true)
    {
        sem_wait(_console::semReceive);
        if (_console::checkTerminalReceive() == true && _console::isConsoleInterrupt())
        {
            _console::bufferGet[_console::headGet] = _console::getCharFromTerminal();
            _console::headGet = (_console::headGet + 1) % _console::NUM_OF_CHARS;

            _console::setConsoleInterrupt(false);
            plic_complete(0xa);
            sem_signal(_console::semGet);
        }
        else
        {
            sem_signal(_console::semReceive);
            thread_dispatch();
        }
    }
}
