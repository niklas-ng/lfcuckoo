#ifndef NBL_BACKOFF_H
#define NBL_BACKOFF_H 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL 0
#endif

#include "Random.h"

typedef struct NBLBackOff {
    int paramBackOffType;
    int paramBackOffInit;
    int paramBackOffMax;
    NBLRandom seed;
} NBLBackOff;

void NBLBackOffInit(NBLBackOff *b);
void NBLBackOffCopy(NBLBackOff *b, const NBLBackOff *old);
void *NBLBackOffGetParameter(NBLBackOff *b, int param);
int NBLBackOffSetParameter(NBLBackOff *b, int param, void *value);
int NBLBackOffDo(NBLBackOff *b, int last);

#ifdef __cplusplus
};
#endif

#endif /* NBL_BACKOFF_H */
