parsort
=======

 * master_slave.c - main programm

 * tools.c - tools library

 * helper.c - helper functions library

compile with:

    mpicc -lrt master_slave.c tools.c helper.c

start with:

    mpirun -n NUM -host ... master_slave -c CHUNKSIZE -v VOLUMESIZE [-C CACHESIZE]
    
      -c, --chunk=CHUNKSIZE      chunk size > 3
      -v, --volume=VOLUMESIZE    volume size (k * chunk size, k>4)
      -C, --cache=CACHESIZE      L3 cache size for insertion sort [default 10000]
      -m, --msort                use local merge sort
      -q, --qsort                use local quick sort
    
      -d, --debug                show debug messages
      -p, --prtopt               show given command line parameters
    
      -?, --help                 Give this help list
          --usage                Give a short usage message
      -V, --version              Print program version
