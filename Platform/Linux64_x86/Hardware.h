#ifndef HARDWARE_H
#define HARDWARE_H

#ifdef __cplusplus
extern "C" {
#endif
  
inline static int testandset(volatile int *adress)
{
    return __sync_lock_test_and_set(adress,1)==0;
}

inline static int compareandswap(volatile int *adress, int oldvalue, int newvalue)
{
    return __sync_bool_compare_and_swap(adress,oldvalue,newvalue);
}

inline static int compareandswap_o(volatile int *adress, int oldvalue, int newvalue)
{
    return __sync_val_compare_and_swap(adress,oldvalue,newvalue);
}

inline static void fetchandadd(volatile int *adress, int value)
{
    __sync_fetch_and_add(adress,value);
}

inline static int atomicswap(volatile int *adress, int value)
{
	int oldvalue;
	int oldval;
	do {
		oldvalue = *adress;
                oldval=__sync_val_compare_and_swap(adress,oldvalue,value);
	} while(oldvalue!=oldval);
	return oldvalue;
}

inline static int64_t testandset64(volatile int64_t *adress)
{
    return __sync_lock_test_and_set(adress,1)==0;
}

inline static int compareandswap64(volatile int64_t *adress, int64_t oldvalue, int64_t newvalue)
{
    return __sync_bool_compare_and_swap(adress,oldvalue,newvalue);
}

inline static int64_t compareandswap64_o(volatile int64_t *adress, int64_t oldvalue, int64_t newvalue)
{
    return __sync_val_compare_and_swap(adress,oldvalue,newvalue);
}

inline static void fetchandadd64(volatile int64_t *adress, int64_t value)
{
    __sync_fetch_and_add(adress,value);
}

inline static int64_t atomicswap64(volatile int64_t *adress, int64_t value)
{
	int64_t oldvalue;
	int64_t oldval;
	do {
		oldvalue = *adress;
        oldval=__sync_val_compare_and_swap(adress,oldvalue,value);
	} while(oldvalue!=oldval);
	return oldvalue;
}

inline static void atomicstore(volatile unsigned int *adress, unsigned int value)
{
    __asm__ __volatile__(
        "lock; xchg %0,%1"
        : "=m"(*adress), "=r"(value)
        : "m"(*adress), "1"(value)
        : "memory"
    );
}

inline static void atomicstore64(int64_t *adress, int64_t value)
{
    __asm__ __volatile__(
        "lock; xchg %0,%1"
        : "=m"(*adress), "=r"(value)
        : "m"(*adress), "1"(value)
        : "memory"
    );
}

#ifdef __cplusplus
}
#endif

#endif
