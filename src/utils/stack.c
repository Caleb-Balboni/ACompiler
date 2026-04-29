#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "utils/stack.h"

Stack* init_stack() {
  Stack* stack = malloc(sizeof(Stack));
  assert(stack != NULL);
  stack->head = NULL;
  return stack;
}

void* pop_stack(Stack* stack) {
  if (!stack->head) {
    return NULL;
  } else {
    void* temp_val = stack->head->val;
    stack->head = stack->head->next;
    return temp_val;
  }
}

void push_stack(Stack* stack, void* val) {
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

void* peek_stack(Stack* stack) {
  if (!stack->head) {
    return NULL;
  } else {
    return stack->head->val;
  }
}

bool stack_is_empty(Stack* stack) {
  return stack->head == NULL;
}

void delete_stack(Stack* stack) {
  stack_node* temp = stack->head;
  while (temp) {
    stack_node* next = temp->next;
    free(temp->val);
    free(temp);
    temp = next;
  }
  free(stack);
}
