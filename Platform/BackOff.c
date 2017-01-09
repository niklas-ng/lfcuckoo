#include "Noble.h"
#include "Primitives.h"
#include "BackOff.h"

void NBLBackOffInit(NBLBackOff *b)
{
    b->paramBackOffType=BOT_RANDOM_EXPONENTIAL;
    b->paramBackOffInit=100;
    b->paramBackOffMax=1000000;
    NBLRandomInit(&b->seed,(unsigned int)(0x12345678^(POINTER_INT)b));
}

void NBLBackOffCopy(NBLBackOff *b, const NBLBackOff *old)
{
    b->paramBackOffType = old->paramBackOffType;
    b->paramBackOffInit = old->paramBackOffInit;
    b->paramBackOffMax = old->paramBackOffMax;
    NBLRandomInit(&b->seed,(unsigned int)(0x12345678^(POINTER_INT)b));
}

void *NBLBackOffGetParameter(NBLBackOff *b, int param)
{
    switch(param) {
    case PARAM_BACK_OFF_TYPE:
      return (void*)(POINTER_INT)(b->paramBackOffType);
      break;
    case PARAM_BACK_OFF_INIT:
      return (void*)(POINTER_INT)(b->paramBackOffInit);
      break;
    case PARAM_BACK_OFF_MAX:
      return (void*)(POINTER_INT)(b->paramBackOffMax);
      break;
    default:
      break;
    }
    return NULL;
}

int NBLBackOffSetParameter(NBLBackOff *b, int param, void *value)
{
    switch(param) {
    case PARAM_BACK_OFF_TYPE:
      b->paramBackOffType=(int)(POINTER_INT)value;
      return 1;
    case PARAM_BACK_OFF_INIT:
      b->paramBackOffInit=(int)(POINTER_INT)value;
      return 1;
    case PARAM_BACK_OFF_MAX:
      b->paramBackOffMax=(int)(POINTER_INT)value;
      return 1;
    default:
      break;
    }
    return 0;
}

int NBLBackOffDo(NBLBackOff *b, int last)
{
    int count;
    volatile int temp=0;
    int a=1;
    if(last<=0 || last>b->paramBackOffMax) {
      if(b->paramBackOffType==BOT_LINEAR || 
	 b->paramBackOffType==BOT_EXPONENTIAL)
	last=b->paramBackOffInit;
      else if(b->paramBackOffType==BOT_RANDOM_LINEAR || 
	      b->paramBackOffType==BOT_RANDOM_EXPONENTIAL)
	last=NBLRandomGenNumber(&b->seed)%b->paramBackOffInit;	      
    }

    for(count=0;count<last;count++) {
      a=a*count;
    }
    *((volatile int *)&temp)=a;

    if(b->paramBackOffType==BOT_LINEAR)
      return last+b->paramBackOffInit;
    else if(b->paramBackOffType==BOT_EXPONENTIAL)
      return last+last;
    else if(b->paramBackOffType==BOT_RANDOM_LINEAR)
      return last+(NBLRandomGenNumber(&b->seed)%b->paramBackOffInit);
    else if(b->paramBackOffType==BOT_RANDOM_EXPONENTIAL)
      return last+last+(NBLRandomGenNumber(&b->seed)%b->paramBackOffInit);
    else
      return last;
}
