#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utils/arraylist.h"

ArrayList* initArrayList(unsigned int capacity) {
	
	ArrayList* array = malloc(sizeof(ArrayList) * 1);
	array->length = 0;
	array->capacity = capacity;
	array->items = malloc(sizeof(void*) * capacity);
	return array;
}

void destoryArrayList(ArrayList* array) {

	for (int i = 0; i < array->length; i++) {
		free(array->items[i]);	
	}

	free(array->items);
	free(array);	
}

void* getArrayList(ArrayList* array, unsigned int index) {
	
	if (index < array->length) {
		return array->items[index];
	}

	return NULL;
}

bool appendArrayList(ArrayList* array, void* item) {
	
	if (!array) return false;
	if (array->length == array->capacity) {
		unsigned int newcap = array->capacity * RESIZE_MUL;
		array->items = realloc(array->items, sizeof(void*) * newcap);
		array->capacity = newcap;
	}

	array->items[array->length++] = item;
	return true;
}
