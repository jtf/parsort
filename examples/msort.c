#include <stdio.h>
#include <stdlib.h>

#include "mergesort.h"
#include "tools.h"


int main(int argc, char *argv[])
{
  struct buffer b;
  struct buffer tmpbuf;

  int size = 23;

  if ( allocbuf(&b, size) && allocbuf(&tmpbuf, size) )
    {
      for (int i=0; i<size; i++)
	b.data[i]=size-i;

      mergesort(&b, &tmpbuf);

      prtbhead(&b, b.size);

      printf("\n");

      freebuf(&tmpbuf);
      freebuf(&b);
    }
  else
    {
      printf("Fehler beim Allozieren der Puffer\n");
    }
}
