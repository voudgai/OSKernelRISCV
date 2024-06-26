#include "../h/syscall_cpp.hpp"
#include "../h/_console.hpp"
#include "../test/printing.hpp"

static Semaphore *waitForAll;

class testThreadJUN : public Thread
{
public:
    void run() override
    {
        int i = 3;
        while (i > 0)
        {
            int id = get_thread_ID();
            for (int k = 0; k < 1; k++) // testing the console
            {

                printString("Thread number: ");
                printInt(id - 5);
                printString("\n");
            }
            for (int j = 0; j < 100000000; j++)
                ;
            thread_dispatch();
            i--;
        }
        waitForAll->signal();
    }
};

int modifikacijaJun()
{
    waitForAll = new Semaphore(0);

    Thread *threads[20];

    for (int i = 0; i < 20; i++)
    {
        threads[i] = new testThreadJUN();
    }
    for (int i = 0; i < 20; i++)
    {
        threads[i]->start();
        if (i == 9)
        {
            thread_dispatch();
            int time = 10;
            printString("Joining first ");
            printInt(i + 1);
            printString(" threads or ");
            printInt(time);
            printString("/10 sec...\n");

            int childWokeMeUp = join_all(time);

            printString("Returned from join because ");
            if (childWokeMeUp)
                printString("children are finished!\n");
            else
                printString("time exceeded!\n");
        }
    }
    for (int i = 0; i < 20; i++)
    {
        waitForAll->wait();
    }
    return 0;
}