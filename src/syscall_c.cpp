
#include "../h/syscall_c.h"
#include "../h/_memoryAllocator.hpp"
#include "../h/_thread.hpp"
#include "../h/_sem.hpp"
#include "../lib/console.h"
static constexpr int HANDLE_NULL = -3;

void *mem_alloc(size_t size)
{
    static const int volatile trapCode = 0x01;

    size_t volatile numOfBlocks = (_memoryAllocator::SIZE_HEADER + size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;

    __asm__ volatile("mv a0, %[trapCode]" : : [trapCode] "r"(trapCode));
    __asm__ volatile("mv a1, %[numOfBlocks]" : : [numOfBlocks] "r"(numOfBlocks));
    __asm__ volatile("ecall");

    void *volatile pResult;
    __asm__ volatile("mv %0, a0" : "=r"(pResult));

    return pResult;
}

int mem_free(void *volatile ptr)
{
    static const int volatile trapCode = 0x02;

    __asm__ volatile("mv a0, %[trapCode]" : : [trapCode] "r"(trapCode));
    __asm__ volatile("mv a1, %[ptr]" : : [ptr] "r"(ptr));
    __asm__ volatile("ecall");

    int volatile result;
    __asm__ volatile("mv %0, a0" : "=r"(result));

    return result;
}

int thread_create(thread_t *handle, void (*start_routine)(void *), void *arg) // 0x11
{
    static const int volatile trapCode = 0x11;

    uint64 *stack_space = (uint64 *)mem_alloc(DEFAULT_STACK_SIZE);
    if (stack_space == nullptr)
        return -1;
    stack_space = &stack_space[DEFAULT_STACK_SIZE / sizeof(uint64)];

    __asm__ volatile("mv a4, %[stack_space]" : : [stack_space] "r"(stack_space));
    __asm__ volatile("mv a3, %[arg]" : : [arg] "r"(arg));
    __asm__ volatile("mv a2, %[start_routine]" : : [start_routine] "r"(start_routine));
    __asm__ volatile("mv a1, %[handle]" : : [handle] "r"(handle));
    __asm__ volatile("mv a0, %[trapCode]" : : [trapCode] "r"(trapCode));

    __asm__ volatile("ecall");

    int volatile result;
    __asm__ volatile("mv %0, a0" : "=r"(result));

    return result;
}

int thread_exit()
{
    static const int volatile trapCode = 0x12;

    __asm__ volatile("mv a0, %[trapCode]" : : [trapCode] "r"(trapCode));
    __asm__ volatile("ecall");

    int volatile result;
    __asm__ volatile("mv %0, a0" : "=r"(result));

    return result;
} // 0x12

void thread_dispatch()
{
    static const int volatile trapCode = 0x13;

    __asm__ volatile("mv a0, %[trapCode]" : : [trapCode] "r"(trapCode));
    __asm__ volatile("ecall");

    int volatile result;
    __asm__ volatile("mv %0, a0" : "=r"(result));
} // 0x13

int sem_open(sem_t *handle, unsigned init)
{
    static const int volatile trapCode = 0x21;

    __asm__ volatile("mv a2, %[init]" : : [init] "r"(init));
    __asm__ volatile("mv a1, %[handle]" : : [handle] "r"(handle));
    __asm__ volatile("mv a0, %[trapCode]" : : [trapCode] "r"(trapCode));

    __asm__ volatile("ecall");

    int volatile result;
    __asm__ volatile("mv %0, a0" : "=r"(result));

    return result;
}

int sem_close(sem_t handle)
{
    if (handle == nullptr)
        return HANDLE_NULL;

    static const int volatile trapCode = 0x22;

    __asm__ volatile("mv a1, %[handle]" : : [handle] "r"(handle));
    __asm__ volatile("mv a0, %[trapCode]" : : [trapCode] "r"(trapCode));

    __asm__ volatile("ecall");

    int volatile result;
    __asm__ volatile("mv %0, a0" : "=r"(result));

    return result;
}

int sem_wait(sem_t id)
{
    if (id == nullptr)
        return HANDLE_NULL;

    static const int volatile trapCode = 0x23;

    __asm__ volatile("mv a1, %[id]" : : [id] "r"(id));
    __asm__ volatile("mv a0, %[trapCode]" : : [trapCode] "r"(trapCode));
    __asm__ volatile("ecall");

    int volatile result;
    __asm__ volatile("mv %0, a0" : "=r"(result));

    return result;
}

int sem_signal(sem_t id)
{
    if (id == nullptr)
        return HANDLE_NULL;

    static const int volatile trapCode = 0x24;

    __asm__ volatile("mv a1, %[id]" : : [id] "r"(id));
    __asm__ volatile("mv a0, %[trapCode]" : : [trapCode] "r"(trapCode));
    __asm__ volatile("ecall");

    int volatile result;
    __asm__ volatile("mv %0, a0" : "=r"(result));
    return result;
}

int sem_timedwait(sem_t id, time_t timeout)
{
    if (id == nullptr)
        return HANDLE_NULL;

    static const int volatile trapCode = 0x25;

    __asm__ volatile("mv a2, %[time]" : : [time] "r"(timeout));
    __asm__ volatile("mv a1, %[id]" : : [id] "r"(id));
    __asm__ volatile("mv a0, %[trapCode]" : : [trapCode] "r"(trapCode));
    __asm__ volatile("ecall");

    int volatile result;
    __asm__ volatile("mv %0, a0" : "=r"(result));
    return result;
}

int sem_trywait(sem_t id)
{
    static const int volatile trapCode = 0x26;

    if (id == nullptr)
        return HANDLE_NULL;

    __asm__ volatile("mv a1, %[id]" : : [id] "r"(id));
    __asm__ volatile("mv a0, %[trapCode]" : : [trapCode] "r"(trapCode));
    __asm__ volatile("ecall");

    int volatile result;
    __asm__ volatile("mv %0, a0" : "=r"(result));
    return result;
}

int time_sleep(time_t timeAsleep)
{
    static const int volatile trapCode = 0x31;

    if (timeAsleep == 0)
        return 0;

    __asm__ volatile("mv a1, %[time]" : : [time] "r"(timeAsleep));
    __asm__ volatile("mv a0, %[trapCode]" : : [trapCode] "r"(trapCode));
    __asm__ volatile("ecall");

    int volatile result;
    __asm__ volatile("mv %0, a0" : "=r"(result));
    return result;
}
char getc()
{
    static const int volatile trapCode = 0x41;

    __asm__ volatile("mv a0, %[trapCode]" : : [trapCode] "r"(trapCode));
    __asm__ volatile("ecall");

    char result;
    __asm__ volatile("mv %0, a0" : "=r"(result));
    return result;
}

void putc(char c)
{
    static const int volatile trapCode = 0x42;
    __asm__ volatile("mv a1, %[ch]" : : [ch] "r"(c));
    __asm__ volatile("mv a0, %[trapCode]" : : [trapCode] "r"(trapCode));
    __asm__ volatile("ecall");

    return;
}

// DODATNO
int get_thread_ID()
{
    static const int volatile trapCode = 0x51;

    __asm__ volatile("mv a0, %[trapCode]" : : [trapCode] "r"(trapCode));
    __asm__ volatile("ecall");

    int result;
    __asm__ volatile("mv %0, a0" : "=r"(result));
    return result;
}

uint64 modification_sys_call(uint64 param1, uint64 param2, uint64 param3, uint64 param4, uint64 param5)
{
    static const int volatile trapCode = 0x61;

    __asm__ volatile("mv a5, %[param]" : : [param] "r"(param5));
    __asm__ volatile("mv a4, %[param]" : : [param] "r"(param4));
    __asm__ volatile("mv a3, %[param]" : : [param] "r"(param3));
    __asm__ volatile("mv a2, %[param]" : : [param] "r"(param2));
    __asm__ volatile("mv a1, %[param]" : : [param] "r"(param1));
    __asm__ volatile("mv a0, %[trapCode]" : : [trapCode] "r"(trapCode));

    __asm__ volatile("ecall");

    uint64 result;
    __asm__ volatile("mv %0, a0" : "=r"(result));

    return result;
}