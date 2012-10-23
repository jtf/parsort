#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "tools.h"

int main()
{
  int randdev;
  int rand;
  struct buffer b;

  // Anzahl der Zahlen
  //  int size = 1024*1024*1024;
  int size = 102;

  /* 4GB                */
  /* real    26m1.151s  */
  /* user    0m42.967s  */
  /* sys     25m17.659s */

  /* 400MB              */
  /* real    3m35.596s  */
  /* user    0m10.365s  */
  /* sys     3m24.865s  */

  b.size = size;

  b.data = (int*) malloc(b.size*sizeof(int)); //1024*1024*1024);
  // Schlägt Allokieren fehl?
  if( b.data != NULL )
    {

      // random-device öffnen
      randdev  = open("/dev/urandom", O_RDONLY);

      int i = size;
      while( i-- )
	{
	  read(randdev, &rand, sizeof(rand));
	  b.data[i] = rand;
	  printf("%d ", rand);
	}

      // aufräumen
      close(randdev);
      free(b.data);
      return EXIT_SUCCESS;
    }
  return EXIT_FAILURE;
}

