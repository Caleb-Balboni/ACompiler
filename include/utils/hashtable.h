#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include "tokenizer/tokens.h"
#include <stdbool.h>

#ifndef HASH_TABLE
#define HASH_TABLE

typedef struct node_t {
	char* key;
	void* value;
	struct node_t* next;

} node_t;

typedef struct hashtable_t {
	unsigned int size;
	unsigned int capacity;
	node_t** nodes;	

} hashtable_t;

// creates a node with a key value pair
// @param key - the key to add to the node
// @param value - the value to add to  the node 
node_t* create_node(const char* key, void* value);

// hashes the key to the hashtable
// @param: hashtable - the hashtable we are hashing within
// @param: key - the key we are hashing
// @return: the hashed version of the key (also position) 
unsigned int hash(hashtable_t* hashtable, const char* key);

// constructor for the hashtable
// @param: capacity - the maximum capacity for this hashtable
// @return: a pointer to the newly created hash table
hashtable_t* createHashTable(const unsigned int capacity);

// adds a value - key pair to the hashtable
// @param hashtable - the hashtable to add to
// @param key - the key for the value in the hash table
// @param value - the value the given key will be attached to
void addHashTable(hashtable_t* hashtable, const char* key, void* value);

// returns a value from the hash table based upon a given key
// @param hashtable - the hashtable to search
// @param key - the key in the key value pair
// @return: the value the given key is pointing to
void* getHashTable(hashtable_t* hashtable, const char* key);

// remove a value from the hashtable given a paricular key
// @param hashtable - the hashtable to be removed from
// @param key - the key to be removed from the hashtable
bool removeHashTable(hashtable_t* hashtable, const char* key);

#endif
