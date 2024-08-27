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
	lb->hashring = (unsigned int *)malloc(sizeof(unsigned int));
	lb->servers = (server **)malloc(1 * sizeof(server *));
	lb->hash_function_docs = hash_string;
	lb->hash_function_servers = hash_uint;
	return lb;
}

void swap_servers(server **a, server **b) {
	server *aux = *a;
	*a = *b;
	*b = aux;
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
	unsigned int hash, pos;
	
	for (unsigned int i = 0; i < replicas; ++i) {
		if (!i)
			hash = main->hash_function_servers(&sv->id);

		pos = 0;
		while (pos < main->size && main->hashring[pos] < hash)
			++pos;

		for (unsigned int j = main->size + 1; j >= pos + 1; --j) {
			main->servers[j] = main->servers[j - 1];
			main->hashring[j] = main->hashring[j - 1];
		}
		main->servers[pos] = sv;
		main->hashring[pos] = hash;
		++main->size;
	}

	// tre sa verific daca tre mutate documente,
	// daca au de executat task uri din cozi sa le execute
	char *doc_name, *doc_content;
	ll_node_t *elem;
	for (unsigned int i = pos + 1; i < main->size; ++i) {
		sv = main->servers[i];
		
		for (unsigned int j = 0; j < sv->data_base->hmax; ++j) {
			elem = sv->data_base->buckets[j]->head;

			while (elem) {
				doc_name = ((info *)elem->data)->key;
				hash = main->hash_function_docs(doc_name);
				if (hash < main->hashring[pos]) {


				}

			}
		}
	}

}

void loader_remove_server(load_balancer* main, int server_id) {
	/* TODO */
}

response *loader_forward_request(load_balancer* main, request *req) {
	if (!main || !req) return NULL;

	server *sv;
	unsigned int hash_val = main->hash_function_docs(req->doc_name);
	unsigned int sv_index = hash_val % main->size;
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
