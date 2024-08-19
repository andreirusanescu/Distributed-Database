/*
 * Copyright (c) 2024, <>
 */

#ifndef SERVER_H
#define SERVER_H

#include "utils.h"
#include "constants.h"
#include "lru_cache.h"
#include "queue.h"

#define TASK_QUEUE_SIZE         1000
#define MAX_LOG_LENGTH          1000
#define MAX_RESPONSE_LENGTH     4096

typedef struct server {
    lru_cache *cache;
    queue_t *queue;
    hashtable_t *data_base;
} server;

typedef struct request {
    request_type type;
    char *doc_name;
    char *doc_content;
} request;

typedef struct response {
    char *server_log; // MAX_LOG_LENGTH string
    char *server_response; // MAX_RESPONSE_LENGTH string
    int server_id; // of the server that executed the request
} response;


server *init_server(unsigned int cache_size);

/**
 * @brief Should deallocate completely the memory used by server,
 *     taking care of deallocating the elements in the queue, if any,
 *     without executing the tasks
 */
void free_server(server **s);

/**
 * server_handle_request() - Receives a request from the load balancer
 *      and processes it according to the request type
 * 
 * @param s: Server which processes the request.
 * @param req: Request to be processed.
 * 
 * @return response*: Response of the requested operation, which will
 *      then be printed in main.
 * 
 * @brief Based on the type of request, should call the appropriate
 *     solver, and should execute the tasks from queue if needed (in
 *     this case, after executing each task, PRINT_RESPONSE should
 *     be called).
 */
response *server_handle_request(server *s, request *req);

#endif  /* SERVER_H */
