#ifndef CONCCKHASHTABLE_H
#define CONCCKHASHTABLE_H 1

#include "globaldefinitions.h"
#include "Platform/Primitives.h"
#include <assert.h>
#include "getCacheLineSize.h"
/*#ifdef LINUX64_X86
#define CACHE_L1_LINE_SIZE 64
#define SIZEOF_POINTER 8
#define NR_POINTER_PER_LINE (CACHE_L1_LINE_SIZE/SIZEOF_POINTER)
#define CORRECT_IDX(x) (NR_POINTER_PER_LINE*x)
#else
#define CACHE_L1_LINE_SIZE 64
#define SIZEOF_POINTER 8
#define NR_POINTER_PER_LINE 8
//(CACHE_L1_LINE_SIZE/SIZEOF_POINTER)
#define LOG_NR_POINTER_PER_LINE 3
#define CORRECT_IDX(x) x<<3 
//(NR_POINTER_PER_LINE*x)
#endif
*/

enum Find_Result {
	Not_Found,
	In_First,
	In_Second,
	In_Both
};

static __thread MyRandom rand_gen;
static __thread MyRandom rand_table;	

typedef unsigned int (*Hash_Function)(int);

template <typename TKey, 
			 typename TValue, 
			 typename FHash1,
			 typename FHash2>
class ConcCukooHashtable
{
	class HashEntry{
		public:
			TKey _key; 
			TValue _value;
	};

	struct relocate_data {
	//	int table;
		int index;
		HashEntry *entry;
	};
private:
	HashEntry **_table[2];
	//HashEntry **_table1;
	//HashEntry **_table2;
	int _capacity, _log2size;
	long _sizeMask;
	int _actualSize;
	int _pad_size;

public:
#ifdef DEBUG
	static __thread int _total_relocations;
	static __thread int _total_help_relocations;
	static __thread int _nr_relocation;
	static __thread int _nr_failed_insert;
#endif
public:
	ConcCukooHashtable(int capacity);
	~ConcCukooHashtable(void);
	bool Search(TKey &key, TValue &value);
	bool Insert(TKey key, TValue value);
	void Remove(const TKey &key);
	Find_Result Find(const TKey &key, HashEntry *&entry1, HashEntry *&entry2, bool isInsert=false);//HashEntry **p_entry);
	bool relocate(HashEntry **src, Hash_Function calc_s, int pos1, HashEntry **dest, Hash_Function calc_d, int& rep_counter);
	int newrelocate(unsigned int which, const uint64_t buck_pos,  int& rep_counter);

private:
	int _n_hash;
	static const int MAX_REPLACE = 500;
	bool help_relocate(HashEntry **src, Hash_Function calc_s, int pos1, HashEntry **dest, Hash_Function calc_d);
	bool newhelp_relocate(int table, int index);

	//void read(void * table[], unsigned int index, POINTER_INT &pointer, uint64 &time);

	/*
	This function check the timestamps in the pointers if they are 2 units differences. This is to be used to see 
	if there is possibly two relocations of an item from one table to another and then back.
	*/
	bool inline check2Differences(uint64_t t1, uint64_t t2, uint64_t t1x, uint64_t t2x)//, uint64_t t2pre) 
	{
		if ( (t1x>=t1+2 || (t1x<t1 && t1x+MAX_TIMESTAMP>=t1+2) )
			&& (t2x>=t2+2 || (t2x<t2 && t2x+MAX_TIMESTAMP>=t2+2)) 
			&& (t2x>=t1+3 || (t2x<t1 && t2x+MAX_TIMESTAMP>=t1+3)) ) //&& (t1x>=t2pre+2 || (t1x<t2pre && t1x+MAX_TIMESTAMP>=t2pre+2)) 
			return true;
		return false;
	}
};
#endif
