#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <semaphore.h>

#include "Platform/Primitives.h"

/* Primitives for local usage */

/* Semaphores */

static void *NBLSemaphoreCreateFn(int init);
static void NBLSemaphoreDeleteFn(void *sem);
static void NBLSemaphoreWaitFn(void *sem);
static int NBLSemaphoreTryWaitFn(void *sem);
static void NBLSemaphoreSignalFn(void *sem);

void *NBLSemaphoreCreateFn(int init)
{
    sem_t *sem;
    sem=(sem_t *)NBLMalloc(sizeof(sem_t));
    sem_init(sem,1,init);
    return (void *)sem;
}

void NBLSemaphoreDeleteFn(void *sem)
{
    sem_destroy((sem_t *)sem);
    NBLFree((void *)sem);
}

void NBLSemaphoreWaitFn(void *sem)
{
    sem_wait((sem_t *)sem);
}

int NBLSemaphoreTryWaitFn(void *sem)
{
    return !sem_trywait((sem_t *)sem);
}

void NBLSemaphoreSignalFn(void *sem)
{
    sem_post((sem_t *)sem);
}

void *(*NBLSemaphoreCreate)(int init)=NBLSemaphoreCreateFn;
void (*NBLSemaphoreDelete)(void *sem)=NBLSemaphoreDeleteFn;
void (*NBLSemaphoreWait)(void *sem)=NBLSemaphoreWaitFn;
int (*NBLSemaphoreTryWait)(void *sem)=NBLSemaphoreTryWaitFn;
void (*NBLSemaphoreSignal)(void *sem)=NBLSemaphoreSignalFn;

