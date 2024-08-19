#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "queue.h"

queue_t *q_create(unsigned int data_size, unsigned int max_size) {
	queue_t *queue = malloc(sizeof(queue_t));
    DIE(!queue, "malloc() failed");
	queue->data_size = data_size;
	queue->max_size = max_size;
	queue->read_idx = 0;
	queue->write_idx = 0;
	queue->buff = malloc(max_size * data_size);
	return queue;
}

/*
 * Functia intoarce numarul de elemente din coada al carei pointer este trimis
 * ca parametru.
 */
unsigned int q_get_size(queue_t *q) {
	return q->size;
}

/*
 * Functia intoarce 1 daca coada este goala si 0 in caz contrar.
 */
unsigned int q_is_empty(queue_t *q) {
	return q->size == 0;
}

/* 
 * Functia intoarce primul element din coada, fara sa il elimine.
 */
void *q_front(queue_t *q) {
	return q->buff[q->read_idx];
}

/*
 * Functia scoate un element din coada. Se va intoarce 1 daca operatia s-a
 * efectuat cu succes (exista cel putin un element pentru a fi eliminat) si
 * 0 in caz contrar.
 */
int q_dequeue(queue_t *q) {
	if (!q || q->size == 0)
		return 0;
	void *aux = q->buff[0];
	free(aux);
	q->read_idx = (q->read_idx + 1) % q->max_size;
	q->size--;
	return 1;
}

/* 
 * Functia introduce un nou element in coada. Se va intoarce 1 daca
 * operatia s-a efectuat cu succes (nu s-a atins dimensiunea maxima) 
 * si 0 in caz contrar.
 */
int q_enqueue(queue_t *q, void *new_data) {
	if (q->size == q->max_size)
		return 0;
	q->buff[q->write_idx] = malloc(q->data_size);
	memcpy(q->buff[q->write_idx], new_data, q->data_size);
	q->write_idx = (q->write_idx + 1) % q->max_size;
	q->size++;
	return 1;
}

/*
 * Functia elimina toate elementele din coada primita ca parametru.
 */
void q_clear(queue_t *q) {
	while (q_dequeue(q));
}

/*
 * Functia elibereaza toata memoria ocupata de coada.
 */
void q_free(queue_t *q) {
	free(q->buff);
	free(q);
}
