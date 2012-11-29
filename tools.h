#ifndef TOOLS_H
#define TOOLS_H

//void funktion();

struct buffer {
  int * data;
  int size;
};

int allocbuf(struct buffer * b, size_t size);
void freebuf(struct buffer * b);
void randbuf(struct buffer * b);
void set_buffer(struct buffer *b, int * data, int size);

void prtbhead(struct buffer * b, int num);
void prtbtail(struct buffer * b, int num);

void quicksort(struct buffer * b);
void mergesort(struct buffer * b, struct buffer * tmp, int csize);
void inssort(struct buffer *b);

void merge(struct buffer * b1, struct buffer * b2, struct buffer * ret);

#endif
