#!/bin/bash


BIN=./bin/mpi-ms
LOGFILE=./log

# num of nodes
NODES=2

HOSTS=1

# init sizes
VOLUME=10000000
CHUNK=11

while [ ${CHUNK} -lt ${VOLUME} ]
do
    echo "Berechne: $CHUNK $VOLUME"

    # logging
    LOGSTRING=`hostname | tr -d "\n" ; echo -n " - "; date | tr -d "\n"`

    mpirun -n $NODES $BIN -c $CHUNK -v $VOLUME >>"data/$HOSTS-$NODES-$VOLUME-$CHUNK.log"


    # logging
    echo "$LOGSTRING: $HOSTS-$NODES-$VOLUME-$CHUNK">>$LOGFILE

    CHUNK=$((CHUNK*10))
done
