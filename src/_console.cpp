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

_sem *_console::mutexInt;

/*_sem *_console::semTransfer;
_sem *_console::semPut;
_sem *_console::semReceive;
_sem *_console::semGet;*/

void character_putter_thread(void *)
{
    _console::init();
    while (true)
    {
        while (_console::headPrint == _console::tailPrint)
            thread_dispatch();
        // sem_wait(_console::semTransfer);
        if (_console::checkTerminalTransfer() == true && _console::isConsoleInterrupt())
        {
            char ch = _console::bufferPrint[_console::tailPrint];
            _console::tailPrint = (_console::tailPrint + 1) % _console::NUM_OF_CHARS;

            _console::putCharInTerminal(ch);

            // sem_signal(_console::semPut);
        }
        else
        {
            if (_console::checkTerminalReceive() == false &&
                _console::checkTerminalTransfer() == false &&
                _console::isConsoleInterrupt() == true)
            {
                _console::setConsoleInterrupt(false); // if there was interrupt and the job is done,
                                                      // there is nothing more to do, reset flag and signal the console
                plic_complete(0xa);
            }

            // sem_signal(_console::semTransfer);
            thread_dispatch();
        }
    }
}
void character_getter_thread(void *)
{
    _console::init();
    while (true)
    {
        while (_console::headGet + 1 == _console::tailPrint)
            thread_dispatch();
        // sem_wait(_console::semReceive);
        if (_console::checkTerminalReceive() == true && _console::isConsoleInterrupt())
        {
            _console::bufferGet[_console::headGet] = _console::getCharFromTerminal();
            _console::headGet = (_console::headGet + 1) % _console::NUM_OF_CHARS;

            // sem_signal(_console::semGet);
        }
        else
        {
            if (_console::checkTerminalReceive() == false &&
                _console::checkTerminalTransfer() == false &&
                _console::isConsoleInterrupt() == true)
            {
                _console::setConsoleInterrupt(false); // if there was interrupt and the job is done,
                                                      // there is nothing more to do, reset flag and signal the console
                plic_complete(0xa);
            }

            // sem_signal(_console::semReceive);
            thread_dispatch();
        }
    }
}
