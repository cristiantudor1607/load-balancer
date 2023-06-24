/* Copyright 2023 <Tudor Cristian-Andrei> */
#include "datastruct_funcs.h"

/**
 * @section Single Linked Lists
 */
list_t* ll_create(unsigned int data_size)
{
	list_t* ll = calloc(1, sizeof(*ll));
	DIE(!ll, "Failed ll_create\n");

	ll->data_size = data_size;

	return ll;
}

node_t* get_nth_node(list_t* list, unsigned int n)
{
	unsigned int len = list->size - 1;
	unsigned int i;
	node_t* node = list->head;

	n = MIN(n, len);

	for (i = 0; i < n; ++i)
		node = node->next;

	return node;
}

node_t* create_node(const void* new_data, unsigned int data_size)
{
	node_t* node = calloc(1, sizeof(*node));
	DIE(!node, "Failed create_node\n");

	node->data = malloc(data_size);
	DIE(!node->data, "Failed create_node -> data\n");

	memcpy(node->data, new_data, data_size);

	return node;
}

void ll_add_nth_node(list_t* list, unsigned int n, const void* new_data)
{
	node_t *new_node, *prev_node;

	if (!list)
		return;

	new_node = create_node(new_data, list->data_size);

	if (!n || !list->size) {
		new_node->next = list->head;
		list->head = new_node;
	} else {
		prev_node = get_nth_node(list, n - 1);
		new_node->next = prev_node->next;
		prev_node->next = new_node;
	}

	++list->size;
}

node_t* ll_remove_nth_node(list_t* list, unsigned int n)
{
	node_t *prev_node, *removed_node;

	if (!list || !list->size)
		return NULL;

	if (!n) {
		removed_node = list->head;
		list->head = removed_node->next;
		removed_node->next = NULL;
	} else {
		prev_node = get_nth_node(list, n - 1);
		removed_node = prev_node->next;
		prev_node->next = removed_node->next;
		removed_node->next = NULL;
	}

	--list->size;

	return removed_node;
}

unsigned int ll_get_size(list_t* list)
{
	return !list ? 0 : list->size;
}

void ll_free(list_t** pp_list)
{
	node_t* node;

	if (!pp_list || !*pp_list)
		return;

	while ((*pp_list)->size) {
		node = ll_remove_nth_node(*pp_list, 0);
		free(node->data);
		free(node);
	}

	free(*pp_list);
	*pp_list = NULL;
}

/**
 * @section Hashtable 
 */
hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
						void (*key_val_free_function)(void *),
						int (*compare_function)(void*, void*))
{
	hashtable_t *ht = (hashtable_t *)malloc(sizeof(hashtable_t));
	DIE(!ht, "Failed ht_create\n");
	ht->size = 0;
	ht->hmax = hmax;
	ht->buckets = (list_t **)malloc(hmax * sizeof(list_t *));
	DIE(!ht->buckets, "Failed while creating buckets for hashtable\n");
	for (unsigned int i = 0; i < hmax; i++) {
		ht->buckets[i] = ll_create(sizeof(pair_t));
		DIE(!ht->buckets[i], "Failed while creating a bucket\n");
	}

	ht->hash_function = hash_function;
	ht->key_val_free_function = key_val_free_function;
	ht->compare_function = compare_function;

	return ht;
}

int ht_has_key(hashtable_t *ht, void *key)
{
	unsigned int idx = ht->hash_function(key);
	idx = idx % ht->hmax;

	node_t *curr = ht->buckets[idx]->head;

	while (curr != NULL) {
		if (ht->compare_function(key, ((pair_t *)curr->data)->key) == 0)
			return 1;

		curr = curr->next;
	}

	return 0;
}

void *ht_get(hashtable_t *ht, void *key)
{
	unsigned int idx = ht->hash_function(key);
	idx = idx % ht->hmax;

	node_t *curr = ht->buckets[idx]->head;

	while (curr != NULL) {
		if (ht->compare_function(key, ((pair_t *)curr->data)->key) == 0)
			return ((pair_t *)curr->data)->value;

		curr = curr->next;
	}

	return NULL;
}

void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	if (ht_has_key(ht, key) == 1)
		return;

	unsigned int idx = ht->hash_function(key);
	idx %= ht->hmax;

	pair_t pair;
	pair.key = malloc(key_size);
	DIE(!pair.key, "Error while creating a (key, value) pair.\n");
	memcpy(pair.key, key, key_size);
	pair.value = malloc(value_size);
	DIE(!pair.value, "Error while creating a (key, value) pair.\n");
	memcpy(pair.value, value, value_size);

	ll_add_nth_node(ht->buckets[idx], 0, &pair);
	ht->size++;
}

void ht_remove_entry(hashtable_t *ht, void *key)
{
	unsigned int idx = ht->hash_function(key);
	idx = idx % ht->hmax;

	node_t *curr = ht->buckets[idx]->head;
	unsigned int i = 0;
	while(curr != NULL) {
		if (ht->compare_function(key, ((pair_t *)curr->data)->key) == 0) {
			node_t *erase = ll_remove_nth_node(ht->buckets[idx], i);
			ht->key_val_free_function(erase->data);
			free(erase->data);
			free(erase);
			ht->size--;
			return;
		}

		curr = curr->next;
		i++;
	}
}

void ht_free(hashtable_t *ht)
{
	for (unsigned int i = 0; i < ht->hmax; i++) {
		node_t *curr = ht->buckets[i]->head;
		for (unsigned int j = 0; j < ht->buckets[i]->size; j++) {
			ht->key_val_free_function(curr->data);
			curr = curr->next;
		}

		ll_free(&ht->buckets[i]);
	}
	free(ht->buckets);
	free(ht);
}

unsigned int ht_get_size(hashtable_t *ht)
{
	if (ht == NULL)
		return -1;

	return ht->size;
}

unsigned int ht_get_hmax(hashtable_t *ht)
{
	if (ht == NULL)
		return -1;

	return ht->hmax;
}

void key_val_free_function(void *data)
{
	free(((pair_t *)data)->key);
	free(((pair_t *)data)->value);
}

int compare_function_ints(void *a, void *b)
{
	int int_a = *((int *)a);
	int int_b = *((int *)b);

	if (int_a == int_b) {
		return 0;
	} else if (int_a < int_b) {
		return -1;
	} else {
		return 1;
	}
}

int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}
