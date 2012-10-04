#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mergesort.h"
#include "tools.h"

// set buffer on new values
// usage: setbuffer(&c, b.data +3, b.size);
void setbuffer(struct buffer *b, int * data, int size)
{
  //printf(".");
  b->data = data;
  b->size = size;
}

void mergesort(struct buffer *b, struct buffer *tmp)
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
    {

      // set range for new partitions
      setbuffer(&leftb,   b->data,   b->size/2);
      setbuffer(&ltmpb, tmp->data, tmp->size/2);
      setbuffer(&rightb,  b->data +   b->size/2,   (b->size+1)/2);
      setbuffer(&rtmpb, tmp->data + tmp->size/2, (tmp->size+1)/2);

      /* recursion */
      if (leftb.size > 1)  mergesort(&leftb,  &ltmpb);

      if (rightb.size > 1) mergesort(&rightb, &rtmpb);

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
      while (l <= rightb.data)
      	{
      	  *t++ = *l++;   //same as: *t = *l; l++; t++
      	}
      //add remaining right tail
      while (r <= last)
      	{
      	  *t++ = *r++;   //same as: *t = *r; l++; r++
      	}

      /* copy tmp buffer back to original buffer */
      memcpy(b->data, tmp->data, tmp->size * sizeof(int));
   }
}

int main(int argc, char *argv[])
{
  struct buffer b;
  struct buffer tmpbuf;

  int size = 23;

  if ( allocbuf(&b, size) && allocbuf(&tmpbuf, size) )
    {
      for (int i=0; i<size; i++)
	b.data[i]=size-i;

      mergesort(&b, &tmpbuf);

      //      for (int i=0; i<size; i++)
      //printf("%d ", b.data[i]);

      prtbhead(&b, b.size);

      printf("\n");

      freebuf(&tmpbuf);
      freebuf(&b);
    }
  else
    {
      printf("Fehler beim Allozieren der Puffer\n");
    }
}
