/*
This code is modified to suit our library and code.. the original one is provided my Maged Micheal.
*/
#include <assert.h>
#include "HPMemRec.h"
//#include "doacbench.h"
#include "Platform/Primitives.h"
#include <iostream>

using namespace std;

template<typename T>
ListHPNode<T>::ListHPNode() 
{
	next = NULL;
}

template<typename T, int K>
HPRecType<T,K>::~HPRecType(void) {
	//delete retired nodes in rlist, and rlist itself
	ReclNode *tmpListNode;

	while (rlist != NULL) {
		tmpListNode = rlist;
		rlist = rlist->next;
#ifdef _DEBUG
		//cout << tmpListNode->pNode << " deleted" << endl;
#endif        
		delete tmpListNode->pNode;
		delete tmpListNode;
#ifdef DEBUG
		rcount--;
#endif
	}
#ifdef DEBUG
	if(rcount)
	{
		cout<<"fatal error: rcount or freeCount not zeor"<<endl;
		cout<<"rcount="<<rcount<<endl;
	}
#endif
}

template<typename T, int K>
HPRecType<T,K>::HPRecType(void) 
{
	memset(this, 0, sizeof(HPRecType));
}

template<typename T, int K> 
HPMemRecl<T, K> HPMemRecl<T, K >::global_HPMem;
	
template<typename T, int K> 
HPMemRecl<T, K> * HPMemRecl<T, K >::getHPMemRecl() {
	//static HPMemRecl<T, K> HPMemRecl<T, K >::global_HPMem();
	return &global_HPMem;
}

template<typename TT, int K> 
void retireHPRec(void * thr_local) {
		HPRecType<TT, K> *my_hprec = (HPRecType<TT, K> *) thr_local;
		if (my_hprec != NULL) {
			for (int i = 0; i < K; i++)
				my_hprec->HP[i] = NULL;

#ifdef _DEBUG
			//cout << "Retiring HPRec: "<< my_hprec << endl;
#endif
			my_hprec->active = 0;
		}
}


template<typename T, int K>
HPMemRecl<T,K>::HPMemRecl(void)
{
	threadLocal = new ThreadLocal<HP_Rec *>(&retireHPRec<T, K>);
	this->headHPRec = NULL;
	R_MAX = MINIMAL_RLIST_LEN;
}

template<typename T, int K>
HPMemRecl<T,K>::~HPMemRecl(void)
{
	HP_Rec *hprec =	this->headHPRec;
	HP_Rec *tmp_hprec = NULL;
	while (hprec != NULL) {
		tmp_hprec = hprec;
		#ifdef _DEBUG
		if(tmp_hprec->active) {
			HP_Rec *my_hprec = threadLocal->get();
			if(tmp_hprec!=my_hprec)
			cout << "Warning some HP is active" << endl;
		}
		#endif
		hprec = hprec->next;
		delete tmp_hprec;
	}
	delete threadLocal;
}

template<typename T, int K>
void HPMemRecl<T,K>::employHP(HP_Rec *hp, int index, NodeType *ptrNode)
{
	assert(index < K);
	HP_Rec *my_hprec = getHPRec();
	my_hprec->HP[index] = ptrNode;
}

template<typename T, int K>
void HPMemRecl<T,K>::retireHP(NodeType *node)
{
	HP_Rec my_hprec = getHPRec();
	for(int i = 0; i < K; ++i) {
		if(my_hprec->HP[i] == node) {
			my_hprec->HP[i] = NULL;
		}
	}
}

template<typename T, int K>
void HPMemRecl<T,K>::retireHP(int index)
{
	assert(index < K);
	HP_Rec *my_hprec = getHPRec();
	my_hprec->HP[index] = NULL;
}

template<typename T, int K>
typename HPMemRecl<T,K>::HP_Rec * HPMemRecl<T,K>::allocHPRec()
{
	HP_Rec *hprec;
	HP_Rec *oldhead;

	//First try to reuse a retired HP record
	for (hprec = this->headHPRec; hprec != NULL; hprec = hprec->next) {
		if (hprec->active != 0) {
			continue;
		}
		int b = 0;
		/* if hprec is active, continue */

		if (!CAS(&hprec->active, b, (int)1)) {
			continue;
		}

#ifdef DEBUG
                //cout << "Reuse HPRec: "<< hprec << endl << flush;

                //assert(hprec->fr_smr_count == 0);
                //assert(hprec->fr_data_count == 0);
                //hprec->verifySMRList();
#endif
			//Succeeded in locking an inactive HP record
			threadLocal->set(hprec);
                  //  hprec->init();
#ifdef DEBUG
			//	cout<<"reuse hp:"<<hprec<<"tid"<<pthread_self()<<endl;
#endif

		return hprec;
	}
	
	FAA(&this->hpCount,1);
	if (hpCount >= MINIMAL_RLIST_LEN / 2) {
		R_MAX = hpCount * 2;
	}

	hprec = new HP_Rec ();
	hprec->active=1;

	do {
		oldhead = this->headHPRec;
		hprec->next = oldhead;
		hprec->nrThreads = hpCount;
	}while (CASP((int64_t *)&headHPRec, (int64_t)oldhead, (int64_t)hprec) == false);

	/* Set this to thread_local variable */
	threadLocal->set(hprec);
	return hprec;
}

template<typename T, int K>
typename HPMemRecl<T,K>::HP_Rec * HPMemRecl<T,K>::getHPRec()
{
	HP_Rec *my_hprec = threadLocal->get();
	if (my_hprec==NULL)
		my_hprec = allocHPRec();
	return my_hprec;
}

/**
 * This method will scan the hazard pointers and retired pointer
 * list of current thread. If there is no match, this method will
 * delete pointers inside retired list.
 */
template<typename T, int K>
void HPMemRecl<T,K>::scan(HP_Rec* head) 
{
	HP_Rec *hprec = head;
	volatile NodeType *hptr;
	unsigned long pc, nrcount;
	ReclNode *tmpList = NULL;

	HP_Rec *my_hprec = threadLocal->get();
	ReclNode *rlist = my_hprec->rlist;
	unsigned int max_threads = hprec->nrThreads;

	//this array is supposed to be allocated from stack instead of heap.
	volatile NodeType** plist = new volatile NodeType*[max_threads * K];

	//stage 1. Scan HP list and insert non-null values into plist
	pc = 0;
	for (; hprec != NULL; hprec = hprec->next) {
		for (int j = 0; j < K; j++) {
			hptr = hprec->HP[j];
			if (hptr != NULL)
				plist[pc++] = hptr;
		}
	}

	assert(pc <= max_threads * K);
	// stage 2. sort plist, and search every retired node at plist
	sort(plist, plist + pc);
	volatile NodeType ** newlast = unique(plist, plist + pc);

	nrcount = 0;
	while (rlist != NULL) {
		ReclNode *tmpListNode = rlist;
		rlist = rlist->next;

		if (binary_search(plist, newlast, tmpListNode->pNode)) {
			// if a retired node is still required by other threads,
			// we put it into a new list
#ifdef _DEBUG
			//cout << tmpListNode->pNode << " still required by others" << endl;
#endif 
			tmpListNode->next = tmpList;
			tmpList = tmpListNode;
			nrcount++;
			
		} else {
			// delete a retired node if no other thread is using it

			delete tmpListNode->pNode;
			delete tmpListNode;
		}
	}
	//stage 4. Update the retired list to the new list
	my_hprec->rlist = tmpList;
	my_hprec->rcount = nrcount;
	delete[] plist;
}


template<typename T, int K>
void HPMemRecl<T,K>::delNode(NodeType *node)
{
	HP_Rec *my_hprec = getHPRec();
	ReclNode *tmpListNode = new ReclNode();
	tmpListNode->pNode = node;
	tmpListNode->next = my_hprec->rlist;
	my_hprec->rlist = tmpListNode;
	my_hprec->rcount++;
	if (my_hprec->rcount >= R_MAX) {
		HP_Rec *head = headHPRec;
		scan(head);
	}
}


template<typename T, int K>
void HPMemRecl<T,K>::helpScan()
{
}
