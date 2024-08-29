#include "../h/syscall_cpp.hpp"
#include "../h/_console.hpp"
#include "../test/printing.hpp"

static Semaphore *waitForAll1;

class testThreadAVG2023 : public Thread
{

public:
    void run() override
    {
        static int ID = 0;
        int id = ID++;
        int i = 1; // dont change, be careful with this test
        while (i > 0)
        {
            int x = waitForAll1->timedWait(id * id);

            printString("Thread ID: ");
            printInt(id);
            printString("; ");

            printString("Kod za TimedWait : ");
            printInt(x, 10, 1);
            printString(". \n");

            i--;
        }
        ID--;

        if (ID == 0)
            waitForAll1->signal();
    }
};

int modifikacijaAvg2023()
{
    // ukljuci setMaxThreads u mainu, zakomentarisano je
    waitForAll1 = new Semaphore(0);

    Thread *threads[20];

    for (int i = 0; i < 20; i++)
    {
        threads[i] = new testThreadAVG2023();
    }
    for (int i = 0; i < 20; i++)
    {
        threads[i]->start();
    }
    thread_dispatch();
    // for (int i = 0; i < 20; i++)
    // {
    waitForAll1->wait();
    // }
    return 0;
}