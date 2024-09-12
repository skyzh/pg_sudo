#!/bin/bash

make -j $(getconf _NPROCESSORS_ONLN) install PG_CONFIG=$HOME/Work/neon/pg_install/v16/bin/pg_config
