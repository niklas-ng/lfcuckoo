#include "concckhashtable.h"
#include <string.h>

/*template <typename TKey, 
			 typename TValue, 
			 typename FHash1,
			 typename FHash2>
TValue ConcCukooHashtable<TKey, TValue, FHash1, FHash2>::read(void * table[], 
	unsigned int index, HashEntry &*pointer, uint64_t &time, uint64_t mark_bits)
{
	HashEntry *tmp = table[index];
	pointer = (HashEntry *)GET_UNMARKED_POINTER(tmp);
	time = GET_TIMESTAMP(tmp);
	mark_bits = tmp & 0x3
}*/

template <typename TKey, 
			 typename TValue, 
			 typename FHash1,
			 typename FHash2>
ConcCukooHashtable<TKey, TValue, FHash1, FHash2>::ConcCukooHashtable(int capacity)
{
	int power2 = 1;
	_log2size = 0;
	_sizeMask = 0x0;
	for (;power2<capacity;power2*=2, _log2size++){}
	_sizeMask=(1U<<_log2size) -1;
	_capacity = power2;
	_actualSize = _capacity*NR_POINTER_PER_LINE; 
	_table[0] = new HashEntry*[_actualSize];
	_table[1] = new HashEntry*[_actualSize];

	memset(_table[0], 0, _actualSize*sizeof(HashEntry*));
	memset(_table[1], 0, _actualSize*sizeof(HashEntry*));
	
	//MyRandomInit(&rand_gen, 1234);
	//MyRandomInit(&rand_table,41205);
#ifdef DEBUG
//	_nr_relocation = 0;
//	_total_relocations = 0;
#endif

}


/*
Search operation tries to locate the existence of a key on table 1 or table 2. Search is performed with two rounds,
each round checks the existence in, in order, table 1 and table 2. Two round checking is to not missing an item which 
can be relocated from the second table to the first table. Even though, two round checking can miss an item if it is 
continuously relocated between tables. Particularly, in case there is a sequence of events like this: 
check_1/relocate(table 2->table1)/check_2/ relocate(table 1-> table 2)/check_1/relocate(table 2->table1)/check_2.

The timestamp in the items of the tables is made used to know if such a sequence of events is happening. Timestamp is changed
whenever an item is relocated. Search operation can check if the timestamps of the same items are changes many times enough
that an sequence of relocations like above can happen during the search (check2Differences). If this is the case, the search 
is restarted from the beginning.
*/
template <typename TKey, 
			 typename TValue, 
			 typename FHash1,
			 typename FHash2>
bool ConcCukooHashtable<TKey, TValue, FHash1, FHash2>::Search(TKey &key, TValue &value)
{
	HashEntry *e2, *e1, *e2x, *e1x;
	uint64_t ts1, ts2, ts1x, ts2x;
	int h1 = FHash1::Calc(key)&_sizeMask;
	int h2 = FHash2::Calc(key)&_sizeMask;
	for (;;) {
		//e2pre = (HashEntry*)MEM_READ_P(&_table[1][CORRECT_IDX(h2)]);

		e1 = (HashEntry*)MEM_READ_P(&_table[0][CORRECT_IDX(h1)]);
		if (!IS_NULL(e1)) {
			HashEntry *node1 = (HashEntry *)GET_UNMARKED_POINTER(e1);
			if (node1->_key == key) {
				value = node1->_value;
				return true;
			}
		}

		e2 = (HashEntry*)MEM_READ_P(&_table[1][CORRECT_IDX(h2)]);
		if (!IS_NULL(e2)) {
			HashEntry *node2 = (HashEntry *)GET_UNMARKED_POINTER(e2);
			if (node2->_key == key) {
				value = node2->_value;
				return true;
			}
		}

		e1x=(HashEntry*)MEM_READ_P(&_table[0][CORRECT_IDX(h1)]);
		if (!IS_NULL(e1x)) {
			HashEntry *node1 = (HashEntry *)GET_UNMARKED_POINTER(e1x);
			if (node1->_key == key) {
				value = node1->_value;
				return true;
			}
		}
		
		ts1 = GET_TIMESTAMP(e1);
		ts2 = GET_TIMESTAMP(e2);
		ts1x = GET_TIMESTAMP(e1x);
		if (ts1x<ts1+2 || (ts1x==0 && ts1==MAX_TIMESTAMP)) 
			return false;
		
		e2x = (HashEntry*)MEM_READ_P(&_table[1][CORRECT_IDX(h2)]);
		ts2x = GET_TIMESTAMP(e2x);
		if (!IS_NULL(e2x)) {
			HashEntry *node2 = (HashEntry *)GET_UNMARKED_POINTER(e2x);
			if (node2->_key == key) {
				value = node2->_value;
				return true;
			}
		}
		
		//ts2pre = GET_TIMESTAMP(e2pre);

		if (!check2Differences(ts1, ts2, ts1x, ts2x))//, ts2pre))
			return false;			
	}
}


/*
Fidn operation is similar to search but:
(1) Instead of return when a key is found, Find search the other table as well. If a key is found on both tables,
the one in the second table is deleted as we perceive that the second is overwritten by the one found in the first table.
(2) Perform helping if it see an ongoing relocate operation (the LSB of the item is marked 1).
*/
template <typename TKey, 
			 typename TValue, 
			 typename FHash1,
			 typename FHash2>
Find_Result ConcCukooHashtable<TKey, TValue, FHash1, FHash2>::Find(const TKey &key, HashEntry* &entry1, 
		HashEntry* &entry2, bool isInsert)
//HashEntry **pp_entry)
{
	HashEntry *e2, *e1, *e2x, *e1x;
	uint64_t ts1, ts2, ts1x, ts2x;
	int h1 = FHash1::Calc(key)&_sizeMask;
	int h2 = FHash2::Calc(key)&_sizeMask;
	HashEntry *node1, *node2, *node1x, *node2x;
	Find_Result result; 
 for(;;) {
	result = Not_Found;
	//e2pre = (HashEntry*)MEM_READ_P(&_table[1][CORRECT_IDX(h2)]);
	e1 = (HashEntry*)MEM_READ_P(&_table[0][CORRECT_IDX(h1)]);
	if (!IS_NULL(e1)) {
		node1 = (HashEntry *)GET_UNMARKED_POINTER(e1);
		if (!isInsert&&IS_MARKED_REPLACE(e1)&&node1->_key == key) {
			//help_relocate(_table[0], FHash1::Calc, h1, _table[1], FHash2::Calc);
			newhelp_relocate(0, h1);
#ifdef DEBUG
			_total_help_relocations += 1;
#endif
			continue;
		}
		if (node1->_key == key) {
			result = In_First;
			//*pp_entry = e1;
		}
	}

	e2 = (HashEntry*)MEM_READ_P(&_table[1][CORRECT_IDX(h2)]);
	if (!IS_NULL(e2)) {
		node2 = (HashEntry *)GET_UNMARKED_POINTER(e2);
		if (!isInsert&&IS_MARKED_REPLACE(e2)&&node2->_key == key) {
			//help_relocate(_table[1], FHash2::Calc, h2, _table[0], FHash1::Calc);
			newhelp_relocate(1, h2);
#ifdef DEBUG
			_total_help_relocations += 1;
#endif
			continue;
		}
		if (node2->_key == key) {
			if (result == In_First) {
				HashEntry *tmp = (HashEntry*)MEM_READ_P(&_table[0][CORRECT_IDX(h1)]);
				if (tmp!=e1) continue;
				HashEntry *deleted = (HashEntry *)DELETE_REFERENCE(e2);
				if (CASP(&_table[1][CORRECT_IDX(h2)], e2, deleted))
					entry2 = deleted;
			}else {
				result = In_Second;
				//*pp_entry = e2;
			}

		}
	}

	entry1 = e1;
	entry2 = e2;
	if (result!=Not_Found) {
		return result;
	}

	e1x = (HashEntry*)MEM_READ_P(&_table[0][CORRECT_IDX(h1)]);
	if (!IS_NULL(e1x)) {
		node1x = (HashEntry *)GET_UNMARKED_POINTER(e1x);
		if (!isInsert&&IS_MARKED_REPLACE(e1x)&&node1x->_key == key) {
			//help_relocate(_table[0], FHash1::Calc, h1, _table[1], FHash2::Calc);
			newhelp_relocate(0, h1);
#ifdef DEBUG
			_total_help_relocations += 1;
#endif
			continue;
		}
		if (node1x->_key == key) {
			result = In_First;
			//*pp_entry = e1x;
		}
	}

	ts1 = GET_TIMESTAMP(e1);
	ts2 = GET_TIMESTAMP(e2);
	ts1x = GET_TIMESTAMP(e1x);
	entry1 =e1x;
	if (result==Not_Found)
		if (ts1x<ts1+2 || (ts1x==0 && ts1==MAX_TIMESTAMP))
			return result;

	e2x = (HashEntry*)MEM_READ_P(&_table[1][CORRECT_IDX(h2)]);
	ts2x = GET_TIMESTAMP(e2x);
	entry2 =e2x;
	if (!IS_NULL(e2x)) {
		node2x = (HashEntry *)GET_UNMARKED_POINTER(e2x);
		if (!isInsert&&IS_MARKED_REPLACE(e2x)&&node2x->_key == key) {
			//help_relocate(_table[1], FHash2::Calc, h2, _table[0], FHash1::Calc);
			newhelp_relocate(1, h2);
#ifdef DEBUG
			_total_help_relocations += 1;
#endif
			continue;
		}
		if (node2x->_key == key) {
			if (result == In_First) {
				HashEntry *tmp = (HashEntry*)MEM_READ_P(&_table[0][CORRECT_IDX(h1)]);
				if (tmp!=e1x) continue;
				HashEntry *deleted = (HashEntry *)DELETE_REFERENCE(e2x);
				if (CASP(&_table[1][CORRECT_IDX(h2)], e2x, deleted))
					entry2 =deleted;
			}else {
				result = In_Second;
				//*pp_entry = e2x;
			}

		}
	}
	if (result!=Not_Found) {
		return result;
	}

	//ts2pre = GET_TIMESTAMP(e2pre);

	if (!check2Differences(ts1, ts2, ts1x, ts2x)) //, ts2pre))
		break;
 }

	return result;
}

template <typename TKey, 
			 typename TValue, 
			 typename FHash1,
			 typename FHash2>
void ConcCukooHashtable<TKey, TValue, FHash1, FHash2>::Remove(const TKey &key)
{
	//HashEntry *e1, *e2, 
	HashEntry *p_entry1, * p_entry2;
	int h1 = FHash1::Calc(key)&_sizeMask;
	int h2 = FHash2::Calc(key)&_sizeMask;
	HashEntry * deleted = NULL, *node1 = NULL, *deleted2 = NULL, *node2=NULL;
	
start_remove:
	p_entry1 = p_entry2 = 0;
	//Find_Result res = Find(key, &p_entry);
	Find_Result res = Find(key, p_entry1,p_entry2);
	if (res == Not_Found) return;
	if (res == In_First) goto remove1;
	if (res == In_Second) goto remove2;
	deleted = NULL; 
	node1 = NULL; 
	deleted2 = NULL; 
	node2=NULL; 
	/*remove key found in the _table[0]*/	
remove1:
	/*e1 = (HashEntry*)MEM_READ_P(&_table[0][CORRECT_IDX(h1)]);
	if (e1 != p_entry || IS_MARKED_ANY(e1)) goto start_remove;
	node1 = (HashEntry *)GET_UNMARKED_POINTER(e1);
	assert(node1->_key == key);
	deleted = (HashEntry *)DELETE_REFERENCE(e1);*/

	node1 = (HashEntry *)GET_UNMARKED_POINTER(p_entry1);
	assert(node1->_key == key);
	deleted = (HashEntry *)DELETE_REFERENCE(p_entry1);
	/*compare and swap*/
	if (_table[1][CORRECT_IDX(h2)]!=p_entry2)
		goto start_remove;
	if (!CASP(&_table[0][CORRECT_IDX(h1)], p_entry1, deleted))
		goto start_remove;
	return;

remove2:
	/*e2 = (HashEntry*)MEM_READ_P(&_table[1][CORRECT_IDX(h2)]);
	if (e2 != p_entry || IS_MARKED_ANY(e2)) goto start_remove;
	node2 = (HashEntry *)GET_UNMARKED_POINTER(e2);
	assert(node2->_key == key);
	deleted2 = (HashEntry *)DELETE_REFERENCE(e2);*/
	node2 = (HashEntry *)GET_UNMARKED_POINTER(p_entry2);
	assert(node2->_key == key);
	deleted2 = (HashEntry *)DELETE_REFERENCE(p_entry2);
	/*compare and swap*/
	if (_table[0][CORRECT_IDX(h1)]!=p_entry1)
		goto start_remove;
	if (!CASP(&_table[1][CORRECT_IDX(h2)], p_entry2, deleted2))
		goto start_remove;
	//else
	//	delete node2;
	return;
}

template <typename TKey, 
			 typename TValue, 
			 typename FHash1,
			 typename FHash2> 
bool ConcCukooHashtable<TKey, TValue, FHash1, FHash2>::Insert(TKey key, TValue value)
{
	//HashEntry *e1, *e2, 
	HashEntry *p_entry1, *p_entry2;
	int h1 = FHash1::Calc(key)&_sizeMask;
	int h2 = FHash2::Calc(key)&_sizeMask;
	HashEntry *new_entry = new HashEntry();
	new_entry->_key = key;
	new_entry->_value = value;
	HashEntry *to_insert = 0;
	int rep_counter = 0;

start_insert:
	rep_counter = 0;
	p_entry1 = p_entry2 = 0;
	Find_Result res = Find(key, p_entry1, p_entry2, true);
	if (res == In_First || res == In_Second) 
		return false;
	
	/*insert key to _table[0]*/	
/*insert1:
	e1 = (HashEntry*)MEM_READ_P(&_table[0][CORRECT_IDX(h1)]);
	if (IS_NULL(e1)) {
		to_insert = (HashEntry*)(ASSIGN_TIMESTAMP(new_entry, GET_TIMESTAMP(e1)));
		if (!CASP(&_table[0][CORRECT_IDX(h1)], e1, to_insert) )
			goto insert1;
		return true;
	}*/
	
	if (IS_NULL(p_entry1)) {
		to_insert = (HashEntry*)(ASSIGN_TIMESTAMP(new_entry, GET_TIMESTAMP(p_entry1)));
		if (!CASP(&_table[0][CORRECT_IDX(h1)], p_entry1, to_insert) )
			goto start_insert;
		return true;
	}

/*insert2:
	e2 = (HashEntry*)MEM_READ_P(&_table[1][CORRECT_IDX(h2)]);
	if (IS_NULL(e2)) {
		to_insert = (HashEntry*)(ASSIGN_TIMESTAMP(new_entry, GET_TIMESTAMP(e2)));
		if (!CASP(&_table[1][CORRECT_IDX(h2)], e2, to_insert))
			goto insert2;
		return true;
	}*/
	if (IS_NULL(p_entry2)) {
		to_insert = (HashEntry*)(ASSIGN_TIMESTAMP(new_entry, GET_TIMESTAMP(p_entry2)));
		if (!CASP(&_table[1][CORRECT_IDX(h2)], p_entry2, to_insert))
			goto start_insert;
		return true;
	}
	
	//both places are occupied, try relocate
	bool relocate_res = false;

	/*if (MyRandomGenNumber(&rand_gen)&0x1) {
		relocate_res = relocate(_table[1], FHash2::Calc, h2, _table[0], FHash1::Calc, rep_counter);
#ifdef DEBUG
	_nr_relocation++;
	_total_relocations += rep_counter;
#endif
		if (relocate_res) {
			p_entry2 = (HashEntry*)MEM_READ_P(&_table[1][CORRECT_IDX(h2)]);
			if (IS_NULL(p_entry2)) {
				to_insert = (HashEntry*)(ASSIGN_TIMESTAMP(new_entry, GET_TIMESTAMP(p_entry2)));
				if (CASP(&_table[1][CORRECT_IDX(h2)], p_entry2, to_insert))
					return true;
			}else goto start_insert;
		}
	}
	else {*/

		//relocate_res = relocate(_table[0], FHash1::Calc, h1, _table[1], FHash2::Calc, rep_counter);
	relocate_res = newrelocate(0, h1, rep_counter);
#ifdef DEBUG
	_nr_relocation++;
	_total_relocations += rep_counter;
#endif
		if (relocate_res) {
			p_entry1 = (HashEntry*)MEM_READ_P(&_table[0][CORRECT_IDX(h1)]);
			if (IS_NULL(p_entry1)) {
				to_insert = (HashEntry*)(ASSIGN_TIMESTAMP(new_entry, GET_TIMESTAMP(p_entry1)));
				if (CASP(&_table[0][CORRECT_IDX(h1)], p_entry1, to_insert))
					return true;
			}else goto start_insert;
		}else {
		  rep_counter = 0;
  		  //relocate_res = relocate(_table[1], FHash2::Calc, h2, _table[0], FHash1::Calc, rep_counter);
		  relocate_res = newrelocate(1, h2, rep_counter);
		  if (relocate_res) {
			p_entry2 = (HashEntry*)MEM_READ_P(&_table[1][CORRECT_IDX(h2)]);
			if (IS_NULL(p_entry2)) {
				to_insert = (HashEntry*)(ASSIGN_TIMESTAMP(new_entry, GET_TIMESTAMP(p_entry2)));
				if (CASP(&_table[1][CORRECT_IDX(h2)], p_entry2, to_insert))
					return true;
			}else goto start_insert;
		  }
		}
	//}
	
	//if (rep_counter<=MAX_REPLACE && relocate_res==true)
	//	goto start_insert;

#ifdef DEBUG
	_nr_failed_insert++;
#endif
	return false;
}

template <typename TKey, 
			 typename TValue, 
			 typename FHash1,
			 typename FHash2> 
bool ConcCukooHashtable<TKey, TValue, FHash1, FHash2>::relocate(HashEntry **src, Hash_Function calc_s, int h_s, 
	HashEntry **dest, Hash_Function calc_d, int& p_counter)
{
	HashEntry *e_s, *e_s1, *e_d;
	HashEntry *node1;
	int h_d = 0;
	TKey key_s;
	
	for (;;) { 
		if (p_counter>MAX_REPLACE) return false;
		e_s = (HashEntry*)MEM_READ_P(&src[CORRECT_IDX(h_s)]);
		if (IS_NULL(e_s)) return true;
		/*change start here*/
		node1 = (HashEntry *)GET_UNMARKED_POINTER(e_s);
		key_s = node1->_key;
		h_d = calc_d((int)key_s)&_sizeMask;
		
		e_d = (HashEntry*)MEM_READ_P(&dest[CORRECT_IDX(h_d)]);
		if (!IS_NULL(e_d)) { 
		   //p_counter++;
		   if (!relocate(dest, calc_d, h_d, src, calc_s, ++p_counter))
			return false;
		}
restart_replace:
		e_s = (HashEntry*)MEM_READ_P(&src[CORRECT_IDX(h_s)]);
		if (!IS_MARKED_REPLACE(e_s)) {
			if (!CASP(&src[CORRECT_IDX(h_s)], e_s, GET_MARKED_REPLACE(e_s)))
				goto restart_replace;
		}
		
		if (help_relocate(src, calc_s, h_s, dest, calc_d)) 
			return true;
		else{ //if helping failed, unmark the REPLACE BIT if I am the one who marked it!
			e_s1 = (HashEntry*)MEM_READ_P(&src[CORRECT_IDX(h_s)]);
			if (IS_MARKED_REPLACE(e_s1)&&GET_UNMARKED_REPLACE(e_s1)==GET_UNMARKED_REPLACE(e_s)) {
				CASP(&src[CORRECT_IDX(h_s)], e_s1, GET_UNMARKED_REPLACE(e_s1));
			}else 
			    if (IS_NULL(e_s1)) return true;	
		}
	}
	return false;
}

template <typename TKey, 
			 typename TValue, 
			 typename FHash1,
			 typename FHash2> 
bool ConcCukooHashtable<TKey, TValue, FHash1, FHash2>::help_relocate(HashEntry **src, Hash_Function calc_s, int pos1, 
	HashEntry **dest, Hash_Function calc_d)
{
	HashEntry *e_s, *e_d;
	HashEntry *node1, *new_node;
	int ts_s, ts_d;
	int h_s = pos1;
	int h_d;
	TKey key_s;

	for (;;) { 
		e_s = (HashEntry*)MEM_READ_P(&src[CORRECT_IDX(h_s)]);
		if (IS_NULL(e_s) || !IS_MARKED_REPLACE(e_s)) 
			return true;
		
		node1 = (HashEntry *)GET_UNMARKED_POINTER(e_s);
		key_s = node1->_key;
		h_d = calc_d((int)key_s)&_sizeMask;

		e_d = (HashEntry*)MEM_READ_P(&dest[CORRECT_IDX(h_d)]);

		if (IS_NULL(e_d)) {
			ts_d = GET_TIMESTAMP(e_d);
			ts_s = GET_TIMESTAMP(e_s);
			new_node = (HashEntry *)(ASSIGN_TIMESTAMP(node1, (ts_s>ts_d?ts_s:ts_d)+1));
			if (CASP(&dest[CORRECT_IDX(h_d)], e_d, new_node)) {
				CASP(&src[CORRECT_IDX(h_s)], e_s, INCREASE_TIMESTAMP(DELETE_REFERENCE(e_s)));
				break;
			}
		} else {
			if (GET_UNMARKED_POINTER(e_s) == GET_UNMARKED_POINTER(e_d)) {
				CASP(&src[CORRECT_IDX(h_s)], e_s, INCREASE_TIMESTAMP(DELETE_REFERENCE(e_s)));
				break;
			}
			else
			   return false; //relocate(dest, calc_d, h_d, src, calc_s, p_cnunter);
		}
	}
	return true;
}

template <typename TKey, 
			 typename TValue, 
			 typename FHash1,
			 typename FHash2> 
int ConcCukooHashtable<TKey, TValue, FHash1, FHash2>::newrelocate(unsigned int which, const uint64_t buck_pos, int& p_counter) 
{
	assert(which==0 || which==1);
	relocate_data route[MAX_REPLACE];
	int start_level = 0;
	TKey key;
	int next_index;
	int cur_table = which;
	int cur_index = buck_pos;
	HashEntry *cur_entry = NULL, *full_entry=NULL; 

retry_relocate1:
	bool found = false;
	int depth=start_level;
	while (depth<MAX_REPLACE && !found)
	{
		full_entry = (HashEntry*)MEM_READ_P(&_table[cur_table][CORRECT_IDX(cur_index)]);
		if ((uint64_t)full_entry & 1) {
			newhelp_relocate(cur_table, cur_index);
			continue;
		} 

		route[depth].index = cur_index;
		route[depth].entry = full_entry;

		if (IS_NULL(full_entry)) {
			found = true;
			break;
		}
		cur_entry = (HashEntry *)GET_UNMARKED_POINTER(full_entry);
		key = cur_entry->_key;
		if (cur_table==0) {
			cur_index = FHash2::Calc(key)&_sizeMask;
		} else {
			cur_index = FHash1::Calc(key)&_sizeMask;
		}
		cur_table = 1- cur_table;
		++depth;
	}

	HashEntry *next_entry;
	if (found) {
		cur_table = 1-cur_table;
		for (int idx = depth-1; idx>=0; --idx, cur_table = 1-cur_table) {
			assert((idx%2==0 && cur_table==which) || (idx%2==1 && cur_table==1-which));
			cur_index = route[idx].index;
			full_entry = (HashEntry*)MEM_READ_P(&_table[cur_table][CORRECT_IDX(cur_index)]);
			if (IS_NULL(full_entry)) {
				continue;
			}
			if ((uint64_t)full_entry &1) {
				newhelp_relocate(cur_table, cur_index);
				continue;
			}
			cur_entry = (HashEntry *)GET_UNMARKED_POINTER(full_entry);
			next_index = cur_table==0 ? FHash2::Calc(cur_entry->_key)&_sizeMask : FHash1::Calc(cur_entry->_key)&_sizeMask;
			next_entry = (HashEntry*)MEM_READ_P(&_table[1-cur_table][CORRECT_IDX(next_index)]);
			if (!IS_NULL(next_entry)) {
				start_level = idx+1;
				cur_index = next_index;
				cur_table=1-cur_table;
				goto retry_relocate1;
			}
			if (!CASP(&_table[cur_table][CORRECT_IDX(cur_index)], full_entry, GET_MARKED_REPLACE(full_entry))) {
				start_level = idx;
				goto retry_relocate1;
			}
			++p_counter;
			newhelp_relocate(cur_table, cur_index);
		}
	}
	return found;
}


template <typename TKey, 
			 typename TValue, 
			 typename FHash1,
			 typename FHash2> 
bool ConcCukooHashtable<TKey, TValue, FHash1, FHash2>::newhelp_relocate(int table, int index)
{
	HashEntry *e_s, *e_d;
	HashEntry *node1, *new_node;
	int ts_s, ts_d;
	int h_d;
	TKey key_s;

	e_s = (HashEntry*)MEM_READ_P(&_table[table][CORRECT_IDX(index)]);
	if (IS_NULL(e_s) || !IS_MARKED_REPLACE(e_s)) 
		return true;
		
	node1 = (HashEntry *)GET_UNMARKED_POINTER(e_s);
	key_s = node1->_key;
	if (table==0)
		h_d = FHash2::Calc((int)key_s)&_sizeMask;
	else h_d = FHash1::Calc((int)key_s)&_sizeMask;

	e_d = (HashEntry*)MEM_READ_P(&_table[1-table][CORRECT_IDX(h_d)]);
	if (IS_NULL(e_d)) {
		ts_d = GET_TIMESTAMP(e_d);
		ts_s = GET_TIMESTAMP(e_s);
		new_node = (HashEntry *)(ASSIGN_TIMESTAMP(node1, (ts_s>ts_d?ts_s:ts_d)+1));
		if (CASP(&_table[1-table][CORRECT_IDX(h_d)], e_d, new_node)) {
			CASP(&_table[table][CORRECT_IDX(index)], e_s, INCREASE_TIMESTAMP(DELETE_REFERENCE(e_s)));
			return true;
		}
	} else {
		if (GET_UNMARKED_POINTER(e_s) == GET_UNMARKED_POINTER(e_d)) {
			CASP(&_table[table][CORRECT_IDX(index)], e_s, INCREASE_TIMESTAMP(DELETE_REFERENCE(e_s)));
			return true;
		}
	}
		
	CASP(&_table[table][CORRECT_IDX(index)], e_s, GET_UNMARKED_REPLACE(e_s));
	return false;
}