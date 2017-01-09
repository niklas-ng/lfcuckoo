#ifndef PRIMITIVES_H
#define PRIMITIVES_H 1

#ifdef __cplusplus
extern "C" {
#endif

extern void * (*NBLMalloc)(int size);
extern void (*NBLFree)(void *p);
extern void * (*NBLLocalMalloc)(int size);
extern void (*NBLLocalFree)(void *p);

extern void *(*NBLMutexCreate)();
extern void (*NBLMutexDelete)(void *sem);
extern void (*NBLMutexWait)(void *sem);
extern int (*NBLMutexTryWait)(void *sem);
extern void (*NBLMutexSignal)(void *sem);

//#define CHECK_LICENSE 1

#ifdef CHECK_LICENSE
int CheckTimeLicense();
#define CHECK_TIME_LICENSE if(!CheckTimeLicense()) return NULL;
#else
#define CHECK_TIME_LICENSE
#endif /* CHECK_LICENSE */

#ifdef OSX_POWERPC
#include "OSX_PowerPC/Semaphore.h"
#include "OSX_PowerPC/Hardware.h"

#define POINTER_INT int

#define SWAPP(x,y) (void *)(atomicswap((unsigned*)(x),(unsigned)(y)))
#define SWAP(x,y) atomicswap((unsigned*)(x),(unsigned)(y))
#define CASP(x,y,z) compareandswap((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS(x,y,z) compareandswap((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CASP_O(x,y,z) compareandswap_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS_O(x,y,z) compareandswap_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define TAS(x) testandset(x)
#define FAA(x,y) fetchandadd(x,y)
#define MEM_WRITE(x,y) *(x)=y
#define MEM_READ(x) *(x)
#define MEM_WRITE_P(x,y) *(x)=y
#define MEM_READ_P(x) *(x)
#endif


#ifdef AIX_POWERPC
#include "AIX_PowerPC/Semaphore.h"
#include "AIX_PowerPC/Hardware.h"

#define POINTER_INT int

#define SWAPP(x,y) (void *)(atomicswap((unsigned*)(x),(unsigned)(y)))
#define SWAP(x,y) atomicswap((unsigned*)(x),(unsigned)(y))
#define CASP(x,y,z) compareandswap((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS(x,y,z) compareandswap((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CASP_O(x,y,z) compareandswap_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS_O(x,y,z) compareandswap_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define TAS(x) testandset(x)
#define FAA(x,y) fetchandadd(x,y)
#define MEM_WRITE(x,y) atomicwrite((unsigned int *)(x),(unsigned int)(y))
#define MEM_READ(x) atomicread((unsigned int *)(x))
#define MEM_WRITE_P(x,y) atomicwrite((unsigned int *)(x),(unsigned int)(y))
#define MEM_READ_P(x) (void*)atomicread((unsigned int *)(x))
#endif

#ifdef AIX64_POWERPC
#include "AIX_PowerPC/Semaphore.h"
#include "AIX_PowerPC/Hardware64.h"

#define POINTER_64
#define POINTER_INT long
#define int64_t long

#define SWAPP(x,y) (void *)(atomicswap64((unsigned long *)(x),(unsigned long)(y)))
#define SWAP(x,y) atomicswap((unsigned*)(x),(unsigned)(y))
#define CASP(x,y,z) compareandswap64((unsigned long *)(x),(unsigned long)(y),(unsigned long)(z))
#define CAS(x,y,z) compareandswap((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CASP_O(x,y,z) compareandswap64_o((unsigned long *)(x),(unsigned long)(y),(unsigned long)(z))
#define CAS_O(x,y,z) compareandswap_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define TAS(x) testandset(x)
#define FAA(x,y) fetchandadd(x,y)
#define MEM_WRITE(x,y) atomicwrite((unsigned int *)(x),(unsigned int)(y))
#define MEM_READ(x) atomicread((unsigned int *)(x))
#define MEM_WRITE_P(x,y) atomicwrite64((unsigned long *)(x),(unsigned long)(y))
#define MEM_READ_P(x) (void*)atomicread64((unsigned long *)(x))
#endif


#ifdef Linux_POWERPC
#include "Linux_PowerPC/Semaphore.h"
#include "Linux_PowerPC/Hardware.h"

#define POINTER_INT int

#define SWAPP(x,y) (void *)(atomicswap((unsigned*)(x),(unsigned)(y)))
#define SWAP(x,y) atomicswap((unsigned*)(x),(unsigned)(y))
#define CASP(x,y,z) compareandswap((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS(x,y,z) compareandswap((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CASP_O(x,y,z) compareandswap_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS_O(x,y,z) compareandswap_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define TAS(x) testandset(x)
#define FAA(x,y) fetchandadd(x,y)
#define MEM_WRITE(x,y) *(x)=y
#define MEM_READ(x) *(x)
#define MEM_WRITE_P(x,y) *(x)=y
#define MEM_READ_P(x) *(x)
#endif

#ifdef LINUX32_X86

#define POINTER_INT int

#include "Linux32_x86/Semaphore.h"
#include "Linux32_x86/Hardware.h"

#define SWAPP(x,y) (void *)(atomicswap((unsigned*)(x),(unsigned)(y)))
#define SWAP(x,y) atomicswap((int *)(x),(int)(y))
#define CASP(x,y,z) compareandswap((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS(x,y,z) compareandswap((int *)(x),(int)(y),(int)(z))
#define CASP_O(x,y,z) compareandswap_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS_O(x,y,z) compareandswap_o((int *)(x),(int)(y),(int)(z))
#define TAS(x) testandset((int *)(x))
#define FAA(x,y) fetchandadd((int *)(x),(int)(y))
#define MEM_WRITE(x,y) atomicstore((unsigned *)(x),(unsigned)(y))
#define MEM_READ(x) *(unsigned volatile *)(x)
#define MEM_WRITE_P(x,y) atomicstore((unsigned *)(x),(unsigned)(y))
#define MEM_READ_P(x) *(void * volatile *)(x)
#endif

#ifdef LINUX64_X86

#define POINTER_64
#define POINTER_INT long
#define int64_t long

#include "Linux64_x86/Semaphore.h"
#include "Linux64_x86/Hardware.h"

#define SWAPP(x,y) (void *)(atomicswap64((int64_t*)(x),(int64_t)(y)))
#define SWAP(x,y) atomicswap((int *)(x),(int)(y))
#define CASP(x,y,z) compareandswap64((int64_t*)(x),(int64_t)(y),(int64_t)(z))
#define CAS(x,y,z) compareandswap((int *)(x),(int)(y),(int)(z))
#define CASP_O(x,y,z) compareandswap64_o((int64_t*)(x),(int64_t)(y),(int64_t)(z))
#define CAS_O(x,y,z) compareandswap_o((int *)(x),(int)(y),(int)(z))
#define TAS(x) testandset((int *)(x))
#define FAA(x,y) fetchandadd((int *)(x),(int)(y))
#define MEM_WRITE(x,y) atomicstore((unsigned *)(x),(unsigned)(y))
#define MEM_READ(x) *(unsigned volatile *)(x)
#define MEM_WRITE_P(x,y) atomicstore64((int64_t *)(x),(int64_t)(y))
#define MEM_READ_P(x) *(void * volatile *)(x)
#endif

#ifdef _WIN64
#include "Intelx64/Semaphore.h"
#include "Intelx64/Hardware.h"

#define __thread __declspec(thread)

#define POINTER_INT __int64
#ifndef int64_t
#define int64_t long long
#endif

#define SWAPP(x,y) (void *)(atomicswap64((__int64*)(x),(__int64)(y)))
#define SWAP(x,y) atomicswap((int *)(x),(int)(y))
#define DWCASP(x,y1,y2,z1,z2) compareandswap128((__int64*)(x),(__int64)(y1),(__int64)(y2),(__int64)(z1),(__int64)(z2))
#define CASP(x,y,z) compareandswap64((__int64*)(x),(__int64)(y),(__int64)(z))
#define CAS(x,y,z) compareandswap((int *)(x),(int)(y),(int)(z))
#define CASP_O(x,y,z) compareandswap64_o((__int64*)(x),(__int64)(y),(__int64)(z))
#define CAS_O(x,y,z) compareandswap_o((int *)(x),(int)(y),(int)(z))
#define TAS(x) testandset((int *)(x))
#define FAA(x,y) fetchandadd((int *)(x),(int)(y))
#define MEM_WRITE(x,y) atomicstore((unsigned *)(x),(unsigned)(y))
//#define MEM_WRITE(x,y) *(x)=y
#define MEM_READ(x) *(unsigned volatile *)(x)
#define MEM_WRITE_P(x,y) atomicstore64((__int64 *)(x),(__int64)(y))
//#define MEM_WRITE_P(x,y) *(void * volatile *)(x)=(void *)(y)
#define MEM_READ_P(x) *(void * volatile *)(x)
#endif

#ifdef WIN32
#ifndef _WIN64
#include "IntelWin32/Semaphore.h"
#include "IntelWin32/Hardware.h"

#define __thread __declspec(thread)

#define POINTER_INT int

#define SWAPP(x,y) (void *)(atomicswap((unsigned*)(x),(unsigned)(y)))
#define SWAP(x,y) atomicswap((unsigned*)(x),(unsigned)(y))
#define CASP(x,y,z) compareandswap((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS(x,y,z) compareandswap((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CASP_O(x,y,z) compareandswap_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS_O(x,y,z) compareandswap_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define TAS(x) testandset((unsigned*)(x))
#define FAA(x,y) fetchandadd((unsigned volatile *)(x),(unsigned)(y))
#define MEM_WRITE(x,y) atomicstore((unsigned *)(x),(unsigned)(y))
//#define MEM_READ(x) atomicload((unsigned *)(x))
#define MEM_WRITE_P(x,y) atomicstore((unsigned *)(x),(unsigned)(y))
//#define MEM_READ_P(x) (void *)atomicload((unsigned *)(x))

//#define MEM_WRITE(x,y) *(x)=y
#define MEM_READ(x) *(unsigned volatile *)(x)
//#define MEM_WRITE_P(x,y) *(void * volatile *)(x)=(void *)(y)
#define MEM_READ_P(x) *(void * volatile *)(x)

#endif
#endif


#ifdef SOLARIS_ULTRASPARC
#include "Solaris_UltraSparc/Semaphore.h"
#include "Solaris_UltraSparc/Primitives.h"

#define POINTER_INT int

#define SWAPP(x,y) (void *)(atomicswap((unsigned*)(x),(unsigned)(y)))
#define SWAP(x,y) atomicswap((unsigned*)(x),(unsigned)(y))
#define CASP(x,y,z) cas((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS(x,y,z) cas((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CASP_O(x,y,z) cas_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS_O(x,y,z) cas_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define TAS(x) tas((unsigned*)(x))
#define FAA(x,y) atomic_fas((unsigned*)(x),(unsigned)(y))
#define MEM_WRITE(x,y) mem_write((unsigned int *)(x),(unsigned int)(y))
#define MEM_READ(x) mem_read((unsigned int *)(x))
#define MEM_WRITE_P(x,y) mem_write((unsigned int *)(x),(unsigned int)(y))
#define MEM_READ_P(x) (void*)mem_read((unsigned int *)(x))
#endif


#ifdef SOLARIS64_ULTRASPARC
#include "Solaris_UltraSparc/Semaphore.h"
#include "Solaris_UltraSparc/Primitives64.h"

#define POINTER_64
#define POINTER_INT long
#define int64_t long

#define SWAPP(x,y) (void *)(atomicswap64((unsigned long*)(x),(unsigned long)(y)))
#define SWAP(x,y) atomicswap((unsigned*)(x),(unsigned)(y))
#define SWAP64(x,y) atomicswap64((unsigned long*)(x),(unsigned long)(y))
#define CASP(x,y,z) cas64((unsigned long *)(x),(unsigned long)(y),(unsigned long)(z))
#define CAS(x,y,z) cas((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CASP_O(x,y,z) cas64_o((unsigned long *)(x),(unsigned long)(y),(unsigned long)(z))
#define CAS_O(x,y,z) cas_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define TAS(x) tas((unsigned*)(x))
#define FAA(x,y) atomic_fas((unsigned*)(x),(unsigned)(y))
#define CAS64(x,y,z) cas64((unsigned long*)(x),(unsigned long)(y),(unsigned long)(z))
#define TAS64(x) tas64((unsigned long*)(x))
#define FAA64(x,y) atomic_fas64((unsigned long*)(x),(unsigned long)(y))
#define MEM_WRITE(x,y) mem_write((unsigned int *)(x),(unsigned int)(y))
#define MEM_READ(x) mem_read((unsigned int *)(x))
#define MEM_WRITE_P(x,y) mem_write64((unsigned long *)(x),(unsigned long)(y))
#define MEM_READ_P(x) (void*)mem_read64((unsigned long *)(x))
#define MEM_WRITE_64(x,y) mem_write64((unsigned long *)(x),(unsigned long)(y))
#define MEM_READ_64(x) mem_read64((unsigned long *)(x))
#endif


#ifdef IRIX_MIPS
#include "IRIX_MIPS/Semaphore.h"
#include "IRIX_MIPS/Primitives64.h"

#define POINTER_INT int

#define SWAPP(x,y) (void *)(atomicswap((unsigned*)(x),(unsigned)(y)))
#define SWAP(x,y) atomicswap((unsigned*)(x),(unsigned)(y))
#define CASP(x,y,z) cas((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS(x,y,z) cas((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CASP_O(x,y,z) cas_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS_O(x,y,z) cas_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define TAS(x) (!tas((unsigned*)(x)))
#define FAA(x,y) faa((unsigned*)(x),(unsigned)(y))
#define MEM_WRITE(x,y) *(x)=y
#define MEM_READ(x) *(x)
#define MEM_WRITE_P(x,y) *(x)=y
#define MEM_READ_P(x) *(x)
#endif


#ifdef IRIX64_MIPS
#include "IRIX_MIPS/Semaphore.h"
#include "IRIX_MIPS/Primitives64.h"

#define POINTER_64 1
#define POINTER_INT long
#define int64_t long

#define SWAPP(x,y) (void *)(atomicswap64((unsigned long*)(x),(unsigned long)(y)))
#define SWAP(x,y) atomicswap((unsigned*)(x),(unsigned)(y))
#define SWAP64(x,y) atomicswap64((unsigned long*)(x),(unsigned long)(y))
#define CASP(x,y,z) cas64((unsigned long*)(x),(unsigned long)(y),(unsigned long)(z))

#define CAS(x,y,z) cas((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CASP_O(x,y,z) cas64_o((unsigned long*)(x),(unsigned long)(y),(unsigned long)(z))

#define CAS_O(x,y,z) cas_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define TAS(x) (!tas((unsigned int *)(x)))
#define FAA(x,y) faa((unsigned int *)(x),(unsigned int)(y))
#define CAS64(x,y,z) cas64((unsigned long*)(x),(unsigned long)(y),(unsigned long)(z))
#define TAS64(x) (!tas64((unsigned long *)(x)))
#define FAA64(x,y) faa64((unsigned long *)(x),(unsigned long)(y))
#define MEM_WRITE(x,y) *(x)=y
#define MEM_READ(x) *(x)
#define MEM_WRITE_P(x,y) *(x)=y
#define MEM_READ_P(x) *(x)
#define MEM_WRITE_64(x,y) *(x)=y
#define MEM_READ_64(x) *(x)
#endif


#ifdef LINUX_PENTIUM
#include "Linux_Pentium/Hardware.h"
#include "Linux_Pentium/Semaphore.h"

#define POINTER_INT int
#define int64_t long long

#define SWAPP(x,y) (void *)(atomicswap((unsigned*)(x),(unsigned)(y)))
#define SWAP(x,y) atomicswap((unsigned*)(x),(unsigned)(y))
#define SWAP64(x,y) atomicswap64((unsigned long long *)(x),(unsigned long long)(y))
#define CASP(x,y,z) compareandswap((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS(x,y,z) compareandswap((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS64(x,y,z) compareandswap64((unsigned long long*)(x),(unsigned long long)(y),(unsigned long long)(z))
#define CASP_O(x,y,z) compareandswap_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS_O(x,y,z) compareandswap_o((unsigned*)(x),(unsigned)(y),(unsigned)(z))
#define CAS64_O(x,y,z) compareandswap64_o((unsigned long long*)(x),(unsigned long long)(y),(unsigned long long)(z))
#define TAS(x) testandset((unsigned*)(x))
#define TAS64(x) testandset64((unsigned long long*)(x))
#define FAA(x,y) fetchandadd((unsigned*)(x),(unsigned)(y))
#define FAA64(x,y) fetchandadd64((unsigned long long*)(x),(unsigned long long)(y))
#define MEM_WRITE(x,y) *(x)=y
#define MEM_READ(x) *(x)
#define MEM_WRITE_64(x,y) *(x)=y
#define MEM_READ_64(x) *(x)
#define MEM_WRITE_P(x,y) *(x)=y
#define MEM_READ_P(x) *(x)
#endif


#ifdef __cplusplus
}
#endif

#endif /* PRIMITIVES_H */
