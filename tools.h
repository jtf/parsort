#ifndef TOOLS_H
#define TOOLS_H

int allocbuf(struct buffer * b, size_t size);
void freebuf(struct buffer * b);
void randbuf(struct buffer * b);
void quicksort(struct buffer * b);

#endif
