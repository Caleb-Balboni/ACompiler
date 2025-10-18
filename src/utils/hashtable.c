#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <assert.h>
#include "utils/hashtable.h"
#include "tokenizer/tokens.h"

node_t* create_node(const char* key, void* value) {
	
	node_t* node = malloc(sizeof(node_t) * 1);
  assert(node);
	node->key = malloc(strlen(key) + 1);
	strcpy(node->key, key);
	node->value = value;
	node->next = NULL;

	return node;
}

unsigned int hash(hashtable_t* hashtable, const char* key) {
	
	unsigned long int value = 1;
	unsigned int key_len = strlen(key);

	for (int i = 0; i < key_len; i++) {
		value = value * 29 * key[i];
	}

	value = value % hashtable->capacity;

	return value;	
}

hashtable_t* create_ht(const unsigned int capacity) {
	
	hashtable_t* hashtable = malloc(sizeof(hashtable_t) * 1);
	hashtable->capacity = capacity;
	hashtable->size = 0;
	hashtable->nodes = calloc(capacity, sizeof(node_t*));

	return hashtable;
}

void add_ht(hashtable_t* hashtable, const char* key, void* value) {
	
	unsigned int slot = hash(hashtable, key);
		
	node_t* node = hashtable->nodes[slot];
	if (node == NULL) {
		hashtable->nodes[slot] = create_node(key, value); 
		hashtable->size += 1;
		return;
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
	hashtable->size += 1;
}

void* get_ht(hashtable_t* hashtable, const char* key) {
	
	unsigned int slot = hash(hashtable, key);

	node_t* node = hashtable->nodes[slot];

	if (node == NULL) {
		return NULL; 
	} 
	
	while (node != NULL) { 

		if (strcmp(node->key, key) == 0) { 
			return node->value;
		}

		node = node->next;
	}

	return NULL;
}

bool remove_ht(hashtable_t* hashtable, const char* key) {
	
	unsigned int slot = hash(hashtable, key);

	node_t* node = hashtable->nodes[slot];
	
	node_t* prev = NULL;

	while (node != NULL) {

		if (strcmp(node->key, key) == 0) {
		  
      if (prev) {
        prev->next = node->next; 
      } else {
        hashtable->nodes[slot] = node->next;
      }
			free(node->key);
			free(node);	

			return true;
		}
		
		prev = node;
		node = prev->next;
	}

	return false;
}

bool destroy_ht(hashtable_t* hashtable) {
  
  for (int i = 0; i < hashtable->capacity; i++) {
    
    if (hashtable->nodes[i] == NULL) {
      continue;
    }
    node_t* next = hashtable->nodes[i];

    while (next != NULL) {
      node_t* temp = next->next;
      free(next->key);
      free(next->value);
      free(next);
      next = temp;
    }
  }

  free(hashtable->nodes);
  free(hashtable);
}
