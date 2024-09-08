/*
 * Copyright (c) 2024, Andrei Rusanescu <andreirusanescu154gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "queue.h"
#include "server.h"

queue_t *q_create(unsigned int data_size, unsigned int max_size) {
	queue_t *queue = calloc(1, sizeof(queue_t));
    DIE(!queue, "calloc() failed");
	queue->data_size = data_size;
	queue->max_size = max_size;
	queue->read_idx = 0;
	queue->write_idx = 0;
	queue->buff = malloc(max_size * data_size);
	return queue;
}

unsigned int q_get_size(queue_t *q) {
	return q->size;
}

unsigned int q_is_empty(queue_t *q) {
	return q->size == 0;
}

void *q_front(queue_t *q) {
	return q->buff[q->read_idx];
}

int q_dequeue(queue_t *q) {
	if (!q || q->size == 0)
		return 0;
	request *aux = q->buff[q->read_idx];
	free(aux->doc_content);
	aux->doc_content = NULL;
	free(aux->doc_name);
	aux->doc_name = NULL;
	free(aux);
	aux = NULL;
	q->read_idx = (q->read_idx + 1) % q->max_size;
	q->size--;
	return 1;
}

void deep_copy(void **elem, void *data) {
	request *src = (request *)data;
	int content_len = strlen(src->doc_content);
	int name_len = strlen(src->doc_name);
	((request *)(*elem))->doc_content = (char *)malloc((content_len + 1) *
													   sizeof(char));
	((request *)(*elem))->doc_name = (char *)malloc((name_len + 1) * sizeof(char));
	memcpy(((request *)(*elem))->doc_content, src->doc_content, content_len + 1);
	memcpy(((request *)(*elem))->doc_name, src->doc_name, name_len + 1);
	((request *)(*elem))->type = ((request *)(data))->type;
}

int q_enqueue(queue_t *q, void *new_data) {
	if (q->size == q->max_size)
		return 0;
	q->buff[q->write_idx] = malloc(1 * q->data_size);
	deep_copy(&q->buff[q->write_idx], new_data);
	q->write_idx = (q->write_idx + 1) % q->max_size;
	q->size++;
	return 1;
}

void q_clear(queue_t *q) {
	while (q_dequeue(q)) {}
}

void q_free(queue_t *q) {
	q_clear(q);
	free(q->buff);
	free(q);
	q = NULL;
}
