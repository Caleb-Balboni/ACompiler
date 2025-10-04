#include <criterion/criterion.h>
#include "utils/arraylist.h"
#include "utils/hashtable.h"
#include <stdbool.h>

// test for init array
Test(arraylist, init) {
  
  ArrayList* test_arr = initArrayList(10);
  cr_assert(test_arr != NULL);
  cr_assert(test_arr->length == 0);
  cr_assert(test_arr->capacity == 10);
}

// test for destorying array
Test(arraylist, destory) {
  ArrayList* test_arr = initArrayList(10);
  cr_assert(test_arr != NULL);
  
  int* num1 = malloc(sizeof(int));
  int* num2 = malloc(sizeof(int));
  *num1 = 1;
  *num2 = 2;
  appendArrayList(test_arr, num1);
  appendArrayList(test_arr, num2);
  int* index1 = (int*)getArrayList(test_arr, 0);
  int* index2 = (int*)getArrayList(test_arr, 1);
  cr_assert(index1 != NULL && *index1 == 1);
  cr_assert(index2 != NULL && *index2 == 2);
  
  destroyArrayList(test_arr);
}

// test that appending an array is working properly, and getting array list
Test(arraylist, append) {

  ArrayList* test_arr = initArrayList(1);
  
  for (int i = 0; i < 100; i++) {
    int* x = malloc(sizeof(int));
    *x = i;
    appendArrayList(test_arr, x);
    cr_assert(test_arr->length == i + 1);
  }

  for (int i = 0; i < 100; i++) {
    int* x = getArrayList(test_arr, i);
    cr_assert(*x == i);
    x = NULL;
  }
  
  destroyArrayList(test_arr);
}
  
Test(hashtable, init) {
  hashtable_t* table = createHashTable(10);
  cr_assert(table != NULL);
  cr_assert(table->size == 0);
  cr_assert(table->capacity == 10);
}

Test(hashtable, append) {
  hashtable_t* table = createHashTable(1);

  for (int i = 0; i < 100; i++) {
    int* value = malloc(sizeof(int));
    *value = i;
    char key[3];
    sprintf(key, "%d", i);
    addHashTable(table, key, value);
  }

  for (int i = 0; i < 100; i++) {
    char key[3];
    sprintf(key, "%d", i);
    int* value = (int*)getHashTable(table, key);
    cr_assert(i == *value);
  }
}

Test(hashtable, remove) {

  hashtable_t* table = createHashTable(10);

  for (int i = 0; i < 100; i++) {
    int* value = malloc(sizeof(int));
    *value = i;
    char key[4];
    key[4] = '\0';
    sprintf(key, "%d", i);
    addHashTable(table, key, value);
  }

  for (int i = 0; i < 100; i++) {
    char key[4];
    sprintf(key, "%d", i);
    key[4] = '\0';
    cr_assert(removeHashTable(table, key) == true);
    cr_assert(getHashTable(table, key) == NULL);
  }
}
