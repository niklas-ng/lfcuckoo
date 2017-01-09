/* ***********************************************************************

   Algorithm: Commodore Business Machines Inc. SID. 1982.
   Implementation: Håkan Sundell

   *********************************************************************** */

#include "Random.h"

void NBLRandomInit(NBLRandom *r, unsigned int seed)
{
    int i;
    for(i=0;i<4;i++) {
      r->seed[i]=seed;
      seed=seed*12345+seed%12345;
    }
}

unsigned int NBLRandomGenNumber(NBLRandom *r)
{
    int i;
    unsigned int number=0;
    for(i=0;i<4;i++) {
      unsigned int data=((r->seed[i]>>(22-7))&0x80)|((r->seed[i]>>(20-6))&0x40)|
	((r->seed[i]>>(16-5))&0x20)|((r->seed[i]>>(13-4))&0x10)|
	((r->seed[i]>>(11-3))&0x08)|((r->seed[i]>>(7-2))&0x04)|
	((r->seed[i]>>(4-1))&0x02)|((r->seed[i]>>(2-0))&0x01);
      r->seed[i]=(r->seed[i]+r->seed[i])|(((r->seed[i]>>22)^(r->seed[i]>>17))&0x01);
      number=(number<<8)+data;
    }
    return number;
}
