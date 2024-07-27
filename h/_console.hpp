#include "../lib/hw.h"
#include "./syscall_c.h"
#include "../h/_sem.hpp"
class _console
{
public:
    static bool isThereAnythingToPrint() { return (headPrint - tailPrint) != 0; }
    static void putter_wrapper(void *);
    static void getter_wrapper(void *);
    static void PRINT_CONSOLE_IN_EMERGENCY();

private:
    _console() = delete;
    static void inline init();
    static void character_putter_thread(void *);
    static void character_getter_thread(void *);
    friend class Riscv;

    static inline int putCharInBuffer(char ch);
    static inline char getCharFromBuffer();

    static constexpr uint64 NUM_OF_CHARS = 1024;

    static uint64 headPrint;
    static uint64 tailPrint;
    static char bufferPrint[NUM_OF_CHARS];

    static uint64 headGet;
    static uint64 tailGet;
    static char bufferGet[NUM_OF_CHARS];

    static _sem *characterReadyToGet;
    /*static _sem *semTransfer;
    static _sem *semPut;
    static _sem *semReceive;
    static _sem *semGet;*/

    enum TERMINAL_STATUS_CHECKERS
    {
        CONSOLE_STATUS_RECEIVE = 1UL << 0,
        CONSOLE_STATUS_TRANSFER = 1UL << 5
    };
    static bool consoleInterrupt;
    static void setConsoleInterrupt(bool value) { consoleInterrupt = value; }
    static bool isConsoleInterrupt() { return consoleInterrupt; }

    static inline bool checkTerminalReceive();
    static inline bool checkTerminalTransfer();

    static inline void putCharInTerminal(char ch);
    static inline char getCharFromTerminal();

    static uint64 consoleStatusAddr;   // (DO NOT CHANGE)console status location, if can receive or can transmit do it
    static uint64 consoleTransferAddr; // (DO NOT CHANGE)on this location I store character for printing
    static uint64 consoleReceiveAddr;  // (DO NOT CHANGE)from this location I read character
};

inline int _console::putCharInBuffer(char ch)
{
    init();

    if ((headPrint + 1) % NUM_OF_CHARS == tailPrint)
        return -1;
    _console::bufferPrint[headPrint] = ch;
    headPrint = (headPrint + 1) % NUM_OF_CHARS;

    return 0;
}

inline char _console::getCharFromBuffer()
{
    init();

    characterReadyToGet->wait();

    if (headGet == tailGet)
        return -1;
    char ch = _console::bufferGet[tailGet];
    tailGet = (tailGet + 1) % NUM_OF_CHARS;

    return ch;
}

inline bool _console::checkTerminalReceive()
{
    init();
    uint8 console_status = *((char *)consoleStatusAddr);
    /*__asm__("lb %[status], (%[statusAdr])\n": [status] "=r"(console_status): [statusAdr] "r"(consoleStatusAddr));*/

    console_status &= CONSOLE_STATUS_RECEIVE;
    return console_status != 0; // returns true if I can receive char
}

inline bool _console::checkTerminalTransfer()
{
    init();
    uint8 console_status = *((char *)consoleStatusAddr);

    console_status &= CONSOLE_STATUS_TRANSFER;
    return console_status != 0; // returns true if I can transfer char
}

inline void _console::putCharInTerminal(char ch)
{
    init();
    *((char *)consoleTransferAddr) = ch;
}

inline char _console::getCharFromTerminal()
{
    init();
    char ch = *((char *)consoleReceiveAddr);
    return ch;
}

inline void _console::init()
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

    characterReadyToGet = new _sem(0);

    // mutexInt = new _sem(1);
    /*semPut = new _sem(NUM_OF_CHARS);
    semReceive = new _sem(NUM_OF_CHARS);
    semGet = new _sem(0);*/

    initialized = true;
}