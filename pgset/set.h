#include "stdafx.h"

#ifndef SET_H
#define SET_H

#ifdef ENV64
#define PGSET_SET_ADDRESS(s) s->address = (long long)s
#define PGSET_CHANGED_ADDRESS(s) s->address != (long long)s
#else
#define PGSET_SET_ADDRESS(s) s->address = (int)s
#define PGSET_CHANGED_ADDRESS(s) s->address != (int)s
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define DETOAST_SET(s,arg) (Set*)DatumGetPointer(PG_DETOAST_DATUM(PG_GETARG_DATUM(arg))); if(PGSET_CHANGED_ADDRESS(s)) pgset_adjust_address(s);

typedef void *SetValue;
typedef int SetEntry;

/*
struct _SetEntry {
	SetValue data;
	SetEntry *next;
};
*/

typedef unsigned long (*SetHashFunc)(void* value);
typedef int (*SetEqualFunc)(void* value1, void* value2);
typedef void (*SetFreeFunc)(SetValue value);
typedef void (*SetAssignFunc)(void *value);


//int specific functions
unsigned long set_int_hash(void* value);
int set_int_equal(void* value1, void* value2);


//typedef struct _Set Set;

/**
 * An object used to iterate over a set.
 *
 * @see set_iterate
 */

typedef struct _SetIterator SetIterator;


enum PgSetType {
	INTEGER = 0,
	DOUBLE = 1,
	STRING = 2
};

/* element sizes */
static int set_element_sizes[] = {sizeof(int),sizeof(double),sizeof(char*)};




struct _Set {
	int32		vl_len_; /* varlena header (do not touch directly!) */
	PgSetType	set_type;
	/* VALUE TABLE */
	int			alloced_mem_size;
	int			alloced_size;
	int			num_values;
	/* SET */
	int			entries;
	int			table_size;
	int			prime_index;

	#ifdef ENV64
	long long	value_table_offset;
	long long	table_offset;
	long long	address;
	#else
	int			value_table_offset;
	int			table_offset;
	int			address;
	#endif

	void*		value_table;
	int			*table;
};


typedef struct _Set Set;

/**
 * Definition of a @ref SetIterator.
 */

struct _SetIterator {
	Set *set;
	int ht_position;
};

/**
 * A null @ref SetValue.
 */

#define SET_NULL ((void *) 0)
#define PGSET_VALUE(t,p) (void*)((char*)(t->value_table)+ p * set_element_sizes[t->set_type])


Set* pgset_new(PgSetType type);
BOOL pgset_insert(Set* set, void *data);
void *pgset_toast(Set** set, int *byte_size);


Set *set_new(SetHashFunc hash_func, SetEqualFunc equal_func);
void set_free(Set *set);
void set_register_free_function(Set *set, SetFreeFunc free_func);

BOOL pgset_insert_hashtable(Set *set,int vt_index, void* data);

BOOL pgset_remove(Set *set, void *data);
BOOL pgset_query(Set *set, void* value);
int pgset_num_entries(Set *set);
Set *pgset_union(Set *set1, Set *set2);
Set *pgset_intersection(Set *set1, Set *set2);
BOOL pgset_intersects(Set *set1, Set *set2);
BOOL pgset_equals(Set *set1, Set *set2);
Set *pgset_difference(Set *set1, Set *set2);

void set_iterate(Set *set, SetIterator *iter);
int set_iter_has_more(SetIterator *iterator);
int set_iter_next(SetIterator *iterator);
void pgset_adjust_address(Set* set);

void pgset_print(Set* set, StringInfo buffer);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef SET_H */
