// pgset.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include "pgset.h"
#include "set.h"


extern "C"{
PG_MODULE_MAGIC;
}


extern "C" {
PG_FUNCTION_INFO_V1(set_in);


Datum set_in(PG_FUNCTION_ARGS)
{
	char	   *str = PG_GETARG_CSTRING(0);

	Set *set = pgset_new(PgSetType::INTEGER);
	char* pos = str;

	do
	{
		int i = atoi(pos);
		if (errno == ERANGE)
		{
			ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("invalid input syntax for integer set: \"%s\"",
					str)));
		}

		if(!pgset_insert(set, &i))
			ereport(NOTICE,(errcode(ERRCODE_DATA_EXCEPTION), errmsg("Value %d present in set.", i)));

		pos = strchr(pos, ',');
		if(pos!=NULL) pos++;
	} while (pos!=NULL);

	int size = 0;
	pgset_toast(&set, &size);
	SET_VARSIZE(set, size);

	PG_RETURN_POINTER(set);
}

PG_FUNCTION_INFO_V1(set_out);

Datum set_out(PG_FUNCTION_ARGS)
{

	Set *set = (Set*)DatumGetPointer(PG_DETOAST_DATUM(PG_GETARG_DATUM(0)));

	if(PGSET_CHANGED_ADDRESS(set))
		pgset_adjust_address(set);


	StringInfoData buffer;
	initStringInfo(&buffer);


	pgset_print(set, &buffer);

	PG_RETURN_CSTRING(buffer.data);

}

PG_FUNCTION_INFO_V1(set_add_int);

Datum set_add_int(PG_FUNCTION_ARGS)
{
	if(PG_NARGS()==0)
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("missing arguments")));
	}
	if(PG_ARGISNULL(0))
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("NULL set")));
	}


	Set *set = DETOAST_SET(set,0);


	int data = PG_GETARG_INT32(1);

	if(!pgset_insert(set, &data))
	{
		ereport(NOTICE,(errcode(ERRCODE_DATA_EXCEPTION), errmsg("Value %d present in set.", data)));
	}

	int size=0;
	pgset_toast(&set, &size);
	SET_VARSIZE(set, size);
	PG_RETURN_POINTER(set);
}

PG_FUNCTION_INFO_V1(set_count);

Datum set_count(PG_FUNCTION_ARGS)
{
	if(PG_NARGS()==0)
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("missing arguments")));
	}
	if(PG_ARGISNULL(0))
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("NULL set")));
	}

	Set *set = DETOAST_SET(set,0);

	return pgset_num_entries(set);
}

PG_FUNCTION_INFO_V1(set_contains);

Datum set_contains(PG_FUNCTION_ARGS)
{
	if(PG_NARGS()==0)
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("missing arguments")));
	}
	if(PG_ARGISNULL(0))
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("NULL set")));
	}

	Set *set = DETOAST_SET(set,0);

	int data = PG_GETARG_INT32(1);
	return pgset_query(set, &data);
}


PGDLLEXPORT Datum set_remove(PG_FUNCTION_ARGS);


PG_FUNCTION_INFO_V1(set_remove);

Datum set_remove(PG_FUNCTION_ARGS)
{
	if(PG_NARGS()==0)
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("missing arguments")));
	}
	if(PG_ARGISNULL(0))
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("NULL set")));
	}

	Set *set = DETOAST_SET(set,0);
	int data = PG_GETARG_INT32(1);

	if(!pgset_remove(set,&data))
	{
		ereport(NOTICE,(errcode(ERRCODE_DATA_EXCEPTION), errmsg("Value %d not present in set.", data)));
	}

	PG_RETURN_POINTER(set);
}


PG_FUNCTION_INFO_V1(set_union);

Datum set_union(PG_FUNCTION_ARGS)
{
	if(PG_NARGS()==0)
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("missing arguments")));
	}
	if(PG_ARGISNULL(0))
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("NULL set")));
	}

	Set *set1 = DETOAST_SET(set1,0);
	Set *set2 = DETOAST_SET(set2,1);
	Set *set3 = pgset_union(set1, set2);

	int size=0;
	pgset_toast(&set3, &size);
	SET_VARSIZE(set3, size);
	PG_RETURN_POINTER(set3);
}




PG_FUNCTION_INFO_V1(set_intersect);

Datum set_intersect(PG_FUNCTION_ARGS)
{
	if(PG_NARGS()==0)
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("missing arguments")));
	}
	if(PG_ARGISNULL(0))
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("NULL set")));
	}

	Set *set1 = DETOAST_SET(set1,0);
	Set *set2 = DETOAST_SET(set2,1);
	Set *set3 = pgset_intersection(set1, set2);

	int size=0;
	pgset_toast(&set3, &size);
	SET_VARSIZE(set3, size);
	PG_RETURN_POINTER(set3);
}

PG_FUNCTION_INFO_V1(set_intersects);
Datum set_intersects(PG_FUNCTION_ARGS)
{
	if(PG_NARGS()==0)
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("missing arguments")));
	}
	if(PG_ARGISNULL(0))
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("NULL set")));
	}

	Set *set1 = DETOAST_SET(set1,0);
	Set *set2 = DETOAST_SET(set2,1);

	// check intersection
	PG_RETURN_BOOL(pgset_intersects(set2, set1));
}

PG_FUNCTION_INFO_V1(set_equals);
Datum set_equals(PG_FUNCTION_ARGS)
{
	if(PG_NARGS()==0)
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("missing arguments")));
	}
	if(PG_ARGISNULL(0) || PG_ARGISNULL(1))
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("NULL set")));
	}

	Set *set1 = DETOAST_SET(set1,0);
	Set *set2 = DETOAST_SET(set2,1);

	PG_RETURN_BOOL(pgset_equals(set1, set2));
}

PG_FUNCTION_INFO_V1(set_difference);
Datum set_difference(PG_FUNCTION_ARGS)
{
	if(PG_NARGS()==0)
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("missing arguments")));
	}
	if(PG_ARGISNULL(0))
	{
		ereport(ERROR,(errcode(ERRCODE_DATA_EXCEPTION),
				 errmsg("NULL set")));
	}

	Set *set1 = DETOAST_SET(set1,0);
	Set *set2 = DETOAST_SET(set2,1);
	Set *set3 = pgset_difference(set1, set2);

	int size=0;
	pgset_toast(&set3, &size);
	SET_VARSIZE(set3, size);
	PG_RETURN_POINTER(set3);
}



PG_FUNCTION_INFO_V1(set_create);

Datum set_create(PG_FUNCTION_ARGS)
{
	ArrayType	*input_array = PG_GETARG_ARRAYTYPE_P(0);
	Set *set = NULL;
	Datum *datums;
	bool *nulls;
	int elements;

	if (ARR_NDIM(input_array)>1)
		ereport(ERROR,(errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR), errmsg("Only 1-dimensional array supported.")));

	double *dta=NULL;

	switch (ARR_ELEMTYPE(input_array))
	{
		case INT4OID:
			deconstruct_array(input_array, // one-dimensional array
				ARR_ELEMTYPE(input_array), // of integers
				4, // size of integer in bytes
				true, // int4 is pass-by value
				'i', // alignment type is 'i'
				&datums, &nulls, &elements); // result here

			set = pgset_new(PgSetType::INTEGER);
			for(int i=0;i<elements;i++) {
				if(!nulls[i]) {
					int value = DatumGetInt32(datums[i]);
					pgset_insert(set, &value);
				}
			}
		break;

		case FLOAT8OID:
			deconstruct_array(input_array, // one-dimensional array
				ARR_ELEMTYPE(input_array), // of integers
				8, // size of double in bytes
				true, // double is pass-by value
				'i', // alignment type is 'i'
				&datums, &nulls, &elements); // result here
			//ARR_DATA_PTR
			set = pgset_new(PgSetType::DOUBLE);

			dta = (double*)ARR_DATA_PTR(input_array);

			for(int i=0;i<elements;i++) {
				if(!nulls[i]) {
					pgset_insert(set, &dta[i]);
				}
			}
		break;

		default:
			ereport(ERROR,(errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR), errmsg("This array type is not supported.")));
		break;
	}


	int size=0;
	pgset_toast(&set, &size);
	SET_VARSIZE(set, size);
	PG_RETURN_POINTER(set);
}

}
