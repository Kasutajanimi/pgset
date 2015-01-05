#include "set.h"

extern "C" {

PGDLLEXPORT Datum gpgset_in(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum gpgset_out(PG_FUNCTION_ARGS);

PGDLLEXPORT Datum gin_compare_pgset(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum gin_extract_pgset(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum gin_extract_query_pgset(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum gin_consistent_pgset(PG_FUNCTION_ARGS);
}


// must map values for operator class
#define PgSetExistsAnyStrategyNumber		7
#define PgSetExistsAllStrategyNumber		9



typedef struct
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	PgSetType	type;
	char		data[1];
} GinPgSetElement;