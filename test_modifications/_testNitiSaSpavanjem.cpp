#include "../h/syscall_c.h"
#include "../h/syscall_cpp.hpp"
#include "../test/printing.hpp"
int finishedSpavanje = 0;

void testNitiStampac(void *);

void testNitiSpavanjac(void *ptr)
{
    int *num = (int *)ptr;
    printString("sleepTest ");
    printInt(*num);
    putc('\n');
    for (int i = 0; i < 10; i++)
    {
        time_sleep(100);
        printString("dremnuo ");
        printInt(*num);
        putc('\n');
    }
    printString("uspehSleep1\n");
    if (*num == 20)
        finishedSpavanje = 1;
}

void testNitiStampac(void *ptr)
{
    int *num = (int *)ptr;
    printString("stampac ");
    printInt(*num);
    putc('\n');
    uint64 i = 0;
    while (!finishedSpavanje)
    {
        i++;
        if (i % 150000000 == 0)
        {
            printString("stampanje gluposti!\n");
        }
    }
}

void testNitiWrapper(void *)
{
    printString("test krenuo!\n");
    int nStamp = 3;
    int brojevi[nStamp];
    _thread *handlesPrint[nStamp];
    for (int i = 0; i < nStamp; i++)
    {
        brojevi[i] = i + 1;
        thread_create(&handlesPrint[i], testNitiStampac, &brojevi[i]);
        // printString("napravljen stampac\n");
    }

    nStamp *= 2;
    int brojevi2[nStamp];
    _thread *handlesWait[nStamp];
    for (int i = 0; i < nStamp; i++)
    {
        brojevi2[i] = i + 1;
        thread_create(&handlesWait[i], testNitiSpavanjac, &brojevi2[i]);
        // printString("napravljen SPAVAC\n");
    }
    while (!finishedSpavanje)
    {
        ;
    }
}