#!/bin/bash
DIRECTORY=obj
if [ ! -d "$DIRECTORY" ]; then
    mkdir $DIRECTORY
fi

#!/bin/bash
S_DIRECTORY=../shared/obj
if [ ! -d "$S_DIRECTORY" ]; then
    mkdir $S_DIRECTORY
fi

LOG_DIRECTORY=cfg
if [ ! -d "$LOG_DIRECTORY" ]; then
    mkdir $LOG_DIRECTORY
fi

FILE=example.out
sudo make test
if test -f "./$FILE"; then
   LD_LIBRARY_PATH="obj/" ./$FILE "/home/utnso/tp-2021-2c-3era-es-la-vencida/mate-lib/cfg/mate-lib.conf"
fi