
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"


int main(int argc, char *argv[])
{

  struct buffer a,b,c;


  if (allocbuf(&a, 7) && allocbuf(&b, 3) && allocbuf(&c, a.size + b.size))
    {
      printf("yay alloziert\n");

      for (int i=0; i<a.size; i++)
	{
	  a.data[i]=i-3;
	}

      for (int i=0; i<b.size; i++)
	{
	  b.data[i]=i-9;
	}

      merge(&a, &b, &c);
      
      for (int i=0; i< a.size + b.size; i++)
	printf("%d ", c.data[i]);

      printf("\nGrößen: a: %d b: %d c: %d\n", a.size, b.size, c.size);

      freebuf(&a);
      freebuf(&b);
      freebuf(&c);
    }
  else
    {
      printf("Fehler beim Allozieren des Puffers\n");
    }
}
