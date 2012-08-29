
#include <stdlib.h>
#include <stdio.h>


int i[3] = {1, 2, 3};
int *p;

int main()
{
  p=i;
  p++;
  printf("%d \n", *p);


}
