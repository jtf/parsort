#include <stdio.h>
#include <stdlib.h>

#include "tools.h"

int main(int argc, char *argv[])
{
  struct buffer b;

  int size = 7;

  if ( allocbuf(&b, size) )
    {
      for (int i=0; i<size; i++)
	b.data[i]=9-i;//size-i;

      printf(" davor: ");
      prtbhead(&b, b.size);
      printf("\n");

      inssort(&b);

      printf("\n danach: ");
      prtbhead(&b, b.size);
      printf("\n");

      freebuf(&b);
    }
  else
    {
      printf("Fehler beim Allozieren der Puffer\n");
    }
}
