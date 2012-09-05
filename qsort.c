
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mergesort.h"
#include "tools.h"


int main(int argc, char *argv[])
{

  struct buffer b;
  int size = 100;


  if (allocbuf(&b, size))
    {
      for (int i=0; i<size; i++)
	b.data[i]=100-i*2;
      
      quicksort(&b);

      for (int i=0; i<size; i++)
	printf("%d ", b.data[i]);
      
      freebuf(&b);
    }
  else
    {
      printf("Fehler beim Allozieren des Puffers\n");
    }
}
