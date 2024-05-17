//
// Created by marko on 20.4.22..
//

#include "../lib/hw.h"
#include "../h/_thread.hpp"
#include "../h/print.hpp"
#include "../h/syscall_c.hpp"
void workerBodyA(_sem* mutexAB)
{
    for (uint64 i = 0; i < 10; i++)
    {
        printString("A: i=");
        printInteger(i);
        printString("\n");
        thread_dispatch();

        sem_wait(mutexAB);
        printString("A: B blocked\n");

        for (uint64 j = 0; j < 5; j++)
        {
            for (uint64 k = 0; k < 300; k++)
            {
                // busy wait
            }
            printString("A: THREAD A DISPATCHES\n");
            thread_dispatch();
        }

        printString("A: UNBLOCKING B :*\n");
        sem_signal(mutexAB);
        thread_dispatch();
    }
    printString("A: THREAD A DYING BYE BYE.\n");
}

void workerBodyB(_sem* mutexAB)
{
    for (uint64 i = 0; i < 16; i++)
    {
        printString("B: i=");
        printInteger(i);
        printString("\n");
        for (uint64 j = 0; j < 5; j++)
        {
            uint x =(uint64)sem_wait(mutexAB);
            printString("B sem_wait: ");
            printInteger(x);
            printString("\n");

            printString("B: A blocked\n\n");

            for (uint64 k = 0; k < 300; k++)
            {
                // busy wait
            }
            printString("B: THREAD B DISPATCHES.\n");
            thread_dispatch();
            printString("B: A unblocked\n");
            sem_signal(mutexAB);
        }
        printString("B: THREAD B DISPATCHES.\n");
        thread_dispatch();
    }

    printString("B: THREAD B DYING BYE BYE.\n");
}

static uint64 fibonacci(uint64 n)
{
    if (n == 0 || n == 1) { return n; }
    if (n % 10 == 0) {
        /*printString("FIBONACCI DISPATCH\n");*/
        thread_dispatch();
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void workerBodyC()
{
    uint8 i = 0;
    for (; i < 3; i++)
    {
        printString("C: i=");
        printInteger(i);
        printString("\n");
    }

    printString("C: yield\n");
    __asm__ ("li t1, 7");
    thread_dispatch();

    uint64 t1 = 0;
    __asm__ ("mv %[t1], t1" : [t1] "=r"(t1));

    printString("C: t1=");
    printInteger(t1);
    printString("\n");

    printString("C: THREAD C DYING BYE BYE.\n");
    thread_exit();
    printString("C: SIKE, somehow lived\n");

    uint64 result = fibonacci(12);
    printString("C: fibonaci=");
    printInteger(result);
    printString("\n");

    for (; i < 6; i++)
    {
        printString("C: i=");
        printInteger(i);
        printString("\n");
    }
//    TCB::yield();
}

void workerBodyD()
{
    uint8 i = 10;
    for (; i < 13; i++)
    {
        printString("D: i=");
        printInteger(i);
        printString("\n");
    }

    printString("D: yield\n");
    __asm__ ("li t1, 5");
    thread_dispatch();

    uint64 result = fibonacci(16);
    printString("D: fibonaci=");
    printInteger(result);
    printString("\n");

    for (; i < 16; i++)
    {
        printString("D: i=");
        printInteger(i);
        printString("\n");
    }
    printString("D: THREAD D DYING BYE BYE.\n");
}