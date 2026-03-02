#include <stdio.h>
#include <stdlib.h>
#include "parser/parser.h"

typedef struct {
  const char* name;
  long stack_offset;
  var_t type;
} symbol_t;

typedef struct {
  hashtable_t* symbols;
  long base_offset;
} scope_t;

typedef struct {
  emitter* emitter;
  Stack* scope_stk;
  long cur_offset;
  unsigned int label_count;
} asm_ctx;

asm_ctx* asm_init(const char* output_file);
