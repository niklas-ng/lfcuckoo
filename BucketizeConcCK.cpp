#include <string.h>
#include <cstdlib>
#include "BucketizeConcCK.h"
//#include "HPMemRec.cpp"

static inline uint64_t exchange(desc_t *desc, bool initiator) {
	uint64_t mdesc = (uint64_t)desc | 1;

	if (desc->result == SUCCESS || desc->result == SECONDFAIL) {
		if ((uint64_t)desc == ((uint64_t)desc->ptr2 & (~1))) {
			CASP(desc->ptr2, mdesc, desc->old2);
		}else {
			CASP(desc->ptr1, mdesc, desc->old1);
		}
		return desc->result;
	}

	if (initiator && !CASP(desc->ptr1, desc->old1, mdesc))
		return FIRSTFAIL;

	int p2set = CASP(desc->ptr2, desc->old2, mdesc);
	if (!p2set) {
		if (desc->ptr2!=desc) {
			CASP(&desc->result, UNDECIDED, SECONDFAIL);
			if (desc->result==SUCCESS)
				return desc->result;
			if (desc->result==SECONDFAIL) {
				CASP(desc->ptr2, desc, desc->old1);
				return desc->result;
			}
		}
	}
	CASP(&desc->result, UNDECIDED, mdesc);
	if (desc->result==SECONDFAIL) {
		if (p2set) CASP(desc->ptr2, mdesc, desc->old2);
		return desc->result;
	}
	CASP(desc->ptr1, mdesc, desc->new1);
	CASP(desc->ptr2, desc->result, desc->new2);
	desc->result = SUCCESS;
	return desc->result;
}

static void * read(void *ptr) {
	uint64_t result = (uint64_t)MEM_READ_P(ptr);
	//HPMemRecl<desc_t, 1>::HP_Rec * hp = hp_desc->getHPRec();
	while (result & 0x1) {
		//hp_desc->employHP(hp, 0, (desc_t *)(result & (~1)));
		if (result==(uint64_t)MEM_READ_P(ptr))
			exchange((desc_t *)(result & (~1)),false);
		result = (uint64_t)MEM_READ_P(ptr);
	}
	return (void *)result;
}

static inline
void
compute_hash(const void *key, size_t key_len, uint64_t *h1, uint64_t *h2)
{
  /* Initial values are arbitrary.  */
  *h1 = 0x3ac5d673;
  *h2 = 0x6d7839d0;
  SpookyHash::Hash128(key, key_len, h1, h2);
  if (*h1 != *h2)
    {
      return;
    }
  else
    {
      *h2 = ~*h2;
    }
}

template <typename TKey, 
			 typename TValue>
BucketizeConcCK<TKey,TValue>::BucketizeConcCK(int capacity)
{
	_bin_size = BIN_SIZE;
	_log2size = 0;
	int nr_items = _bin_size;
	for (;nr_items<capacity; nr_items<<=1, _log2size++){}
	_sizeMask=(1U<<_log2size) -1;
	_capacity = nr_items;
	_nrBuckets = nr_items/_bin_size;

	_table = (HashElement **)calloc(_capacity, sizeof(*_table));
//	hp_node = HPMemRecl<HashElement,2>::getHPMemRecl();
//	hp_desc = HPMemRecl<desc_t,1>::getHPMemRecl();
}

template <typename TKey, 
			 typename TValue>
BucketizeConcCK<TKey,TValue>::~BucketizeConcCK(void)
{
	HashElement **elem = _table;
	HashElement **end = _table + _capacity;
	HashElement * entry = (HashElement *)read(elem);
	for (; elem!=end; elem++, entry = (HashElement *)read(elem)) {
		if (!IS_NULL(entry)) {
			delete (HashElement *)(GET_UNMARKED_POINTER(entry));
		}
	}
	free(_table);
}

template <typename TKey, 
			 typename TValue>
bool BucketizeConcCK<TKey,TValue>::checkDESC(desc_t *tmp, const TKey &key, TValue &value) {
	HashElement *entry = (HashElement *)GET_UNMARKED_POINTER(tmp->old1);
	if (entry->_item._key == key) {
		value = entry->_item._value;
		return true;
	}
	return false;
}

template <typename TKey, 
			 typename TValue>
bool BucketizeConcCK<TKey,TValue>::Search(const TKey &key, TValue &value) 
{
	uint64_t hash1=0, hash2=0;
	compute_hash(&key, sizeof(key), &hash1, &hash2);
//	typename HPMemRecl<HashElement, 2>::HP_Rec * hp = hp_node->getHPRec();
//	typename HPMemRecl<HashElement, 2>::HP_Rec * hpdesc = hp_desc->getHPRec();
retry:
	HashElement **elem = bin_at(hash1 & _sizeMask);
	HashElement *fullentry = NULL; 
	HashElement *entry = NULL;
	uint64_t *time1 = new uint64_t[_bin_size];
	uint64_t *time2 = new uint64_t[_bin_size]; 
	for (int i=0; i<_bin_size; ++i, ++elem) {
		while (true) {
			fullentry = *elem;
			time1[i] = GET_TIMESTAMP(fullentry);
			if (IS_NULL(fullentry)) {
				break;
			}
			if ((uint64_t)fullentry & 0x1L) {
				desc_t * tmp = (desc_t *)((uint64_t)fullentry & (~1));
				//hpdesc->employHP(hp, 0, (desc_t *)tmp);
				if ((uint64_t)tmp!= ((uint64_t)*elem & (~1)))
					continue;
				if (checkDESC(tmp, key, value)) {
//					hp_node->retireHP(0);
					return true;
				}	
				time1[i] = GET_TIMESTAMP(tmp->old1);
				break;
			}
			else {
				entry = (HashElement *)GET_UNMARKED_POINTER(fullentry);
//				hp_node->employHP(hp, 0, (HashElement *)entry);
				if ((HashElement *)GET_UNMARKED_POINTER(*elem)!=entry)
					continue;
				if (entry->_hash1 == hash1 && entry->_hash2 == hash2 
					&& entry->_item._key == key) {
					value = entry->_item._value;
//					hp_node->retireHP(0);
					return true;
				}
				break;
			}
		}		
	}

	elem = bin_at(hash2 & _sizeMask);
	for (int i=0; i<_bin_size; ++i,++elem) {
		while (true) {
			fullentry = *elem;
			time2[i] = GET_TIMESTAMP(fullentry);
			if (IS_NULL(fullentry)) {
				break;
			}
			if ((uint64_t)fullentry & 0x1L) {
				desc_t * tmp = (desc_t *)((uint64_t)fullentry & (~0x1L));
				//hpdesc->employHP(hp, 0, (desc_t *)tmp);
				if ((uint64_t)tmp != ((uint64_t)*elem & (~1)))
					continue;
				if (checkDESC(tmp, key, value)) {
//					hp_node->retireHP(0);
					return true;
				}	
				time2[i] = GET_TIMESTAMP(tmp->old1);
				break;
			}
			else {
				entry = (HashElement *)GET_UNMARKED_POINTER(fullentry);
//				hp_node->employHP(hp, 0, (HashElement *)entry);
				if ((HashElement *)GET_UNMARKED_POINTER(*elem)!=entry)
					continue;
				if (entry->_hash1 == hash1 && entry->_hash2 == hash2 
					&& entry->_item._key == key) {
					value = entry->_item._value;
//					hp_node->retireHP(0);
					return true;
				}
				break;
			}
		}
	}

	elem = bin_at(hash1 & _sizeMask);
	uint64_t timestamp = 0;
	for (int i=0; i<_bin_size; ++i, ++elem) {
		while (true) {
			fullentry = *elem;
			timestamp = GET_TIMESTAMP(fullentry);
			if (IS_NULL(fullentry)) {
				if (timestamp>=time1[i]+2 || (timestamp<time1[i] && timestamp+MAX_TIMESTAMP>=time1[i]+2))
					goto retry;
				break;
			}
			if ((uint64_t)fullentry & 1) {
				desc_t * tmp = (desc_t *)((uint64_t)fullentry & (~1));
				//hpdesc->employHP(hp, 0, (desc_t *)tmp);
				if ((uint64_t)tmp!= ((uint64_t)*elem & (~1)))
					continue;
				if (checkDESC(tmp, key, value)) {
//					hp_node->retireHP(0);
					return true;
				}	
				timestamp = GET_TIMESTAMP(tmp->old1);
				if (timestamp>=time1[i]+2 || (timestamp<time1[i] && timestamp+MAX_TIMESTAMP>=time1[i]+2))
					goto retry;
				break;
			}
			else {
				entry = (HashElement *)GET_UNMARKED_POINTER(fullentry);
//				hp_node->employHP(hp, 0, (HashElement *)entry);
				if ((HashElement *)GET_UNMARKED_POINTER(*elem)!=entry)
					continue;
				if (entry->_hash1 == hash1 && entry->_hash2 == hash2 
					&& entry->_item._key == key) {
					value = entry->_item._value;
//					hp_node->retireHP(0);
					return true;
				}
				if (timestamp>=time1[i]+2 || (timestamp<time1[i] && timestamp+MAX_TIMESTAMP>=time1[i]+2))
					goto retry;
				break;
			}
		}
	}
//	hp_node->retireHP(0);
	return false;
}

template <typename TKey, 
			 typename TValue>
bool BucketizeConcCK<TKey,TValue>::Insert(const TKey &key, const TValue &value)
{
	uint64_t hash1=0, hash2=0;
	compute_hash(&key, sizeof(key), &hash1, &hash2);
	HashElement *newelem = new HashElement();
	newelem->_item._key = key;
	newelem->_item._value = value;
	HashElement * to_insert = NULL;

	HashElement **elem = bin_at(hash1 & _sizeMask);
	newelem->_hash1 = hash1;
	newelem->_hash2 = hash2;
	HashElement *fullentry = NULL; 
	HashElement *entry = NULL;
	HashElement *deleted = NULL;
	int result = 0;
//	typename HPMemRecl<HashElement, 2>::HP_Rec * hp = hp_node->getHPRec();

	for (int i=0; i<_bin_size; ++i, ++elem) {
		do {
			fullentry = (HashElement *)read(elem);
			entry = (HashElement *)GET_UNMARKED_POINTER(fullentry);
//			hp_node->employHP(hp, 0, (HashElement *)entry);
		}while (fullentry!=*elem);
		
		if (IS_NULL(entry)) 
		{
			to_insert = (HashElement*)(ASSIGN_TIMESTAMP(newelem, GET_TIMESTAMP(fullentry)));
			if (CASP(elem, fullentry, to_insert)) {
//				hp_node->retireHP(0);
				result = 1;break;
			}
		}
		else if (entry->_hash1 == hash1 && entry->_hash2 == hash2 
				&& entry->_item._key == key) 
		{
				SWAPP(&(entry->_item._value), value);
//				hp_node->retireHP(0);
				result = 2;break;
		}
	}

	if (result==0)
	{
		elem = bin_at(hash2 & _sizeMask);
		fullentry = NULL;
		for (int i=0; i<_bin_size; ++i,++elem) 
		{
			do {
				fullentry = (HashElement *)read(elem);
				entry = (HashElement *)GET_UNMARKED_POINTER(fullentry);
	//			hp_node->employHP(hp, 0, (HashElement *)entry);
			}while (fullentry!=*elem);

			if (IS_NULL(entry))
			{
				to_insert = (HashElement*)(ASSIGN_TIMESTAMP(newelem, GET_TIMESTAMP(fullentry)));
				if (CASP(elem, fullentry, to_insert)) {
	//				hp_node->retireHP(0);
					result = 1;break;
				}
			}
			else if (entry->_hash2 == hash2 && entry->_hash1 == hash1
				&& entry->_item._key == key) 
			{
				SWAPP(&(entry->_item._value), value);
	//			hp_node->retireHP(0);
				result = 2;break;
			}
		}
	}

//	hp_node->retireHP(0);
	if (result==0)
	{
		int offset = -1;
		elem = bin_at(hash1 & _sizeMask);
		offset = relocate(hash1 & _sizeMask);
		while (result==0 && 0<=offset && offset<_bin_size) 
		{
			fullentry = (HashElement *)read(elem+offset);
			if (IS_NULL(fullentry))
			{
				to_insert = (HashElement*)(ASSIGN_TIMESTAMP(newelem, GET_TIMESTAMP(fullentry)));
				if (CASP(elem, fullentry, to_insert))
					result = 1;break;
			}else 
				offset = relocate(hash1 & _sizeMask);
		}
	}
	//otherwise, insert fails
	if (result==0 || result==2)
		delete newelem;
	return (result!=0);
}
	
template <typename TKey, 
			 typename TValue>
void BucketizeConcCK<TKey,TValue>::Remove(const TKey &key) 
{
	uint64_t hash1=0, hash2=0;
	compute_hash(&key, sizeof(key), &hash1, &hash2);
//	typename HPMemRecl<HashElement, 2>::HP_Rec * hp = hp_node->getHPRec();
retry_remove:
	HashElement **elem = bin_at(hash1 & _sizeMask);
	HashElement *fullentry = NULL; 
	HashElement *entry = NULL;
	HashElement *deleted = NULL;
	uint64_t *time1 = new uint64_t[_bin_size];
	uint64_t *time2 = new uint64_t[_bin_size];
	uint64_t timestamp;
	for (int i=0; i<_bin_size; ++i, ++elem) {
		do {
			fullentry = (HashElement *)read(elem);
			entry = (HashElement *)GET_UNMARKED_POINTER(fullentry);
//			hp_node->employHP(hp, 0, (HashElement *)entry);
		}while (fullentry!=*elem);
		time1[i] = GET_TIMESTAMP(fullentry);

		if (IS_NULL(entry)) 
			continue;
		if (entry->_hash1 == hash1 && entry->_hash2 == hash2 
				&& entry->_item._key == key) {
				deleted = (HashElement *)DELETE_REFERENCE(fullentry);
				if (CASP(elem, fullentry, deleted)) {
//					hp_node->retireHP(0);
//					hp_node->delNode(entry);
					return;
				}
		}
	}

	elem = bin_at(hash2 & _sizeMask);
	fullentry = NULL;
	for (int i=0; i<_bin_size; ++i,++elem) 
	{
		do 
		{
			fullentry = (HashElement *)read(elem);
			entry = (HashElement *)GET_UNMARKED_POINTER(fullentry);
//			hp_node->employHP(hp, 0, (HashElement *)entry);
		}while (fullentry!=*elem);
		time2[i] = GET_TIMESTAMP(fullentry);
		if (IS_NULL(entry))
			continue;
		if (entry->_hash2 == hash2 && entry->_hash1 == hash1
			&& entry->_item._key == key) 
		{
			deleted = (HashElement *)DELETE_REFERENCE(fullentry);
			if (CASP(elem, fullentry, deleted)) 
			{
//				hp_node->retireHP(0);
//				hp_node->delNode(entry);
				return;
			}
		}
	}

	elem = bin_at(hash1 & _sizeMask);
	fullentry = NULL; 
	entry = NULL;
	for (int i=0; i<_bin_size; ++i, ++elem) {
		do 
		{
			fullentry = (HashElement *)read(elem);
			entry = (HashElement *)GET_UNMARKED_POINTER(fullentry);
//			hp_node->employHP(hp, 0, (HashElement *)entry);
		}while (fullentry!=*elem);
		timestamp = GET_TIMESTAMP(fullentry);
		if (IS_NULL(entry)) 
		{	
			if (timestamp>=time1[i]+2 || (timestamp<time1[i] && timestamp+MAX_TIMESTAMP>=time1[i]+2))
				goto retry_remove;
		}
		else 
		{
			if (entry->_hash2 == hash2 && entry->_hash1 == hash1 
				&& entry->_item._key == key) 
			{
				deleted = (HashElement *)DELETE_REFERENCE(fullentry);
				if (CASP(elem, fullentry, deleted))
				{
//					hp_node->retireHP(0);
//					hp_node->delNode(entry);
					return;
				}//TODO: if CASP fails, should re-read the item and check!
			}
			if (timestamp>=time1[i]+2 || (timestamp<time1[i] && timestamp+MAX_TIMESTAMP>=time1[i]+2))
				goto retry_remove;
		}
	}
//	hp_node->retireHP(0);
}

static int inline next_offset(int offset) {
	return ++offset;
}

template <typename TKey, 
			 typename TValue>
int BucketizeConcCK<TKey,TValue>::relocate(const uint64_t buck_pos) 
{
	int result_offset = -1;
	HashElement **cur_buck = NULL;
	HashElement **pre_buck = NULL;
	HashElement *elem;
	void * fullentry;
	int offset = 0;
	uint64_t route[MAX_RELOCATE][3];
	memset(&route, 0 , sizeof(route));
	int depth = 0;
	uint64_t cur_pos = buck_pos;
	int start_level = 0;
//	typename HPMemRecl<HashElement, 2>::HP_Rec * hp = hp_node->getHPRec();
	//typename HPMemRecl<HashElement, 2>::HP_Rec * hpdesc = hp_desc->getHPRec();
retry_relocate:
	bool found = false;
	for (depth=start_level; depth<MAX_RELOCATE; ++depth)
	{
		cur_buck = bin_at(cur_pos);
		for (int i=0; i<_bin_size; i++) 
		{
			fullentry = read(cur_buck+i);
			if (IS_NULL(fullentry)) {
				found = true;
				route[depth][0] = cur_pos;
				route[depth][1] = i;
				route[depth][2] = (uint64_t)fullentry;
				break;
			}
		}
		if (found) break;
		do {
			fullentry = read(cur_buck+offset);
			elem = (HashElement *)GET_UNMARKED_POINTER(fullentry);
			if (elem==NULL) {found = true; break;}
//			hp_node->employHP(hp, 0, (HashElement *)elem);
		}while (fullentry != *(cur_buck+offset));
		route[depth][0] = cur_pos;
		route[depth][1] = offset;
		route[depth][2] = (uint64_t)fullentry;
		if (found) break;
		assert((elem->_hash2 & _sizeMask)==cur_pos || (elem->_hash1 & _sizeMask)==cur_pos);
		/*if ((elem->_hash2 & _sizeMask)!=cur_pos && (elem->_hash1 & _sizeMask)!=cur_pos)                 {
                        printf("%lu %lu %lu\n", elem->_hash2 & _sizeMask, (elem->_hash1 & _sizeMask), cur_pos);
                        fflush(stdout);
         }*/

		cur_pos = (elem->_hash2 & _sizeMask) + (elem->_hash1 & _sizeMask) - cur_pos;
		offset = next_offset(offset);
//		printf("%lu %lu %lu %llx  ",  (elem->_hash2 & _sizeMask), (elem->_hash1 & _sizeMask), cur_pos, offset);
		if (offset>=_bin_size) 
			offset=0;
	}

	int end_path = depth;
	void *cur, *pre;
	uint64_t new_pre = 0, new_cur = 0; 
	if (found) {
		//desc_t *p_desc = new desc_t[end_path+1]; /*TODO: take care of the unallocation/delete of this*/
		for (int idx = end_path; idx>0; --idx) {
			desc_t *desc = new desc_t;

			cur_buck = bin_at(route[idx][0]);
			cur = read(cur_buck+route[idx][1]);
			if (!IS_NULL(cur)) {
				start_level = idx;
				cur_pos = route[idx][0];
				offset = (int)route[idx][1];
				//hp_desc->delNode(p_desc);
				goto retry_relocate;
			}
			desc->ptr2 = cur_buck+route[idx][1];
			desc->old2 = (uint64_t)cur;
			pre_buck = bin_at(route[idx-1][0]);
			do {
				pre = read(pre_buck+route[idx-1][1]);
				if ((uint64_t)pre!=route[idx-1][2]) {
					start_level = idx-1;
					cur_pos = route[idx-1][0];
					offset = (int)route[idx-1][1];
					//hp_desc->delNode(p_desc);
					goto retry_relocate;
				}
				//hp_node->employHP(hp, 0, GET_UNMARKED_POINTER(pre));
			}while(pre!=*(pre_buck+route[idx-1][1]));
			new_pre = INCREASE_TIMESTAMP(DELETE_REFERENCE(pre));
			new_cur = ASSIGN_TIMESTAMP(pre, GET_TIMESTAMP(cur)+1);
			desc->ptr1 = pre_buck+route[idx-1][1];
			desc->old1 = (uint64_t)pre;
			desc->new1 = new_pre;
			desc->new2 = new_cur;
			desc->result = UNDECIDED;
#ifdef DEBUG
			/*HashElement *xelem = (HashElement *)GET_UNMARKED_POINTER(pre);
			printf("%lu %lu %lu %lu  ",  (elem->_hash2 & _sizeMask), (elem->_hash1 & _sizeMask), route[idx][0], route[idx-1][0]);*/
#endif
			uint64_t result = exchange(desc, true);
			if (result==SECONDFAIL) {
				start_level = idx;
				cur_pos = route[idx][0];
				offset = (int)route[idx][1];
//				hp_desc->delNode(p_desc);
//				hp_node->retireHP(0);
				goto retry_relocate;
			}
			if (result==FIRSTFAIL) {
				start_level = idx-1;
				cur_pos = route[idx-1][0];
				offset = (int)route[idx-1][1];
//				hp_desc->delNode(p_desc);
//				hp_node->retireHP(0);
				goto retry_relocate;
			}
			assert(result==SUCCESS);
			//hp_desc->delNode(p_desc);
		}
		result_offset = (int)route[0][1];
	}
//	hp_node->retireHP(0);
	return result_offset;
}