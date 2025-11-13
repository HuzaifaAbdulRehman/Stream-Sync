#ifndef PRODUCER_H
#define PRODUCER_H

#define BUFFER_SIZE 10

typedef struct {
    char* data;
    int read_count;
} BufferSlot;

void* producer();

#endif