/* Copyright 2023 <Tudor Cristian-Andrei> */
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "datastruct_funcs.h"
#include "data_structs.h"
#include "utils.h"

extern unsigned int hash_function_key(void *a);

server_memory_t *init_server_memory()
{
	server_memory_t *new_server = malloc(sizeof(server_memory_t));
	DIE(!new_server, "Failed while creating a new server.\n");
	new_server->storage = ht_create(HMAX, hash_function_key, key_val_free_function,
									compare_function_strings);

	DIE(!new_server->storage, "Failed while creating a new server.\n");

	return new_server;
}

void server_store(server_memory_t *server, char *key, char *value) {
	ht_put(server->storage, key, (strlen(key) + 1) * sizeof(char), value,
			(strlen(value) + 1) * sizeof(char));
}

char *server_retrieve(server_memory_t *server, char *key) {
	char *value = ht_get(server->storage, key);
	if (value == NULL)
		return NULL;

	return value;
}

void server_remove(server_memory_t *server, char *key) {
	ht_remove_entry(server->storage, key);
}

void free_server_memory(server_memory_t *server) {
	ht_free(server->storage);
	free(server);
}
