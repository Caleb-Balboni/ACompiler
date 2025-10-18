#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utils/arraylist.h"

ArrayList* init_list(unsigned int capacity) {
	
	ArrayList* array = malloc(sizeof(ArrayList) * 1);
	array->length = 0;
	array->capacity = capacity;
	array->items = malloc(sizeof(void*) * capacity);
	return array;
}

void destroy_list(ArrayList* array) {

	for (int i = 0; i < array->length; i++) {
		free(array->items[i]);	
    array->items[i] = NULL;
	}

	free(array->items);
  array->items = NULL;
	free(array);	
  array = NULL;
}

void* get_list(ArrayList* array, unsigned int index) {
	
	if (index < array->length) {
		return array->items[index];
	}

	return NULL;
}

bool add_list(ArrayList* array, void* item) {
	
	if (!array) return false;
	if (array->length == array->capacity) {
		unsigned int newcap = array->capacity * RESIZE_MUL;
		array->items = realloc(array->items, sizeof(void*) * newcap);
		array->capacity = newcap;
	}

	array->items[array->length++] = item;
	return true;
}
