#ifndef GLOBAL_DEFINITION_H
#define GLOBAL_DEFINITION_H 1

#include <stdint.h>
#include <stddef.h>

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

#define TABLE_SIZE 4096
#define TABLE_SIZE_LOG2 12
#define DENSITY 0.3

#define KEY_TYPE int
#define VALUE_TYPE int

#ifdef LINUX64_X86
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

/*#ifndef POINTER_INT
#define POINTER_INT __int64
#endif
#ifndef uint64_t
#define uint64_t long long
#endif
#ifndef uint64_t
#define uint64_t unsigned long long
#endif*/
/*
#undef IS_MARKED_DELETE
#define IS_MARKED_DELETE(x) (((POINTER_INT)(x))&0x01)
#undef GET_MARKED_DELETE
#define GET_MARKED_DELETE(x) (node_t *)(((POINTER_INT)(x))|0x01)
#undef GET_UNMARKED_DELETE
#define GET_UNMARKED_DELETE(x) (node_t *)(((POINTER_INT)(x))&(~((POINTER_INT)1)))
*/
#undef IS_MARKED_REPLACE
#define IS_MARKED_REPLACE(x) (((uint64_t)(x))&0x01)
#undef GET_MARKED_REPLACE
#define GET_MARKED_REPLACE(x) (uint64_t)(((uint64_t)(x))|0x01)
#undef GET_UNMARKED_REPLACE
#define GET_UNMARKED_REPLACE(x) (uint64_t)(((uint64_t)(x))&(~((uint64_t)1)))
/*
#undef IS_MARKED_ANY
#define IS_MARKED_ANY(x) (((POINTER_INT)(x))&0x03)
#undef GET_UNMARKED_BOTH
#define GET_UNMARKED_BOTH(x) (POINTER_INT)(((POINTER_INT)(x))&(~((POINTER_INT)3)))
*/
#define TIMESTAMP_SHIFT 48
#define MAX_TIMESTAMP 0xffff
#define TIMESTAMP_MASK ((uint64_t)((uint64_t)(0xffff))<<TIMESTAMP_SHIFT)
#define TIMESTAMP_MARK_MASK ((((uint64_t)(0xffff))<<TIMESTAMP_SHIFT) | 0x03)
#define GET_TIMESTAMP(x) (uint64_t)((((uint64_t)(x))&TIMESTAMP_MASK)>>TIMESTAMP_SHIFT)
#define GET_POINTER(x) (((uint64_t)(x))&(~TIMESTAMP_MASK))
#define GET_UNMARKED_POINTER(x) (uint64_t)(((uint64_t)(x))&(~TIMESTAMP_MARK_MASK))
#define ASSIGN_TIMESTAMP(x,ts) ((((uint64_t)(x))&(~TIMESTAMP_MASK)) | (((uint64_t)ts)<<TIMESTAMP_SHIFT))
#define INCREASE_TIMESTAMP(x) (((uint64_t)(x))+((uint64_t)1<<TIMESTAMP_SHIFT))
#define DELETE_REFERENCE(x) (((uint64_t)(x)) & TIMESTAMP_MASK)
#define IS_NULL(x) ((((uint64_t)(x)) & 0x0000fffffffffffc)==0)
#define GET_UNMARKED(x) (uint64_t)(((uint64_t)(x))&(~((uint64_t)1)))

/* random generatot*/
struct MyRandom {
  unsigned int seed[4];
};

void MyRandomInit(MyRandom *r, unsigned int seed);
unsigned int MyRandomGenNumber(MyRandom *r);

class HASH_INT {
public:
	static const unsigned int _EMPTY_HASH;
	static const unsigned int _BUSY_HASH;
	static const int _EMPTY_KEY;
	static const int _EMPTY_DATA;

	inline static unsigned int Calc(int key) {
		key += (key << 15) ^ 0xD8430DED;
		key ^= (key >> 10);
		key += (key <<  3);
		key ^= (key >>  6);
		key += (key <<  2) + (key << 14);
		key ^= (key >> 16);
		return key;
		/*key = ((~key) + (key << 21) )^ 0xD8430DED; // key = (key << 21) - key - 1;
		key = key ^ (key >> 24);
		key = (key + (key << 3)) + (key << 8); // key * 265
		key = key ^ (key >> 14);
		key = (key + (key << 2)) + (key << 4); // key * 21
		key = key ^ (key >> 28);
		key = key + (key << 31);
		return key;*/
	}

	inline static bool IsEqual(int left_key, int right_key) {
		return left_key == right_key;
	}
};

class HASH_INT1 {
public:
	static const unsigned int _EMPTY_HASH;
	static const unsigned int _BUSY_HASH;
	static const int _EMPTY_KEY;
	static const int _EMPTY_DATA;

	inline static unsigned int Calc (int key) {
		key = (~key) + (key << 21); // key = (key << 21) - key - 1;
		key = key ^ (key >> 24);
		key = (key + (key << 3)) + (key << 8); // key * 265
		key = key ^ (key >> 14);
		key = (key + (key << 2)) + (key << 4); // key * 21
		key = key ^ (key >> 28);
		key = key + (key << 31);
		return key;
	}

	inline static bool IsEqual(int left_key, int right_key) {
		return left_key == right_key;
	}
};

#endif