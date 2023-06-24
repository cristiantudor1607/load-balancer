/* Copyright 2023 <Tudor Cristian-Andrei> */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"
#include "utils.h"

unsigned int hash_function_servers(void *a) {
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int hash_function_key(void *a) {
	unsigned char *puchar_a = (unsigned char *)a;
	unsigned int hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

load_balancer_t *init_load_balancer() {
	load_balancer_t *load_balancer = malloc(sizeof(load_balancer_t));
	DIE(!load_balancer, "Failed while creating the load_balancer.\n");

	/**
	 * I will allocate memory for a relative small number of servers in the
	 * first place, number which can be change within the SERVER_INC macro
	 * (it comes from SERVER INCRESE NUMBER). Every time the maximum number
	 * of servers is reached, i will resize the array with 10 more places for
	 * new servers (again, the number 10 cand be changed within the macro)
	 */
	load_balancer->servers = malloc(SERVER_INC * sizeof(server_t));
	DIE(!load_balancer->servers, "Failed while creating the load_balancer.\n");

	load_balancer->max_servers = SERVER_INC;
	load_balancer->num_servers = 0;

	/**
	 * For the hashring, I need triple space, because, for one server added,
	 * there are 3 places occupied in the hashring.
	 */
	load_balancer->hashring = malloc(3 * SERVER_INC * sizeof(ring_t));
	DIE(!load_balancer->hashring, "Failed while creating the load_balancer.\n");

	load_balancer->hashring_size = 0;

	return load_balancer;
}

void loader_add_server(load_balancer_t *main, int server_id)
{
	/**
	 * Increase the memory for the arrays, if it is needed
	 */
	if (main->max_servers == main->num_servers) {
		main->max_servers += SERVER_INC;

		main->servers = realloc(main->servers, main->max_servers *
								sizeof(server_t));

		main->hashring = realloc(main->hashring, 3 * main->max_servers *
								sizeof(ring_t));
	}

	/**
	 * Create and add a new server on the last position. Because of the way
	 * the hashring works I don't bother sorting the servers, I don't need
	 * them to be sorted.
	 */
	unsigned int idx = main->num_servers;
	main->servers[idx].memory = init_server_memory();
	main->servers[idx].server_id = server_id;
	main->num_servers++;

	/**
	 * Add corresponding hashes to the hashring.
	 */
	add_hash(main, server_id);

	/**
	 * Get all the corresponding hashes from the hashring.
	 */
	unsigned int org_hash, dup1, dup2;
	get_duplicates(server_id, &org_hash, &dup1, &dup2);

	/**
	 * Remap the objects from the neighbours of the 3 replicas. 
	 */
	remap_objects(main, org_hash);
	remap_objects(main, dup1);
	remap_objects(main, dup2);
}

void loader_remove_server(load_balancer_t *main, int server_id) {
	/**
	 * Remove the hashes corresponding to the server id within
	 * the hashring.
	 */
	remove_id(main, server_id);

	/**
	 * I have to remap all the objects from the server, and delete the
	 * server from the array, so I need to know it's index.
	 */
	unsigned int idx = get_index(main, server_id);

	/**
	 * I have to go through the all buckets of hashtable 
	 */
	for (unsigned int i = 0; i < HMAX; i++) {
		list_t *list = main->servers[idx].memory->storage->buckets[i];

		if (list->size == 0)
			continue;

		for (unsigned int j = 0; j < list->size; j++) {
			node_t *curr = get_nth_node(list, j);
			char *key = (char *)((pair_t *)curr->data)->key;
			char *value = (char *)((pair_t *)curr->data)->value;

			unsigned int key_hash = hash_function_key(key);

			/**
			 * I already deleted the hashes from the hashring, so the
			 * get_server function will ignore the server we want to delete,
			 * and will find the next suitable place for the objects.
			 */
			unsigned int serv_id = get_server(main, key_hash);
			unsigned int idx_to_store = get_index(main, serv_id);

			server_store(main->servers[idx_to_store].memory, key, value);
		}
	}

	/**
	 * After I transfered the objects from the server to others, I can
	 * properly remove the server.
	 */
	delete_server(main, idx);
}

void loader_store(load_balancer_t *main, char *key, char *value,
				  int *server_id)
{
	/**
	 * Get the hash of the key, find the server where to put the
	 * object, and then get the index of the server from the servers
	 * array.
	 */
	unsigned int hash = hash_function_key(key);
	unsigned int serv_id = get_server(main, hash);
	unsigned int idx = get_index(main, serv_id);

	server_store(main->servers[idx].memory, key, value);

	*server_id = serv_id;
}

char *loader_retrieve(load_balancer_t *main, char *key, int *server_id)
{
	/**
	 * Find the server where the key is stored
	 */
	unsigned int hash = hash_function_key(key);
	unsigned int serv_id = get_server(main, hash);
	unsigned int idx = get_index(main, serv_id);

	*server_id = serv_id;

	/**
	 * Retrive the value from the server, if it exists. If it doesn't, it
	 * will return NULL.
	 */
	char *value = server_retrieve(main->servers[idx].memory, key);

	return value;
}

void free_load_balancer(load_balancer_t *main)
{
	/**
	 * Firstly, I delete all the allocated servers.
	 */
	while (main->num_servers > 0)
		delete_server(main, 0);

	/**
	 * Then, delete the arrays.
	 */
	free(main->servers);
	free(main->hashring);

	/**
	 * And then, free the memory occupied by the Load Balancer
	 */
	free(main);
}

void order_rings(load_balancer_t *main)
{
	unsigned int i1, i2;
	i1 = 0;
	i2 = main->hashring_size - 3;

	/**
	 * If the two indices are equal, then it's the first time, we put
	 * something in our array.
	 */
	if (i1 == i2)
		return;

	ring_t *sorted_arr = calloc(main->hashring_size, sizeof(ring_t));
	DIE(!sorted_arr, "Failed while modifying the hashring.\n");

	unsigned int idx = 0;

	/**
	 * Concatenate the last 3 items of the array with the rest of the array.
	 */
	while (i1 < main->hashring_size - 3 && i2 < main->hashring_size) {
		if (main->hashring[i1].hash < main->hashring[i2].hash) {
			sorted_arr[idx] = main->hashring[i1];
			idx++;
			i1++;
			continue;
		}

		sorted_arr[idx] = main->hashring[i2];
		idx++;
		i2++;
	}

	/**
	 * After one of the i's comes to the end, one of the two for's will
	 * actually be efective.
	 */
	while (i1 < main->hashring_size - 3) {
		sorted_arr[idx] = main->hashring[i1];
		idx++;
		i1++;
	}

	while (i2 < main->hashring_size) {
		sorted_arr[idx] = main->hashring[i2];
		idx++;
		i2++;
	}

	/**
	 * Transfer the elements of the sorted array in the hashring array.
	 */
	for (unsigned int i = 0; i < main->hashring_size; i++)
		main->hashring[i] = sorted_arr[i];

	free(sorted_arr);
}

void insert_ring(load_balancer_t *main, ring_t new_ring)
{
	/**
	 * Every time I add a hash, it will be placed at the end of the hashring.
	 */
	unsigned int idx = main->hashring_size;
	main->hashring[idx] = new_ring;
	main->hashring_size++;
}

void add_hash(load_balancer_t *main, unsigned int server_id)
{
	unsigned int hash = hash_function_servers(&server_id);

	unsigned int label1 = 1 * 100000 + server_id;
	unsigned int label2 = 2 * 100000 + server_id;

	unsigned int hash_dup1 = hash_function_servers(&label1);
	unsigned int hash_dup2 = hash_function_servers(&label2);

	unsigned int first, second, third;

	first = hash_dup1;
	second = hash_dup2;
	third = hash;

	/**
	 * Firstly, I order hash_dup1, and hash_dup2. At the begginig we considered
	 * that hash_dup1 is bigger than hash_dup2.
	 */
	if (hash_dup2 < hash_dup1) {
		first = hash_dup2;
		second = hash_dup1;
	}

	/**
	 * At this point we now that hash_dup1, and hash_dup2 are in the correct
	 * order, and they are stored in "first" and "second"
	 * 
	 */
	if (hash < first) {
		third = second;
		second = first;
		first = hash;
	}

	/**
	 * Make comparisions with the hash, to determine the order
	 */
	if (hash > second)
		third = hash;

	if (hash > first && hash < second) {
		third = second;
		second = hash;
	}

	/**
	 * I store the infos into 3 rings that have to be placed in the hashring
	 */
	ring_t ring1, ring2, ring3;
	ring1.server_id = ring2.server_id = ring3.server_id = server_id;
	ring1.hash = first;
	ring2.hash = second;
	ring3.hash = third;

	insert_ring(main, ring1);
	insert_ring(main, ring2);
	insert_ring(main, ring3);

	order_rings(main);
}

unsigned int get_server(load_balancer_t *main, unsigned int key_hash)
{
	unsigned int curr_hash, next_hash;
	curr_hash = main->hashring[0].hash;

	if (key_hash < curr_hash)
		return main->hashring[0].server_id;

	for (unsigned int i = 0; i < main->hashring_size - 1; i++) {
		curr_hash = main->hashring[i].hash;
		next_hash = main->hashring[i + 1].hash;

		if (curr_hash < key_hash &&  key_hash <= next_hash)
			return main->hashring[i + 1].server_id;
	}

	/**
	 * If it reaches this point means that the hash is bigger that all
	 * the other hashes in the hashring, and it has to be stored on the
	 * first server.
	 */
	return main->hashring[0].server_id;
}

unsigned int get_index(load_balancer_t *main, unsigned int server_id)
{
	for (unsigned int i = 0; i < main->num_servers; i++)
		if (main->servers[i].server_id == server_id)
			return i;

	/**
	 * It will never reach this point, beacause the parameter server_id we 
	 * send it is a valid id, that already exists in the array of servers.
	 */
	return IRELLEVANT;
}

void remove_id(load_balancer_t *main, unsigned int server_id)
{
	unsigned int i = 0;

	/**
	 * Delete all the hashes associated with the server_id, and keep the
	 * array sorted.
	 */
	while (i < main->hashring_size) {
		if (main->hashring[i].server_id == server_id) {
			for (unsigned int j = i; j < main->hashring_size - 1; j++)
				main->hashring[j] = main->hashring[j + 1];

			main->hashring_size--;
			continue;
		}

		i++;
	}
}

void delete_server(load_balancer_t *main, unsigned int index)
{
	/**
	 * I don't need the servers to be in ascending or descending or some
	 * special order, so I can remove a server faster by making a switch
	 * between the last server in the array, and the one we want to erase,
	 * and just deleting the last server, without moving the rest of the
	 * servers.
	 */
	unsigned int last = main->num_servers - 1;

	server_t aux = main->servers[index];
	main->servers[index] = main->servers[last];
	main->servers[last] = aux;

	free_server_memory(main->servers[last].memory);

	main->num_servers--;
}

void remap_objects(load_balancer_t *main, unsigned int hash)
{
	/**
	 * At the time this functions is called, the hashes corresponding to
	 * this server had already been placed on the hashring, and is very
	 * easily to remap the objects.
	 */

	/**
	 * Search for the id of the server that has the potential to share objects
	 * with the new server.
	 */
	unsigned int next_id;
	for (unsigned int i = 0; i < main->hashring_size; i++) {
		if (main->hashring[i].hash == hash) {
			/**
			 * The case when the server was added on the last position, and
			 * it's neighbour is the first server.
			 */
			if (i == main->hashring_size - 1) {
				next_id = main->hashring[0].server_id;
				break;
			}

			next_id = main->hashring[i + 1].server_id;
			break;
		}
	}

	/**
	 * Get the index of the server, and store the objects in the new one.
	 */
	unsigned int serv_idx = get_index(main, next_id);
	for (unsigned int i = 0; i < HMAX; i++) {
		list_t *list = main->servers[serv_idx].memory->storage->buckets[i];

		if (list->size == 0)
			continue;

		node_t *temp = list->head;

		while (temp != NULL) {
			char *key = (char *)((pair_t *)temp->data)->key;
			char *value = (char *)((pair_t *)temp->data)->value;

			unsigned int temp_hash = hash_function_key(key);
			unsigned int to_store = get_server(main, temp_hash);
			unsigned int new_idx = get_index(main, to_store);

			/**
			 * Move objects from the server if it is needed.
			 */
			if (server_retrieve(main->servers[new_idx].memory, key) == NULL) {
				server_store(main->servers[new_idx].memory, key, value);
				temp = temp->next;
				server_remove(main->servers[serv_idx].memory, key);
				continue;
			}

			temp = temp->next;
		}
	}
}

void get_duplicates(unsigned int server_id, unsigned int *original_hash,
					unsigned int *duplicate1, unsigned int *duplicate2)
{
	/**
	 * Calculate the hashes for the server_id.
	 */
	unsigned int hash = hash_function_servers(&server_id);
	*original_hash = hash;

	unsigned int temp = 1 * 100000 + server_id;

	*duplicate1 = hash_function_servers(&temp);

	temp = 2 * 100000 + server_id;

	*duplicate2 = hash_function_servers(&temp);
}
