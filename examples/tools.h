#ifndef TOOLS_H
#define TOOLS_H

int allocbuf(struct buffer * b, size_t size);
void freebuf(struct buffer * b);
void randbuf(struct buffer * b);

void prtbhead(struct buffer * b, int num);
void prtbtail(struct buffer * b, int num);

void quicksort(struct buffer * b);
void mergesort(struct buffer * b, struct buffer * tmp);

#endif
