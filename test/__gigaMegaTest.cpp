#include "../h/syscall_c.h"
#include "../h/syscall_cpp.hpp"
#include "./printing.hpp"

int producer = 0;
int consumer = 0;
sem_t mutex;

void wrapProduce(void *v)
{
    for (int k = 0; k < 10; k++)
    {
        for (int i = 0; i < 30000; i++)
        {
            for (int j = 0; j < 30000; j++)
            {
            }
        }
        sem_signal(mutex);
        if (k == 7)
        {
            sem_close(mutex);
            mutex = nullptr;
            break;
        }
    }
    producer++;
}

void wrapConsumer(void *v)
{

    if (mutex)
    {
        int x = sem_timedwait(mutex, 150);
        if (x == 0)
        {
            printString("izbacen sa strane semafora\n");
        }
        else if (x == -1)
        {
            printString("izbacen zbog mrtvog semafora\n");
        }
        else
        {
            printString("izbacen zbog timeouta\n");
        }
    }
    consumer++;
}

void wrapWait(void *v)
{
    time_sleep(200);
    printString("uspehSleep1\n");

    if (mutex)
    {
        int x = sem_wait(mutex);
        if (x == 0)
        {
            printString("izbacen sa strane semafora\n");
        }
        else if (x == -1)
        {
            printString("izbacen zbog mrtvog semafora\n");
        }
    }

    time_sleep(100);
    printString("uspehSleep2\n");

    consumer++;
}

void waitTest()
{
    sem_open(&mutex, 0);
    printString("MUTEX created\n");

    thread_t userMainsWait[10];
    for (int i = 0; i < 10; i++)
    {
        thread_create(&userMainsWait[i], &wrapWait, mutex);
        printString("USERMAINW created ");
        printInt(i);
        printString("\n");
        for (int j = 0; j < 30000; j++)
        {
            for (int k = 0; k < 3000; k++)
                ;
        }
    }

    thread_t userMainsConsume[20];
    for (int i = 0; i < 20; i++)
    {
        thread_create(&userMainsConsume[i], &wrapConsumer, mutex);
        printString("USERMAINC created ");
        printInt(i);
        printString("\n");
        for (int j = 0; j < 30000; j++)
        {
            for (int k = 0; k < 3000; k++)
                ;
        }
    }

    thread_t userMain1Produce;
    thread_create(&userMain1Produce, &wrapProduce, mutex);
    printString("USERMAIN1P created\n");

    while (producer != 1 || consumer != 30)
    {
        thread_dispatch();
    }
}

void mainCopeTest(void *p)
{

    // ovo treba se pozove

    waitTest();

    return;
}