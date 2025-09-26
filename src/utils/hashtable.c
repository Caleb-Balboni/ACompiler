#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <assert.h>
#include "utils/hashtable.h"
#include "tokenizer/tokens.h"

node_t* create_node(const char* key, const Token_type value) {
	
	node_t* node = malloc(sizeof(node_t*) * 1);
	node->key = malloc(strlen(key) * 1);
	strcpy(node->key, key);
	node->value = value;

	node->next = NULL;

	return node;
}

unsigned int hash(hashtable_t* hashtable, const char* key) {
	
	unsigned long int value = 0;
	unsigned int key_len = strlen(key);

	for (int i = 0; i < key_len; i++) {
		value = value * 29 * key[i];
	}

	value = value % hashtable->capacity;

	return value;	
}

hashtable_t* createHashTable(const unsigned int capacity) {
	
	hashtable_t* hashtable = malloc(sizeof(struct hashtable_t*) * 1);
	hashtable->capacity = capacity;
	hashtable->size = 0;
	hashtable->nodes = (node_t**)malloc(sizeof(struct node_t*) * capacity);

	for (int i = 0; i < capacity; i++) {
		hashtable->nodes[i] = NULL;
	}

	return hashtable;
}

void addHashTable(hashtable_t* hashtable, const char* key, const Token_type value) {
	
	unsigned int slot = hash(hashtable, key);
		
	node_t* node = hashtable->nodes[slot];
	if (node == NULL) {
		hashtable->nodes[slot] = create_node(key, value); 
	}

	node_t* prev;

	while (node != NULL) {
		
		if (strcmp(node->key, key) == 0){		
			node->value = value;
			return;
		}
		
		prev = node;
		node = prev->next;	
	}

	prev->next = create_node(key, value);
}

Token_type getHashTable(hashtable_t* hashtable, const char* key) {
	
	unsigned int slot = hash(hashtable, key);

	node_t* node = hashtable->nodes[slot];

	if (node == NULL) {
		return UNKNOWN; 
	} 
	
	while (node != NULL) { 

		if (strcmp(node->key, key) == 0) { 
			return node->value;
		}

		node = node->next;
	}

	return UNKNOWN;
}

bool removeHashTable(hashtable_t* hashtable, const char* key) {
	
	unsigned int slot = hash(hashtable, key);

	node_t* node = hashtable->nodes[slot];
	
	node_t* prev = node;
	while (node != NULL) {

		if (strcmp(node->key, key) == 0) {
			
			node_t* next = node->next;
			prev->next = next;
			free(node->key);
			free(node);	
	        	node = NULL;

			return true;
		}
		
		prev = node;
		node = prev->next;
	}

	return false;
}
