#ifndef HELPER_H
#define HELPER_H

#include <stdlib.h>
#include <argp.h>

//SlaveMessages
#define WORK_TAG    1
#define END_TAG     2
#define DO_RECV_TAG 3
#define DO_SEND_TAG 4

//SlaveStates
#define SLAVE_NOT_READY -2
#define SLAVE_DEAD      -1
#define SLAVE_BUSY       0

/* Used by argparse to communicate with argp. */
struct config
{
  int volumesize;
  int chunksize;
  int cachesize;
  int qsort;
  int msort;
  int isort;
};

// argument passing function
void argparse(int argc, char **argv, struct config *cfg);

// slave status functions
int max_array(int * a, int num);
int min_array(int * a, int num, int maxPos);
int slaveBusy(int * a, int num);
int moreThanTwoSlaves(int * a, int num);
int slavesReady(int * a, int num);
int getReadySlave(int * a, int num);
int waitingForResults(int * a, int num);

void prtslavestate(int * slaveState, int numNodes);

#endif
