#include "mertwist.h"

class BackOff {
private:
	int _initBOValue, _MaxBOValue;
	int backoff_value; //hold value for the next backoff call
public:
	BackOff(int initValue, int limit) {
		_initBOValue = initValue;
		_MaxBOValue = limit;
		backoff_value  = initValue;
		init_genrand(45435);
	}

	int doBackOff(int last) {
		int count;
		volatile int temp=0;
		int a=1;
		if (last=0) backoff_value = _initBOValue;
		if (backoff_value>_MaxBOValue)
			backoff_value = _initBOValue;
		for(count=0;count<backoff_value;count++) {
			a=a*count;
		}
		*((volatile int *)&temp)=a;
		backoff_value = backoff_value + backoff_value + (doac_rand_uniform()%_initBOValue);
		return backoff_value;
	}
};