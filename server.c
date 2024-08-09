/*
 * Copyright (c) 2024, <>
 */

#include <stdio.h>
#include "server.h"
#include "lru_cache.h"

#include "utils.h"

static response
*server_edit_document(server *s, char *doc_name, char *doc_content) {
    /* TODO */
    return NULL;
}

static response
*server_get_document(server *s, char *doc_name) {
    /* TODO */
    return NULL;
}

server *init_server(unsigned int cache_size) {
    /* TODO */
    return NULL;
}

response *server_handle_request(server *s, request *req) {
    /* TODO */
    return NULL;
}

void free_server(server **s) {
    /* TODO */
}
