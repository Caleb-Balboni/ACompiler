#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parser/parser.h"
#include "utils/hashtable.h"
#include "utils/stack.h"
#include "assembler/emitter.h"

typedef struct {
  const char* name;
  long stack_offset;
  var_t type;
  bool is_global;
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
  unsigned int push_depth;
  const char* epilogue_label;
} asm_ctx;

/// initalizes an asm_ctx that emits to the given file path
/// @param output_file the path to the assembly output file
/// @return the initalized asm_ctx
asm_ctx* asm_init(const char* output_file);

/// initalizes an asm_ctx around an existing FILE (for testing)
/// @param file the file the emitter writes to
/// @return the initalized asm_ctx
asm_ctx* asm_init_file(FILE* file);

/// frees an asm_ctx and closes its output file
/// @param ctx the asm_ctx to free
void asm_free(asm_ctx* ctx);

/// frees an asm_ctx without closing its output file
/// @param ctx the asm_ctx to free
void asm_free_keep_file(asm_ctx* ctx);

/// emits assembly for a parsed program AST
/// @param ctx the asm_ctx to emit through
/// @param program the AST program node
void gen_program(asm_ctx* ctx, Node* program);

/// emits assembly for a single function decleration
/// @param ctx the asm_ctx to emit through
/// @param node the function decl node
void gen_func_decl(asm_ctx* ctx, Node* node);

#endif
