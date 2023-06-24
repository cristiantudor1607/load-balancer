/* Copyright 2023 <Tudor Cristian-Andrei> */
#ifndef LOAD_BALANCER_H_
#define LOAD_BALANCER_H_

#include "server.h"
#include "data_structs.h"
#include "datastruct_funcs.h"

/**
 * @brief Initializes the memory for a new load balancer and its fields.
 *
 * @return Pointer to the load balancer struct.
 */
load_balancer_t *init_load_balancer();

/**
 * @brief Frees the memory of every field that is related to the
 * load balancer (servers, hashring).
 *
 * @param main Load balancer to free.
 **/
void free_load_balancer(load_balancer_t *main);

/**
 * @brief Stores the key-value pair inside the system. The load balancer 
 * will use Consistent Hashing to distribute the load across the servers.
 * The chosen server ID will be returned using the last parameter.
 *
 * @param main Load balancer which distributes the work.
 * @param key Key represented as a string.
 * @param value Value represented as a string.
 * @param server_id This function will RETURN via this parameter
 * the server ID which stores the object.
 */
void loader_store(load_balancer_t *main, char *key, char *value,
				  int *server_id);

/**
 * @brief Gets a value associated with the key. The load balancer will search
 * for the server which should posess the value associated to the key.
 * The server will return NULL in case the key does NOT exist in the system.
 *
 * @param main Load balancer which distributes the work.
 * @param key Key represented as a string.
 * @param server_id This function will RETURN the server ID which stores
 * the value via this parameter.
 */
char *loader_retrieve(load_balancer_t *main, char *key, int *server_id);

/**
 * @brief  Adds a new server to the system. The load balancer will generate
 * 3 replica labels and it will place them inside the hash ring. The neighbor
 * servers will distribute some the objects to the added server.
 * 
 * @param main Load balancer which distributes the work.
 * @param server_id ID of the new server.
 */
void loader_add_server(load_balancer_t *main, int server_id);

/**
 * @brief Removes a specific server from the system. The load balancer will 
 * distribute ALL objects stored on the removed server and will delete ALL
 * replicas from the hash ring.
 *
 * @param main Load balancer which distributes the work.
 * @param server_id ID of the removed server.
 */
void loader_remove_server(load_balancer_t *main, int server_id);

/**
 * @brief Based on the server id, there will be generated 3 different hashes,
 * one for each replica, and they will be aded at the end of the hashring as
 * ring_t structs. They will be aded at the end in ascending order. 
 * 
 * @param main The Load Balancer which distributes the work.
 * @param server_id The id of the server to be added in the hashring.
*/
void add_hash(load_balancer_t *main, unsigned int server_id);

/**
 * @brief Inserts a new ring at the end of the hashring, and increase the size
 * of the hashring.
 * 
 * @param main The Load Balancer which distributes the work.
 * @param new_ring The new ring_t struct that has to be inserted in the hashring.
 */
void insert_ring(load_balancer_t *main, ring_t new_ring);

/**
 * @brief AOrder the hashring in the ascending order by hash. Actually, this 
 * function makes a concatenation between the last 3 elements of the hashring,
 * and the rest of the hashring, because it is always called after 3 calls of
 * insert_ring(), and the hashring remains sorted in the rest of the time.
 * 
 * @param main The Load Balancer which distributes the work.
 */
void order_rings(load_balancer_t *main);

/**
 * @brief Search through the hashring to find the server where the new object
 * with the key_hash has to be put.
 * 
 * @param main The Load Balancer which distributes the work.
 * @param key_hash The hash of the key that have to be added.
 * @return The id of the server which will host the key.
 */
unsigned int get_server(load_balancer_t *main, unsigned int key_hash);

/**
 * @brief Search through the array of servers within the load balancer, to
 * find the index of the server with the given server_id.
 * 
 * @param main The Load Balancer which distributes the work.
 * @param server_id The id of the server to find.
 * @return The index of the server, or the IRELLEVANT value, but it never
 * reaches that far in the code, that's why it is called irellevant.
 */
unsigned int get_index(load_balancer_t *main, unsigned int server_id);

/**
 * @brief Remove all the rings in the hashring that posses a given id.
 * 
 * @param main The Load Balancer which distributes the work.
 * @param server_id The id of the server that wants to be removed from
 * the hashring.
 */
void remove_id(load_balancer_t *main, unsigned int index);

/**
 * @brief Deletes a server from the array of servers with the given index.
 * 
 * @param main The Load Balancer which distributes the work.
 * @param index The index of the server that have to be removed.
 */
void delete_server(load_balancer_t *main, unsigned int idx);

/**
 * @brief Rebalance the objects when a new server is added. The function is called
 * for each replica of the server.
 * 
 * @param main The Load Balancer which distributes the work.
 * @param hash The hash from the hashring, that corresponds to the new server. It
 * also can be the hash of a replica of the new server.
 */
void remap_objects(load_balancer_t *main, unsigned int hash);

/**
 * @brief Get all the 3 hashes corresponding to a server id, from the hashring.
 * The hashes are returned by the other 3 params.
 * 
 * @param server_id The id of the server which hashes you want to get.
 * @param original_hash The original hash.
 * @param duplicate1 The hash of the first duplicate.
 * @param duplicate2 The hash of the second duplicate.
 */
void get_duplicates(unsigned int server_id, unsigned int *original_hash,
					unsigned int *duplicate1, unsigned int *duplicate2);

#endif /* LOAD_BALANCER_H_ */
