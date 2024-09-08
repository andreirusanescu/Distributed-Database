/*
 * Copyright (c) 2024, Andrei Rusanescu <andreirusanescu154gmail.com>
 */

#include <stdio.h>

#include "server.h"
#include "lru_cache.h"
#include "utils.h"

static response
*server_edit_document(server *s, char *doc_name,
					  char *doc_content, unsigned int id) {
	void *evicted_key = NULL;
	response *res = (response *)malloc(1 * sizeof(response));
	res->server_id = id;
	char sv_log[MAX_LOG_LENGTH], sv_res[MAX_RESPONSE_LENGTH];
	size_t log_size, res_size;

	if (lru_cache_get(s->cache, doc_name)) {
		sprintf(sv_log, LOG_HIT, doc_name);
		sprintf(sv_res, MSG_B, doc_name);
		lru_cache_put(s->cache, doc_name, doc_content, &evicted_key);
		ht_put(s->data_base, doc_name, strlen(doc_name) + 1,
			   doc_content, strlen(doc_content) + 1);
		if (evicted_key) {
			free(((info *)((node *)evicted_key)->data)->key);
			free(((info *)((node *)evicted_key)->data)->value);
			free(((node *)evicted_key)->data);
			((node *)evicted_key)->data = NULL;
			free(evicted_key);
			evicted_key = NULL;
		}
		goto res_fin;
	}

	if (ht_has_key(s->data_base, doc_name)) {
		sprintf(sv_res, MSG_B, doc_name);
	} else {
		sprintf(sv_res, MSG_C, doc_name);
	}
	lru_cache_put(s->cache, doc_name, doc_content, &evicted_key);
	ht_put(s->data_base, doc_name, strlen(doc_name) + 1,
		   doc_content, strlen(doc_content) + 1);
	if (evicted_key) {
		// adding evicted key to memory
		sprintf(sv_log, LOG_EVICT, doc_name,
				(char *)(((info *)((node *)evicted_key)->data)->key));
		ht_put(s->data_base, ((info *)((node *)evicted_key)->data)->key,
			   strlen(((info *)((node *)evicted_key)->data)->key) + 1,
			   ((info *)((node *)evicted_key)->data)->value,
			   strlen((((info *)((node *)evicted_key)->data)->value)) + 1);

		free(((info *)((node *)evicted_key)->data)->key);
		free(((info *)((node *)evicted_key)->data)->value);
		free(((node *)evicted_key)->data);
		((node *)evicted_key)->data = NULL;

		free(evicted_key);
		evicted_key = NULL;
	} else {
		sprintf(sv_log, LOG_MISS, doc_name);
	}

res_fin:
	log_size = strlen(sv_log) + 1, res_size = strlen(sv_res) + 1;
	res->server_log = (char *)malloc(log_size * sizeof(char));
	res->server_response = (char *)malloc(res_size * sizeof(char));
	memcpy(res->server_log, sv_log, log_size);
	memcpy(res->server_response, sv_res, res_size);

	return res;
}

static response
*server_get_document(server *s, char *doc_name, unsigned int id) {
	void *evicted_key = NULL;
	bool fault = false;
	response *res = (response *)malloc(1 * sizeof(response));
	res->server_id = id;

	char sv_log[MAX_LOG_LENGTH], sv_res[MAX_RESPONSE_LENGTH];
	size_t log_size, res_size;
	char *cached_content = lru_cache_get(s->cache, doc_name);
	if (cached_content) {
		sprintf(sv_log, LOG_HIT, doc_name);
		sprintf(sv_res, "%s", cached_content);
		goto res_fin2;
	}

	char *db_content = ht_get(s->data_base, doc_name);
	if (db_content) {
		sprintf(sv_log, LOG_MISS, doc_name);
		sprintf(sv_res, "%s", db_content);
		lru_cache_put(s->cache, doc_name, db_content, &evicted_key);

		if (evicted_key) {
			sprintf(sv_log, LOG_EVICT, doc_name,
					(char *)(((info *)((node *)evicted_key)->data)->key));
			ht_put(s->data_base, ((info *)((node *)evicted_key)->data)->key,
				   strlen(((info *)((node *)evicted_key)->data)->key) + 1,
				   ((info *)((node *)evicted_key)->data)->value,
				   strlen((((info *)((node *)evicted_key)->data)->value)) + 1);

			free(((info *)((node *)evicted_key)->data)->key);
			free(((info *)((node *)evicted_key)->data)->value);
			free(((node *)evicted_key)->data);
			((node *)evicted_key)->data = NULL;
			free(evicted_key);
			evicted_key = NULL;
		}
	} else {
		sprintf(sv_log, LOG_FAULT, doc_name);
		res->server_response = NULL;
		fault = true;
	}

res_fin2:
	log_size = strlen(sv_log) + 1;
	res->server_log = (char *)malloc(log_size * sizeof(char));
	memcpy(res->server_log, sv_log, log_size);

	if (!fault) {
		res_size = strlen(sv_res) + 1;
		res->server_response = (char *)malloc(res_size * sizeof(char));
		memcpy(res->server_response, sv_res, res_size);
	}

	return res;
}

server *init_server(unsigned int cache_size) {
	server *sv = (server *)malloc(1 * sizeof(server));
	sv->cache = init_lru_cache(cache_size);
	sv->queue = q_create(sizeof(request), TASK_QUEUE_SIZE);
	sv->data_base = ht_create(HMAX, hash_string, compare_function_strings,
							  key_val_free_function, simple_copy);
	return sv;
}

/* Executes the requests for the server `s` with id `id` */
void execute_queue(server *s, unsigned int id) {
	request *aux;
	response *res;
	while (s->queue->size) {
		aux = (request *)q_front(s->queue);
		res = server_edit_document(s, aux->doc_name, aux->doc_content, id);
		PRINT_RESPONSE(res);
		q_dequeue(s->queue);
	}
}

response *server_handle_request(server *s, request *req, unsigned int id) {
	response *res;

	if (req->type == EDIT_DOCUMENT) {
		q_enqueue(s->queue, req);
		res = (response *)malloc(1 * sizeof(response));
		res->server_id = id;
		char sv_log[MAX_LOG_LENGTH], sv_res[MAX_RESPONSE_LENGTH];

		sprintf(sv_log, LOG_LAZY_EXEC, s->queue->size);
		sprintf(sv_res, MSG_A, get_request_type_str(req->type), req->doc_name);

		size_t log_size = strlen(sv_log) + 1, res_size = strlen(sv_res) + 1;

		res->server_log = (char *)malloc(log_size * sizeof(char));
		res->server_response = (char *)malloc(res_size * sizeof(char));

		memcpy(res->server_log, sv_log, log_size);
		memcpy(res->server_response, sv_res, res_size);
	} else if (req->type == GET_DOCUMENT) {
		execute_queue(s, id);
		res = server_get_document(s, req->doc_name, id);
	}

	return res;
}

void free_server(server **s) {
	if (s && *s) {
		q_free((*s)->queue);
		ht_free((*s)->data_base);
		free_lru_cache(&(*s)->cache);
		free(*s);
		*s = NULL;
	}
}
