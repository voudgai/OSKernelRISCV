#include "../h/syscall_c.hpp"

#include "printing.hpp"

static volatile bool finished[2];

static void sleepyRun(void *arg)
{
    time_t sleep_time = *((time_t *)arg);
    int i = 6;
    while (--i > 0)
    {

        printString("Hello ");
        printInt(sleep_time);
        printString(" !\n");
        time_sleep(sleep_time);
    }
    if (sleep_time % 100 == 0)
        finished[sleep_time / 100 - 1] = true;
    else
        finished[sleep_time / 40 + 1] = true;
}

void testSleeping()
{
    const int sleepy_thread_count = 4;
    time_t sleep_times[sleepy_thread_count] = {100, 200, 40, 80};
    thread_t sleepyThread[sleepy_thread_count];

    for (int i = 0; i < sleepy_thread_count; i++)
    {
        thread_create(&sleepyThread[i], sleepyRun, sleep_times + i);
    }

    while (!(finished[0] && finished[1] && finished[2] && finished[3]))
    {
        thread_dispatch();
    }
}
