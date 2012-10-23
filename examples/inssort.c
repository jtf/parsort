#include <stdio.h>
#include <stdlib.h>

#include "tools.h"

void inssort(struct buffer *b)
{
  int * last;
  int * i;
  int * j;

  int item;

  last = b->data + b->size-1;
  i = b->data++; // mit dem Zweiten beginnen

  while(i <= last++){
    item = *i;

    j = i;
    //solange j nicht am Anfang und item kleiner als Vorgänger
    while ((j > b->data) && (item < *(i--)))
      {
	*j = *(j--)	
      }
    i++;
  }
}

int main(int argc, char *argv[])
{
  struct buffer b;
  struct buffer tmpbuf;

  int size = 23;

  if ( allocbuf(&b, size) && allocbuf(&tmpbuf, size) )
    {
      for (int i=0; i<size; i++)
	b.data[i]=size-i;

      inssort(&b);

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
