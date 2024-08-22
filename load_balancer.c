/*
 * Copyright (c) 2024, <>
 */

#include "load_balancer.h"
#include "server.h"

load_balancer *init_load_balancer(bool enable_vnodes) {
    load_balancer *lb = (load_balancer *)malloc(sizeof(load_balancer));
    lb->test_server = NULL;
    lb->hash_function_docs = hash_string;
    lb->hash_function_servers = hash_uint;
    return lb;
}

void loader_add_server(load_balancer* main, int server_id, int cache_size) {
    /* TODO: Remove test_server after checking the server implementation */
    main->test_server = init_server(cache_size);
    main->test_server->id = server_id;
}

void loader_remove_server(load_balancer* main, int server_id) {
    /* TODO */
}

response *loader_forward_request(load_balancer* main, request *req) {
    if (!main || !req) return NULL;

    server *sv;
    unsigned int hash_val = main->hash_function_docs(req->doc_name);
    // unsigned int sv_index = hash_val % main->num_sv;
    // sv = main->servers[sv_index];
    sv = main->test_server;
    response *res = server_handle_request(sv, req);
    return res;
}

void free_load_balancer(load_balancer** main) {
    /* TODO: get rid of test_server after testing the server implementation */
    free_server(&(*main)->test_server);
    free(*main);

    *main = NULL;
}


