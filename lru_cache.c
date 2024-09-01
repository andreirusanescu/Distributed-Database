/*
 * Copyright (c) 2024, Andrei Rusanescu <andreirusanescu154gmail.com>
 */

#include <stdio.h>
#include <string.h>

#include "lru_cache.h"
#include "utils.h"

lru_cache *init_lru_cache(unsigned int cache_capacity) {
	lru_cache *lru = (lru_cache *)calloc(1, sizeof(lru_cache));
	DIE(!lru, "malloc() failed");
	lru->capacity = cache_capacity;
	lru->map_string_to_node = ht_create(HMAX, hash_string,
										compare_function_strings,
										key_val_free_function, node_copy);
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
		key_val_free_function(tmp->data);
		free(tmp);
		tmp = NULL;
	}
	ht_free((*cache)->map_string_to_node);
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
		/* update */
		free(((info *)elem->data)->value);
		int len = strlen(value) + 1;
		((info *)elem->data)->value = calloc(1, len);
		memcpy(((info *)elem->data)->value, value, len);

		/* bring forward */
		lru_cache_get(cache, key);
		return false;
	}

	/* cache is full, the last key has to be evicted */
	if (cache->size == cache->capacity) {
		*evicted_key = cache->tail;
		cache->tail = cache->tail->prev;

		if (cache->tail)
			cache->tail->next = NULL;

		cache->size--;
		if (!cache->size)
			cache->head = cache->tail = NULL;
		((node *)*evicted_key)->prev = NULL;
		((node *)*evicted_key)->next = NULL;
		ht_remove_entry(cache->map_string_to_node,
						((info *)(((node *)*evicted_key)->data))->key);
	}


	/* puts the pair (key, value) in the cache */

	elem = malloc(1 * sizeof(node));
	elem->data = calloc(1 , sizeof(info));
	size_t key_size = strlen(key) + 1;
	size_t value_size = strlen(value) + 1;
	((info *)elem->data)->key = calloc(1, key_size);
	((info *)elem->data)->value = calloc(1, value_size);
	memcpy(((info *)elem->data)->key, key, key_size);
	memcpy(((info *)elem->data)->value, value, value_size);

	/* puts the node at the beginning */
	elem->next = cache->head;
	elem->prev = NULL;
	if (cache->head)
		cache->head->prev = elem;
	cache->head = elem;
	if (!cache->size)
		cache->tail = cache->head;

	cache->size++;
	ht_put(cache->map_string_to_node, key, key_size, elem, sizeof(node));
	return true;
}

void *lru_cache_get(lru_cache *cache, void *key) {
	node *elem = (node *)ht_get(cache->map_string_to_node, key);

	if (elem) {
		if (elem != cache->head) {
			elem->prev->next = elem->next;

			if (elem->next) {
				elem->next->prev = elem->prev;
			} else {
				cache->tail = cache->tail->prev;
				cache->tail->next = NULL;
			}

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
	if (!cache->size)
		return;
	node *elem = (node *)ht_get(cache->map_string_to_node, key);
	if (!elem)
		return;

	if (elem->prev)
		elem->prev->next = elem->next;
	else
		cache->head = elem->next;

	if (elem->next)
		elem->next->prev = elem->prev;
	else
		cache->tail = elem->prev;
	--cache->size;
	/* frees pair elem->data */
	key_val_free_function(elem->data);
	ht_remove_entry(cache->map_string_to_node, key);
	free(elem);
	elem = NULL;
}
