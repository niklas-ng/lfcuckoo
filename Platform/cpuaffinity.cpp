#include "cpuaffinity.h"

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <parallel/algorithm>
#endif

void setThreadAffinity(int ordThread, platform_desc_t *platform) {
#ifdef _MSC_VER
	SetThreadAffinityMask(GetCurrentThread(),1<<((ordThread)%4));
#else //OTHER OS than Windows
	cpu_set_t mask;
	CPU_ZERO(&mask);
	if (platform->_strat == JUMP_SOCKET) {
		if (platform->_arch == AMD_BULLDOZER) {
			int id = (platform->_nrPU_per_sock*(ordThread%platform->_nrSockets) + ordThread/platform->_nrSockets)%CPU_SETSIZE;
			CPU_SET(id,&mask);
			sched_setaffinity(0, sizeof(cpu_set_t),
				&mask);
		} else {//INTEL or any other is assumed JUMP_SOCKET
			int id = ordThread%CPU_SETSIZE;
			CPU_SET(id,&mask);
			sched_setaffinity(0, sizeof(cpu_set_t),
				&mask);
		}	
  	} else {
		if (platform->_strat == FILL_SOCKET) {
			if (platform->_arch == AMD_BULLDOZER) {
				int id = ordThread%CPU_SETSIZE;
				CPU_SET(id,&mask);
				sched_setaffinity(0, sizeof(cpu_set_t),
					&mask);	
			}else {
				int id = (platform->_nrSockets*(ordThread%platform->_nrPU_per_sock) + ordThread/platform->_nrPU_per_sock)%CPU_SETSIZE;
				CPU_SET(id,&mask);
				sched_setaffinity(0, sizeof(cpu_set_t),
					&mask);
			}
		} else {
			if (platform->_arch == AMD_BULLDOZER) {
				int id = (platform->_nrPU_per_sock*(ordThread%platform->_nrSockets) + ordThread/platform->_nrSockets)%CPU_SETSIZE;
				CPU_SET(id,&mask);
				sched_setaffinity(0, sizeof(cpu_set_t),
					&mask);
			}else {// INTEL or unknown CPU will be ti -> pi
				int id = ordThread%CPU_SETSIZE;
				CPU_SET(id,&mask);
				sched_setaffinity(0, sizeof(cpu_set_t),
				&mask);
			}	
		}
	}
#endif // _MSC_VER
}
