#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "assembler/emitter.h"
#include "parser/parser.h"

emitter* emitter_init(const char* file_name) {
  emitter* emitter = malloc(sizeof(emitter));
  emitter->file = fopen(file_name, "rw");
  emitter->indent = 0;
  return emitter;
}

emitter* emitter_init2(FILE* file) {
  emitter* emitter = malloc(sizeof(emitter));
  emitter->file = file;
  emitter->indent = 0;
  return emitter;
}

void emit_print(emitter* emitter, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char buf[256];
  unsigned int buf_idx = 0;
  if (emitter->indent > 0) {
    fprintf(emitter->file, "%*c", emitter->indent, ' ');
  }
  for (int i = 0; fmt[i] != '\0'; i++) {
    if (fmt[i] == '%') {
      i++;
      if (buf_idx != 0) {
        buf[buf_idx] = '\0';
        fprintf(emitter->file, "%s", buf);
        memset(buf, 0, sizeof(buf));
        buf_idx = 0;
      }
      if (fmt[i] == 's') {
        fprintf(emitter->file, "%s", va_arg(args, char*));
        continue;
      } else if (fmt[i] == 'd') {
        fprintf(emitter->file, "%d", va_arg(args, int));
        continue;
      }
    } else {
      buf[buf_idx] = fmt[i];
      buf_idx++;
    }
  }
  if (buf_idx > 0) {
    buf[buf_idx] = '\0';
    fprintf(emitter->file, "%s", buf);
  }
  fprintf(emitter->file, "\n");
}

void emit_text(emitter* emitter) {
  emitter->indent = 0;
  emit_print(emitter, ".text");
}

void emit_data(emitter* emitter) {
  emitter->indent = 0;
  emit_print(emitter, ".data");
}

void emit_label(emitter* emitter, const char* name) {
  assert(strlen(name) > 0);
  emit_print(emitter, "%s:", name);
}

void emit_globl(emitter* emitter, const char* name) {
  assert(strlen(name) > 0);
  emit_print(emitter, ".globl %s", name);
}

static const char* reg_to_str(regsize size, regid id) {
  // TODO (should convert register to its string representation)
}

static const char* operand_to_str(operand_t* op) {
  switch(op->kind) {
    case OP_REG:
      reg_t reg = op->reg;
      return reg_to_str(reg->size, reg->id);    
    case OP_IMM:
      char* num_str;
      sprintf(num_str, "%d", op->imm);
      assert(strlen(num_str) > 0);
      return num_str;
    case OP_MEM:
      
  }
}

void emit_mov(emitter* emitter, operand_t* src, operand_t* dest) {

}
