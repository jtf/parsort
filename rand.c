#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

int main()
{
  int randdev;
  int rand;
  int *buffer;

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

  int i = size;

  buffer = (int*) malloc(i*sizeof(int)); //1024*1024*1024);
  // Schlägt Allokieren fehl?
  if( buffer != NULL )
    {

      // random-device öffnen
      randdev  = open("/dev/urandom", O_RDONLY);

      while( i-- )
	{
	  read(randdev, &rand, sizeof(rand));
	  buffer[i] = rand;
	  	  printf("%d ", rand);
	}

      // aufräumen
      close(randdev);
      free(buffer);
      return EXIT_SUCCESS;
    }
  return EXIT_FAILURE;
}

