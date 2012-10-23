#include <stdio.h>
#include <stdlib.h>

#include "tools.h"

void inssort(struct buffer *b)
{
  int * i;
  int * j;
  int item;

  // mit dem Zweiten beginnen
  i = b->data + 1; 
  // bis zum Ende
  while(i < b->data + b->size){
    item = *i;
    //    printf("%d\t", item);
    j = i;
    //solange j nicht am Anfang und item kleiner als Vorgänger
    while ((j > b->data) && (item < *(i-1)))
      {
	// eins weiter kopieren und eins zurück gehen
    	*j-- = *(j - 1);
      }
    *j = item;
    i++;
  }
}

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
