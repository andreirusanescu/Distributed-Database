/*
 * Copyright (c) 2024, <>
 */

#include <stdio.h>
#include <string.h>
#include "lru_cache.h"
#include "utils.h"

lru_cache *init_lru_cache(unsigned int cache_capacity) {
	lru_cache *lru = (lru_cache *)calloc(1, sizeof(lru_cache));
	DIE(!lru, "malloc() failed");
	lru->capacity = cache_capacity;
	lru->map_string_to_node = ht_create(HMAX, hash_string, compare_function_strings, key_val_free_function, node_copy);
	return lru;
}

bool lru_cache_is_full(lru_cache *cache) {
	return cache->size == cache->capacity;
}

// void print_cache(lru_cache *cache) {
// 	node *elem = cache->head;
// 	printf("---------\n");
// 	while (elem) {
// 		printf("%s,%s\n", ((info *)elem->data)->key, ((info *)elem->data)->value);
// 		elem = elem->next;
// 	}
// 	printf("---------\n");
// }

void free_lru_cache(lru_cache **cache) {
	if (!*cache || !cache) return;

	node *elem = (*cache)->head, *tmp;
	
	ht_free((*cache)->map_string_to_node);
	while (elem) {
		tmp = elem;
		elem = elem->next;
		key_val_free_function(tmp->data);
		free(tmp);
		tmp = NULL;
	}
	free((*cache));
	*cache = NULL;
}

bool lru_cache_put(lru_cache *cache, void *key, void *value,
				   void **evicted_key) {

	/* checks if it already has key
	   if it does, updates the value and moves
	   node to the beginning of the list 
	*/

	node *elem = (node *)ht_get(cache->map_string_to_node, key);
	if (elem) {
		// update
		if (((info *)elem->data)->value != value) {
			free(((info *)elem->data)->value);
			((info *)elem->data)->value = NULL;
			((info *)elem->data)->value = calloc(1, strlen(value) + 1);
			memcpy(((info *)elem->data)->value, value, strlen(value) + 1);
		}
		/* bring forward */
		lru_cache_get(cache, key);
		return false;
	}

	/* cache is full, the last key has to be evicted */

	if (cache->size == cache->capacity) {
		node *aux = cache->tail;
		cache->tail = cache->tail->prev;
		if (cache->tail)
			cache->tail->next = NULL;
		*evicted_key = aux->data;
		ht_remove_entry(cache->map_string_to_node, ((info *)aux->data)->key);
		free(aux);
		aux = NULL;
		cache->size--;
		if (!cache->size)
			cache->head = cache->tail = NULL;
	}


	/* puts the pair (key, value) in the cache */
	elem = (node *)calloc(1, sizeof(node));
	elem->data = calloc(1, sizeof(info));
	size_t key_size = strlen(key) + 1;
	size_t value_size = strlen(value) + 1;
	((info *)elem->data)->key = calloc(1, key_size);
	((info *)elem->data)->value = calloc(1, value_size);
	memcpy(((info *)elem->data)->key, key, key_size);
	memcpy(((info *)elem->data)->value, value, value_size);

	/* puts the node at the begginning */
	elem->next = cache->head;
	elem->prev = NULL;
	if (cache->head)
		cache->head->prev = elem;
	cache->head = elem;
	if (!cache->size)
		cache->tail = elem;

	ht_put(cache->map_string_to_node, key, key_size, elem, sizeof(node));
	cache->size++;
	return true;
}

void *lru_cache_get(lru_cache *cache, void *key) {
	node *elem = (node *)ht_get(cache->map_string_to_node, key);

	if (elem) {
		if (elem != cache->head) {
			if (elem->prev)
				elem->prev->next = elem->next;

			if (elem->next)
				elem->next->prev = elem->prev;

			elem->next = cache->head;
			elem->prev = NULL;
			cache->head->prev = elem;
			
			cache->head = elem;
		}
		return ((info *)elem->data)->value;
	}
	return NULL;
}

void lru_cache_remove(lru_cache *cache, void *key) {
	node *elem = (node *)ht_get(cache->map_string_to_node, key);
	if (!elem)
		return;

	if (elem->prev) {
		elem->prev->next = elem->next;
	} else {
		cache->head = elem->next;
	}

	if (elem->next) {
		elem->next->prev = elem->prev;
	} else {
		cache->tail = elem->prev;
	}

	/* frees pair elem->data */
	key_val_free_function(elem->data);
	ht_remove_entry(cache->map_string_to_node, key); 
	free(elem);
}
