extern "C" {

PGDLLEXPORT Datum set_in(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum set_out(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum set_add_int(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum set_count(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum set_contains(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum set_remove(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum set_union(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum set_intersect(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum set_intersects(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum set_difference(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum set_equals(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum set_create(PG_FUNCTION_ARGS);

}