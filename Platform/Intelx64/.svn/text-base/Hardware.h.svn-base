#ifndef HARDWARE_H
#define HARDWARE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <intrin.h>

__inline static int __cdecl testandset(volatile int *adress)
{
    return InterlockedCompareExchange((volatile LONG *)adress,1,0)==0;
}

__inline static int compareandswap(volatile int *adress, int oldvalue, int newvalue)
{
    return InterlockedCompareExchange((volatile LONG *)adress,newvalue,oldvalue)==oldvalue;
}

__inline static int compareandswap_o(volatile int *adress, int oldvalue, int newvalue)
{
    return InterlockedCompareExchange((volatile LONG *)adress,newvalue,oldvalue);
}

__inline static void fetchandadd(volatile int *adress, int value)
{
    InterlockedExchangeAdd((volatile LONG *)adress,value);
}

__inline static int atomicswap(volatile int *adress, int value)
{
    return InterlockedExchange((volatile LONG *)adress,value);
}

__inline static __int64 __cdecl testandset64(volatile __int64 *adress)
{
    return InterlockedCompareExchange64(adress,1,0)==0;
}

__inline static int compareandswap64(volatile __int64 *adress, __int64 oldvalue, __int64 newvalue)
{
    return InterlockedCompareExchange64(adress,newvalue,oldvalue)==oldvalue;
}

__inline static __int64 compareandswap64_o(volatile __int64 *adress, __int64 oldvalue, __int64 newvalue)
{
    return InterlockedCompareExchange64(adress,newvalue,oldvalue);
}

__inline static void fetchandadd64(volatile __int64 *adress, __int64 value)
{
    InterlockedExchangeAdd64(adress,value);
}

__inline static __int64 atomicswap64(volatile __int64 *adress, __int64 value)
{
    return InterlockedExchange64(adress,value);
}

__inline static void atomicstore(volatile unsigned int *adress, unsigned int value)
{
    InterlockedExchange((volatile LONG *)adress,value);
}

__inline static void atomicstore64(__int64 *adress, __int64 value)
{
    InterlockedExchange64(adress,value);
}

__inline static unsigned char compareandswap128(volatile __int64 *adress, __int64 oldvalue1, __int64 oldvalue2, __int64 newvalue1, __int64 newvalue2)
{
    __int64 oldvalues[2]={oldvalue1,oldvalue2};
    return _InterlockedCompareExchange128(adress,newvalue1,newvalue2,oldvalues);
}



#ifdef __cplusplus
}
#endif

#endif
