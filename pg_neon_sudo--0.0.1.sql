\echo Use "CREATE EXTENSION pg_neon_sudo" to load this file. \quit

CREATE SCHEMA IF NOT EXISTS sudo;
CREATE FUNCTION sudo.anon_start_dynamic_masking() RETURNS text
AS '$libdir/pg_neon_sudo'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION sudo.anon_stop_dynamic_masking() RETURNS text
AS '$libdir/pg_neon_sudo'
LANGUAGE C IMMUTABLE STRICT;
