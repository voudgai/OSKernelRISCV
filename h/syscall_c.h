#ifndef _syscall_c
#define _syscall_c

#include "../lib/hw.h"

void *mem_alloc(size_t size); // 0x01 // ZAOKRUZITI size NA CELOBROJNE BLOKOVE I POZVATI SISTEMSKI POZIV

int mem_free(void *); // 0x02

class _thread;
typedef _thread *thread_t;
int thread_create(thread_t *handle, void (*start_routine)(void *), void *arg); // 0x11

int thread_exit(); // 0x12

void thread_dispatch(); // 0x13

class _sem;
typedef _sem *sem_t;
int sem_open(
    sem_t *handle,
    unsigned init);

int sem_close(sem_t handle);

int sem_wait(sem_t id);

int sem_signal(sem_t id);

int sem_timedwait(
    sem_t id,
    time_t timeout);

int sem_trywait(sem_t id);

typedef unsigned long time_t;
int time_sleep(time_t);

const int EOF = -1;
char getc();

void putc(char);

#endif