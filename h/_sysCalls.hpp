#ifndef SYSCALLS__WRAPPERS__HPP
#define SYSCALLS__WRAPPERS__HPP

#include "_SModePrinter.hpp"
#include "_sem.hpp"
#include "_thread.hpp"
#include "_console.hpp"
#include "_time.hpp"

class _time;

class _sysCallsHandler
{

    using Body = void (*)(void *);

public:
    static uint64 ecall_handler(uint64 ecallCode, uint64 a1, uint64 a2, uint64 a3, uint64 a4, uint64 a5);
    static inline void checkForNestedSysCalls(uint64 sepc);

private:
    static inline uint64 mem_alloc_wrapper(uint64 numOfBlocks);
    static inline uint64 mem_free_wrapper(void *ptr);
    static inline uint64 thread_create_wrapper(_thread **handle, Body body, void *arg, uint64 *stack_space);
    static inline uint64 thread_exit_wrapper();
    static inline uint64 thread_dispatch_wrapper();
    static inline uint64 sem_open_wrapper(_sem **handle, uint64 val);
    static inline uint64 sem_close_wrapper(_sem *sem);
    static inline uint64 sem_wait_wrapper(_sem *sem);
    static inline uint64 sem_signal_wrapper(_sem *sem);
    static inline uint64 sem_timedwait_wrapper(_sem *sem, uint64 maxTime);
    static inline uint64 sem_trywait_wrapper(_sem *sem);
    static inline uint64 time_sleep_wrapper(uint64 timeForSleep);
    static inline uint64 putc_wrapper(char c);
    static inline uint64 getc_wrapper();

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

        GET_THREAD_ID = 0x51
    };
};

inline void _sysCallsHandler::checkForNestedSysCalls(uint64 sepc)
{
    if (_thread::get_runningThread() != nullptr && _thread::get_runningThread()->get_bodyPtr() != nullptr) // only main has body == nullptr
    {
        _SModePrinter::priority_print("Nested system calls at: 0x");
        _SModePrinter::priority_print_int(sepc, 16, 0);
        _SModePrinter::priority_print(" !\n");
    }
}

inline uint64 _sysCallsHandler::mem_alloc_wrapper(uint64 numOfBlocks)
{
    return (uint64)_memoryAllocator::_kmalloc(numOfBlocks);
}

inline uint64 _sysCallsHandler::mem_free_wrapper(void *ptr)
{
    return _memoryAllocator::_kmfree(ptr);
}

inline uint64 _sysCallsHandler::thread_create_wrapper(_thread **handle, Body body, void *arg, uint64 *stack_space)
{
    *handle = _thread::createThread(body, arg, stack_space);
    return (*handle != nullptr) ? 0 : -1;
}

inline uint64 _sysCallsHandler::thread_exit_wrapper()
{
    _context::exit();
    return -1;
}

inline uint64 _sysCallsHandler::thread_dispatch_wrapper()
{
    _context::dispatch();
    return 0;
}

inline uint64 _sysCallsHandler::sem_open_wrapper(_sem **handle, uint64 val)
{
    *handle = new _sem(val);
    return (*handle == nullptr) ? -1 : 0;
}

inline uint64 _sysCallsHandler::sem_close_wrapper(_sem *sem)
{
    if (!sem)
        return -1;

    delete sem;
    return 0;
}

inline uint64 _sysCallsHandler::sem_wait_wrapper(_sem *sem)
{
    if (!sem)
        return -1;

    return sem->wait();
}

inline uint64 _sysCallsHandler::sem_signal_wrapper(_sem *sem)
{
    if (!sem)
        return -1;

    return sem->signal(); // 1 if signaled any thread, 0 if didnt
}

inline uint64 _sysCallsHandler::sem_timedwait_wrapper(_sem *sem, uint64 maxTime)
{
    if (!sem)
        return -1;

    return sem->timedWait(maxTime);
}

inline uint64 _sysCallsHandler::sem_trywait_wrapper(_sem *sem)
{
    if (!sem)
        return -1;

    return sem->tryWait();
}

inline uint64 _sysCallsHandler::time_sleep_wrapper(uint64 timeForSleeping)
{

    if (timeForSleeping > ((uint64)1 << 63))
        timeForSleeping = timeForSleeping >> 1;

    _time::put_runningThread_to_sleep(timeForSleeping);
    return 0;
}

inline uint64 _sysCallsHandler::getc_wrapper()
{
    return (uint64)_console::getCharFromBuffer();
}

inline uint64 _sysCallsHandler::putc_wrapper(char c)
{
    return _console::putCharInBuffer(c);
}
#endif