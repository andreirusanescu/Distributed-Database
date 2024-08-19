#ifndef QUEUE_H
#define QUEUE_H


typedef struct queue_t queue_t;
struct queue_t
{
	/* Dimensiunea maxima a cozii */
	unsigned int max_size;
	/* Dimensiunea cozii */
	unsigned int size;
	/* Dimensiunea in octeti a tipului de date stocat in coada */
	unsigned int data_size;
	/* Indexul de la care se vor efectua operatiile de front si dequeue */
	unsigned int read_idx;
	/* Indexul de la care se vor efectua operatiile de enqueue */
	unsigned int write_idx;
	/* Bufferul ce stocheaza elementele cozii */
	void **buff;
};

queue_t *q_create(unsigned int data_size, unsigned int max_size);

/*
 * Functia intoarce numarul de elemente din coada al carei pointer este trimis
 * ca parametru.
 */
unsigned int q_get_size(queue_t *q);

/*
 * Functia intoarce 1 daca coada este goala si 0 in caz contrar.
 */
unsigned int q_is_empty(queue_t *q);

/* 
 * Functia intoarce primul element din coada, fara sa il elimine.
 */
void *q_front(queue_t *q);

/*
 * Functia scoate un element din coada. Se va intoarce 1 daca operatia s-a
 * efectuat cu succes (exista cel putin un element pentru a fi eliminat) si
 * 0 in caz contrar.
 */
int q_dequeue(queue_t *q);

/* 
 * Functia introduce un nou element in coada. Se va intoarce 1 daca
 * operatia s-a efectuat cu succes (nu s-a atins dimensiunea maxima) 
 * si 0 in caz contrar.
 */
int q_enqueue(queue_t *q, void *new_data);

/*
 * Functia elimina toate elementele din coada primita ca parametru.
 */
void q_clear(queue_t *q);

/*
 * Functia elibereaza toata memoria ocupata de coada.
 */
void q_free(queue_t *q);


#endif /* QUEUE_H */