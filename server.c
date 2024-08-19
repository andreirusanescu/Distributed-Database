/*
 * Copyright (c) 2024, <>
 */

#include <stdio.h>
#include "server.h"
#include "lru_cache.h"

#include "utils.h"

static response
*server_edit_document(server *s, char *doc_name, char *doc_content) {
    void **evicted_key = NULL;
    lru_cache_put(s->cache, doc_name, doc_content, evicted_key);

    if (evicted_key) {
        ht_put(s->data_base, ((info *)(*evicted_key))->key, strlen(((info *)(*evicted_key))->key),
               ((info *)(*evicted_key))->value, strlen(((info *)(*evicted_key))->value));
    }

    ht_put(s->data_base, doc_name, strlen(doc_name), doc_content, strlen(doc_content));
    response *res = (response *)malloc(sizeof(response));
    res->server_id = 1;
    res->server_log = LOG_LAZY_EXEC;
    res->server_response = MSG_A;
    return res;
}

static response
*server_get_document(server *s, char *doc_name) {
	void **evicted_key = NULL;
    response *res = (response *)malloc(sizeof(response));
    res->server_id = 1;

    char *cached_content = lru_cache_get(s->cache, doc_name);
    if (cached_content) {
        res->server_log = LOG_HIT;
        res->server_response = cached_content;
        return res;
    }

    char *db_content = ht_get(s->data_base, doc_name);
    if (db_content) {
        res->server_log = LOG_MISS;
        res->server_response = db_content;
        lru_cache_put(s->cache, doc_name, db_content, evicted_key);

        if (evicted_key) {
            ht_put(s->data_base, ((info *)(*evicted_key))->key, strlen(((info *)(*evicted_key))->key),
                   ((info *)(*evicted_key))->value, strlen(((info *)(*evicted_key))->value));
            res->server_log = LOG_EVICT;
        }

        return res;
    }

    res->server_log = LOG_FAULT;
    res->server_response = NULL;
    return res;
}

server *init_server(unsigned int cache_size) {
	server *sv = (server *)malloc(sizeof(server));
	sv->cache = init_lru_cache(cache_size);
	sv->queue = q_create(sizeof(request), TASK_QUEUE_SIZE);
	sv->data_base = ht_create(HMAX, hash_string, compare_function_strings, key_val_free_function);
	return sv;
}

response *server_handle_request(server *s, request *req) {
    response *res;

    if (req->type == EDIT_DOCUMENT) {
        res = server_edit_document(s, req->doc_name, req->doc_content);
        q_enqueue(s->queue, req);
        return res;

    } else if (req->type == GET_DOCUMENT) {
        while (!q_is_empty(s->queue)) {
            request *aux = (request *)q_front(s->queue);
            if (aux->type == EDIT_DOCUMENT) {
                PRINT_RESPONSE(server_edit_document(s, aux->doc_name, aux->doc_content));
            }
            q_dequeue(s->queue);
        }
        res = server_get_document(s, req->doc_name);
		PRINT_RESPONSE(res);
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
