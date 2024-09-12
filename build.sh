#!/bin/bash

make clean PG_CONFIG=$HOME/Work/neon/pg_install/v16/bin/pg_config
make -j $(getconf _NPROCESSORS_ONLN) install PG_CONFIG=$HOME/Work/neon/pg_install/v16/bin/pg_config
make clean PG_CONFIG=$HOME/Work/neon/pg_install/v15/bin/pg_config
make -j $(getconf _NPROCESSORS_ONLN) install PG_CONFIG=$HOME/Work/neon/pg_install/v15/bin/pg_config
make clean PG_CONFIG=$HOME/Work/neon/pg_install/v14/bin/pg_config
make -j $(getconf _NPROCESSORS_ONLN) install PG_CONFIG=$HOME/Work/neon/pg_install/v14/bin/pg_config
