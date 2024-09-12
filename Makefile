MODULES = pg_neon_sudo
EXTENSION = pg_neon_sudo
DATA = pg_neon_sudo--0.0.1.sql
OBJS = pg_neon_sudo.o

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
