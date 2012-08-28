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
  int i = 100*1024*1024;
  /* Abunai!  = 400MB*/
  /* real    3m35.596s */
  /* user    0m10.365s */
  /* sys     3m24.865s */

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
	  //	  printf("%d ", rand);
	}

      // aufräumen
      close(randdev);
      free(buffer);
      return EXIT_SUCCESS;
    }
  return EXIT_FAILURE;
}
 
