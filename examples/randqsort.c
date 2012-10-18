#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

static int cmpint(const int *i1, const int *i2)
{
  if (*i1 < *i2) return -1;
  else if (*i1 > *i2) return 1;
  else return 0;
}

void quicksort(int *buffer, int size)
{
  qsort(buffer, size, sizeof(int), (int(*)(const void*,const void*))cmpint);
}


int main()
{
  int randdev;
  int rand;
  int *buffer;

  // Anzahl der Zahlen
    int size = 1024*1024;
  //  int size = 1024;

  int i = size;

  buffer = (int*) malloc(i*sizeof(int)); //1024*1024*1024);
  // Schlägt Allokation fehl?
  if( buffer != NULL )
    {

      // random-device öffnen
      randdev  = open("/dev/urandom", O_RDONLY);

      while( i-- )
	{
	  read(randdev, &rand, sizeof(rand));
	  buffer[i] = rand;
	  //	  	  printf("%d ", rand);
	}

      // aufräumen
      close(randdev);
      
      // sortieren und ausgeben
      quicksort(buffer, size);
      // for (i=0; i<size; i++)
      // printf("%d\n", buffer[i]);

      free(buffer);
      return EXIT_SUCCESS;
    }
  return EXIT_FAILURE;
}

