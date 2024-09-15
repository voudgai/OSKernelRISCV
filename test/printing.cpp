//
// Created by os on 5/18/22.
//

#include "printing.hpp"

uint64 lockPrint = 0;

#define LOCK()                             \
    while (copy_and_swap(lockPrint, 0, 1)) \
    thread_dispatch()
#define UNLOCK() while (copy_and_swap(lockPrint, 1, 0))

void printString(char const *string)
{
    LOCK();
    while (*string != '\0')
    {
        putc(*string);
        string++;
    }
    UNLOCK();
}

char *getString(char *buf, int max)
{
    LOCK();
    int i, cc;
    char c;

    for (i = 0; i + 1 < max;)
    {
        cc = getc();
        if (cc < 1)
            break;
        c = cc;
        buf[i++] = c;
        if (c == '\n' || c == '\r')
            break;
    }
    buf[i] = '\0';

    UNLOCK();
    return buf;
}

int stringToInt(const char *s)
{
    int n;

    n = 0;
    while ('0' <= *s && *s <= '9')
        n = n * 10 + *s++ - '0';
    return n;
}

char digits[] = "0123456789ABCDEF";

void printInt(int xx, int base, int sgn)
{
    LOCK();
    char buf[16];
    int i, neg;
    uint x;

    neg = 0;
    if (sgn && xx < 0)
    {
        neg = 1;
        x = -xx;
    }
    else
    {
        x = xx;
    }

    i = 0;
    do
    {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);
    if (neg)
        buf[i++] = '-';

    while (--i >= 0)
        putc(buf[i]);

    UNLOCK();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

static int numOfStdTests = 7;
static int numOfStdQuestions = 1;
static int numOfModifications = 0;

static char const *stdTestNames[][2] = {
    {"Implementirani testovi: \n", "Implemented tests: \n"},
    {"\t1 - 4 niti sa promenom konteksta (C API).\n", "\t1 - 4 threads with context switching (C API).\n"},
    {"\t2 - 4 niti sa promenom konteksta (CPP API).\n", "\t2 - 4 threads with context switching (CPP API).\n"},
    {"\t3 - potrošač-proizvođač (C API).\n", "\t3 - consumer-producer (C API).\n"},
    {"\t4 - potrošač-proizvođač (CPP API).\n", "\t4 - consumer-producer (CPP API).\n"},
    {"\t5 - 2 niti koje spavaju različito vreme.\n", "\t5 - 2 threads sleeping different times.\n"},
    {"\t6 - potrošač-proizvođač sa preotimanjem CPU-a, bez sinhronizovane promene konteksta.\n", "\t6 - consumer-producer with preemption, without synchronized context switch.\n"},
    {"\t7 - test greške, nelegalna instrukcija.\n", "\t7 - error test, illegal instruction.\n"},
    {"\t8 - za biranje DRUGIH testova (modifikacija)\n", "\t8 - for choosing OTHER tests (modifications)\n"}};

static char const *modificationTestNames[][2] = {
    {"\tNema implementiranih modifikacija :(\n", "\tNo modifications implemented :(\n"}};

static char const *chooseTest[][2] = {{"Odaberite test: ", "Choose test number: "}};

static int chooseLanguage();
static inline int chooseTestOrModification(bool inEnglish);
static inline int chooseFromModifications(bool inEnglish);

int getTestID_printTestInfo()
{

    static int inEnglish = chooseLanguage();
    if (inEnglish < 0)
        return -1;

    int test = chooseTestOrModification(inEnglish);

    if (test == numOfStdQuestions + numOfStdTests) // chose from modifications
        test = chooseFromModifications(inEnglish);

    if (test < 0)
    {
        printString("Wrong input!\n");
        return -1;
    }

    putc('\n');
    putc('\n');
    return test;
}

static int chooseLanguage()
{
    printString("Enter 1 for ENGLISH, enter 0 for SERBIAN: ");

    int inEnglish = getc() - '0';
    getc(); // to get enter

    printString("\n\n");
    if (inEnglish < 0 || inEnglish > 1)
    {
        printString("Wrong input!\n");
        return -1;
    }
    return inEnglish;
}

static inline int chooseTestOrModification(bool inEnglish)
{
    for (int i = 0; i <= numOfStdTests + numOfStdQuestions; i++)
    {
        printString(stdTestNames[i][inEnglish]);
    }
    printString(chooseTest[0][inEnglish]);

    int test = getc() - '0';
    getc(); // for enter after number
    putc('\n');

    if (test > numOfStdTests + numOfStdQuestions || test < 1)
        return -1;

    return test;
}

static inline int chooseFromModifications(bool inEnglish)
{
    for (int i = 0; i <= numOfModifications; i++)
    {
        printString(modificationTestNames[i][inEnglish]);
    }
    printString(chooseTest[0][inEnglish]);

    int test = getc() - '0';
    getc(); // for enter after number

    if (test > numOfStdTests + numOfStdQuestions + numOfModifications || test < 1)
        return -1;

    return test;
}