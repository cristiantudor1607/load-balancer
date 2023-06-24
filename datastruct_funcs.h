/* Copyright 2023 <Tudor Cristian-Andrei> */
#ifndef DATASTRUCT_FUNCS_H_
#define DATASTRUCT_FUNCS_H_

#include <stdlib.h>
#include <string.h>
#include "data_structs.h"
#include "utils.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))

/**
 * Functions for Single Linked Lists from the lab
 * Those are not my implementations, they were implemented by the team
*/
list_t* ll_create(unsigned int data_size);
node_t* get_nth_node(list_t* list, unsigned int n);
void ll_add_nth_node(list_t* list, unsigned int n, const void* new_data);
node_t* ll_remove_nth_node(list_t* list, unsigned int n);
unsigned int ll_get_size(list_t* list);
void ll_free(list_t** pp_list);

/**
 * Functions for Hashtables
 * Those are my implementations from the Hashtable Lab
*/
hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
                        void (*key_val_free_function)(void *),
                        int (*compare_function)(void*, void*));
int ht_has_key(hashtable_t *ht, void *key);
void *ht_get(hashtable_t *ht, void *key);
void ht_put(hashtable_t *ht, void *key, unsigned int key_size, void *value,
            unsigned int value_size);
void ht_remove_entry(hashtable_t *ht, void *key);
void ht_free(hashtable_t *ht);
unsigned int ht_get_size(hashtable_t *ht);
unsigned int ht_get_hmax(hashtable_t *ht);
void key_val_free_function(void *data);

/**
 * Those functions are taken from the lab
 * General purpose functions
*/
int compare_function_ints(void *a, void *b);
int compare_function_strings(void *a, void *b);

#endif  // DATASTRUCT_FUNCS_H_
