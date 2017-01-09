#ifndef NBL_RANDOM_H
#define NBL_RANDOM_H 1


#ifdef __cplusplus
extern "C" {
#endif

typedef struct NBLRandom {
  unsigned int seed[4];
} NBLRandom;

void NBLRandomInit(NBLRandom *r, unsigned int seed);
unsigned int NBLRandomGenNumber(NBLRandom *r);

#ifdef __cplusplus
};
#endif

#endif /* NBL_RANDOM_H */
