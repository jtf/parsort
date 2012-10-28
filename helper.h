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

void argparse(int argc, char **argv, struct config *cfg);

#endif
