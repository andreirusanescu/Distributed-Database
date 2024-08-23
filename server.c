/*
 * Copyright (c) 2024, <>
 */

#include <stdio.h>
#include "server.h"
#include "lru_cache.h"

#include "utils.h"

static response
*server_edit_document(server *s, char *doc_name, char *doc_content) {
	void *evicted_key = NULL;
	response *res = (response *)calloc(1, sizeof(response));
	res->server_id = s->id;
	char sv_log[MAX_LOG_LENGTH], sv_res[MAX_RESPONSE_LENGTH];
	int log_length, res_length;

	if (lru_cache_get(s->cache, doc_name)) {
		sprintf(sv_log, LOG_HIT, doc_name);
		sprintf(sv_res, MSG_B, doc_name);
		lru_cache_put(s->cache, doc_name, doc_content, &evicted_key);
		ht_put(s->data_base, doc_name, strlen(doc_name) + 1, doc_content, strlen(doc_content) + 1);
		goto res_fin;
	}

	
	if (ht_has_key(s->data_base, doc_name)) {
		sprintf(sv_res, MSG_B, doc_name);
	} else {
		sprintf(sv_res, MSG_C, doc_name);
	}
	lru_cache_put(s->cache, doc_name, doc_content, &evicted_key);
	ht_put(s->data_base, doc_name, strlen(doc_name) + 1, doc_content, strlen(doc_content) + 1);
	if (evicted_key) {
		// adding evicted key to memory
		ht_put(s->data_base, ((info *)(evicted_key))->key, strlen(((info *)(evicted_key))->key) + 1,
			   ((info *)(evicted_key))->value, strlen(((info *)(evicted_key))->value) + 1);
		sprintf(sv_log, LOG_EVICT, doc_name, (char *)(((info *)(evicted_key))->key));
	} else {
		sprintf(sv_log, LOG_MISS, doc_name);
	}

res_fin:

	log_length = strlen(sv_log), res_length = strlen(sv_res);
	res->server_log = (char *)calloc((log_length + 1), sizeof(char));
	res->server_response = (char *)calloc((res_length + 1), sizeof(char));
	memcpy(res->server_log, sv_log, log_length + 1);
	memcpy(res->server_response, sv_res, res_length + 1);

	return res;
}


void print_cache(server *s) {
	node *elem = s->cache->head;
	printf("---------\n");
	while (elem) {
		printf("%s,%s\n", ((info *)elem->data)->key, ((info *)elem->data)->value);
		elem = elem->next;
	}
	printf("---------\n");
}

static response
*server_get_document(server *s, char *doc_name) {
	void *evicted_key = NULL;
	bool fault = false;
	response *res = (response *)calloc(1, sizeof(response));
	res->server_id = s->id;

	char sv_log[MAX_LOG_LENGTH], sv_res[MAX_RESPONSE_LENGTH];
	int log_length, res_length;
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
			ht_put(s->data_base, ((info *)(evicted_key))->key, strlen(((info *)(evicted_key))->key) + 1,
				   ((info *)(evicted_key))->value, strlen(((info *)(evicted_key))->value) + 1);
			sprintf(sv_log, LOG_EVICT, doc_name, ((char *)((info *)(evicted_key))->key));
		}
	} else {
		sprintf(sv_log, LOG_FAULT, doc_name);
		res->server_response = NULL;
		fault = true;
	}

res_fin2:
	log_length = strlen(sv_log);
	res->server_log = (char *)calloc((log_length + 1), sizeof(char));
	memcpy(res->server_log, sv_log, log_length + 1);

	if (!fault) {
		res_length = strlen(sv_res);
		res->server_response = (char *)calloc((res_length + 1), sizeof(char));
		memcpy(res->server_response, sv_res, res_length + 1);
	}
	

	return res;
}

server *init_server(unsigned int cache_size) {
	server *sv = (server *)calloc(1, sizeof(server));
	sv->cache = init_lru_cache(cache_size);
	sv->queue = q_create(sizeof(request), TASK_QUEUE_SIZE);
	sv->data_base = ht_create(HMAX, hash_string, compare_function_strings, key_val_free_function, simple_copy);
	return sv;
}

response *server_handle_request(server *s, request *req) {
	response *res;

	if (req->type == EDIT_DOCUMENT) {
		q_enqueue(s->queue, req);
		res = (response *)calloc(1, sizeof(response));
		res->server_id = s->id;
		char sv_log[MAX_LOG_LENGTH], sv_res[MAX_RESPONSE_LENGTH];

		sprintf(sv_log, LOG_LAZY_EXEC, s->queue->size);
		sprintf(sv_res, MSG_A, get_request_type_str(req->type), req->doc_name);

		int log_length = strlen(sv_log), res_length = strlen(sv_res);

		res->server_log = (char *)calloc((log_length + 1), sizeof(char));
		res->server_response = (char *)calloc((res_length + 1), sizeof(char));

		memcpy(res->server_log, sv_log, log_length + 1);
		memcpy(res->server_response, sv_res, res_length + 1);
		return res;
	} else if (req->type == GET_DOCUMENT) {
		request *aux;
		while (s->queue->size) {
			aux = (request *)q_front(s->queue);
			res = server_edit_document(s, aux->doc_name, aux->doc_content);
			PRINT_RESPONSE(res);
			
			q_dequeue(s->queue);
		}
		
		res = server_get_document(s, req->doc_name);
		return res;
	}

	return NULL;
}

void free_server(server **s) {
	if (s && *s) {
		free_lru_cache(&(*s)->cache);
		ht_free((*s)->data_base);
		q_free((*s)->queue);
		free(*s);
	}
}
