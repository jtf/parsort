#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "helper.h"


void prtall(int * sState, int numNodes)
{
  int maxN = max_array(sState, numNodes);

  prtslavestate(sState, numNodes);
  printf("max_array: %d\t", max_array(sState, numNodes));
  printf("min_array: %d\t", min_array(sState, numNodes, maxN));
  printf("slaveBusy: %d\t", slaveBusy(sState, numNodes));
  printf("moreThan2slaves: %d\t", moreThanTwoSlaves(sState, numNodes));
  printf("slavesReady: %d\t", slavesReady(sState, numNodes));
  printf("getReadySlave: %d\t", getReadySlave(sState, numNodes));
  printf("\n");
}

int main ()
{

  int numNodes = 8;
  int *sState;
  int i;

  sState = malloc(sizeof(int) * numNodes);


  for(i=0; i<numNodes; i++) sState[i]=-2;

  prtall(sState, numNodes);

  for(i=1; i<numNodes; i++)
    {
      printf("\tnach 0 <%d>\n", i);
      sState[i] = 0;
      prtall(sState, numNodes);
    }
  for(i=1; i<numNodes; i++)
    {
      printf("\tnach -1 <%d>\n", i);
      sState[i] = -1;
      prtall(sState, numNodes);
    }

  printf("\n\tZwei Werte =\n");
  sState[3]= 240;
  sState[4]= 240;
  prtall(sState, numNodes);

  printf("\n\tZwei Werte >\n");
  sState[3]= 230;
  sState[4]= 240;
  prtall(sState, numNodes);

  printf("\n\tZwei Werte <\n");
  sState[3]= 240;
  sState[4]= 230;
  prtall(sState, numNodes);

  for(i=1; i<numNodes; i++)
    {
      printf("\tnach 0 <%d>\n", i);
      sState[i] = 0;
      prtall(sState, numNodes);
    }

}
