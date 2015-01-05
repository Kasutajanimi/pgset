select * from ceste


--DROP FUNCTION complex_in(cstring) CASCADE
CREATE FUNCTION set_in(cstring)
   RETURNS pgset
   AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;


--DROP FUNCTION set_out(pgset) CASCADE
CREATE FUNCTION set_out(pgset)
   RETURNS cstring
   AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;


--DROP FUNCTION set_add_int(pgset,integer)
CREATE FUNCTION set_add_int(pgset, integer)
   RETURNS pgset
   AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;


CREATE FUNCTION set_count(pgset)
   RETURNS int
   AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

--DROP FUNCTION set_contains(pgset)
CREATE FUNCTION set_contains(pgset, integer)
   RETURNS boolean
   AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION set_remove(pgset, integer)
   RETURNS pgset
   AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;


CREATE FUNCTION set_union(pgset, pgset)
   RETURNS pgset
   AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

   CREATE FUNCTION set_intersect(pgset, pgset)
   RETURNS pgset
   AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

   --drop function set_intersects(pgset, pgset)
   CREATE FUNCTION set_intersects(pgset, pgset)
   RETURNS boolean
   AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;


   CREATE FUNCTION set_equals(pgset, pgset)
   RETURNS boolean
   AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

   CREATE FUNCTION set_difference(pgset, pgset)
   RETURNS pgset
   AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION set_create(VARIADIC a int[]) RETURNS pgset
AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION set_create(VARIADIC a float[]) RETURNS pgset
AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
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




-- GIN SUPPORT


CREATE FUNCTION gpgset_in(cstring)
RETURNS gpgset
AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
LANGUAGE C STRICT IMMUTABLE;

CREATE FUNCTION gpgset_out(gpgset)
RETURNS cstring
AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
LANGUAGE C STRICT IMMUTABLE;



   CREATE FUNCTION gin_compare_pgset(gpgset,gpgset)
   RETURNS int
   AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

   --DROP FUNCTION gin_extract_pgset(internal, internal)
   CREATE FUNCTION gin_extract_pgset(internal, internal)
   RETURNS internal
   AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

   CREATE FUNCTION gin_extract_query_pgset(internal, internal, int2, internal, internal)
   RETURNS internal
   AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

   CREATE FUNCTION gin_consistent_pgset(internal, int2, internal, int4, internal, internal)
   returns boolean
   AS 'C:\kasutajanimi\Projects\pgsql\pgset2012\Debug\pgset.dll'
   LANGUAGE C IMMUTABLE STRICT;

--index element
--DROP TYPE gpgset CASCADE
CREATE TYPE gpgset (
   input = gpgset_in,
   output = gpgset_out,
   INTERNALLENGTH = -1
);


--DROP OPERATOR  IF EXISTS @ (pgset, pgset) CASCADE
CREATE OPERATOR @ (
	LEFTARG = pgset,
	RIGHTARG = pgset,
	PROCEDURE = set_intersects,
	RESTRICT = eqsel,
	JOIN = eqjoinsel
);

--DROP OPERATOR  IF EXISTS = (pgset, pgset) CASCADE
CREATE OPERATOR = (
	LEFTARG = pgset,
	RIGHTARG = pgset,
	PROCEDURE = set_equals,
	RESTRICT = eqsel,
	JOIN = eqjoinsel
);

--DROP OPERATOR CLASS gin_set_ops USING gin CASCADE
CREATE OPERATOR CLASS gin_set_ops
DEFAULT FOR TYPE pgset USING gin
AS
	OPERATOR        7       @, --intersects
	OPERATOR        9       =, --equals
	FUNCTION        1       gin_compare_pgset(gpgset,gpgset),
	FUNCTION        2       gin_extract_pgset(internal, internal),
	FUNCTION        3       gin_extract_query_pgset(internal, internal, int2, internal, internal),
	FUNCTION        4       gin_consistent_pgset(internal, int2, internal, int4, internal, internal),
	STORAGE         gpgset;



select set_count('10,5,20,1,4,16,8,2')
select set_count('10,5,20,1,1,4,4,9,16,8,8,8,8,8,10,5,20,1,1,4,9')
select set_contains('10,5,20,1,4,16,8,2',14)
select set_difference(set_remove('10,5,20,1,4,16,8,2',-16),'10,20')
select set_union('10,5,2,12','1,4,16,8,12')
select set_count(set_union('10,5,2,12','1,4,16,8,12'))
select set_contains(set_union('10,5,2,12','1,4,16,8,12'),222222)
select set_difference('10,5,2,12','1,4,16,8,12,2,5')
select set_union('10,5,2,12','1,4,16,8,12,2,5')
select set_intersect('10,5,2,12,4','1,4,16,8,12')

SELECT set_intersect(set_create(1.11,1.1,2.0,3.1,0.5,5.5),set_create(2.0,3.1))

SELECT set_create(1,2,3) = set_create(1,2,3)
SELECT set_create(1,2,3) @ set_create(1,2,3,4)

select set_add_int(p,1) FROM (
select '10,5,20,1,1,4,4,9,16,8,2'::pgset as p
) Q

select set_add_int('5,4,3,2,1,6'::pgset,6)

--DROP table test_set
CREATE TABLE test_set (
	a	pgset
);

DELETE FROM test_set
select * from test_Set
INSERT INTO test_set VALUES ('11,6,120,21,2')

INSERT INTO test_set VALUES ('111,12,32,321,312')

--drop index tdx_set
CREATE INDEX tdx_set ON test_set USING gin(a);


set enable_seqscan=off;
explain
select a from test_set where a @ (SELECT a from test_set where a @ (select set_create(11)))
select a from test_set where a @ (SELECT set_create(80877))
select a from test_set where a @ set_create(80877)
select a from test_set where a @ set_create(80877,27625,88240,98147,49956,76401);
select a from test_set where a @ (SELECT '70708, 80877,27625,88240,98147,49956,76401'::pgset );
select a from test_set where a @ '80877,27625,88240,98147,49956,76401'::pgset

select '80877,27625,88240,98147,49956,76401'::pgset @ '80877,11,38806,89228,8590,94538'::pgset
explain
select t2.* from test_set t1
inner join test_set t2 ON t2.a @ t1.a AND t1.a @ (select  set_create(11))


select * from test_set WHERE a@'80877,11,38806,89228,8590,94538'

select * from test_set where a @ set_create(94538)

UPDATE test_set
SET a = set_add_int(a,22)

select count(*) from test_set
select a from test_set


SELECT * FROM pg_stat_activity
