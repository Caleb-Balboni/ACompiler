#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct stack_node stack_node;

struct stack_node {
  void* val;
  stack_node* next;
};

typedef struct {
  stack_node* head;
} Stack;

/// creates a new Stack
/// @return the newly created stack
Stack* init_stack();

/// pops the top elements value of the top of the stack
/// @param stack the stack to pop off off
/// @return the value on the top of the stack
void* pop_stack(Stack* stack);

/// pushes a value onto the top of the stack
/// @param stack the stack to push onto
/// @param val the value to push onto the stack
void push_stack(Stack* stack, void* val);

/// peeks the top value of the stack
/// @param stack the stack to peek into
/// @return the value on the top of the stack
void* peek_stack(Stack* stack);

/// returns if the stack is empty or not
/// @param stack the stack to check if empty or not
/// @return true if the stack is empty, false otherwise
bool stack_is_empty(Stack* stack);

/// deletes all elements allocated on the stack
/// @param stack the stack to de-allocate
void delete_stack(Stack* stack);
