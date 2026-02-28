#include <stdio.h>
#include <string.h>
#include <criterion/criterion.h>
#include <stdbool.h>
#include "utils/arraylist.h"
#include "utils/hashtable.h"
#include "utils/stack.h"

// ============================================================
// Stack: Extended Tests (peek, isEmpty, edge cases)
// ============================================================

Test(stack_ext, peek_returns_top_without_removing) {
  stack_t* stack = init_stack();
  int* a = malloc(sizeof(int)); *a = 10;
  int* b = malloc(sizeof(int)); *b = 20;
  push_stack(stack, a);
  push_stack(stack, b);

  int* top = (int*)peek_stack(stack);
  cr_assert(*top == 20);

  // peek again - should still be 20
  top = (int*)peek_stack(stack);
  cr_assert(*top == 20);

  // pop should also be 20
  int* popped = (int*)pop_stack(stack);
  cr_assert(*popped == 20);

  // now peek should be 10
  top = (int*)peek_stack(stack);
  cr_assert(*top == 10);

  delete_stack(stack);
  free(b);  // popped items are not freed by delete_stack
}

Test(stack_ext, peek_empty_returns_null) {
  stack_t* stack = init_stack();
  cr_assert(peek_stack(stack) == NULL);
  delete_stack(stack);
}

Test(stack_ext, is_empty_true) {
  stack_t* stack = init_stack();
  cr_assert(stack_is_empty(stack) == true);
  delete_stack(stack);
}

Test(stack_ext, is_empty_false) {
  stack_t* stack = init_stack();
  int* val = malloc(sizeof(int)); *val = 42;
  push_stack(stack, val);
  cr_assert(stack_is_empty(stack) == false);
  delete_stack(stack);
}

Test(stack_ext, is_empty_after_pop) {
  stack_t* stack = init_stack();
  int* val = malloc(sizeof(int));
  *val = 5;
  push_stack(stack, val);
  cr_assert(stack_is_empty(stack) == false);
  pop_stack(stack);
  cr_assert(stack_is_empty(stack) == true);
  free(val);
  delete_stack(stack);
}

Test(stack_ext, pop_empty_returns_null) {
  stack_t* stack = init_stack();
  cr_assert(pop_stack(stack) == NULL);
  delete_stack(stack);
}

Test(stack_ext, push_pop_many) {
  stack_t* stack = init_stack();
  int* values[100];
  for (int i = 0; i < 100; i++) {
    values[i] = malloc(sizeof(int));
    *values[i] = i;
    push_stack(stack, values[i]);
  }

  for (int i = 99; i >= 0; i--) {
    int* val = (int*)pop_stack(stack);
    cr_assert(*val == i);
    free(val);
  }

  cr_assert(stack_is_empty(stack) == true);
  delete_stack(stack);
}

Test(stack_ext, single_element) {
  stack_t* stack = init_stack();
  int* val = malloc(sizeof(int)); *val = 99;
  push_stack(stack, val);

  cr_assert(stack_is_empty(stack) == false);
  cr_assert(*(int*)peek_stack(stack) == 99);
  int* popped = (int*)pop_stack(stack);
  cr_assert(*popped == 99);
  cr_assert(stack_is_empty(stack) == true);

  free(popped);
  delete_stack(stack);
}

// ============================================================
// ArrayList: Extended Tests
// ============================================================

Test(arraylist_ext, get_out_of_bounds) {
  ArrayList* arr = init_list(5);
  int* val = malloc(sizeof(int));
  *val = 1;
  add_list(arr, val);

  cr_assert(get_list(arr, 0) != NULL);
  cr_assert(get_list(arr, 1) == NULL);  // out of bounds
  cr_assert(get_list(arr, 100) == NULL);

  destroy_list(arr);
}

Test(arraylist_ext, add_to_null_array) {
  cr_assert(add_list(NULL, NULL) == false);
}

Test(arraylist_ext, resize_capacity_doubles) {
  ArrayList* arr = init_list(2);
  cr_assert(arr->capacity == 2);

  int* a = malloc(sizeof(int)); *a = 1;
  int* b = malloc(sizeof(int)); *b = 2;
  int* c = malloc(sizeof(int)); *c = 3;

  add_list(arr, a);
  add_list(arr, b);
  cr_assert(arr->capacity == 2);

  // This should trigger resize
  add_list(arr, c);
  cr_assert(arr->capacity == 4);
  cr_assert(arr->length == 3);

  // Verify data integrity after resize
  cr_assert(*(int*)get_list(arr, 0) == 1);
  cr_assert(*(int*)get_list(arr, 1) == 2);
  cr_assert(*(int*)get_list(arr, 2) == 3);

  destroy_list(arr);
}

Test(arraylist_ext, single_capacity_init) {
  ArrayList* arr = init_list(1);
  cr_assert(arr->capacity == 1);
  cr_assert(arr->length == 0);

  int* a = malloc(sizeof(int)); *a = 42;
  add_list(arr, a);
  cr_assert(arr->length == 1);
  cr_assert(arr->capacity == 1);

  // Next add triggers resize
  int* b = malloc(sizeof(int)); *b = 99;
  add_list(arr, b);
  cr_assert(arr->length == 2);
  cr_assert(arr->capacity == 2);

  destroy_list(arr);
}

// ============================================================
// Hashtable: Extended Tests
// ============================================================

Test(hashtable_ext, get_nonexistent_key) {
  hashtable_t* table = create_ht(10);
  cr_assert(get_ht(table, "nonexistent") == NULL);
  destroy_ht(table);
}

Test(hashtable_ext, overwrite_existing_key) {
  hashtable_t* table = create_ht(10);
  int* val1 = malloc(sizeof(int)); *val1 = 1;
  int* val2 = malloc(sizeof(int)); *val2 = 2;

  add_ht(table, "key", val1);
  cr_assert(*(int*)get_ht(table, "key") == 1);

  add_ht(table, "key", val2);
  cr_assert(*(int*)get_ht(table, "key") == 2);

  // Size should not increase on overwrite
  cr_assert(table->size == 1);

  destroy_ht(table);
}

Test(hashtable_ext, remove_nonexistent_key) {
  hashtable_t* table = create_ht(10);
  cr_assert(remove_ht(table, "ghost") == false);
  destroy_ht(table);
}

Test(hashtable_ext, remove_then_get) {
  hashtable_t* table = create_ht(10);
  int* val = malloc(sizeof(int)); *val = 42;
  add_ht(table, "mykey", val);
  cr_assert(*(int*)get_ht(table, "mykey") == 42);

  cr_assert(remove_ht(table, "mykey") == true);
  cr_assert(get_ht(table, "mykey") == NULL);

  free(val);
  destroy_ht(table);
}

Test(hashtable_ext, many_keys_same_slot) {
  // Use small capacity to force collisions
  hashtable_t* table = create_ht(1);
  int* a = malloc(sizeof(int)); *a = 1;
  int* b = malloc(sizeof(int)); *b = 2;
  int* c = malloc(sizeof(int)); *c = 3;

  add_ht(table, "alpha", a);
  add_ht(table, "beta", b);
  add_ht(table, "gamma", c);

  cr_assert(*(int*)get_ht(table, "alpha") == 1);
  cr_assert(*(int*)get_ht(table, "beta") == 2);
  cr_assert(*(int*)get_ht(table, "gamma") == 3);
  cr_assert(table->size == 3);

  destroy_ht(table);
}

Test(hashtable_ext, remove_middle_of_chain) {
  hashtable_t* table = create_ht(1);
  int* a = malloc(sizeof(int)); *a = 1;
  int* b = malloc(sizeof(int)); *b = 2;
  int* c = malloc(sizeof(int)); *c = 3;

  add_ht(table, "x", a);
  add_ht(table, "y", b);
  add_ht(table, "z", c);

  // Remove middle element
  remove_ht(table, "y");
  cr_assert(get_ht(table, "y") == NULL);
  // Others should still work
  cr_assert(*(int*)get_ht(table, "x") == 1);
  cr_assert(*(int*)get_ht(table, "z") == 3);

  free(b);
  destroy_ht(table);
}

Test(hashtable_ext, destroy_empty_table) {
  hashtable_t* table = create_ht(10);
  cr_assert(table->size == 0);
  destroy_ht(table);
  // No crash = pass
}

Test(hashtable_ext, hash_deterministic) {
  hashtable_t* table = create_ht(100);
  unsigned int h1 = hash(table, "test");
  unsigned int h2 = hash(table, "test");
  cr_assert(h1 == h2);
  destroy_ht(table);
}

Test(hashtable_ext, hash_within_capacity) {
  hashtable_t* table = create_ht(50);
  unsigned int h = hash(table, "anything");
  cr_assert(h < 50);
  destroy_ht(table);
}
