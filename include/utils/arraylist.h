#ifndef ARRAYLIST
#define ARRAYLIST
#include <stdio.h>
#include <stdbool.h>

typedef struct {
	unsigned int length;
	unsigned int capacity;
	void** items;
} ArrayList;

// inits the arraylist 
// @param capacity - the capacity of the array
// @return - an initalized array list
ArrayList* initArrayList(unsigned int capacity);

// gets the element at a particular index in the array list
// @param array - the array to get from
// @param index - the index of the array to get from
// @return - the item at the index of the given array
void* getArrayList(ArrayList* array, unsigned int index);

// appends an item to the given array
// @param array - the array to append to
// @return true if the append was successful false otherwise
bool appendArrayList(ArrayList* array, void* item);

// destorys an arraylist and all the contents within it
// @param array - the array to destory
void destoryArrayList(ArrayList* array);

#define RESIZE_MUL 2

#endif
