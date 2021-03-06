// Allgemeine Header
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// fürs Random-Device
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// eigene Definitionen
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
  printf("Head (%d): ", num);
  int j;
  for (j=0; (j< b->size)&&(j<num); j++)
        printf("%d ", b->data[j]);
}

/* Gib die letzten num Elemente aus */
void prtbtail(struct buffer *b, int num)
{
  printf("Tail: (%d): ", num);
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

/* ------------------------------------
 *  Mergesort (lokal)
 * ------------------------------------ */
// set given buffer on new values
// usage: set_buffer(&c, b.data +3, b.size);
void set_buffer(struct buffer *b, int * data, int size)
{
  //printf(".");
  b->data = data;
  b->size = size;
}

void mergesort(struct buffer *b, struct buffer *tmp, int csize)
{
  // buffers for each partition
  struct buffer leftb;
  struct buffer rightb;

  struct buffer ltmpb;
  struct buffer rtmpb;

  // temp int for swap
  int tempi;

  // working pointer for left and right partition, temp buffer and last of b
  int * l;
  int * r;
  int * t;
  int * last;


  /* buffer size check - termination check*/
  // sort buffer elements
  if (b->size == 2)
    {
      // swap if first element is greater
      if (b->data[0] > b->data[1])
	{
	  tempi = b->data[0];
	  b->data[0] = b->data[1];
	  b->data[1] = tempi;
	}
      return;
    }
  else
  // if buffer smaller than cache size do insertion sort
  if (b->size <= csize)
    {
        inssort(b);
    }
  else
    {

      // set range for new partitions
      set_buffer(&leftb,   b->data,   b->size/2);
      set_buffer(&ltmpb, tmp->data, tmp->size/2);
      set_buffer(&rightb,  b->data +   b->size/2,   (b->size+1)/2);
      set_buffer(&rtmpb, tmp->data + tmp->size/2, (tmp->size+1)/2);

      /* recursion */
      if (leftb.size > 1)  mergesort(&leftb,  &ltmpb, csize);

      if (rightb.size > 1) mergesort(&rightb, &rtmpb, csize);

      /* merge - merge it by pointer only merge : ) */
      // left, right and temp working pointer
      l = leftb.data;
      r = rightb.data;
      t = tmp->data;
      last = b->data + b->size-1;

      // inc over all l and r pointer by smallest element
      while ( l <= rightb.data && r <= last)
	{
	  if ( *l <= *r )
	    {
	      *t = *l++; //same as: *t = *l; l++;
	    } else {
	    *t = *r++;   //same as: *t = *r; r++;
	  }
	  t++;
	}
      // add remaining left tail
      while (l < rightb.data)
	{
	  *t++ = *l++;   //same as: *t = *l; l++; t++
	}
      //add remaining right tail
      while (r < last)
	{
	  *t++ = *r++;   //same as: *t = *r; l++; r++
	}

      /* copy tmp buffer back to original buffer */
      memcpy(b->data, tmp->data, tmp->size * sizeof(int));
   }
}

/* ------------------------------------
 *  insertion sort (local)
 * ------------------------------------ */

void inssort(struct buffer *b)
{
  int * i;
  int * j;
  int item;

  // mit dem Zweiten beginnen
  i = b->data + 1;
  // bis zum Ende
  while(i < b->data + b->size)
  {
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

/* ------------------------------------
 *  merge two runs in a third
 * ------------------------------------ */
void merge(struct buffer * b1, struct buffer * b2, struct buffer * ret)
{
  int * l = b1->data;
  int * r = b2->data;

  int * last1 = b1->data + b1->size - 1;
  int * last2 = b2->data + b2->size - 1;

  //  struct buffer * retbuf;
  int * t;

  //  allocbuf(retbuf, b1->size + b2->size);

  t = ret->data; 

  // inc over all l and r pointer by smallest element
  while ( l <= last1 && r <= last2)
    {
      if ( *l <= *r )
	{
	  *t = *l++; //same as: *t = *l; l++;
	} else {
	*t = *r++;   //same as: *t = *r; r++;
      }
      t++;
    }
  // add remaining left tail
  while (l <= last1)
    {
      *t++ = *l++;   //same as: *t = *l; l++; t++
    }
  //add remaining right tail
  while (r <= last2)
    {
      *t++ = *r++;   //same as: *t = *r; l++; r++
    }
}
