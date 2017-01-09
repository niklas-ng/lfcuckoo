#ifndef BUCKETIZECONCURRENTCK_H
#define BUCKETIZECONCURRENTCK_H 1

#include "globaldefinitions.h"
#include "Platform/Primitives.h"
#include <assert.h>
#include "getCacheLineSize.h"
#include "SpookyV2.h"
//#include "HPMemRec.h"

#define BIN_SIZE 8

#define UNDECIDED 1
#define FIRSTFAIL 2
#define SECONDFAIL 3
#define SUCCESS 4

/**/
struct desc_t {
	void * ptr1, * ptr2;
	uint64_t old1, old2;
	uint64_t new1, new2;
	int64_t result;	

};

//extern HPMemRecl<desc_t, 1> * hp_desc;

template <typename TKey, 
			 typename TValue>
struct HashItem{
	public:
		TKey _key; 
		TValue _value;
};

template <typename TKey, 
			 typename TValue>
struct HashElement_T{
		HashItem<TKey, TValue> _item;
		uint64_t _hash1;
		uint64_t _hash2;
	};


/*Constraint of use: a key is inserted only once.*/
template <typename TKey, 
			 typename TValue>
class BucketizeConcCK
{
	typedef HashElement_T<TKey, TValue> HashElement;
private:
	static const int MAX_RELOCATE = 500;
	HashElement **_table;
	int _capacity, _log2size, _nrBuckets, _bin_size;
	uint64_t _sizeMask;

	//harzard pointers
public:
	//HPMemRecl<HashElement, 2> * hp_node;
#ifdef DEBUG
	static __thread int _total_relocations;
	static __thread int _total_help_relocations;
	static __thread int _nr_relocation;
	static __thread int _nr_failed_insert;
#endif

private:
	inline HashElement ** bin_at(uint64_t index) {
		return (_table + index*_bin_size);
	}
	bool checkDESC(desc_t *tmp, const TKey &key, TValue &value);

public:
	BucketizeConcCK(int capacity);
	~BucketizeConcCK(void);
	bool Search(const TKey &key, TValue &value);
	bool Insert(const TKey &key, const TValue &value);
	void Remove(const TKey &key);
	int relocate(const uint64_t buck_pos);
};
#endif