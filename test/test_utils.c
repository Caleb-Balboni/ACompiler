#include <stdio.h>
#include <criterion/criterion.h>
#include <stdbool.h>
#include "utils/arraylist.h"
#include "utils/hashtable.h"
#include "utils/stack.h"

// test for init array
Test(arraylist, init) {
  
  ArrayList* test_arr = init_list(10);
  cr_assert(test_arr != NULL);
  cr_assert(test_arr->length == 0);
  cr_assert(test_arr->capacity == 10);
}

// test for destorying array
Test(arraylist, destory) {
  ArrayList* test_arr = init_list(10);
  cr_assert(test_arr != NULL);
  
  int* num1 = malloc(sizeof(int));
  int* num2 = malloc(sizeof(int));
  *num1 = 1;
  *num2 = 2;
  add_list(test_arr, num1);
  add_list(test_arr, num2);
  int* index1 = (int*)get_list(test_arr, 0);
  int* index2 = (int*)get_list(test_arr, 1);
  cr_assert(index1 != NULL && *index1 == 1);
  cr_assert(index2 != NULL && *index2 == 2);
  
  destroy_list(test_arr);
}

// test that appending an array is working properly, and getting array list
Test(arraylist, append) {

  ArrayList* test_arr = init_list(1);
  
  for (int i = 0; i < 100; i++) {
    int* x = malloc(sizeof(int));
    *x = i;
    add_list(test_arr, x);
    cr_assert(test_arr->length == i + 1);
  }

  for (int i = 0; i < 100; i++) {
    int* x = get_list(test_arr, i);
    cr_assert(*x == i);
    x = NULL;
  }
  
  destroy_list(test_arr);
}

// tests init a hashtable
Test(hashtable, init) {
  hashtable_t* table = create_ht(10);
  cr_assert(table != NULL);
  cr_assert(table->size == 0);
  cr_assert(table->capacity == 10);
}

// test appending to a hashtable
Test(hashtable, append) {
  hashtable_t* table = create_ht(1);

  for (int i = 0; i < 100; i++) {
    int* value = malloc(sizeof(int));
    *value = i;
    char key[3];
    sprintf(key, "%d", i);
    add_ht(table, key, value);
  }

  for (int i = 0; i < 100; i++) {
    char key[3];
    sprintf(key, "%d", i);
    int* value = (int*)get_ht(table, key);
    cr_assert(i == *value);
  }
}

// test removing from a hashtable
Test(hashtable, remove) {

  hashtable_t* table = create_ht(10);

  for (int i = 0; i < 100; i++) {
    int* value = malloc(sizeof(int));
    *value = i;
    char key[4];
    key[4] = '\0';
    sprintf(key, "%d", i);
    add_ht(table, key, value);
  }

  for (int i = 0; i < 100; i++) {
    char key[4];
    sprintf(key, "%d", i);
    key[4] = '\0';
    cr_assert(remove_ht(table, key) == true);
    cr_assert(get_ht(table, key) == NULL);
  }
}

Test(stack, init) {
  stack_t* stack = init_stack();
  cr_assert(stack != NULL);
  cr_assert(stack->head == NULL);
  delete_stack(stack);
}

Test(stack, pushandpop) {
  stack_t* stack = init_stack();
  int arr[5] = {5, 4, 3, 2, 1};
  for (int i = 0; i < 5; i++) {
    push_stack(stack, &arr[i]);
  }
  
  for (int i = 0; i < 5; i++) {
    int x = *(int*)pop_stack(stack);
    cr_assert(x == arr[4 - i]);
  }
  delete_stack(stack); 
}
