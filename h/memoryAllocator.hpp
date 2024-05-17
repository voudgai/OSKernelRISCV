#ifndef MEM_ALLOC_HPP
#define MEM_ALLOC_HPP

#include "../lib/hw.h"

extern const void *HEAP_START_ADDR, *HEAP_END_ADDR;
extern const size_t MEM_BLOCK_SIZE;
// 64 <= MEM_BLOCK_SIZE <= 1024
class memoryAllocator
{
public:
    memoryAllocator() = delete;
    static constexpr uint64 SIZE_HEADER = 2 * 8;

    static void *_kmalloc(size_t numOfBlocks);
    static int _kmfree(void *);

    // static void printMemory();
private:
    static uint64 totalBlocks;
    static void *headFree;
    static void *headTaken;

    static void init();
};

#endif