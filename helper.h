#ifndef HELPER_H
#define HELPER_H

#include <stdlib.h>
#include <argp.h>

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
int slaveBusy(int * a, int num);
int moreThanTwoSlaves(int * a, int num);
int max_array(int * a, int num);
int min_array(int * a, int num, int maxPos);
int slavesReady(int * a, int num);
int getReadySlave(int * a, int num);

#endif
