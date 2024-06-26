#include "../h/syscall_cpp.hpp"
#include "../h/_console.hpp"
#include "../test/printing.hpp"

static Semaphore *waitForAll1;

class testThreadAVG2023 : public Thread
{

public:
    void run() override
    {
        int i = 1;
        while (i > 0)
        {
            int id = get_thread_ID();
            time_sleep(id * 2);
            printString("Thread ID: ");
            printInt(id);
            printString(".\n");
            i--;
        }
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
    for (int i = 0; i < 20; i++)
    {
        waitForAll1->wait();
    }
    return 0;
}