
#ifndef HASH_H
#define HASH_H

#define MAX_STRING_SIZE	256
#define HMAX 100

/********************************************HASH IMPLEMENTATION********************************************/

typedef struct ll_node_t
{
	void* data;
	struct ll_node_t* next;
} ll_node_t;

typedef struct linked_list_t
{
	ll_node_t* head;
	unsigned int data_size;
	unsigned int size;
} linked_list_t;

typedef struct info info;
struct info {
	void *key;
	void *value;
};

typedef struct hashtable_t hashtable_t;
struct hashtable_t {
	linked_list_t **buckets; // Array de liste simplu-inlantuite.
	// Nr. total de noduri existente curent in toate bucket-urile.
	unsigned int size;
	unsigned int hmax; /* Nr. de bucket-uri. */
	// (Pointer la) Functie pentru a calcula valoarea hash asociata cheilor.
	unsigned int (*hash_function)(void*);
	// (Pointer la) Functie pentru a compara doua chei.
	int (*compare_function)(void*, void*);
	// (Pointer la) Functie pentru a elibera memoria ocupata de cheie si valoare.
	void (*key_val_free_function)(void*);
	void (*copy_func)(void **, void *, unsigned int);
};

linked_list_t* ll_create(unsigned int data_size);

// adauga un nou nod cu nod->data = new_data pe pozitia n;
void ll_add_nth_node(linked_list_t* list, unsigned int n, void* new_data);

/*  elimina nodul de pe pozitia n, daca n < 0 se ignora, daca e mai mare decat
*  numarul de noduri din lista, se elimina ultimul;
*/
ll_node_t* ll_remove_nth_node(linked_list_t* list, unsigned int n);

// Functia intoarce nr de noduri din lista
unsigned int ll_get_size(linked_list_t* list);

// elibereaza tot
void ll_free(linked_list_t** pp_list);

// Functii de comparare a cheilor:
int compare_function_ints(void *a, void *b);
int compare_function_strings(void *a, void *b);

// elibereaza memoria folosita de data;
// in acest caz stiu ca lucrez cu structuri de tipul info;
void key_val_free_function(void *data);

// creeaza un hashtable;
hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*),
		void (*copy_func)(void **, void*, unsigned int));

// verifica tot tabelul dupa cheie;
int ht_has_key(hashtable_t *ht, void *key);

// intoarce pointerul catre data asociata cheii key;
void *ht_get(hashtable_t *ht, void *key);

// incarca / updateaza un camp din hashtable;
void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size);

// elibereaza un camp din hashtable pe baza cheii
void ht_remove_entry(hashtable_t *ht, void *key);

// elibereaza tot;
void ht_free(hashtable_t *ht);
unsigned int ht_get_size(hashtable_t *ht);
unsigned int ht_get_hmax(hashtable_t *ht);

void node_copy(void **dst, void *src, unsigned int src_size);
void simple_copy(void **dst, void *src, unsigned int src_size);

/************************************************************************************************************/

#endif /* HASH_H */