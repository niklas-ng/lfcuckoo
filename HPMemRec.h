#ifndef HPMEMREC_H
#define HPMEMREC_H

#include <algorithm>
#include <vector>
#include "threadLocal.h"

#include "Platform/Primitives.h"

template<typename T, int K> class HPMemRecl;

template<typename TT, int K> 
void retireHPRec(void * thr_local);

template<typename P_HP>
class ListHPNode {
public:
	P_HP pNode;
	ListHPNode *next;
	ListHPNode();
};

template<typename T, int K>
class HPRecType {
public:
	typedef T NodeType;
	typedef ListHPNode<NodeType *> ReclNode;
	volatile NodeType *HP[K];
	HPRecType *next;
	int active;
	ReclNode *rlist;
	volatile int rcount;
	int nrThreads;

public:
	HPRecType(void);
	~HPRecType(void);
};

template<typename T, int K>
class HPMemRecl
{
public:
	static HPMemRecl<T, K> global_HPMem;
	static HPMemRecl<T, K> * getHPMemRecl();
	static const int MINIMAL_RLIST_LEN = 5;

public:
	typedef T NodeType;
	typedef HPRecType<NodeType, K> HP_Rec;
	typedef ListHPNode<NodeType *> ReclNode;

private:
	ThreadLocal<HP_Rec *> *threadLocal;
	HP_Rec *headHPRec;
	volatile int hpCount;
	int R_MAX;

public:
	HPMemRecl(void);
	~HPMemRecl(void);


	void delNode(NodeType *node);
	void employHP(HP_Rec *hp, int index, NodeType *node);
	void retireHP(NodeType *node);
	void retireHP(int index);
	void scan(HP_Rec *head);
	HP_Rec * allocHPRec();
	HP_Rec * getHPRec();
	void helpScan();
};

#endif