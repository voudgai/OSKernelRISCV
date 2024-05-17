#include "../h/memoryAllocator.hpp"
#include "../h/print.hpp"

void* memoryAllocator::headFree = (void*)(HEAP_START_ADDR);

void* memoryAllocator::headTaken = nullptr;

uint64 memoryAllocator::totalBlocks = ((uint64)HEAP_END_ADDR - (uint64)HEAP_START_ADDR) / MEM_BLOCK_SIZE;


void memoryAllocator::init() {
    static bool initialazed = false;


    if(initialazed) return;

    headFree = (void*)(HEAP_START_ADDR);
    headTaken = nullptr;
    totalBlocks = ((uint64)HEAP_END_ADDR - (uint64)HEAP_START_ADDR) / MEM_BLOCK_SIZE;

    uint64 *p_headFree = (uint64*) headFree;
    p_headFree[0] = 0;
    p_headFree[1] = totalBlocks;


    initialazed = true;
    return;
}

void *memoryAllocator::_kmalloc(size_t numOfBlocks) {
    init();
    if(numOfBlocks <= 0) return nullptr;

    //printString("ALLOCATING "); printString(" SIZE ");printInteger(numOfBlocks * MEM_BLOCK_SIZE);printString("\n");

    uint64* cur64 = (uint64*) headFree, *prev64 = nullptr;
    for(; cur64 && cur64[1] < numOfBlocks ; prev64 = cur64, cur64 = (uint64*)cur64[0]);

    if(cur64 == nullptr || headFree == nullptr) return nullptr;

    uint64* next64 = (uint64*)cur64[0];
    uint64* leftover64 = (uint64*)((uint64)cur64 + numOfBlocks * MEM_BLOCK_SIZE);

    if(numOfBlocks == cur64[1]){
        // leftover ne postoji
        if(prev64){
            prev64[0] = (uint64)next64;
        }
        else{
            headFree = next64;
        }
    }
    else{
        // leftover postoji
        leftover64[0] = (uint64) next64;
        leftover64[1] = cur64[1] - numOfBlocks;

        if(prev64){
            prev64[0] = (uint64)leftover64;
        }
        else{
            headFree = leftover64;
        }
    }
    cur64[1] = numOfBlocks;
    cur64[0] = 0;

    //sad ubacivanje u zauzete

    cur64[0] = (uint64)headTaken;
    headTaken = cur64;

    return (void*)((uint64)cur64 + SIZE_HEADER);
}

int memoryAllocator::_kmfree(void * toBeFreed) {
    if(toBeFreed == nullptr ) return 0;

    uint64* cur64 = (uint64*)((uint64)toBeFreed - SIZE_HEADER), *prev64 = nullptr, *next64 = (uint64*)headTaken;

    //printString("DEALLOCATING "); printInteger((uint64)cur64); printString(" OF SIZE ");printInteger(cur64[1] * MEM_BLOCK_SIZE);printString("\n");

    for(; next64 && next64 != cur64; prev64 = next64,next64 = (uint64*)next64[0]);

    if(next64 == nullptr) return -1; // nije alocirana memorija prethodno

    next64 = (uint64*)next64[0]; // sad je stvarno sledeci, do sad je bio tekuci
    if(prev64){
        prev64[0] = (uint64)next64;
    }
    else{
        headTaken = next64;
    }
    // sad ubacivanje u listu slobodnih

    cur64[0] = 0;
    prev64 = nullptr;
    next64 = (uint64*)headFree;
    for(;next64 && next64 < cur64; prev64 = next64, next64 = (uint64*)next64[0] );

    if(prev64 == nullptr){
        headFree = cur64;
    }
    else{
        if((uint64)prev64 + prev64[1] * MEM_BLOCK_SIZE == (uint64)cur64){
            prev64[1] += cur64[1];
            cur64 = prev64;
        }
        else{
            cur64[0] = (uint64) prev64[0];
            prev64[0] = (uint64) cur64;
        }
    }

    if(next64 != nullptr){
        if((uint64)cur64 + cur64[1] * MEM_BLOCK_SIZE == (uint64)next64){
            cur64[1] += next64[1];
            cur64[0] = (uint64)next64[0];
        }
        else{
            cur64[0] = (uint64)next64;
        }
    }
    return 0;
}

void memoryAllocator::printMemory() {
    init();
    uint64* cur64 = (uint64*)headFree;
    printString("FreeMemory:\n");
    for(; cur64; cur64 = (uint64*)cur64[0]){
        printInteger((uint64)cur64);
        printString(" - ");
        printInteger((uint64)cur64 + cur64[1] * MEM_BLOCK_SIZE);
        printString("\n");
    }
    printString("TakenMemory:\n");
    for(cur64 = (uint64*)headTaken; cur64; cur64 = (uint64*)cur64[0]){
        printInteger((uint64)cur64);
        printString(" - ");
        printInteger((uint64)cur64 + cur64[1] * MEM_BLOCK_SIZE);
        printString("\n");
    }
    printString("\n");
}


