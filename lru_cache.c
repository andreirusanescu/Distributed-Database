/*
 * Copyright (c) 2024, <>
 */

#include <stdio.h>
#include <string.h>
#include "lru_cache.h"
#include "utils.h"

lru_cache *init_lru_cache(unsigned int cache_capacity) {
	lru_cache *lru = (lru_cache *)malloc(sizeof(lru_cache));
	DIE(!lru, "malloc() failed");
	lru->capacity = cache_capacity;
	lru->h = ht_create(HMAX, hash_string, compare_function_strings, key_val_free_function, node_copy);
	return lru;
}

bool lru_cache_is_full(lru_cache *cache) {
	return cache->size == cache->capacity;
}

void free_lru_cache(lru_cache **cache) {
	if (!*cache || !cache) return;

	node *elem = (*cache)->head, *tmp;
	while (elem) {
		tmp = elem;
		elem = elem->next;
		key_val_free_function(elem->data);
		free(tmp);
	}
	ht_free((*cache)->h);
	free((*cache));
}

bool lru_cache_put(lru_cache *cache, void *key, void *value,
				   void **evicted_key) {

	/* checks if it already has key
	   if it does, updates the value and moves
	   node to the beginning of the list 
	*/
	node *elem = (node *)ht_get(cache->h, key);
	if (elem) {
		if (elem != cache->head) {
			if (elem->prev && elem->next) {
				elem->prev->next = elem->next;
				elem->next->prev = elem->prev;
			} else if (elem->prev) {
				elem->prev->next = NULL;
				cache->tail = elem->prev;
			}
			elem->next = cache->head;
			cache->head->prev = elem;
			elem->prev = NULL;
			cache->head = elem;
		}
		// printf("%sp%sp\n", ((info *)elem->data)->value, value);
		simple_copy(&((info *)elem->data)->value, value, strlen(value) + 1);
		// (*evicted_key) = NULL;
		printf("MUIE\n");
		return false;
	}

	/* cache is full, the lru key has to be evicted */
	if (cache->size == cache->capacity) {
		if (cache->tail) {
			node *aux = cache->tail;
			cache->tail = cache->tail->prev;
			if (cache->tail)
				cache->tail->next = NULL;
			ht_remove_entry(cache->h, ((info *)aux->data)->key);
			(*evicted_key) = ((info *)aux->data)->key;
			// free(((info *)aux->data)->key);
			// free(((info *)aux->data)->value);
			// free(aux);
			cache->size--;
		}
		if (!cache->size) {
			cache->head = cache->tail = NULL;
		}
	}


	/* puts the pair (key, value) in the cache */

	elem = (node *)malloc(sizeof(node));
	elem->data = malloc(sizeof(info));
	size_t key_size = strlen(key) + 1;
	size_t value_size = strlen(value) + 1;
	((info *)elem->data)->key = malloc(key_size);
	((info *)elem->data)->value = malloc(value_size);
	memcpy(((info *)elem->data)->key, key, key_size);
	memcpy(((info *)elem->data)->value, value, value_size);

	if (cache->size == 0) {
		cache->head = elem;
		cache->tail = cache->head;
	} else {
		elem->next = cache->head;
		elem->prev = NULL;
		cache->head->prev = elem;
		cache->head = elem;
	}

	// aici tre sa pun in loc de value, nodul;
	ht_put(cache->h, key, key_size, elem, sizeof(node));
	cache->size++;
	return true;
}

void *lru_cache_get(lru_cache *cache, void *key) {
	node *elem = (node *)ht_get(cache->h, key);
	if (elem) {
		if (elem != cache->head) {
			if (elem->prev && elem->next) {
				elem->prev->next = elem->next;
				elem->next->prev = elem->prev;
			} else if (elem->prev) {
				elem->prev->next = NULL;
				cache->tail = elem->prev;
			}
			elem->next = cache->head;
			cache->head->prev = elem;
			elem->prev = NULL;
			cache->head = elem;
		}
		return elem->data;
	}
    return NULL;
}

void lru_cache_remove(lru_cache *cache, void *key) {
	if (ht_has_key(cache->h, key)) {
		node *elem = (node *)ht_get(cache->h, key);
		if (elem != cache->head) {
			if (elem->prev && elem->next) {
				elem->prev->next = elem->next;
				elem->next->prev = elem->prev;
			} else if (elem->prev) {
				elem->prev->next = NULL;
				cache->tail = elem->prev;
			}
			ht_remove_entry(cache->h, key);
			key_val_free_function(elem->data);
			free(elem);
		} else {
			ht_remove_entry(cache->h, key);
			key_val_free_function(cache->head->data);
			free(cache->head);
			cache->head = cache->tail = NULL;
		}
	}
}
