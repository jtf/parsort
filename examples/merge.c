
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"


int main(int argc, char *argv[])
{

  struct buffer a,b,c;

  int size = 5;

  if (allocbuf(&a, 2*size) && allocbuf(&b,1* size) && allocbuf(&c, 3*size))
    {
      printf("yay alloziert\n");

      for (int i=0; i<2*size; i++)
	{
	  a.data[i]=2*i;
	  //b.data[i]=i;
	}
      
      merge(&a, &b, &c);
      
      for (int i=0; i<size*2; i++)
	printf("%d ", c.data[i]);
      
      freebuf(&a);
      freebuf(&b);
      freebuf(&c);
    }
  else
    {
      printf("Fehler beim Allozieren des Puffers\n");
    }
}
