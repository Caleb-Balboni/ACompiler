#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "utils/stack.h"

stack_t* init_stack() {
  stack_t* stack = malloc(sizeof(stack_t));
  assert(stack != NULL);
  stack->head = NULL;
  return stack;
}

void* pop_stack(stack_t* stack) {
  if (!stack->head) {
    return NULL;
  } else {
    void* temp_val = stack->head->val;
    stack->head = stack->head->next;
    return temp_val;
  }
}

void push_stack(stack_t* stack, void* val) {
  stack_node* temp = malloc(sizeof(stack_node));
  assert(temp != NULL);
  if (!stack->head) {
    temp->val = val;
    temp->next = NULL;
    stack->head = temp;
  } else {
    temp->val = val;
    temp->next = stack->head;
    stack->head = temp;
  }
}

void* peek_stack(stack_t* stack) {
  if (!stack->head) {
    return NULL;
  } else {
    return stack->head->val;
  }
}

bool stack_is_empty(stack_t* stack) {
  return stack->head == NULL;
}

void delete_stack(stack_t* stack) {
  stack_node* temp = stack->head;
  while (temp) {
    stack_node* next = temp->next;
    free(temp->val);
    free(temp);
    temp = next;
  }
  free(stack);
}
