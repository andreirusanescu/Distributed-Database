/*
 * Copyright (c) 2024, Andrei Rusanescu <andreirusanescu154gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "lru_cache.h"
#include "utils.h"

linked_list_t*
ll_create(unsigned int data_size)
{
	/* TODO */
	linked_list_t *list = (linked_list_t *)malloc(sizeof(linked_list_t));
	DIE(!list, "malloc() failed");
	list->head = NULL;
	list->data_size = data_size;
	list->size = 0;
	return list;
}

void ll_add_nth_node(linked_list_t* list, unsigned int n, void* new_data)
{
	if (!list)
		return;
	ll_node_t *new_node = (ll_node_t *)malloc(sizeof(ll_node_t));
	DIE(!new_node, "malloc() failed");
	new_node->data = new_data;
	new_node->next = NULL;
	if (n == 0 || list->head == NULL) {
		ll_node_t *aux = list->head;
		list->head = new_node;
		new_node->next = aux;
		list->size++;
	} else {
		ll_node_t *p = list->head;
		if (n > list->size)
			n = list->size;
		for (unsigned int i = 0; i < n && p->next != NULL; ++i)
			p = p->next;
		new_node->next = p->next;
		p->next = new_node;
		list->size++;
	}
}

ll_node_t* ll_remove_nth_node(linked_list_t* list, unsigned int n)
{
	if (n == 0 && list->head != NULL) {
		// trebuie eliminat headul;
		ll_node_t *p = list->head;
		list->head = list->head->next;
		list->size--;
		p->next = NULL;
		return p;
	} else if (n > 0) {
		ll_node_t *p = list->head;
		if (n > list->size)
			n = list->size - 1;
		for (unsigned int i = 0; i < n - 1 && p->next != NULL; ++i)
			p = p->next;
		ll_node_t *q = p->next;
		if (q != NULL) {
			p->next = q->next;
			list->size--;
			q->next = NULL;
			return q;
		} else {
			return NULL;
		}
	}
	return NULL;
}

unsigned int ll_get_size(linked_list_t* list)
{
	if (!list)
		return -1;
	return list->size;
}

void ll_free(linked_list_t** pp_list)
{
	if (!pp_list || !*pp_list)
		return;
	ll_node_t *aux;
	while ((*pp_list)->head != NULL) {
		aux = ll_remove_nth_node((*pp_list), 0);
		free(aux->data);
		aux->data = NULL;
		free(aux);
		aux = NULL;
	}
	free(*pp_list);
	*pp_list = NULL;
}

int compare_function_ints(void *a, void *b)
{
	int int_a = *((int *)a);
	int int_b = *((int *)b);
	if (int_a == int_b)
		return 0;
	else if (int_a < int_b)
		return -1;
	return 1;
}

int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;
	return strcmp(str_a, str_b);
}

void key_val_free_function(void *data) {
	info *pair = (info *)data;
	if (pair) {
		if (pair->key != NULL) {
			free(pair->key);
			pair->key = NULL;
		}
		if (pair->value != NULL) {
			free(pair->value);
			pair->value = NULL;
		}
		free(pair);
		pair = NULL;
	}
}

hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*),
		void (*copy_func)(void **, void*, unsigned int))
{
	hashtable_t *ht = (hashtable_t *)malloc(sizeof(hashtable_t));
	DIE(!ht, "malloc() failed");
	ht->hmax = hmax;
	ht->buckets = (linked_list_t **)malloc(hmax * sizeof(linked_list_t *));
	DIE(!ht->buckets, "malloc() failed");
	for (unsigned int i = 0; i < ht->hmax; ++i) {
		ht->buckets[i] = calloc(1, sizeof(linked_list_t));
		if (!ht->buckets[i]) {
			fprintf(stderr, "calloc() faled\n");
			for (int j = i - 1; j >= 0; --j)
				free(ht->buckets[i]);
			free(ht->buckets);
			free(ht);
			return NULL;
		}
		ht->buckets[i]->head = NULL;
		ht->buckets[i]->data_size = sizeof(info);
		ht->buckets[i]->size = 0;
	}
	ht->hash_function = hash_function;
	ht->compare_function = compare_function;
	ht->key_val_free_function = key_val_free_function;
	ht->copy_func = copy_func;
	return ht;
}

int ht_has_key(hashtable_t *ht, void *key)
{
	unsigned int index = ht->hash_function(key) % ht->hmax;
	ll_node_t *node = ht->buckets[index]->head;
	while (node) {
		if (ht->compare_function(((info *)(node->data))->key, key) == 0)
			return 1;
		node = node->next;
	}
	return 0;
}

void *ht_get(hashtable_t *ht, void *key)
{
	if (!ht)
		return NULL;
	unsigned int index = ht->hash_function(key) % ht->hmax;

	ll_node_t *elem = ht->buckets[index]->head;
	while (elem != NULL) {
		info *pair = (info *)elem->data;
		if (ht->compare_function(pair->key, key) == 0) {
			return pair->value;
		}
		elem = elem->next;
	}
	return NULL;
}

void node_copy(void **dst, void *src, unsigned int src_size) {
	*dst = src;
	(void)src_size;
}

void simple_copy(void **dst, void *src, unsigned int src_size) {
	*dst = calloc(1, src_size);
	memcpy((*dst), src, src_size);
}

void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	unsigned int index = ht->hash_function(key) % HMAX;
	ll_node_t *elem = ht->buckets[index]->head;
	while (elem != NULL) {
		/* same key => update value */
		if (!ht->compare_function(((info *)(elem->data))->key, key)) {
			if (((info*)elem->data)->value != value &&
				strcmp(((info*)elem->data)->value, value)) {
				free(((info*)elem->data)->value);
				((info*)elem->data)->value = calloc(1, value_size);
				memcpy(((info*)elem->data)->value, value, value_size);
			}

			return;
		}
		elem = elem->next;
	}
	info *data = (info *)calloc(1, sizeof(info));
	DIE(!data, "malloc() failed");
	data->key = calloc(1, key_size);
	memcpy(data->key, key, key_size);
	ht->copy_func(&data->value, value, value_size);
	ht->size++;
	ll_add_nth_node(ht->buckets[index], 0, data);
}

void ht_remove_entry(hashtable_t *ht, void *key)
{
	if (!ht)
		return;
	unsigned int index = ht->hash_function(key) % ht->hmax;
	ll_node_t *elem = ht->buckets[index]->head;
	for (int i = 0; elem; i++) {
		if (ht->compare_function(((info *)(elem->data))->key, key) == 0) {
			ll_node_t *aux = ll_remove_nth_node(ht->buckets[index], i);
			if (aux) {
				if (aux->data) {
					if (ht->copy_func == simple_copy) {
						free(((info *)aux->data)->value);
						((info *)aux->data)->value = NULL;
					}
					free(((info *)aux->data)->key);
					((info *)aux->data)->key = NULL;
					free(aux->data);
					aux->data = NULL;
				}
				free(aux);
				aux = NULL;
			}
			return;
		}
		elem = elem->next;
	}
}

void ht_free(hashtable_t *ht)
{
	for (unsigned int i = 0; i < ht->hmax; ++i) {
		ll_node_t* elem = ht->buckets[i]->head;
		while (elem) {
			ll_node_t *aux = ll_remove_nth_node(ht->buckets[i], 0);
			if (aux) {
				if (aux->data) {
					if (ht->copy_func == simple_copy) {
						free(((info *)aux->data)->value);
						((info *)aux->data)->value = NULL;
					}
					free(((info *)aux->data)->key);
					((info *)aux->data)->key = NULL;
					free(aux->data);
					aux->data = NULL;
				}
				free(aux);
				aux = NULL;
			}
			elem = ht->buckets[i]->head;
		}
		free(ht->buckets[i]);
	}
	free(ht->buckets);
	free(ht);
}

unsigned int ht_get_size(hashtable_t *ht)
{
	if (!ht)
		return 0;
	return ht->size;
}

unsigned int ht_get_hmax(hashtable_t *ht)
{
	if (!ht)
		return 0;
	return ht->hmax;
}
