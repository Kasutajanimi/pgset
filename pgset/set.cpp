#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include "set.h"

#define PGSET_INT(t,p) *(PGSET_PINT(t,p))
#define PGSET_PINT(t,p) ((int*)(t->value_table)+p)


unsigned long set_int_hash(void* value)
{
	return *(int*)value;
}
int set_int_equal(void* value1, void* value2)
{
	return *(int*)value1==*(int*)value2;
}


unsigned long set_double_hash(void* value)
{
	return (int)*(double*)value * 10;
}
int set_double_equal(void* value1, void* value2)
{
	return *(double*)value1==*(double*)value2;
}


/* init type function tables */
static SetHashFunc set_hash_fn[] = {set_int_hash, set_double_hash, set_int_hash};
static SetEqualFunc set_equal_fn[] = {set_int_equal, set_double_equal, set_int_equal};


/* This is a set of good hash table prime numbers, from:
 *   http://planetmath.org/encyclopedia/GoodHashTablePrimes.html
 * Each prime is roughly double the previous value, and as far as
 * possible from the nearest powers of two. */

static const unsigned int set_primes[] = {
	193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317,
	196613, 393241, 786433, 1572869, 3145739, 6291469,
	12582917, 25165843, 50331653, 100663319, 201326611,
	402653189, 805306457, 1610612741,
};

static const int set_num_primes = sizeof(set_primes) / sizeof(int);


static int pgset_allocate_hashtable(Set *set)
{
	/* Determine the table size based on the current prime index.
	 * An attempt is made here to ensure sensible behavior if the
	 * maximum prime is exceeded, but in practice other things are
	 * likely to break long before that happens. */

	if (set->prime_index < set_num_primes) {
		set->table_size = set_primes[set->prime_index];
	} else {
		set->table_size = set->entries * 10;
	}

	// Allocate the table and initialise to -1 = nodata
	// hashtable contains indexes in value table
	set->table = (int*)palloc(set->table_size * sizeof(int));
	memset(set->table, -1, set->table_size * sizeof(int));
	// adjust table offset - to be used in toast function
	set->table_offset = (char*)set->table - (char*)set;


	return set->table != NULL;
}

static int pgset_allocate_valuetable(Set *set, int size=64)
{
	//set type contains size of element for value table
	int	memsize = size * set_element_sizes[set->set_type];
	void *new_table = palloc(memsize);

	if (set->value_table!=NULL)
	{
		memcpy(new_table, set->value_table,set->alloced_size * set_element_sizes[set->set_type]);
		pfree(set->value_table);
	}

	set->alloced_size = size;
	set->alloced_mem_size = memsize;
	set->value_table = new_table;

	// offset in bytes
	set->value_table_offset = (char*)set->value_table - (char*)set;

	return set->value_table != NULL;
}

//PGSet functions

Set* pgset_new(PgSetType type)
{
	Set	*new_set;

	new_set = (Set*)palloc(sizeof(Set));
	if (new_set == NULL) {
		return NULL;
	}

	// we use that info to check if set is on same location in memory
	// so we don't have to toest
	PGSET_SET_ADDRESS(new_set);

	new_set->set_type = type;
	new_set->num_values = 0;
	new_set->value_table = NULL;
	// allocate value table	- array of values
	if (!pgset_allocate_valuetable(new_set)) {
			pfree(new_set);
			return NULL;
	}

	new_set->entries = 0;
	new_set->prime_index = 0;
//	new_set->free_func = NULL;

	// Allocate hashtable - contains indexes in value table
	if (!pgset_allocate_hashtable(new_set)) {
		pfree(new_set);
		return NULL;
	}


	return new_set;
}

BOOL pgset_insert(Set* set, void *data)
{
	// Possibly resize value table to a larger size
	if (set->num_values >= set->alloced_size) {
		pgset_allocate_valuetable(set, set->alloced_size*2);
	}

	//copy value to value table
	memcpy(PGSET_VALUE(set,set->num_values),data, set_element_sizes[set->set_type]);

	// insert value table index to hashtable
	if(pgset_insert_hashtable(set,set->num_values,data))
	{
		set->num_values++;
		return TRUE;
	}

	return FALSE;
}

void * pgset_toast(Set** set, int *byte_size)
{
	int hash_table_size = set_primes[(*set)->prime_index] * sizeof(int);
	int total_mem_size = sizeof(Set) + (*set)->alloced_mem_size + hash_table_size;

	*byte_size = total_mem_size;

	// check if we need to make a continuos memory block
	if ((*set)->value_table_offset!=sizeof(Set) || (*set)->table_offset!=(*set)->alloced_mem_size)
	{
		Set* new_set = (Set*)palloc(total_mem_size);
		memcpy(new_set, *set, sizeof(Set));

		PGSET_SET_ADDRESS(new_set);
		new_set->value_table_offset = sizeof(Set);
		new_set->table_offset = (*set)->alloced_mem_size;

		//copy value table
		new_set->value_table = (void*)((char*)new_set + sizeof(Set));
		memcpy(new_set->value_table, (*set)->value_table, (*set)->alloced_mem_size);
		//copy hash table
		new_set->table = (int*)((char*)new_set + sizeof(Set) +  (*set)->alloced_mem_size);
		memcpy(new_set->table, (*set)->table, hash_table_size);
		//free old set
		set_free(*set);

		*set = new_set;
	}
	return *set;
}

void pgset_adjust_address(Set* set)
{
	set->value_table = (void*)((char*)set + sizeof(Set));
	set->table = (int*)((char*)set + sizeof(Set) + set->alloced_mem_size);
}

void set_free(Set *set)
{
	pfree(set->table);
	pfree(set->value_table);
	pfree(set);
}

/*void set_register_free_function(Set *set, SetFreeFunc free_func)
{
	set->free_func = free_func;
}*/

static int set_enlarge(Set *set)
{
	int rover;
	SetEntry *old_table;
	int old_table_size;
	int old_prime_index;
	int index;
	int i;

	/* Store the old table */

	old_table = set->table;
	old_table_size = set->table_size;
	old_prime_index = set->prime_index;

	/* Use the next table size from the prime number array */

	++set->prime_index;

	/* Allocate the new table */

	if (!pgset_allocate_hashtable(set)) {
		set->table = old_table;
		set->table_size = old_table_size;
		set->prime_index = old_prime_index;

		return 0;
	}

	/* Iterate through all entries in the old table and add them
	 * to the new one */

	for (i=0; i<old_table_size; ++i) {

		/* Walk along this chain */

		rover = old_table[i];

		if (rover != -1) {
			index = set_hash_fn[set->set_type](PGSET_VALUE(set,i)) % set->table_size;
			set->table[index] = rover;
		}
	}

	/* Free back the old table */

	pfree(old_table);

	/* Resized successfully */

	return 1;
}

BOOL pgset_insert_hashtable(Set *set,int vt_index, void* data)
{
	int ht_index;

	/* The hash table becomes less efficient as the number of entries
	 * increases. Check if the percentage used becomes large. */

	if ((set->entries * 3) / set->table_size > 0) {

		/* The table is more than 1/3 full and must be increased in size */

		if (!set_enlarge(set)) {
			return FALSE;
		}
	}

	// Use the hash of the data to determine an index to insert into the
	// table at.

	ht_index = set_hash_fn[set->set_type](data) % set->table_size;

	int vt_index_tmp = set->table[ht_index];

	while (vt_index_tmp != -1) {

		if (set_equal_fn[set->set_type](data, PGSET_VALUE(set, vt_index_tmp)) != 0) {
			// This data is already in the set
			return FALSE;
		}

		// hash collision? - goto next next
		// also check for table overflow
		ht_index  = (ht_index + 1) % set->table_size;
		vt_index_tmp = set->table[ht_index];
	}

	set->table[ht_index] = vt_index;
	// Keep track of the number of entries in the set
	++set->entries;
	// Added successfully
	return TRUE;
}

BOOL pgset_remove(Set *set, void *data)
{
	int vt_index; //value table index
	int ht_index; // hashtable index

	// Look up the data by its hash key
	ht_index = set_hash_fn[set->set_type](data) % set->table_size;

	// get valute table index from hashtable
	vt_index = set->table[ht_index];

	while (vt_index != -1) {
		if (set_equal_fn[set->set_type](data, PGSET_VALUE(set,vt_index)) != 0) {
			// Found the entry
			// Update counter
			--set->entries;
			// Free the entry and return
			set->table[ht_index] = -1;
			return 1;
		}

		// hash collision - next entry
		// also check for table overflow
		ht_index  = (ht_index + 1) % set->table_size;
		vt_index = set->table[ht_index];
	}

	// Not found in set
	return FALSE;
}

BOOL pgset_query(Set *set, void* value)
{
	int vt_index;
	int ht_index;

	/* Look up the data by its hash key */
	ht_index = set_hash_fn[set->set_type](value) % set->table_size;

	/* Search value table, until the corresponding entry is found */
	vt_index = set->table[ht_index];

	while (vt_index != -1) {
		if (set_equal_fn[set->set_type](PGSET_VALUE(set,vt_index), value) != 0) {
			/* Found the entry */
			return TRUE;
		}

		// hash collision -- Advance to the next entry in the chain
		// also check for table overflow
		ht_index  = (ht_index + 1) % set->table_size;
		vt_index = set->table[ht_index];
	}

	/* Not found */
	return FALSE;
}

int pgset_num_entries(Set *set)
{
	return set->entries;
}


Set *pgset_union(Set *set1, Set *set2)
{
	SetIterator iterator;
	Set *new_set;
	int vt_index;

	new_set = pgset_new(set1->set_type);

	if (new_set == NULL) {
		return NULL;
	}

	// Add all values from the first set
	set_iterate(set1, &iterator);
	while (set_iter_has_more(&iterator)) {

		// Read the next value
		vt_index = set_iter_next(&iterator);
		if (vt_index!=-1)
		{
			pgset_insert(new_set, PGSET_VALUE(set1,vt_index));
		}

	}

	/* Add all values from the second set */

	set_iterate(set2, &iterator);
	while (set_iter_has_more(&iterator)) {
		/* Read the next value */
		vt_index = set_iter_next(&iterator);
		// insert values
		if (vt_index!=-1)
		{
			pgset_insert(new_set, PGSET_VALUE(set2,vt_index));
		}

	}

	return new_set;
}

Set *pgset_intersection(Set *set1, Set *set2)
{
	Set *new_set;
	SetIterator iterator;
	int vt_index;

	new_set = pgset_new(set1->set_type);

	if (new_set == NULL) {
		return NULL;
	}

	/* Iterate over all values in set 1. */

	set_iterate(set1, &iterator);

	while (set_iter_has_more(&iterator)) {

		/* Get the next value */
		vt_index = set_iter_next(&iterator);

		/* Is this value in set 2 as well?  If so, it should be
		 * in the new set. */

		if (pgset_query(set2, PGSET_VALUE(set1, vt_index)) != 0)
			pgset_insert(new_set, PGSET_VALUE(set1,vt_index));

	}

	return new_set;
}

BOOL pgset_intersects(Set *set1, Set *set2)
{
	SetIterator iterator;
	int vt_index;


	//Iterate over all values in set 1.
	set_iterate(set1, &iterator);
	while (set_iter_has_more(&iterator)) {

		// Get the next value
		vt_index = set_iter_next(&iterator);

		/* Is this value in set 2 as well?  If so, it should be
		 * in the new set. */
		if (pgset_query(set2, PGSET_VALUE(set1, vt_index)) != 0)
			return TRUE;

	}

	return FALSE;
}

BOOL pgset_equals(Set *set1, Set *set2)
{
	SetIterator iterator;
	int vt_index;

	//different types are not equal
	if(set1->set_type != set2->set_type)
		return FALSE;

	// different sizes are not equal
	if(set1->entries != set2->entries)
		return FALSE;

	// all elements in set1 must be in set2
	set_iterate(set1, &iterator);
	while (set_iter_has_more(&iterator)) {

		/* Get the next value */
		vt_index = set_iter_next(&iterator);

		if (!pgset_query(set2, PGSET_VALUE(set1, vt_index)))
			return FALSE;

	}

	return TRUE;
}


Set *pgset_difference(Set *set1, Set *set2)
{
	Set *new_set;
	SetIterator iterator;
	int vt_index;

	new_set = pgset_new(set1->set_type);

	if (new_set == NULL) {
		return NULL;
	}

	// all elements in set1 but not in set2
	set_iterate(set1, &iterator);
	while (set_iter_has_more(&iterator)) {

		/* Get the next value */
		vt_index = set_iter_next(&iterator);

		if (!pgset_query(set2, PGSET_VALUE(set1, vt_index)))
			pgset_insert(new_set, PGSET_VALUE(set1,vt_index));

	}

	return new_set;
}

void set_iterate(Set *set, SetIterator *iter)
{
	iter->set = set;
	iter->ht_position = 0;

}

int set_iter_next(SetIterator *iterator)
{
	Set *set;
	int result;
	int ht_current_pos;

	set = iterator->set;

	// empty set
	if (set->entries==0) {
		return -1;
	}

	/* No more entries? */
	if ((int)iterator->ht_position == -1 || (int)iterator->ht_position>=set->table_size) {
		return -1;
	}

	/* We have the result immediately */
	ht_current_pos = iterator->ht_position;
	while((int)(set->table[ht_current_pos])==-1 && ht_current_pos<set->table_size) ++ht_current_pos;

	result = (int)set->table[ht_current_pos];

	/* Advance next_entry to the next SetEntry in the Set. */
	if (ht_current_pos != -1) {
		int next_pos = ht_current_pos+1;
		while((int)(set->table[next_pos])==-1 && next_pos<=set->table_size) ++next_pos;

		// finished
		if (next_pos>set->table_size)
		{
			iterator->ht_position = -1;
		} else {
			iterator->ht_position = next_pos;
		}
	}

	return result;
}

int set_iter_has_more(SetIterator *iterator)
{
	return iterator->ht_position != -1 && !(iterator->ht_position>=iterator->set->table_size);
}

void pgset_print(Set* set, StringInfo buffer)
{
	char *f1 = (char*)palloc0(10);
	char *f2 = (char*)palloc0(10);

	SetIterator iter;
	set_iterate(set, &iter);
	int value_index = set_iter_next(&iter);
	while(value_index>=0)
	{
		switch (set->set_type)
		{
			case PgSetType::DOUBLE:
				if (set_iter_has_more(&iter))
				{
					appendStringInfo(buffer, "%f,", *(double*)PGSET_VALUE(set,value_index));
				} else {
					appendStringInfo(buffer, "%f", *(double*)PGSET_VALUE(set,value_index));
				}
				break;
			default:
				if (set_iter_has_more(&iter))
				{
					appendStringInfo(buffer, "%d,", *(int*)PGSET_VALUE(set,value_index));
				} else {
					appendStringInfo(buffer, "%d", *(int*)PGSET_VALUE(set,value_index));
				}
			break;
		}

		value_index = set_iter_next(&iter);
	}
}
