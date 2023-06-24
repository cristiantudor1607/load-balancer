/* Copyright 2023 <Tudor Cristian-Andrei> */
#ifndef DATA_STRUCTS_H_
#define DATA_STRUCTS_H_

/* Structures used for Simple Linked List */
typedef struct node_t node_t;
typedef struct list_t list_t;

/* Structures used for Hashtables */
typedef struct hashtable_t hashtable_t;
typedef struct pair_t pair_t;

/* Structures used for Load Balancer */
typedef struct server_memory_t server_memory_t;
typedef struct server_t server_t;
typedef struct ring_t ring_t;
typedef struct load_balancer_t load_balancer_t;

struct node_t {
	void *data;
	node_t *next;
};

struct list_t {
	node_t *head;
	unsigned int data_size;
	unsigned int size;
};

struct hashtable_t {
	list_t **buckets;
	unsigned int hmax;
	unsigned int size;
	unsigned int (*hash_function)(void*);
	int (*compare_function)(void*, void*);
	void (*key_val_free_function)(void*);
};

/* The pair_t struct is the same as info_t from the lab */
struct pair_t {
	void *key;
	void *value;
};

/* The server_memory_t is just a hashtable*/
struct server_memory_t {
	hashtable_t *storage;
};

/* Those structs are used to make an array of servers; My idea is to store
together the server memory and it's unique id */
struct server_t {
	server_memory_t *memory;	/* the actual memory of the server*/
	unsigned int server_id;		/* the unique id of the server */
};

/* The Hashring will be implemented with those "rings"; Because of the
replicas of the servers, I need to store de id together with the hash; I did 
this to know at every point in time whose server belongs the hash */
struct ring_t {
	unsigned int hash;		/* the hash associated to a replica */
	unsigned int server_id;	/* the id of the server coresponding to the hash */
};

/* The Load Balancer */
struct load_balancer_t {
	server_t *servers;  /* the array of servers */
	unsigned int max_servers;   /* the maximum numbers of servers */
	unsigned int num_servers;	/* the actual number of servers */
	ring_t *hashring;	/* the array of rings aka the hashring */
	unsigned int hashring_size;	/* the hashring size */
};

#endif	// DATA_STRUCTS_H_
