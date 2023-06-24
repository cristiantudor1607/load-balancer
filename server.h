/* Copyright 2023 <Tudor Cristian-Andrei> */
#ifndef SERVER_H_
#define SERVER_H_

struct server_memory_t;
typedef struct server_memory_t server_memory_t;

/** 
 * @brief Initialize the memory for a new server struct.
 *
 * @return Pointer to the allocated server_memory struct.
 */
server_memory_t *init_server_memory();

/** 
 * @brief Free the memory used by the server.
 *
 * @param server Server to free.
 */
void free_server_memory(server_memory_t *server);

/**
 * @brief Stores a key-value pair to the server.
 *
 * @param server Server which performs the task.
 * @param key Key represented as a string.
 * @param value Value represented as a string.
 */
void server_store(server_memory_t *server, char *key, char *value);

/**
 * @brief Removes a key-pair value from the server.
 *
 * @param server Server which performs the task.
 * @param key Key represented as a string.
 */
void server_remove(server_memory_t *server, char *key);

/**
 * @brief Gets the value associated with the key.
 * @param server Server which performs the task.
 * @param key Key represented as a string.
 *
 * @return String value associated with the key or
 * NULL (in case the key does not exist).
 */
char *server_retrieve(server_memory_t *server, char *key);

#endif  // SERVER_H_
