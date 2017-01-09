#ifndef INTELWIN32_SEMAPHORE_H
#define INTELWIN32_SEMAPHORE_H 1

#define NBL_SEMAPHORES 1

extern void *(*NBLSemaphoreCreate)(int init);
extern void (*NBLSemaphoreDelete)(void *sem);
extern void (*NBLSemaphoreWait)(void *sem);
extern int (*NBLSemaphoreTryWait)(void *sem);
extern void (*NBLSemaphoreSignal)(void *sem);

#endif /* INTELWIN32_SEMAPHORE_H */
