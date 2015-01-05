pgSet
=====

Postgres plugin for adding 'set' as native data type (even with index support).

### Overview
In mathematical sense, a set is a collection of distinct objects of the same type, e.g. numbers 1, 2, 7 and 5 are distinct objects when considered separately, but when they are considered collectively they form a single set of size four, written {1,2,5,7}. Unlike standard ARRAY type, in a set elements order doesn't play any role. One of the key features of a set data type is guaranteed absense of сoinciding elements, therefore it elimiates time expenses for this check. 

### Requirements
* NT-based versions of Windows operating system (like Windows 7, XP or 2003)
* PostgreSQL 9.x (plugin tested against version 9.3)
* Microsoft Visual Studio 2012 or 2013
* 1024 Mb RAM or more (recommended)

## Usage

List of available functions:

|  №  | Function name          | Description                              | Parameters                         | Output            |
| --- | :---------------------: | :--------------------------------------- | ----------------------------------:| ------------------:|
|  1  | **set_create**         | Create set of Integer                    | *int[]* __els__                    | *pgset* __set__   |
|  2  | **set_create**         | Create set of Float                      | *float[]* __els__                  | *pgset* __set__   |
|  3  | **set_in**             | Internal type cast                       | *cstring*	__set__                | *pgset* __set__   |
|  4  | **set_out**            | Internal type cast                       | *pgset*	__set__                    | *cstring* __set__ |
|  5  | **set_count**          | Count elements in set                    | *pgset*	__set__                    | *int* __cnt__     |
|  6  | **set_add_int**        | Add Integer to set                       | *pgset* __set__, *int* __el__      | *pgset* __set__   |
|  7  | **set_contains**       | Check if set contains element            | *pgset* __set__, *int* __el__      | *bool* __res__    |
|  8  | **set_difference**     | Subtract 2nd set from the 1st            | *pgset* __set1__, *pgset* __set2__ | *pgset* __set__   |
|  9  | **set_remove**         | Remove element from set                  | *pgset* __set__, *int* __el__      | *pgset* __set__   |
|  10 | **set_union**          | Merge two sets                           | *pgset* __set1__, *pgset* __set2__ | *pgset* __set__   |
|  11 | **set_equals (=)**     | Check if two sets are equal              | *pgset* __set1__, *pgset* __set2__ | *bool* __res__    |
|  12 | **set_intersects (@)** | Check if two sets have common element(s) | *pgset* __set1__, *pgset* __set2__ | *bool* __res__    |
|  13 | **set_equals (=)**     | Return union of two sets                 | *pgset* __set1__, *pgset* __set2__ | *pgset* __set__   |

### Installation

1. Compile DLL in Visual Studio
2. Run the following code in psql shell for adding new type and functions:
```sql
--DROP FUNCTION complex_in(cstring) CASCADE
CREATE FUNCTION set_in(cstring)
   RETURNS pgset
   AS 'C:\kasutajanimi\Projects\pgsql\test\x64\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

--DROP FUNCTION set_out(pgset) CASCADE
CREATE FUNCTION set_out(pgset)
   RETURNS cstring
   AS 'C:\kasutajanimi\Projects\pgsql\test\x64\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

--DROP FUNCTION set_add_int(pgset,integer)
CREATE FUNCTION set_add_int(pgset, integer)
   RETURNS pgset
   AS 'C:\kasutajanimi\Projects\pgsql\test\x64\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION set_sort(pgset)
   RETURNS pgset
   AS 'C:\kasutajanimi\Projects\pgsql\test\x64\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION set_count(pgset)
   RETURNS int
   AS 'C:\kasutajanimi\Projects\pgsql\test\x64\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

--DROP FUNCTION set_contains(pgset)
CREATE FUNCTION set_contains(pgset, integer)
   RETURNS boolean
   AS 'C:\kasutajanimi\Projects\pgsql\test\x64\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION set_remove(pgset, integer)
   RETURNS pgset
   AS 'C:\kasutajanimi\Projects\pgsql\test\x64\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION set_union(pgset, pgset)
RETURNS pgset
AS 'C:\kasutajanimi\Projects\pgsql\test\x64\Debug\pgset.dll'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION set_intersect(pgset, pgset)
RETURNS pgset
AS 'C:\kasutajanimi\Projects\pgsql\test\x64\Debug\pgset.dll'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION set_difference(pgset, pgset)
RETURNS pgset
AS 'C:\kasutajanimi\Projects\pgsql\test\x64\Debug\pgset.dll'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION set_create(VARIADIC a int[]) RETURNS pgset
AS 'C:\kasutajanimi\Projects\pgsql\test\x64\Debug\pgset.dll'
LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION set_create(VARIADIC a float[]) RETURNS pgset
AS 'C:\kasutajanimi\Projects\pgsql\test\x64\Debug\pgset.dll'
LANGUAGE C STRICT;

--DROP TYPE  pgset CASCADE

CREATE TYPE pgset (
   input = set_in,
   output = set_out,
   --receive = set_recv,
   --send = set_send,
   --alignment = double,
   INTERNALLENGTH = -1,
   STORAGE = extended
);
```

## Examples
```sql
SELECT set_count('10, 5, 20, 1, 4, 16, 8, 2')
SELECT set_count('10, 5, 20, 1, 1, 4, 4, 9, 16, 8, 8, 8, 8, 8, 10, 5, 20, 1, 1, 4, 9')
SELECT set_contains('10, 5, 20, 1, 4, 16, 8, 2', 14)
SELECT set_difference(set_remove('10, 5, 20, 1, 4, 16, 8, 2', -16), '10, 20')
SELECT set_union('10, 5, 2, 12', '1, 4, 16, 8, 12')
SELECT set_count(set_union('10, 5, 2, 12', '1, 4, 16, 8, 12'))
SELECT set_contains(set_union('10, 5, 2, 12', '1, 4, 16, 8, 12'), 222222)
SELECT set_difference('10, 5, 2, 12', '1, 4, 16, 8, 12, 2, 5')
SELECT set_union('10, 5, 2, 12', '1, 4, 16, 8, 12, 2, 5')
SELECT set_intersect('10, 5, 2, 12, 4', '1, 4, 16, 8, 12')

SELECT set_intersect(set_create(1.11, 1.1, 2.0, 3.1, 0.5, 5.5), set_create(2.0, 3.1))

SELECT set_add_int(p, 1) FROM (
SELECT '10, 5, 20, 1, 1, 4, 4, 9, 16, 8, 2'::pgset as p
) Q

SELECT set_add_int('5, 4, 3, 2, 1, 6'::pgset, 6)

--DROP table test_set
CREATE TABLE test_set (
	a	pgset
);

DELETE FROM test_set

INSERT INTO test_set VALUES ('11, 6, 120, 21, 2')

UPDATE test_set
SET a = set_add_int(a, 22)

SELECT count(*) FROM test_set
SELECT a FROM test_set

SELECT * FROM pg_stat_activity
```

### TO DO
1. Make this plugin cross-platform (build version for Linux at least)
2. Expand the scope to other element types
