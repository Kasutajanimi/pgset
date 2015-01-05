#include "stdafx.h"
#include "pgset_gin.h"
#include "set.h"
#include <stdlib.h>
#include <access/gin.h>
#include <access/skey.h>
//#include <catalog/pg_type.h>


extern "C" {

PG_FUNCTION_INFO_V1(gpgset_in);
Datum gpgset_in(PG_FUNCTION_ARGS)
{
	elog(ERROR, "Not implemented");
	PG_RETURN_DATUM(0);
}

PG_FUNCTION_INFO_V1(gpgset_out);
Datum gpgset_out(PG_FUNCTION_ARGS)
{
	elog(ERROR, "Not implemented");
	PG_RETURN_DATUM(0);
}

// build indexable value
static GinPgSetElement* makeItem(Set *set, int index)
{
	GinPgSetElement* item;
	int size = sizeof(GinPgSetElement) + set_element_sizes[set->set_type];
	item = (GinPgSetElement*)palloc0(size);
	SET_VARSIZE(item, size);

	item->type = set->set_type;

	memcpy(item->data, PGSET_VALUE(set,index), set_element_sizes[set->set_type]);

	return item;
}

PG_FUNCTION_INFO_V1(gin_compare_pgset);
Datum gin_compare_pgset(PG_FUNCTION_ARGS)
{
	GinPgSetElement *e1 = (GinPgSetElement*)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	GinPgSetElement *e2 = (GinPgSetElement*)PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	switch (e1->type)
	{
		case PgSetType::DOUBLE:
			return *(double*)(e1->data)>*(double*)(e2->data)?1:(*(double*)(e1->data)<*(double*)(e2->data)?-1:0);
		break;
		case PgSetType::INTEGER:
			return *(int*)(e1->data)>*(int*)(e2->data)?1:(*(int*)(e1->data)<*(int*)(e2->data)?-1:0);
		break;
	}

	return 0;
}

PG_FUNCTION_INFO_V1(gin_extract_pgset);
PGDLLEXPORT Datum gin_extract_pgset(PG_FUNCTION_ARGS)
{
	Set *set = DETOAST_SET(set,0);
	int32	   *nentries = (int32 *) PG_GETARG_POINTER(1);
	Datum	   *entries = NULL;
	int			count =  pgset_num_entries(set);

	*nentries = count;
	if (count)
		entries = (Datum*) palloc(sizeof(Datum) * count);

	SetIterator iter;
	set_iterate(set, &iter);
	int value_index = set_iter_next(&iter);
	while(value_index>=0)
	{
		entries[value_index] = PointerGetDatum(makeItem(set, value_index));
		value_index = set_iter_next(&iter);
	}

	PG_RETURN_POINTER(entries);
}


//Datum *extractQuery(Datum query, int32 *nkeys, StrategyNumber n, bool **pmatch, Pointer **extra_data, bool **nullFlags, int32 *searchMode)
PG_FUNCTION_INFO_V1(gin_extract_query_pgset);
Datum gin_extract_query_pgset(PG_FUNCTION_ARGS)
{

	int32	   *nentries = (int32 *) PG_GETARG_POINTER(1);
	StrategyNumber strategy = PG_GETARG_UINT16(2);
	int32	   *searchMode = (int32 *) PG_GETARG_POINTER(6);
	Datum	   *entries;

	if (strategy == PgSetExistsAnyStrategyNumber || strategy == PgSetExistsAllStrategyNumber)
	{
		Set *set = DETOAST_SET(set,0);
		// Query is an pgset, so just apply extract...
		entries = (Datum *)
			DatumGetPointer(
				DirectFunctionCall2Coll(gin_extract_pgset, InvalidOid, PG_GETARG_DATUM(0), PointerGetDatum(nentries))
			);
		// except that "contains {}" requires a full index scan
		if (entries == NULL)
			*searchMode = GIN_SEARCH_MODE_ALL;
	}
	/*


	else if (strategy == PgSetExistsStrategyNumber)
	{
		// query is pgset
		text	   *query = PG_GETARG_TEXT_PP(0);
		text	   *item;

		*nentries = 1;
		entries = (Datum *)palloc(sizeof(Datum));
		item = makeitem(set, 0);
		entries[0] = PointerGetDatum(item);
	}*/ /*
	else if (strategy == PgSetExistsAnyStrategyNumber ||
			 strategy == PgSetExistsAllStrategyNumber)
	{
		ArrayType  *query = PG_GETARG_ARRAYTYPE_P(0);
		Datum	   *key_datums;
		bool	   *key_nulls;
		int			key_count;
		int			i,
					j;
		text	   *item;

		deconstruct_array(query,
						  TEXTOID, -1, false, 'i',
						  &key_datums, &key_nulls, &key_count);

		entries = (Datum *) palloc(sizeof(Datum) * key_count);

		for (i = 0, j = 0; i < key_count; ++i)
		{
			// Nulls in the array are ignored, cf hstoreArrayToPairs
			if (key_nulls[i])
				continue;
			item = makeitem(VARDATA(key_datums[i]), VARSIZE(key_datums[i]) - VARHDRSZ, KEYFLAG);
			entries[j++] = PointerGetDatum(item);
		}

		*nentries = j;
		// ExistsAll with no keys should match everything
		if (j == 0 && strategy == PgSetExistsAllStrategyNumber)
			*searchMode = GIN_SEARCH_MODE_ALL;
	}*/
	else
	{
		elog(ERROR, "unrecognized strategy number: %d", strategy);
		entries = NULL;
	}

	PG_RETURN_POINTER(entries);
}



//bool consistent(bool check[], StrategyNumber n, Datum query, int32 nkeys, Pointer extra_data[], bool *recheck, Datum queryKeys[], bool nullFlags[])
PG_FUNCTION_INFO_V1(gin_consistent_pgset);
Datum gin_consistent_pgset(PG_FUNCTION_ARGS)
{
	bool	   *check = (bool *) PG_GETARG_POINTER(0);
	StrategyNumber strategy = PG_GETARG_UINT16(1);

	/* HStore	   *query = PG_GETARG_HS(2); */
	int32		nkeys = PG_GETARG_INT32(3);

	/* Pointer	   *extra_data = (Pointer *) PG_GETARG_POINTER(4); */
	bool	   *recheck = (bool *) PG_GETARG_POINTER(5);
	bool		res = true;
	int32		i;

	if (strategy == PgSetExistsAnyStrategyNumber)
	{
		*recheck = false;

	} else if(strategy==PgSetExistsAllStrategyNumber)
	{
		// if not all the keys are present, we can fail at once.
		*recheck = true;
		for (i = 0; i < nkeys; i++)
		{
			if (!check[i])
			{
				res = false;
				break;
			}
		}
	}
	else
		elog(ERROR, "unrecognized strategy number: %d", strategy);

	PG_RETURN_BOOL(res);
}

}