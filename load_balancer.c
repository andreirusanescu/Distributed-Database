/*
 * Copyright (c) 2024, Andrei Rusanescu <andreirusanescu154gmail.com>
 */

#include "load_balancer.h"
#include "server.h"

load_balancer *init_load_balancer(bool enable_vnodes) {
	load_balancer *lb = (load_balancer *)malloc(sizeof(load_balancer));
	// lb->test_server = NULL;
	lb->size = 0;
	lb->capacity = 1;
	lb->hashring = (unsigned int *)calloc(1, sizeof(unsigned int));
	lb->servers = (server **)malloc(1 * sizeof(server *));
	lb->hash_function_docs = hash_string;
	lb->hash_function_servers = hash_uint;
	(void)enable_vnodes;
	return lb;
}

static
void execute_queue(server *s) {
	request *aux;
	response *res;
	while (s->queue->size) {
		aux = (request *)q_front(s->queue);
		res = server_edit_document(s, aux->doc_name, aux->doc_content);
		PRINT_RESPONSE(res);
		q_dequeue(s->queue);
	}
}

static
unsigned int find_pos(load_balancer *main, unsigned int hash, unsigned int id) {
	unsigned int left = 0, right = main->size, mid;

	while (left < right) {
		mid = left + (right - left) / 2;
		if (main->hashring[mid] > hash ||
			(main->hashring[mid] == hash &&
			 (unsigned int)main->servers[mid]->id > id))
			right = mid;
		else
			left = mid + 1;
	}
	return left;
}

static
unsigned int bisearch(unsigned int *h, unsigned int l,
					  unsigned int r, unsigned int x) {
	unsigned int m;
	while (l < r) {
		m = l + (r - l) / 2;
		if (h[m] == x)
			return m;
		else if (h[m] > x)
			r = m;
		else
			l = m + 1;
	}
	return l;
}

void loader_add_server(load_balancer* main, int server_id, int cache_size) {
	/* TODO: Remove test_server after checking the server implementation */
	// main->test_server = init_server(cache_size);
	// main->test_server->id = server_id;

	if (main->size == main->capacity) {
		main->capacity = main->capacity << 1;
		main->servers = (server **)realloc(main->servers,
										   main->capacity *
										   sizeof(server *));
		main->hashring = (unsigned int *)realloc(main->hashring,
						  						 main->capacity *
												 sizeof(unsigned int));
	}

	server *sv = init_server(cache_size);
	sv->id = server_id;

	const unsigned int replicas = 1;
	unsigned int hash, pos, i, j;

	for (i = 0; i < replicas; ++i) {
		// if (!i)
		hash = main->hash_function_servers(&sv->id);

		/* binary search */
		pos = find_pos(main, hash, server_id);

		for (j = main->size; j >= pos + 1; --j) {
			main->servers[j] = main->servers[j - 1];
			main->hashring[j] = main->hashring[j - 1];
		}
		main->servers[pos] = sv;
		main->hashring[pos] = hash;
		++main->size;
	}

	char *doc_name, *doc_content, *aux_string;
	unsigned int name_len, content_len;
	ll_node_t *elem, *tmp;

	if (pos + 1 < main->size) {
		sv = main->servers[pos + 1];
		execute_queue(main->servers[pos + 1]);
	} else {
		sv = main->servers[0];
		execute_queue(main->servers[0]);
	}

	for (i = 0; i < sv->data_base->hmax; ++i) {
		elem = sv->data_base->buckets[i]->head;
		while (elem) {
			tmp = elem->next;

			doc_name = ((info *)elem->data)->key;
			hash = main->hash_function_docs(doc_name);
			if ((pos > 0 && hash <= main->hashring[pos] &&
					hash > main->hashring[pos - 1]) ||
				(pos == 0 &&
					(hash <= main->hashring[pos] ||
					hash > main->hashring[main->size - 1]))) {
				aux_string = (char *)ht_get(sv->data_base, doc_name);

				content_len = strlen(aux_string) + 1;
				doc_content = (char *)malloc(content_len);
				memcpy(doc_content, aux_string, content_len);

				name_len = strlen(doc_name) + 1;
				doc_name = (char *)malloc(name_len);
				memcpy(doc_name, ((info *)elem->data)->key, name_len);

				ht_remove_entry(sv->data_base, doc_name);
				lru_cache_remove(sv->cache, doc_name);
				ht_put(main->servers[pos]->data_base, doc_name, name_len,
						doc_content, content_len);
				free(doc_content);
				free(doc_name);
			}
			elem = tmp;
		}
	}
}

void loader_remove_server(load_balancer* main, int server_id) {
	ll_node_t *elem, *tmp;
	server *rm;
	const unsigned int replicas = 1;
	unsigned int hash, pos, i, j, content_len, name_len;
	char *doc_name, *doc_content, *aux_string;
	for (i = 0; i < replicas; ++i) {
		hash = main->hash_function_servers(&server_id);
		pos = bisearch(main->hashring, 0, main->size - 1, hash);
		rm = main->servers[pos];
		for (j = pos; j < main->size - 1; ++j) {
			main->servers[j] = main->servers[j + 1];
			main->hashring[j] = main->hashring[j + 1];
		}
		--main->size;

		/* removed server is on the last position,
		   all of its documents go to the first server */

		if (pos == main->size && main->size) {
			pos = 0;
		}

		execute_queue(rm);
		for (j = 0; j < rm->data_base->hmax; ++j) {
			elem = rm->data_base->buckets[j]->head;
			while (elem) {
				tmp = elem->next;
				if (elem->data) {
					doc_name = ((info *)elem->data)->key;
					aux_string = (char *)ht_get(rm->data_base, doc_name);

					content_len = strlen(aux_string) + 1;
					doc_content = (char *)malloc(content_len);
					memcpy(doc_content, aux_string, content_len);

					name_len = strlen(doc_name) + 1;
					doc_name = (char *)malloc(name_len);
					memcpy(doc_name, ((info *)elem->data)->key, name_len);

					ht_remove_entry(rm->data_base, doc_name);
					lru_cache_remove(rm->cache, doc_name);
					ht_put(main->servers[pos]->data_base, doc_name, name_len,
						   doc_content, content_len);
					free(doc_content);
					free(doc_name);
				}
				elem = tmp;
			}
		}
		free_server(&rm);
	}
}

response *loader_forward_request(load_balancer* main, request *req) {
	server *sv;
	unsigned int hash_val = main->hash_function_docs(req->doc_name);
	unsigned int sv_index;

	if (main->hashring[main->size - 1] < hash_val)
		sv_index = 0;
	else
		sv_index = bisearch(main->hashring, 0, main->size - 1, hash_val);
	sv = main->servers[sv_index];

	// sv = main->test_server;
	response *res = server_handle_request(sv, req);
	return res;
}

void free_load_balancer(load_balancer** main) {
	/* TODO: get rid of test_server after testing the server implementation */
	// free_server(&(*main)->test_server);
	for (unsigned int i = 0; i < (*main)->size; ++i) {
		free_server(&(*main)->servers[i]);
	}
	free((*main)->servers);
	(*main)->servers = NULL;
	free((*main)->hashring);
	(*main)->hashring = NULL;
	free(*main);
	*main = NULL;
}
