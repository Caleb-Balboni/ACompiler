#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "assembler/emitter.h"
#include "parser/parser.h"

emitter* emitter_init(const char* file_name) {
  emitter* emitter = malloc(sizeof(emitter));
  emitter->file = fopen(file_name, "w");
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

operand_t* mk_label(const char* label) {
  operand_t* l = malloc(sizeof(operand_t));
  *l = (operand_t){ .kind = OP_LABEL, .op.label = label };
  return l;
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
  switch(id) {
    case REG_RAX:
      switch(size) {
        case SZ_8:  return "%ah";
        case SZ_16: return "%ax";
        case SZ_32: return "%eax";
        case SZ_64: return "%rax";
      }
      break;
    case REG_RBX:
      switch(size) {
        case SZ_8:  return "%bh";
        case SZ_16: return "%bx";
        case SZ_32: return "%ebx";
        case SZ_64: return "%rbx";
      }
      break;
    case REG_RCX:
      switch(size) {
        case SZ_8:  return "%ch";
        case SZ_16: return "%cx";
        case SZ_32: return "%ecx";
        case SZ_64: return "%rcx";
      }
      break;
    case REG_RDX:
      switch(size) {
        case SZ_8:  return "%dh";
        case SZ_16: return "%dx";
        case SZ_32: return "%edx";
        case SZ_64: return "%rdx";
      }
      break;
    case REG_RSI:
      switch(size) {
        case SZ_8:  return "%sil";
        case SZ_16: return "%si";
        case SZ_32: return "%esi";
        case SZ_64: return "%rsi";
      }
      break;
    case REG_RDI:
      switch(size) {
        case SZ_8:  return "%dil";
        case SZ_16: return "%di";
        case SZ_32: return "%edi";
        case SZ_64: return "%rdi";
      }
      break;
    case REG_RBP:
      switch(size) {
        case SZ_8:  return "%bpl";
        case SZ_16: return "%bp";
        case SZ_32: return "%ebp";
        case SZ_64: return "%rbp";
      }
      break;
    case REG_RSP:
      switch(size) {
        case SZ_8:  return "%spl";
        case SZ_16: return "%sp";
        case SZ_32: return "%esp";
        case SZ_64: return "%rsp";
      }
      break;
    case REG_R8:
      switch(size) {
        case SZ_8:  return "%r8b";
        case SZ_16: return "%r8w";
        case SZ_32: return "%r8d";
        case SZ_64: return "%r8";
      }
      break;
    case REG_R9:
      switch(size) {
        case SZ_8:  return "%r9b";
        case SZ_16: return "%r9w";
        case SZ_32: return "%r9d";
        case SZ_64: return "%r9";
      }
      break;
    case REG_R10:
      switch(size) {
        case SZ_8:  return "%r10b";
        case SZ_16: return "%r10w";
        case SZ_32: return "%r10d";
        case SZ_64: return "%r10";
      }
      break;
    case REG_R11:
      switch(size) {
        case SZ_8:  return "%r11b";
        case SZ_16: return "%r11w";
        case SZ_32: return "%r11d";
        case SZ_64: return "%r11";
      }
      break;
    case REG_R12:
      switch(size) {
        case SZ_8:  return "%r12b";
        case SZ_16: return "%r12w";
        case SZ_32: return "%r12d";
        case SZ_64: return "%r12";
      }
      break;
    case REG_R13:
      switch(size) {
        case SZ_8:  return "%r13b";
        case SZ_16: return "%r13w";
        case SZ_32: return "%r13d";
        case SZ_64: return "%r13";
      }
      break;
    case REG_R14:
      switch(size) {
        case SZ_8:  return "%r14b";
        case SZ_16: return "%r14w";
        case SZ_32: return "%r14d";
        case SZ_64: return "%r14";
      }
      break;
    case REG_R15:
      switch(size) {
        case SZ_8:  return "%r15b";
        case SZ_16: return "%r15w";
        case SZ_32: return "%r15d";
        case SZ_64: return "%r15";
      }
      break;
    default:
      printf("Invalid register id\n");
      exit(1);
  }
  printf("Invalid register size\n");
  exit(1);
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
    case SZ_8:  return "b";
    case SZ_16: return "w";
    case SZ_32: return "l";
    case SZ_64: return "q";
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

void emit_pop(emitter* emitter, operand_t* op) {
  char op_str[32];
  operand_to_str(op_str, op);
  const char* size = reg_size_to_str(get_reg_size(op, NULL));
  emit_print(emitter, "pop%s %s", size, op_str);
}

void emit_add(emitter* emitter, operand_t* src, operand_t* dest) {
  char src_str[32];
  operand_to_str(src_str, src);
  char dest_str[32];
  operand_to_str(dest_str, dest);
  const char* size = reg_size_to_str(get_reg_size(src, dest));
  emit_print(emitter, "add%s %s, %s", size, src_str, dest_str);
}

void emit_sub(emitter* emitter, operand_t* src, operand_t* dest) {
  char src_str[32];
  operand_to_str(src_str, src);
  char dest_str[32];
  operand_to_str(dest_str, dest);
  const char* size = reg_size_to_str(get_reg_size(src, dest));
  emit_print(emitter, "sub%s %s, %s", size, src_str, dest_str);
}

void emit_inc(emitter* emitter, operand_t* op) {
  char op_str[32];
  operand_to_str(op_str, op);
  const char* size = reg_size_to_str(get_reg_size(op, NULL));
  emit_print(emitter, "inc%s %s", size, op_str);
}

void emit_dec(emitter* emitter, operand_t* op) {
  char op_str[32];
  operand_to_str(op_str, op);
  const char* size = reg_size_to_str(get_reg_size(op, NULL));
  emit_print(emitter, "dec%s %s", size, op_str);
}

void emit_imul(emitter* emitter, operand_t* src, operand_t* dest) {
  char src_str[32];
  operand_to_str(src_str, src);
  char dest_str[32];
  operand_to_str(dest_str, dest);
  const char* size = reg_size_to_str(get_reg_size(src, dest));
  emit_print(emitter, "imul%s %s, %s", size, src_str, dest_str);
}

void emit_idiv(emitter* emitter, operand_t* divisor, operand_t* dividend) {
  (void)dividend; // dividend is implicitly in rdx:rax
  char div_str[32];
  operand_to_str(div_str, divisor);
  const char* size = reg_size_to_str(get_reg_size(divisor, NULL));
  emit_print(emitter, "idiv%s %s", size, div_str);
}

void emit_cmp(emitter* emitter, operand_t* src, operand_t* dest) {
  char src_str[32];
  operand_to_str(src_str, src);
  char dest_str[32];
  operand_to_str(dest_str, dest);
  const char* size = reg_size_to_str(get_reg_size(src, dest));
  emit_print(emitter, "cmp%s %s, %s", size, src_str, dest_str);
}

void emit_jump(emitter* emitter, binary_expr_t jump_t, operand_t* label) {
  assert(label->kind == OP_LABEL);
  const char* jmp_instr;
  switch (jump_t) {
    case B_EQUAL_EQUAL: jmp_instr = "je";  break;
    case B_NOT_EQUAL:   jmp_instr = "jne"; break;
    case B_LESS:        jmp_instr = "jl";  break;
    case B_GREATER:     jmp_instr = "jg";  break;
    case B_GEQ:         jmp_instr = "jge"; break;
    case B_LEQ:         jmp_instr = "jle"; break;
    default:            jmp_instr = "jmp"; break;
  }
  emit_print(emitter, "%s %s", jmp_instr, label->op.label);
}

void emit_call(emitter* emitter, operand_t* label) {
  assert(label->kind == OP_LABEL);
  emit_print(emitter, "call %s", label->op.label);
}

void emit_ret(emitter* emitter, operand_t* ret_val) {
  if (ret_val != NULL) {
    operand_t* rax = mk_register(REG_RAX, get_reg_size(ret_val, NULL));
    emit_mov(emitter, ret_val, rax);
    free(rax);
  }
  emit_print(emitter, "ret");
}
