#!/bin/bash


BIN=./bin/mpi-ms

# num of nodes
NODES=2

HOSTS=1

# init sizes
VOLUME=1000000000
CHUNK=1

while [ ${CHUNK} -lt ${VOLUME} ]
do
    echo "Berechne: $CHUNK $VOLUME"
    mpirun -n $NODES $BIN -c $CHUNK -v $VOLUME >"data/$HOSTS-$NODES-$VOLUME-$CHUNK.log"


    # logging
    date >> log
    echo "$HOSTS-$NODES-$VOLUME-$CHUNK">>log

    CHUNK=$((CHUNK*10))
done
