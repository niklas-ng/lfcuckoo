#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <time.h>

#include "Primitives.h"

/* Primitives for local usage */


/* Protection */
#ifdef CHECK_LICENSE
int CheckTimeLicense() {
    time_t t;
    struct tm *t1;
    time(&t);
    t1=localtime(&t);
    if(t1->tm_year>LICENSE_EVAL_YEAR || (t1->tm_year==LICENSE_EVAL_YEAR && t1->tm_mon>LICENSE_EVAL_MONTH))
	return 0;
    return 1;
}
#endif

/* Memory Allocation */

static void * NBLMallocFn(int size);
static void NBLFreeFn(void *p);

void * NBLMallocFn(int size) 
{
    return malloc(size);
}

void NBLFreeFn(void *p)
{
    free(p);
}

void * (*NBLMalloc)(int size)=NBLMallocFn;
void (*NBLFree)(void *p)=NBLFreeFn;
void * (*NBLLocalMalloc)(int size)=NBLMallocFn;
void (*NBLLocalFree)(void *p)=NBLFreeFn;

/* Mutexes */

static void *NBLMutexCreateFn();
static void NBLMutexDeleteFn(void *sem);
static void NBLMutexWaitFn(void *sem);
static int NBLMutexTryWaitFn(void *sem);
static void NBLMutexSignalFn(void *sem);

void *NBLMutexCreateFn()
{
    int *sem;
    sem=(int *)NBLMalloc(sizeof(int));
    *sem=0;
    return (void *)sem;
}

void NBLMutexDeleteFn(void *sem)
{
    NBLFree(sem);
}

void NBLMutexWaitFn(void *sem)
{
    while(!TAS(sem)) {
      while(*((volatile int *)sem)&0x01) {
      }
    }
}

int NBLMutexTryWaitFn(void *sem)
{
    if(!(*((int *)sem)&0x01) && TAS(sem)) {
      return 1;
    }
    return 0;
}

void NBLMutexSignalFn(void *sem)
{
    MEM_WRITE(((int*)sem),0);
}

void *(*NBLMutexCreate)()=NBLMutexCreateFn;
void (*NBLMutexDelete)(void *sem)=NBLMutexDeleteFn;
void (*NBLMutexWait)(void *sem)=NBLMutexWaitFn;
int (*NBLMutexTryWait)(void *sem)=NBLMutexTryWaitFn;
void (*NBLMutexSignal)(void *sem)=NBLMutexSignalFn;

