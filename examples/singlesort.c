
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "tools.h"


// zeit in ms
uint64_t timediff(struct timespec* start, struct timespec* end)
{
  return (1000000000*(end->tv_sec - start->tv_sec) + end->tv_nsec - start->tv_nsec)/1000000;
}



int main(int argc, char *argv[])
{
  struct buffer b;
  struct buffer tmpbuf;

  struct timespec t_0, t_1, t_2;

  int size = 8053;//06368;

  clock_gettime(CLOCK_MONOTONIC, &t_0);

  if ( allocbuf(&b, size) )//&& allocbuf(&tmpbuf, size) )
    {


      randbuf(&b);

      clock_gettime(CLOCK_MONOTONIC, &t_1);

//      mergesort(&b, &tmpbuf, 0);
      quicksort(&b);


      freebuf(&tmpbuf);
      freebuf(&b);

      clock_gettime(CLOCK_MONOTONIC, &t_2);

      printf ("Gesamtzeit: %llu \tRandomzeit: %llu\t%llu \tSortierzeit: %llu\n",
          timediff(&t_0, &t_2), timediff(&t_0, &t_1), timediff(&t_1, &t_2));
    }
  else
    {
      printf("Fehler beim Allozieren der Puffer\n");
    }
}
