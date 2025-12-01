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

operand_t* mk_register(regid id, regsize size) {
  operand_t* reg = malloc(sizeof(operand_t));
  reg_t tempr = { .id = id, .size = size };
  *reg = (operand_t){ .kind = OP_REG, .op.reg = tempr };
  return reg;
}

operand_t* mk_immutable(long long imm) {
  operand_t* immutable = malloc(sizeof(operand_t));
  *immutable = (operand_t){ .kind = OP_IMM, .op.imm = imm };
  return immutable;
}

operand_t* mk_mem(regid id, regsize size, long disp) {
  operand_t* mem = malloc(sizeof(operand_t));
  reg_t tempr = { .id = id, .size = size };
  mem_t tempm = { .base = tempr, .disp = disp };
  *mem = (operand_t){ .kind = OP_MEM, .op.mem = tempm };
  return mem;
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
  switch(id) {
    case REG_RAX:
      switch(size) {
        case SZ_8:
          return "%ah";
        case SZ_16:
          return "%ax";
        case SZ_32:
          return "%eax";
        case SZ_64:
          return "%rax";
      }
    case REG_RBX:
      switch(size) {
        case SZ_8:
          return "%bh";
        case SZ_16:
          return "%bx";
        case SZ_32:
          return "%ebx";
        case SZ_64:
          return "%rbx";
      }
    case REG_RCX:
      switch(size) {
        case SZ_8:
          return "%ch";
        case SZ_16:
          return "%cx";
        case SZ_32:
          return "%ecx";
        case SZ_64:
          return "%rcx";
      }
    case REG_RDX:
      switch(size) {
        case SZ_8:
          return "%dh";
        case SZ_16:
          return "%dx";
        case SZ_32:
          return "%edx";
        case SZ_64:
          return "%rdx";
      }
    case REG_RSI:
      switch(size) {
        case SZ_8:
          return "%sil";
        case SZ_16:
          return "%si";
        case SZ_32:
          return "%esi";
        case SZ_64:
          return "%rsi";
      }
    case REG_RDI:
      switch(size) {
        case SZ_8:
          return "%dil";
        case SZ_16:
          return "%di";
        case SZ_32:
          return "%edi";
        case SZ_64:
          return "%rdi";
      }
    case REG_RBP:
      switch(size) {
        case SZ_8:
          return "%blp";
        case SZ_16:
          return "%bp";
        case SZ_32:
          return "%ebp";
        case SZ_64:
          return "%rbp";
      }
    case REG_RSP:
      switch(size) {
        case SZ_8:
          return "%spl";
        case SZ_16:
          return "%sp";
        case SZ_32:
          return "%esp";
        case SZ_64:
          return "%rsp";
      }
    case REG_R10:
      return "%r10";
    case REG_R11:
      return "%r11";
    case REG_R12:
      return "%r12";
    case REG_R13:
      return "%r13";
    case REG_R14:
      return "%r14";
    case REG_R15:
      return "%r15";
  }
}

static void operand_to_str(char buf[32], operand_t* op) {
  switch(op->kind) {
    case OP_REG:
      reg_t reg = op->op.reg;
      strcpy(buf, reg_to_str(reg.size, reg.id));
      break;
    case OP_IMM:
      sprintf(buf, "$%lld", op->op.imm);
      assert(strlen(buf) > 0);
      break;
    case OP_MEM:
      mem_t mem = op->op.mem;
      reg_t reg_base = mem.base;
      const char* reg_str = reg_to_str(reg_base.size, reg_base.id);
      sprintf(buf, "%ld(%s)", mem.disp, reg_str);
      break;
    case OP_LABEL:
      const char* label = op->op.label;
      sprintf(buf, "$%s", label);
      break;
    default:
      printf("Invalid operand type\n");
      exit(1);
  }
}

static regsize get_reg_size(operand_t* src, operand_t* dest) {
  if (dest) {
    if (dest->kind == OP_REG) {
      return dest->op.reg.size;
    }
    if (dest->kind == OP_MEM) {
      return dest->op.mem.base.size;
    }
  }
  if (src) {
    if (src->kind == OP_REG) {
      return src->op.reg.size;
    }
    if (src->kind == OP_MEM) {
      return src->op.mem.base.size;
    }
  }
  return SZ_64;
} 

static const char* reg_size_to_str(regsize size) {
  switch (size) {
    case SZ_8:
      return "b";
    case SZ_16:
      return "w";
    case SZ_32:
      return "l";
    case SZ_64:
      return "q";
    default:
      printf("invalid size");
      exit(1);
  }
}

void emit_mov(emitter* emitter, operand_t* src, operand_t* dest) {
  char src_str[32];
  operand_to_str(src_str, src);
  char dest_str[32];
  operand_to_str(dest_str, dest);
  const char* size = reg_size_to_str(get_reg_size(src, dest));
  emit_print(emitter, "mov%s %s, %s", size, src_str, dest_str); 
}

void emit_push(emitter* emitter, operand_t* op) {
  char op_str[32];
  operand_to_str(op_str, op);
  const char* size = reg_size_to_str(get_reg_size(op, NULL));
  emit_print(emitter, "push%s %s", size, op_str);
}
