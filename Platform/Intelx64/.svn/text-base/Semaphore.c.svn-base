#include <windows.h>

#include "gc_implementation/concMarkSplit/Platform/Primitives.h"

/* Primitives for local usage */

/* Semaphores */

static void *NBLSemaphoreCreateFn(int init);
static void NBLSemaphoreDeleteFn(void *sem);
static void NBLSemaphoreWaitFn(void *sem);
static int NBLSemaphoreTryWaitFn(void *sem);
static void NBLSemaphoreSignalFn(void *sem);

void *NBLSemaphoreCreateFn(int init)
{
    HANDLE *sem;
    sem=NBLMalloc(sizeof(HANDLE));
    *sem=CreateSemaphore(NULL,init,init,NULL);
    return sem;
}

void NBLSemaphoreDeleteFn(void *sem)
{
    CloseHandle(*(HANDLE *)sem);
    NBLFree(sem);
}

void NBLSemaphoreWaitFn(void *sem)
{
    WaitForSingleObject(*(HANDLE *)sem,INFINITE);
}

int NBLSemaphoreTryWaitFn(void *sem)
{
    return WaitForSingleObject(*(HANDLE *)sem,0)==WAIT_OBJECT_0;
}

void NBLSemaphoreSignalFn(void *sem)
{
    ReleaseSemaphore(*(HANDLE *)sem,1,NULL);
}

void *(*NBLSemaphoreCreate)(int init)=NBLSemaphoreCreateFn;
void (*NBLSemaphoreDelete)(void *sem)=NBLSemaphoreDeleteFn;
void (*NBLSemaphoreWait)(void *sem)=NBLSemaphoreWaitFn;
int (*NBLSemaphoreTryWait)(void *sem)=NBLSemaphoreTryWaitFn;
void (*NBLSemaphoreSignal)(void *sem)=NBLSemaphoreSignalFn;

