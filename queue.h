/*
 * Copyright (c) 2024, Andrei Rusanescu <andreirusanescu154gmail.com>
 */

#ifndef QUEUE_H
#define QUEUE_H

typedef struct queue_t queue_t;
struct queue_t
{
	/* Max size for queue */
	unsigned int max_size;
	/* Actual size of queue */
	unsigned int size;
	/* Size in bytes of the data added in queue */
	unsigned int data_size;
	/* Index for dequeue and front operations */
	unsigned int read_idx;
	/* Index for enqueue */
	unsigned int write_idx;
	/* Buffer that contains queue elements */
	void **buff;
};

/*
 * Creates a queue of max_size elements of data_size size in bytes
 */
queue_t *q_create(unsigned int data_size, unsigned int max_size);

/*
 * Returns the size of the queue.
 */
unsigned int q_get_size(queue_t *q);

/*
 * Returns 1 if queue is empty and 0 on the contrary.
 */
unsigned int q_is_empty(queue_t *q);

/* 
 * Returns the first element of the queue.
 */
void *q_front(queue_t *q);

/* 
 * Removes an element from queue. Returns 1 if
 * the element was removed successfully (queue is not empty)
 * and 0 on the contrary
 */
int q_dequeue(queue_t *q);

/* 
 * Adds a new element in queue. Returns 1 if
 * the element was added successfully (queue is not full)
 * and 0 on the contrary
 */
int q_enqueue(queue_t *q, void *new_data);

/*
 * Eliminates all of the elements from the queue
 */
void q_clear(queue_t *q);

/*
 * Frees the queue entirely
 */
void q_free(queue_t *q);


#endif /* QUEUE_H */
