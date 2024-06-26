#include "../h/syscall_cpp.hpp"
#include "../h/_console.hpp"
#include "../test/printing.hpp"

static Semaphore *waitForAll;

void idle(void *arg)
{
    while (true)
    {
        Thread::dispatch();
    }
}

class testThreadPREDROK : public Thread
{
private:
    Semaphore *sem;

public:
    testThreadPREDROK(Semaphore *s) : Thread(), sem(s) {}

    void run() override
    {
        int i = 3;
        while (i > 0)
        {
            int id = get_thread_ID();
            int res = sem->timedWait(id);
            if (res == 0)
            {
                printString("entry -> ");
                printInt(id, 10, 0);
                printString("\n");
                time_sleep(id);
                sem->signal();
                printString("exit <- ");
                printInt(id, 10, 0);
                printString("\n");
                i--;
            }
            else if (res == -2)
            {
                printString("timeout --- ");
                printInt(id, 10, 0);
                printString("\n");
            }
            thread_dispatch();
        }
        waitForAll->signal();
    }
};

int modifikacijaPredrok()
{
    printString("TEST Modifikacija predrok 2024\n\n");
    int **matrica;

    matrica = (int **)mem_alloc(7 * sizeof(int *));
    for (int j = 0; j < 7; j++)
    {
        matrica[j] = (int *)mem_alloc(7 * sizeof(int));
    }

    matrica[0][0] = 3;

    for (int j = 0; j < 7; j++)
    {
        mem_free(matrica[j]);
    }
    mem_free(matrica);

    Semaphore *sem = new Semaphore(1);
    waitForAll = new Semaphore(0);

    Thread *threads[10];

    for (int i = 0; i < 10; i++)
    {
        threads[i] = new testThreadPREDROK(sem);
    }
    for (int i = 0; i < 10; i++)
    {
        threads[i]->start();
    }
    for (int i = 0; i < 10; i++)
    {
        waitForAll->wait();
    }
    return 0;
}