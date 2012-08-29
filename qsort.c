
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mergesort.h"
#include "tools.h"


int main(int argc, char *argv[])
{

  struct buffer b;

  int size = 100;

  allocbuf(&b, size);

  for (int i=0; i<size; i++)
    b.data[i]=100-i*2;

  quicksort(&b);

  
  for (int i=0; i<10; i++)
    printf("%d ", b.data[i]);

    //  printf("%d\n", cmpint(1,2));
}
