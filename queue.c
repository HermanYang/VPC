#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "utils.h"


void init_queue(Queue* self, unsigned int size_per_item)
{
    self->_data = NULL;
    self->_item_size = 0;
    self->_size = 0;
    self->_max_size = 0;
    self->_front_index = -1;
    self->_tail_index = -1;

    self->_data = calloc(QUEUE_BUFFER_UNIT_SIZE, sizeof(void*));

	int i;

	for(i = 0; i < QUEUE_BUFFER_UNIT_SIZE; ++i)
	{
        self->_data[i] = malloc(size_per_item);
	}

    self->_size = 0;
    self->_max_size = QUEUE_BUFFER_UNIT_SIZE;
    self->_item_size = size_per_item;
}

void enqueue(Queue* self, void* ptr_data)
{
	// first enqueue
    if(self->_front_index == -1 && self->_tail_index == -1)
	{
        self->_front_index = 0;
	}

    if(self->_size < self->_max_size)
	{
        self->_tail_index = (self->_tail_index + 1) % self->_max_size;
        memcpy(self->_data[self->_tail_index], ptr_data, self->_item_size);
	}
	else
	{
        void** ptr_new_queue_data = calloc(self->_max_size + QUEUE_BUFFER_UNIT_SIZE, sizeof(void*));

		int i;

        for(i = 0; i < self->_max_size; ++i)
		{
            ptr_new_queue_data[i] = dequeue(self);
		}

        self->_max_size += QUEUE_BUFFER_UNIT_SIZE;

        for(; i < self->_max_size; ++i)
		{
            ptr_new_queue_data[i] = malloc(self->_item_size);
		}


        free(self->_data);
        self->_data = ptr_new_queue_data;

        self->_front_index = 0;

        self->_tail_index = (self->_tail_index + 1) % self->_max_size;
        memcpy(self->_data[self->_tail_index], ptr_data, self->_item_size);
	}

	// calculate size
    self->_size = ((self->_tail_index - self->_front_index + 1) + self->_max_size) % self->_max_size;

    if(self->_size == 0)
	{
        self->_size = self->_max_size;
	}
}


void* dequeue(Queue* self)
{
    if(self->_front_index == -1 || self->_tail_index == -1)
	{
		return NULL;
	}

    void* ptr_data = self->_data[self->_front_index];

	// dequeue last item
    if(self->_front_index == self->_tail_index)
	{
        self->_front_index = -1;
        self->_tail_index = -1;
        self->_size = 0;

	}
	else
	{
        self->_front_index = (self->_front_index + 1 + self->_max_size) % self->_max_size;

        self->_size = ((self->_tail_index - self->_front_index + 1) + self->_max_size) % self->_max_size;
        if(self->_size == 0)
		{
            self->_size = self->_max_size;
		}
	}

	return ptr_data;
}

bool queue_empty(Queue* self)
{
    if(self->_front_index == -1 && self->_tail_index == -1)
	{
        return true;
	}

    return false;
}

void clear(Queue* self)
{
    self->_size = 0;
    self->_front_index = -1;
    self->_tail_index = -1;
}

unsigned int queue_size(Queue* self)
{
    return self->_size;
}

void release_queue(Queue* self)
{
	int i;

    for(i = 0; i < self->_max_size; ++i)
	{
        free(self->_data[i]);
        self->_data[i] = NULL;
	}

    free(self->_data);

    self->_data = NULL;
    self->_item_size = 0;
    self->_size = 0;
    self->_max_size = 0;
    self->_front_index = -1;
    self->_tail_index = -1;
}
