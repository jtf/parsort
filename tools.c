// Allgemeine Header
#include <stdlib.h>
#include <stdio.h>

// fürs Random-Device
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// eigene Definitionen
#include "mergesort.h"
#include "tools.h"

/* ------------------------------------
 *  Pufferverwaltung
 * ------------------------------------ */

/* Alloziert einen Puffer mit Puffergröße size
   Fehlschlag: return 0
   Sonst:      return 1
*/
int allocbuf(struct buffer * b, size_t size)
{
  b->data = (int*) malloc(size * sizeof(int));
  if (b->data != NULL)
    {
      b->size = size;
      return 1;
    }
  else
    {
      b->size = 0;
      return 0;
    }
}

/* Gib den Speicher des Puffers frei */
void freebuf(struct buffer * b)
{
  return free(b->data);
}


/* Fülle Puffer mit Zufall aus /dev/urandom */
void randbuf(struct buffer * b)
{
  int randdev = open("/dev/urandom", O_RDONLY);
  int rand;

  int i = b->size;
  while (i--)
    {
      read(randdev, &rand, sizeof(rand));
      b->data[i] = rand;
    }

  close(randdev);
}

/* Gib die ersten num Elemente aus */
void prtbhead(struct buffer *b, int num)
{
  int j;
  for (j=0; (j< b->size)&&(j<num); j++)
        printf("%d ", b->data[j]);
}

/* Gib die letzten num Elemente aus */
void prtbtail(struct buffer *b, int num)
{
  int j;
  if (b->size < num)
    j = b->size;
  else
    j = b->size - num;
  for (j; j< b->size; j++)
        printf("%d ", b->data[j]);
}

/* ------------------------------------
 *  Quicksort
 * ------------------------------------ */

/* Vergleich für Quicksort */
static int cmpint(const int *i1, const int *i2)
{
  if (*i1 < *i2) return -1;
  else if (*i1 > *i2) return 1;
  else return 0;
}

/* Quicksort auf Puffer anwenden */
void quicksort(struct buffer * b)
{
  qsort(b->data, b->size, sizeof(int), (int(*)(const void*,const void*))cmpint);
}



/* int main() */
/* { */
/*   struct buffer b; */
/*   if (allocbuf(&b, 4)) */
/*     { */
/*       printf(" %d \n", b.size); */
/*       randbuf(&b); */
/*       quicksort(&b); */

/*       for (int j=0; j< b.size; j++) */
/* 	printf("%d ", b.data[j]); */

/*       freebuf(&b); */
/*     } */
/*   else */
/*     { */
/*       printf("Fehlschlag"); */
/*     } */
/* } */

