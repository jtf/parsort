
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int cmpint(const int *i1, const int *i2)
{
  if (*i1 < *i2) return -1;
  else if (*i1 > *i2) return 1;
  else return 0;
}

void quicksort(int *buffer)
{
  qsort(buffer, 10, sizeof(int), (int(*)(const void*,const void*))cmpint);
}

int main(int argc, char *argv[])
{

  int *buffer;
  buffer = (int*) malloc(10 * sizeof(int));
  for (int i=0; i<10; i++)
    buffer[i]=100-i*2;

  quicksort(buffer);

  
  for (int i=0; i<10; i++)
    printf("%d ", buffer[i]);

    //  printf("%d\n", cmpint(1,2));
}
