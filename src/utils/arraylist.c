#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utils/arraylist.h"

ArrayList* initArrayList(unsigned int capacity, unsigned int itemsize) {
	
	ArrayList* array = malloc(sizeof(ArrayList) * 1);
	array->length = 0;
	array->capacity = capacity;
	array->itemsize = itemsize;
	array->items = malloc(sizeof(void*) * capacity);
	return array;
}

void destoryArrayList(ArrayList* array) {
	free(array->items);
	free(array);	
}

void* getArrayList(ArrayList* array, unsigned int index) {
	
	if (index <= array->capacity) {
		return array->items[index];
	}

	return NULL;
}

bool appendArrayList(ArrayList* array, void* item) {
	
	if (!array) return false;
	if (array->length >= array->capacity) {
		unsigned int newcap = array->capacity + 5;
		void** p = realloc(array->items, sizeof(void*) * newcap);
		if (!p) return false;
		array->items =  p;
		array->items[array->length] = item;
		array->capacity = newcap;
		array->length += 1;
	}
	
	array->items[array->length++] = item;
	return true;
}
