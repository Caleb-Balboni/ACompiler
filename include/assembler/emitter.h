#ifndef ASM_EMITTER_H
#define ASM_EMITTER_H

#include <stdio.h>
#include <stdlib.h>
#include "parser/parser.h"

typedef enum {
  REG_RAX,
  REG_RBX,
  REG_RCX,
  REG_RDX,
  REG_RSI,
  REG_RDI,
  REG_RBP,
  REG_RSP,
  REG_R8,
  REG_R9,
  REG_R10,
  REG_R11,
  REG_R12,
  REG_R13,
  REG_R14,
  REG_R15
} regid;

typedef enum {
  SZ_8,  // BYTE
  SZ_16, // WORD
  SZ_32, // DWORD
  SZ_64  // QWORD
} regsize;

typedef enum {
  OP_REG,
  OP_IMM,
  OP_MEM,
  OP_LABEL
} operandkind;

typedef struct {
  regid id;
  regsize size;
} register_t;

typedef struct {
  register_t base;
  long disp;
} mem_t;

typedef struct {
  operandkind kind;
  regsize size;
  union {
    register_t reg;
    long long imm;
    mem_t mem;
    const char* label;
  } op;
} operand_t;

typedef struct {
  FILE* out_file;
  unsigned int indent; 
} asm_emitter;

asm_emitter* asm_init(FILE* out);

void emit_text(asm_emitter* asm);

void emit_data(asm_emitter* asm);

void emit_label(asm_emitter* asm, const char* name);

void emit_globl(asm_emitter* asm, const char* name);

void emit_mov(asm_emitter* asm, operand_t* src, operand_t* dest);

void emit_push(asm_emitter* asm, operand_t* op);

void emit_pop(asm_emitter* asm, operand_t* op);

void emit_add(asm_emitter* asm, operand_t* src, operand_t* dest);

void emit_sub(asm_emitter* asm, operand_t* src, operand_t* dest);

void emit_inc(asm_emitter* asm, operand_t* op);

void emit_dec(asm_emitter* asm, operand_t* op);

void emit_imul(asm_emitter* asm, operand_t* src, operand_t* dest);

void emit_idiv(asm_emitter* asm, operand_t* divisor, operand_t* dividend);

void emit_jump(asm_emitter* asm, binary_expr_t jump_t, operand_t* label);

void emit_cmp(asm_emitter* asm, operand_t* operand_t* src, operand_t* dest);

void emit_call(asm_emitter* asm, operand_t* label);

void emit_ret(asm_emitter* asm, operand_t* ret_val);

#endif
