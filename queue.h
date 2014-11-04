#ifndef QUEUE_H
#define QUEUE_H

#define QUEUE_BUFFER_UNIT_SIZE 5

#include "define.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef struct _Queue
{
    void** _data;
    unsigned int _item_size;
    unsigned int _size;
    unsigned int _max_size;
    int _front_index;
    int _tail_index;
}Queue;

void init_queue(Queue* self, unsigned int size_per_item);

void enqueue(Queue* self, void* ptr_data);
void* dequeue(Queue* self);

bool queue_empty(Queue* self);
void clear(Queue* self);
unsigned int queue_size(Queue* self);

void release_queue(Queue* self);

#ifdef __cplusplus
}
#endif

#endif
