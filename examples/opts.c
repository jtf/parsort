#include <stdlib.h>
#include "helper.h"

int main(int argc, char **argv)
{
  struct config cfg;
     
  argparse(argc, argv, &cfg);
  
  printf("\nVOLUMESIZE = %d\nCHUNKSIZE = %d\n",
	  cfg.volumesize,
	  cfg.chunksize
	  // cfg.qsort ? "yes" : "no");
	  );
     
  exit(0);
}
