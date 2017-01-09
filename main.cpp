#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctime>
#include <iostream>

#ifdef _MSC_VER
#include <random>
#include <Windows.h>
#include <process.h>
#endif

#include "concckhashtable.h"
#include "concckhashtable.cpp"
#include <assert.h>
#include <pthread.h>
#include "simpletimer.h"
#include "Configuration.h"
#include "Platform/cpuaffinity.h"
#include "BucketizeConcCK.h"
#include "BucketizeConcCK.cpp"

using namespace std;

//HPMemRecl<desc_t, 1> * hp_desc=NULL;

//#define TABLE_SIZE 131072
//#define DENSITY 0.3
#ifdef DEBUG
template <> 
__thread int ConcCukooHashtable<uint64_t, uint64_t, HASH_INT, HASH_INT1>::_nr_relocation=0;

template <> 
__thread int ConcCukooHashtable<uint64_t, uint64_t, HASH_INT, HASH_INT1>::_total_relocations=0;

template <> 
__thread int ConcCukooHashtable<uint64_t, uint64_t, HASH_INT, HASH_INT1>::_total_help_relocations=0;

template <> 
__thread int ConcCukooHashtable<uint64_t, uint64_t, HASH_INT, HASH_INT1>::_nr_failed_insert=0;

#endif 

ConcCukooHashtable<uint64_t, uint64_t, HASH_INT, HASH_INT1> *cctable;
BucketizeConcCK<uint64_t, uint64_t> *bucktable;

class TestDS {
public:
	virtual bool Search(uint64_t key, uint64_t &value)=0;
	virtual bool Insert(uint64_t key, uint64_t value)=0;
	virtual void Remove(uint64_t key)=0;
	virtual const char* name()=0;
};

class TestCuckoo: public TestDS {
private:
	ConcCukooHashtable<uint64_t, uint64_t, HASH_INT, HASH_INT1> _ds;
public:
	TestCuckoo(const Params& param):_ds(param.initial_count/2){cctable=&_ds;}
	virtual bool Search(uint64_t key, uint64_t &value) {
		return _ds.Search(key, value);
	}
	virtual bool Insert(uint64_t key, uint64_t value) {
		return _ds.Insert(key, value);
	}
	virtual void  Remove(uint64_t key) {
		return _ds.Remove(key);
	}
	const char* name() {
		return "ConcCK";
	}
};

class TestBucketize : public TestDS {
private:
	BucketizeConcCK<uint64_t, uint64_t> _ds;
public:
	TestBucketize(const Params& param):_ds(param.initial_count/2){}
	virtual bool Search(uint64_t key, uint64_t &value) {
		return _ds.Search(key, value);
	}
	virtual bool Insert(uint64_t key, uint64_t value) {
		return _ds.Insert(key, value);
	}
	virtual void Remove(uint64_t key) {
		return _ds.Remove(key);
	}
	const char* name() {
		return "BuckCK";
	}
};

TestDS* _gTestDS;

Params				_gParams;
volatile long long			*_gDummy_sum = new long long;
volatile unsigned int _gStop_all_threads(0);
int		_gExp_time(5);
unsigned long		_gOps_per_msec;
pthread_barrier_t	_gStartLine1;
SimpleTimer			_gSt1;
unsigned long**		_gResultArr;
unsigned long long	_gTotalOps;
unsigned int		_gNum_threads;
float				_gTotalTime;
unsigned long _nAll[3], _nInsert[3], _nRemove[3], _nSearch[3]; //xx[0]: min, xx[1] max, xx[2] total
int _gTotalRandNum;
int * _gRandNumAry;
MyRandom _gRandGen;

#ifdef DEBUG
int		_gTotalRelocation;
int		_gTotalHelpRelocation;
int		_gTotalOpsRelocation;
int		_gFailedInsert;
#endif

const unsigned int HASH_INT::_EMPTY_HASH = 0;
const unsigned int HASH_INT::_BUSY_HASH  = 1;
const int HASH_INT::_EMPTY_KEY  = 0;
const int HASH_INT::_EMPTY_DATA = 0;

#ifdef _WIN64
void Thread(void *arg);
void ThreadDCAS(void *arg);
void testDCAS();
void testBuckHashTable();
void ThreadBuck(void *arg);
#else
void testBuckHashTable();
void ThreadBuck(void *arg);
void *Thread(void *arg);
#endif

class CallWrap
{
public:
	void ( *call)(unsigned int, void* param);
	void* param;
	int id;
};

int dummywork(float percent)
{
        int dummyre=1;
	int times  = (int)floor(percent*345);
        for(int i=0;i<times;i++)
                dummyre*=234;
        return dummyre;
}

long contentionMaker(int contentionLavel)
{
    long a = 0;
    for (int i = 0; i < 1000 * (2 - contentionLavel); i++)//contentionLevel : 0 - low, 1 - medium, 2 - high
    {
        a = a + (a % 2);
    }
    return a;
}

void FillTable(int table_size) 
{
	int count = 0;
	for (int iRandNum = 0; iRandNum < table_size; ++iRandNum) {
		if (_gTestDS->Insert(_gRandNumAry[iRandNum], _gRandNumAry[iRandNum]))
			++count;
	}
	cerr << (string("    ") + _gTestDS->name() + " Num elm: ") << count << endl;
}

void prepareRandNum() {
	_gRandNumAry = new int[_gTotalRandNum];
	int last_number =  1;
	for (int iRandNum = 0; iRandNum < _gTotalRandNum; ++iRandNum) {
		last_number+=1;
		_gRandNumAry[iRandNum] = last_number;
	}
}

void set_thread_affinity(CallWrap* p) {
#ifdef MAROONF
	platform_desc_t platform_maroon(AMD_BULLDOZER, FILL_SOCKET, 6, 4, 12);
	setThreadAffinity(p->id, &platform_maroon);
	return;
#endif
#ifdef BLUEF 
	platform_desc_t platform_others(INTEL, FILL_SOCKET, 6, 2, 12);
	setThreadAffinity(p->id, &platform_others);
	return;
#endif

#ifdef MAROON
	platform_desc_t platform_maroon(AMD_BULLDOZER, JUMP_SOCKET, 6, 4, 12);
	setThreadAffinity(p->id, &platform_maroon);
#endif
#ifdef BLUE
	platform_desc_t platform_others(INTEL, JUMP_SOCKET, 6, 2, 12);
	setThreadAffinity(p->id, &platform_others);
#endif
}

void test(unsigned tid, void* param)
{
	Params* p = (Params*)param;
	uint64_t dummyvalue = 0;
	TT res = 0;
	_gTestDS = (TestDS *)(p->object);
	int idx=0, idx_op=0;
	int row_idx = tid;
	unsigned int count0 = 0, count1 = 0, count2=0;
	do {
		for (int i=0; i<100; i++){
			switch (p->rand_int[tid][idx_op])  {
			case 0:	
				_gTestDS->Insert(p->rand_key[row_idx][idx], p->rand_key[row_idx][idx]+p->rand_key[row_idx][idx]); 
				count0++; break;
			case 1:	
				_gTestDS->Remove(p->rand_key[row_idx][idx]); 
				count1++; break;
			case 2:	
			//default:
				_gTestDS->Search(p->rand_key[row_idx][idx], dummyvalue); 
				count2++; break;
			default: assert(false); break; 
			}
			idx++;idx_op++;
		}

		if (0!=_gStop_all_threads)
			break;
		if (idx+100>MAX_COL_KEY) {
			idx=0;
			row_idx = (row_idx+1)%48;
		}
		if (idx_op+100>MAX_COL_OP) idx_op=0;
	}while (true);

	_gResultArr[tid][0] = count0;
	_gResultArr[tid][1] = count1;
	_gResultArr[tid][2] = count2;
	*_gDummy_sum += dummyvalue;
}

void testbuck(unsigned tid, void* param)
{
	Params* p = (Params*)param;
	uint64_t dummyvalue = 0;
	TT res = 0;
	_gTestDS = (TestDS*)(p->object);
	int idx=0, idx_op=0;
	int row_idx = tid;
	unsigned int count0 = 0, count1 = 0, count2=0;
	do {
		for (int i=0; i<100; i++){
			switch (p->rand_int[tid][idx_op])  {
			case 0:	
				_gTestDS->Insert(p->rand_key[row_idx][idx], p->rand_key[row_idx][idx]+p->rand_key[row_idx][idx]); 
				count0++; break;
			case 1:	
				_gTestDS->Remove(p->rand_key[row_idx][idx]); 
				count1++; break;
			case 2:	
				_gTestDS->Search(p->rand_key[row_idx][idx], dummyvalue); 
				count2++; break;
			default: assert(false); break; 
			}
			idx++;idx_op++;
		}
		if (0!=_gStop_all_threads)
			break;
		if (idx+100>MAX_COL_KEY) {
			idx=0;
			row_idx = (row_idx+1)%48;
		}
		if (idx_op+100>MAX_COL_OP) idx_op=0;
	}while (true);

	_gResultArr[tid][0] = count0;
	_gResultArr[tid][1] = count1;
	_gResultArr[tid][2] = count2;
	*_gDummy_sum += dummyvalue;
}

void testpattern(unsigned tid, void* param)
{
	Params* p = (Params*)param;
	uint64_t dummyvalue = 0;
	TT res = 0;
	_gTestDS = (TestDS *)(p->object);
	int idx=0, idx_op=0;
	int row_idx = tid;
	unsigned int count0 = 0, count1 = 0, count2=0;
	
	int num_actions = (int)((p->initial_count * p->load_factor) /(p->num_threads));
	int start_suc = tid * num_actions;
	int start_unsuc = start_suc + p->initial_count;
	int end_suc = start_suc + num_actions;
	int end_unsuc = start_unsuc + num_actions;
	int state = MyRandomGenNumber(&rand_gen)&0x1;
	int state2=MyRandomGenNumber(&rand_gen)&0x1;;
	int _num_add					= ((p->nUpdateOps) / 2);
	int _num_remove				= ((p->nUpdateOps) / 2);
	int _num_contain			= 100 - _num_add - _num_remove;
	int curr_counter = _num_add;
	if (1==state2)
		curr_counter = _num_contain;

	int actionCounter=0;
	int i_suc = start_suc;
	int i_unsuc = start_unsuc;
	do {
		if (state==0) {
			if (0==state2)  {
				_gTestDS->Remove(_gRandNumAry[i_suc]); 
				_gTestDS->Insert(_gRandNumAry[i_unsuc], _gRandNumAry[i_unsuc]); 
				actionCounter+=2;count1+=2; 
				++i_suc;++i_unsuc;
				--curr_counter;
				if(curr_counter<=0) {
					state2=1;
					curr_counter = _num_contain;
				}
			}else {
				_gTestDS->Search(_gRandNumAry[i_suc], dummyvalue); 
				_gTestDS->Search(_gRandNumAry[i_unsuc], dummyvalue); 
				actionCounter+=2;count2+=2; 
				++i_suc;++i_unsuc;
				curr_counter-=2;
				if(curr_counter<=0) {
					state2=0;
					curr_counter = _num_add;
				}
			}
			if (i_suc>=end_suc) {
				state = 1;
				i_suc = start_suc;
				i_unsuc = start_unsuc;
			}
		} else {
			if (0==state2)  {
				_gTestDS->Insert(_gRandNumAry[i_suc], _gRandNumAry[i_suc]); 
				_gTestDS->Remove(_gRandNumAry[i_unsuc]); 
				actionCounter+=2;count1+=2; 
				++i_suc;++i_unsuc;
				--curr_counter;
				if(curr_counter<=0) {
					state2=1;
					curr_counter = _num_contain;
				}
			}else {
				_gTestDS->Search(_gRandNumAry[i_suc], dummyvalue); 
				_gTestDS->Search(_gRandNumAry[i_unsuc], dummyvalue); 
				actionCounter+=2;count2+=2; 
				++i_suc;++i_unsuc;
				curr_counter-=2;
				if(curr_counter<=0) {
					state2=0;
					curr_counter = _num_add;
				}
			}
			if (i_suc>=end_suc) {
				state = 0;
				i_suc = start_suc;
				i_unsuc = start_unsuc;
			}
		}
		
		if (actionCounter%100 == 0)
			if (0!=_gStop_all_threads)
				break;
	}while (true);

	_gResultArr[tid][0] = count0;
	_gResultArr[tid][1] = count1;
	_gResultArr[tid][2] = count2;
	*_gDummy_sum += dummyvalue;
}

void testbuckthread(unsigned tid, void* param)
{
	Params* p = (Params*)param;
	uint64_t dummyvalue = 0;
	TT res = 0;
	_gTestDS = (TestDS *)(p->object);
	int idx=0, idx_op=0;
	int row_idx = tid;
	unsigned int count0 = 0, count1 = 0, count2=0;
	int num_actions, start_suc;

	if (tid< p->n_mod_threads) {
		num_actions = (int)((p->initial_count * p->load_factor) /(p->n_mod_threads));
		start_suc = tid * num_actions;
 	}else {
		num_actions = (int)((p->initial_count * p->load_factor) /(p->num_threads - p->n_mod_threads));
		start_suc = (tid - p->n_mod_threads) * num_actions;
	}

	int start_unsuc = start_suc + p->initial_count;
	int end_suc = start_suc + num_actions;
	int end_unsuc = start_unsuc + num_actions;
	int i_suc = start_suc;
	int i_unsuc = start_unsuc;
	int state = 0;

	int actionCounter=0;
	do {
		if (state==0) {
			if (tid<p->n_mod_threads)  {
				
				_gTestDS->Remove(_gRandNumAry[i_suc]); 
				_gTestDS->Insert(_gRandNumAry[i_unsuc], _gRandNumAry[i_unsuc]); 
				actionCounter+=2;count1+=2; 
				++i_suc;++i_unsuc;
			}else {
					_gTestDS->Search(_gRandNumAry[i_suc], dummyvalue); 
					_gTestDS->Search(_gRandNumAry[i_unsuc], dummyvalue); 
					actionCounter+=2;count2+=2; 
					++i_suc;++i_unsuc;
			}
			if (i_suc>=end_suc) {
				state = 1;
				i_suc = start_suc;
				i_unsuc = start_unsuc;
			}
		} else {
			if (tid<p->n_mod_threads)  {
				_gTestDS->Insert(_gRandNumAry[i_suc], _gRandNumAry[i_suc]); 
				_gTestDS->Remove(_gRandNumAry[i_unsuc]); 
				actionCounter+=2;count1+=2; 
				++i_suc;++i_unsuc;
			}else {
				_gTestDS->Search(_gRandNumAry[i_suc], dummyvalue); 
				_gTestDS->Search(_gRandNumAry[i_unsuc], dummyvalue); 
				actionCounter+=2;count2+=2; 
				++i_suc;++i_unsuc;
			}
			if (i_suc>=end_suc) {
				state = 0;
				i_suc = start_suc;
				i_unsuc = start_unsuc;
			}
		}
		
		if (actionCounter%100 == 0)
			if (0!=_gStop_all_threads)
				break;
	}while (true);

	_gResultArr[tid][0] = count0;
	_gResultArr[tid][1] = count1;
	_gResultArr[tid][2] = count2;
	*_gDummy_sum += dummyvalue;
}

void* run_perthread(void* d)
{
#ifdef DEBUG
	cctable->_total_relocations = 0;
	cctable->_total_help_relocations = 0;
	cctable->_nr_relocation = 0;
	cctable->_nr_failed_insert=0;
#endif

	CallWrap* p = (CallWrap*)d;
	set_thread_affinity(p);

	_gSt1.start();
	if(pthread_barrier_wait(&_gStartLine1)==0)
		_gSt1.start();

	p->call(p->id, p->param);

#ifdef DEBUG
	FAA(&_gTotalRelocation, cctable->_total_relocations);
	FAA(&_gTotalHelpRelocation, cctable->_total_help_relocations);
	FAA(&_gTotalOpsRelocation, cctable->_nr_relocation);
	FAA(&_gFailedInsert, cctable->_nr_failed_insert);
#endif
	return 0;
}

void readHashtableInitialData(const char *filename,  TestDS *pobject) {
	FILE *pfile;
	pfile = fopen(filename, "r");
	if (pfile==NULL) {
		return ;
	}
	int num = 0, count=0;
	int total_item = round(TABLE_SIZE*DENSITY);
	while (fscanf(pfile, "%d" , &num) !=EOF) {
		pobject->Insert(num, num+num);
		if (++count>=total_item) break;
	}
	
	fclose(pfile);
}

void readBuckHashtableInitialData(const char *filename, TestDS *pobject) {
	FILE *pfile;
	pfile = fopen(filename, "r");
	if (pfile==NULL) {
		return ;
	}
	uint64_t num = 0, count=0;
	int total_item = round(TABLE_SIZE*DENSITY);
	while (fscanf(pfile, "%lu" , &num) !=EOF) {
		pobject->Insert(num, num+num);
		if (++count>=total_item) break;
	}
	
	fclose(pfile);
}

/////////////
#ifndef _WIN64
int main_linux(int argc, char **argv) {

	_gDummy_sum = new long long;
	_gNum_threads = _gParams.num_threads;
	
	_gResultArr = new unsigned long*[_gNum_threads];
	for (unsigned int i=0; i<_gNum_threads; i++)
		_gResultArr[i] = new unsigned long[3];
	for (unsigned int i=0; i<_gNum_threads; i++)
		_gResultArr[i][0] = _gResultArr[i][1] = _gResultArr[i][2] = 0;

	//cctable = new ConcCukooHashtable<int, int, HASH_INT, HASH_INT1>(TABLE_SIZE);
	//_gTestDS = new TestCuckoo(_gParams);
	//readHashtableInitialData("initialKeys.txt", _gTestDS);
	//_gParams.object = (void *)cctable;
	cout << "Test Cuckoo" << endl;
	_gTestDS = new TestCuckoo(_gParams);
	int table_size = (int)(_gParams.initial_count * _gParams.load_factor);
	FillTable(table_size);
	_gParams.object = (void *)_gTestDS;

	pthread_t* threads = new pthread_t[_gNum_threads];
	pthread_barrier_init(&_gStartLine1, NULL, _gNum_threads);

	CallWrap *cws = new CallWrap[_gNum_threads];
	for(unsigned int i=0;i<_gNum_threads;i++)
	{
		if (_gParams.n_mod_threads<0)
			cws[i].call=&testpattern;
		else
			cws[i].call=&testbuckthread; //test;
		cws[i].param = &_gParams;
		cws[i].id=i;
		pthread_create(&threads[i],0,&run_perthread,&cws[i]);
	}


	mysleep(_gExp_time*1000);
	MEM_WRITE(&_gStop_all_threads,1);

	for(unsigned int i=0;i<_gNum_threads;i++)
	{
		if (int rc = pthread_join(threads[i], 0))
		{
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			::exit(1);
		}
	}

	_gTotalTime = (float)_gSt1.end();

	_gTotalOps = 0;
	_nAll[0] = _nInsert[0] = _nRemove[0] = _nSearch[0] =0xffffffff; //min
	_nAll[1] = _nInsert[1] = _nRemove[1] = _nSearch[1] =0; //max
	_nAll[2] = _nInsert[2] = _nRemove[2] = _nSearch[2] =0; //total
	unsigned long temp;

	for (unsigned int i=0; i<_gNum_threads; i++) {
		if (_gResultArr[i][0]<_nInsert[0]) _nInsert[0] = _gResultArr[i][0];
		if (_gResultArr[i][0]>_nInsert[1]) _nInsert[1] = _gResultArr[i][0];
		_nInsert[2] += _gResultArr[i][0];

		if (_gResultArr[i][1]<_nRemove[0]) _nRemove[0] = _gResultArr[i][1];
		if (_gResultArr[i][1]>_nRemove[1]) _nRemove[1] = _gResultArr[i][1];
		_nRemove[2] += _gResultArr[i][1];

		if (_gResultArr[i][2]<_nSearch[0]) _nSearch[0] = _gResultArr[i][2];
		if (_gResultArr[i][2]>_nSearch[1]) _nSearch[1] = _gResultArr[i][2];
		_nSearch[2] += _gResultArr[i][2];

		temp = _gResultArr[i][0]+_gResultArr[i][1]+_gResultArr[i][2];
		if (temp<_nAll[0]) _nAll[0]=temp;
		if (temp>_nAll[1]) _nAll[1]=temp;
		_nAll[2] += temp;
	}
	
	printf("%lu\t%f", _nAll[2], _gTotalTime);
	printf("\t%lu\t%lu\t%lu\n", _nSearch[2], _nInsert[2], _nRemove[2]);
	
	//printf("%lu\t%f\t%lu\t%lu\n", _nAll[2], _gTotalTime, _nAll[0], _nAll[1]);
	//printf("\t%lu\t%lu\t%lu\n", _nInsert[2], _nInsert[0], _nInsert[1]);
	//printf("\t%lu\t%lu\t%lu\n", _nRemove[2], _nRemove[0], _nRemove[1]);
	//printf("\t%lu\t%lu\t%lu\n", _nSearch[2], _nSearch[0], _nSearch[1]);
	
	_gParams.deallocate();
	for (unsigned int i=0; i<_gNum_threads; i++)
		delete [] _gResultArr[i];
	delete [] _gResultArr;
	delete _gDummy_sum;

	return 0;
}
#endif
int main_linux_bucketize(int argc, char **argv) {

	_gDummy_sum = new long long;
	_gNum_threads = _gParams.num_threads;
	
	_gResultArr = new unsigned long*[_gNum_threads];
	for (unsigned int i=0; i<_gNum_threads; i++)
		_gResultArr[i] = new unsigned long[3];
	for (unsigned int i=0; i<_gNum_threads; i++)
		_gResultArr[i][0] = _gResultArr[i][1] = _gResultArr[i][2] = 0;

	//bucktable = new BucketizeConcCK<uint64_t, uint64_t>(TABLE_SIZE);
	//readBuckHashtableInitialData("initialKeys.txt", bucktable);
	//bucktable = new BucketizeConcCK<uint64_t, uint64_t>(_gParams.initial_count);
	cout << "Test Bucketize" << endl;
	_gTestDS = new TestBucketize(_gParams);
	int table_size = (int)(_gParams.initial_count * _gParams.load_factor);
	FillTable(table_size);
	_gParams.object = (void *)_gTestDS;

	pthread_t* threads = new pthread_t[_gNum_threads];
	pthread_barrier_init(&_gStartLine1, NULL, _gNum_threads);

	CallWrap *cws = new CallWrap[_gNum_threads];
	for(unsigned int i=0;i<_gNum_threads;i++)
	{
		if (_gParams.n_mod_threads<0)
			cws[i].call=&testpattern;
		else
			cws[i].call=&testbuckthread;
		cws[i].param = &_gParams;
		cws[i].id=i;
		pthread_create(&threads[i],0,&run_perthread,&cws[i]);
	}


	mysleep(_gExp_time*1000);
	MEM_WRITE(&_gStop_all_threads,1);

	for(unsigned int i=0;i<_gNum_threads;i++)
	{
		if (int rc = pthread_join(threads[i], 0))
		{
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			::exit(1);
		}
	}

	_gTotalTime = (float)_gSt1.end();

	_gTotalOps = 0;
	_nAll[0] = _nInsert[0] = _nRemove[0] = _nSearch[0] =0xffffffff; //min
	_nAll[1] = _nInsert[1] = _nRemove[1] = _nSearch[1] =0; //max
	_nAll[2] = _nInsert[2] = _nRemove[2] = _nSearch[2] =0; //total
	unsigned long temp;

	for (unsigned int i=0; i<_gNum_threads; i++) {
		if (_gResultArr[i][0]<_nInsert[0]) _nInsert[0] = _gResultArr[i][0];
		if (_gResultArr[i][0]>_nInsert[1]) _nInsert[1] = _gResultArr[i][0];
		_nInsert[2] += _gResultArr[i][0];

		if (_gResultArr[i][1]<_nRemove[0]) _nRemove[0] = _gResultArr[i][1];
		if (_gResultArr[i][1]>_nRemove[1]) _nRemove[1] = _gResultArr[i][1];
		_nRemove[2] += _gResultArr[i][1];

		if (_gResultArr[i][2]<_nSearch[0]) _nSearch[0] = _gResultArr[i][2];
		if (_gResultArr[i][2]>_nSearch[1]) _nSearch[1] = _gResultArr[i][2];
		_nSearch[2] += _gResultArr[i][2];

		temp = _gResultArr[i][0]+_gResultArr[i][1]+_gResultArr[i][2];
		if (temp<_nAll[0]) _nAll[0]=temp;
		if (temp>_nAll[1]) _nAll[1]=temp;
		_nAll[2] += temp;
	}
	
	printf("%lu\t%f", _nAll[2], _gTotalTime);
	printf("\t%lu\t%lu\t%lu\n", _nSearch[2], _nInsert[2], _nRemove[2]);
	
	//printf("%lu\t%f\t%lu\t%lu\n", _nAll[2], _gTotalTime, _nAll[0], _nAll[1]);
	//printf("\t%lu\t%lu\t%lu\n", _nInsert[2], _nInsert[0], _nInsert[1]);
	//printf("\t%lu\t%lu\t%lu\n", _nRemove[2], _nRemove[0], _nRemove[1]);
	//printf("\t%lu\t%lu\t%lu\n", _nSearch[2], _nSearch[0], _nSearch[1]);
	
	_gParams.deallocate();
	for (unsigned int i=0; i<_gNum_threads; i++)
		delete [] _gResultArr[i];
	delete [] _gResultArr;
	delete _gDummy_sum;

	return 0;
}

int main(int argc, char **argv) {
	_gParams.init(argc, argv);
	_gNum_threads = _gParams.num_threads;
	_gExp_time = _gParams.expTime;
	_gTotalRandNum      = 2*(_gParams.initial_count);
	clock_t t = clock();
	MyRandomInit(&_gRandGen, (int)t);
	prepareRandNum();

	if (strcmp(_gParams.algoName, "bucketize")==0) {
		//bucktable = new BucketizeConcCK<uint64_t, uint64_t>(1024);
#ifdef _WIN64
		//testBuckHashTable();
		main_linux_bucketize(argc, argv);
#else
		main_linux_bucketize(argc, argv);
#ifdef DEBUG
		printf("Operations needed relocation: %d\n Total relocation: %d\n Total help relocations: %d\n Average rellocation/times: %f\n Total failed insertions: %d\n",
			_gTotalOpsRelocation, _gTotalRelocation, _gTotalHelpRelocation, (float)_gTotalRelocation/_gTotalOpsRelocation, _gFailedInsert);
#endif
#endif
		return 0;
	}

#ifdef DEBUG
	_gTotalRelocation=0;
	_gTotalOpsRelocation=0;
	MyRandomInit(&rand_gen, 1234);
	MyRandomInit(&rand_table,41205);
#endif
#ifdef _WIN64
	cctable = new ConcCukooHashtable<uint64_t, uint64_t, HASH_INT, HASH_INT1>(1024);
	for(int j=0;j<1;j++) {
		uintptr_t handles[16];

		for(unsigned int i=0;i<_gNum_threads;i++) {
			handles[i] = _beginthread(Thread,0,(void*)i);
		}

		for(unsigned int i=0;i<_gNum_threads;i++) {
			WaitForSingleObject((HANDLE)handles[i],INFINITE);
		}
    }
#ifdef DEBUG
	printf("Operations needed relocation: %d\n Total relocation: %d\n Total help relocations: %d\n Average rellocation/times: %f\n Total failed insertions: %d\n",
		_gTotalOpsRelocation, _gTotalRelocation, _gTotalHelpRelocation, (float)_gTotalRelocation/_gTotalOpsRelocation, _gFailedInsert);
#endif

	getchar();
#else
	main_linux(argc, argv);
#ifdef DEBUG
	printf("Operations needed relocation: %d\n Total relocation: %d\n Total help relocations: %d\n Average rellocation/times: %f\n Total failed insertions: %d\n",
		_gTotalOpsRelocation, _gTotalRelocation, _gTotalHelpRelocation, (float)_gTotalRelocation/_gTotalOpsRelocation, _gFailedInsert);
#endif

#endif
	
	return 0;
}

#ifdef _MSC_VER
void Thread(void *arg) {
	int id = (int)arg;

#else
void *Thread(void *arg) {
	int id = *((int *)arg);
#endif
	int counter = 0;
    for(int i=0;i<0x1000;i++) {
        int key =  (i*0x10000)+0x1000*id;
        int value = (i*0x10000)+0x1000*id+0x1000;
		bool result = cctable->Insert(key, value);
		int value1 = 0;
		if (!result)
			counter++;
    }

	for(int i=0;i<0x1000;i=i+2) {
        int key =  (i*0x10000)+0x1000*id;
        int value = (i*0x10000)+0x1000*id+0x1000;
		cctable->Remove(key);
    }
	printf("Thread %d, %d failed insert\n", id, counter);
#ifdef DEBUG
	FAA(&_gTotalRelocation, cctable->_total_relocations);
	FAA(&_gTotalOpsRelocation, cctable->_nr_relocation);
#endif

#ifdef _MSC_VER
    _endthread();
#endif
}

void testBuckHashTable() {
#ifdef DEBUG
	_gTotalRelocation=0;
	_gTotalOpsRelocation=0;
	MyRandomInit(&rand_gen, 1234);
	MyRandomInit(&rand_table,41205);
#endif
#ifdef _WIN64
	bucktable = new BucketizeConcCK<uint64_t, uint64_t>(2048);
	for(int j=0;j<1;j++) {
		uintptr_t handles[16];

		for(unsigned int i=0;i<_gNum_threads;i++) {
			handles[i] = _beginthread(ThreadBuck,0,(void*)i);
		}

		for(unsigned int i=0;i<_gNum_threads;i++) {
			WaitForSingleObject((HANDLE)handles[i],INFINITE);
		}
    }
#ifdef DEBUG
	printf("Operations needed relocation: %d\n Total relocation: %d\n Total help relocations: %d\n Average rellocation/times: %f\n Total failed insertions: %d\n",
		_gTotalOpsRelocation, _gTotalRelocation, _gTotalHelpRelocation, (float)_gTotalRelocation/_gTotalOpsRelocation, _gFailedInsert);
#endif

	getchar();
#endif
	
	return;
}

#ifdef _MSC_VER
void ThreadBuck(void *arg) {
	int id = (int)arg;
	int repeat= 500;
	unsigned long seed = (unsigned long)time(NULL);
	std::default_random_engine generator(seed);
	std::uniform_int_distribution<int> distribution(1,repeat*_gNum_threads*2);
	
	int counter = 0;
    for(int i=0;i<repeat;i++) {
        int64_t key =  id*(repeat*_gNum_threads*2)+i;
        int64_t value = key;
		bool result = bucktable->Insert(key, value);
		if (!result)
			counter++;
    }

	for(int i=0;i<500;i++) {
        uint64_t key =  distribution(generator)%(repeat*_gNum_threads*2);
		uint64_t value = 0;
		bool result = bucktable->Search(key, value);
		/*if (result)
			printf("found %lu \n", key);*/
		if (i%2==0) {bucktable->Remove(key);}
		if (i%5==0) {key=id*(repeat*_gNum_threads*2)+500+i; bucktable->Insert(key, key);}
    }

	printf("Thread %d, %d failed insert\n", id, counter);
#ifdef DEBUG
	FAA(&_gTotalRelocation, cctable->_total_relocations);
	FAA(&_gTotalOpsRelocation, cctable->_nr_relocation);
#endif

    _endthread();
}
#endif

#ifdef _MSC_VER
/*void testDCAS(){
	word1 = &value[0];
	word2 = &value[1];
	uintptr_t handles[16];
	_gNum_threads = 2;
	for(unsigned int i=0;i<_gNum_threads;i++) {
		handles[i] = _beginthread(ThreadDCAS,0,(void*)i);
	}

	for(unsigned int i=0;i<_gNum_threads;i++) {
		WaitForSingleObject((HANDLE)handles[i],INFINITE);
	}
}

void ThreadDCAS(void *arg) {
	int id = (int)arg;
	for (int i=1; i<10;i++) {
		desc_t *desc = new desc_t();
		desc->ptr1 = &word1;
		desc->ptr2 = &word2;
		desc->old1 = (uint64_t)read(&word1);
		desc->old2 = (uint64_t)read(&word2);
		desc->new1 = (uint64_t)&value[id*10+i];
		desc->new2 = (uint64_t)&value[id*10+i+1];
		desc->result = UNDECIDED;
		exchange(desc, true);
	}
}
*/
#endif
