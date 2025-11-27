#ifndef emitter_EMITTER_H
#define emitter_EMITTER_H

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
} reg_t;

typedef struct {
  register_t base;
  long disp;
} mem_t;

typedef struct {
  operandkind kind;
  union {
    reg_t reg;
    long long imm;
    mem_t mem;
    const char* label;
  } op;
} operand_t;

typedef struct {
  FILE* file;
  unsigned int indent; 
} emitter;

// initalizes the emitter with a FILE
// @param out - the outfile for the emitter
// @return - the initalized emitter
emitter* emitter_init(const char* file_name);

emitter* emitter_init2(FILE* file);

void emit_print(emitter* emitter, const char* fmt, ...);

// emits to a file the text section of the program ".text"
// @paramemitter - the emitter to emit from
void emit_text(emitter* emitter);

// emits the data section of the program ".data"
// @paramemitter - the emitter to emit from
void emit_data(emitter* emitter);

// emits a label for the assembly language
// @param emitter - the emitter to emit from
// @param name - the name of the label
void emit_label(emitter* emitter, const char* name);

// emits a global varialbe
// @param emitter - the emitter to emit from
// @param name - the name of the global variable
void emit_globl(emitter* emitter, const char* name);

// emits a mov istruction 
// @param emitter - the emitter to emit from
// @param src - the source operand to emit from
// @param dest - the destination operand to mov to
void emit_mov(emitter* emitter, operand_t* src, operand_t* dest);

// emits a push instruction
// @param emitter - the emitter to emit from
// @param op - the operand to push to the stack
void emit_push(emitter* emitter, operand_t* op);

// emits a pop instruction
// @param emitter - the emitter to emit from
// @param op - the operand to put the varibale from the stack into
void emit_pop(emitter* emitter, operand_t* op);

// emits an add instruction 
// @param emitter - the emitter to emit from
// @param src - the source operand to add from
// @param dest - the destination operand to add into
void emit_add(emitter* emitter, operand_t* src, operand_t* dest);

// emits a sub instruction
// @param emitter - the emitter to emit from
// @param src - the source operand to subtract from
// @param dest - the desitination operand to subtract from
void emit_sub(emitter* emitter, operand_t* src, operand_t* dest);

// emits an increment instruction
// @param emitter - the emitter to emit from
// @param op - the operand to increment
void emit_inc(emitter* emitter, operand_t* op);

// emits a decrement instruction
// @param emitter - the emitter to emit from
// @param op - the operand to decrement
void emit_dec(emitter* emitter, operand_t* op);

// emits a multiply instruction
// @param emitter - the emitter to emit from
// @param src - the source operand to multiply from
// @param dest - the destination operand to multiply from
void emit_imul(emitter* emitter, operand_t* src, operand_t* dest);

// emits a divide instruction
// @param emitter - the emitter to emit from
// @param divisor - the divisor of the divide instruction
// @param dividiend - the dividend of the divide instruction
void emit_idiv(emitter* emitter, operand_t* divisor, operand_t* dividend);

// emits a jump instruction
// @param emitter - the emitter to emit from
// @param jump_t - the jump condition
// @param label - the label to jump to 
void emit_jump(emitter* emitter, binary_expr_t jump_t, operand_t* label);

// emits a compar instruction
// @param emitter - the emitter to emit from
// @param src - the source value to compare
// @param dest - the destination value to compare from
void emit_cmp(emitter* emitter, operand_t* src, operand_t* dest);

// emits a call instruction
// @param emitter - the emitter to emit from
// @param label - teh label of the call expression
void emit_call(emitter* emitter, operand_t* label);

// emits a reutrn instruction
// @param emitter - the emitter to emit from
// @param ret_val - the value to return
void emit_ret(emitter* emitter, operand_t* ret_val);

#endif
