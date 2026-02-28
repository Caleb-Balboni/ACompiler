#include <stdio.h>
#include <string.h>
#include <criterion/criterion.h>
#include "assembler/emitter.h"
#include <unistd.h>

// Helper: create emitter writing to a pipe for easy output capture
static void setup_pipe_emitter(int fd[2], FILE** file, emitter** emit) {
  pipe(fd);
  *file = fdopen(fd[1], "w");
  setvbuf(*file, NULL, _IONBF, 0);
  *emit = emitter_init2(*file);
}

static int read_output(int fd, char* buf, int bufsize) {
  int n = read(fd, buf, bufsize - 1);
  buf[n] = '\0';
  return n;
}

// ============================================================
// Operand Construction Tests
// ============================================================

Test(emitter_operands, mk_register_test) {
  operand_t* op = mk_register(REG_RAX, SZ_64);
  cr_assert(op != NULL);
  cr_assert(op->kind == OP_REG);
  cr_assert(op->op.reg.id == REG_RAX);
  cr_assert(op->op.reg.size == SZ_64);
  free(op);
}

Test(emitter_operands, mk_register_sizes) {
  regsize sizes[] = {SZ_8, SZ_16, SZ_32, SZ_64};
  for (int i = 0; i < 4; i++) {
    operand_t* op = mk_register(REG_RCX, sizes[i]);
    cr_assert(op->kind == OP_REG);
    cr_assert(op->op.reg.size == sizes[i]);
    free(op);
  }
}

Test(emitter_operands, mk_immutable_test) {
  operand_t* op = mk_immutable(42);
  cr_assert(op != NULL);
  cr_assert(op->kind == OP_IMM);
  cr_assert(op->op.imm == 42);
  free(op);
}

Test(emitter_operands, mk_immutable_zero) {
  operand_t* op = mk_immutable(0);
  cr_assert(op->kind == OP_IMM);
  cr_assert(op->op.imm == 0);
  free(op);
}

Test(emitter_operands, mk_immutable_negative) {
  operand_t* op = mk_immutable(-100);
  cr_assert(op->kind == OP_IMM);
  cr_assert(op->op.imm == -100);
  free(op);
}

Test(emitter_operands, mk_immutable_large) {
  operand_t* op = mk_immutable(1000000000LL);
  cr_assert(op->op.imm == 1000000000LL);
  free(op);
}

Test(emitter_operands, mk_mem_test) {
  operand_t* op = mk_mem(REG_RBP, SZ_64, -8);
  cr_assert(op != NULL);
  cr_assert(op->kind == OP_MEM);
  cr_assert(op->op.mem.base.id == REG_RBP);
  cr_assert(op->op.mem.base.size == SZ_64);
  cr_assert(op->op.mem.disp == -8);
  free(op);
}

Test(emitter_operands, mk_mem_zero_disp) {
  operand_t* op = mk_mem(REG_RSP, SZ_64, 0);
  cr_assert(op->kind == OP_MEM);
  cr_assert(op->op.mem.disp == 0);
  free(op);
}

Test(emitter_operands, mk_mem_positive_disp) {
  operand_t* op = mk_mem(REG_RBP, SZ_64, 16);
  cr_assert(op->op.mem.disp == 16);
  free(op);
}

// ============================================================
// Emit MOV with different operand types
// ============================================================

Test(emitter_mov, reg_to_reg_32bit) {
  int fd[2]; FILE* file; emitter* emit;
  setup_pipe_emitter(fd, &file, &emit);
  char buf[256];

  operand_t* src = mk_register(REG_RAX, SZ_32);
  operand_t* dest = mk_register(REG_RBX, SZ_32);
  emit_mov(emit, src, dest);
  read_output(fd[0], buf, sizeof(buf));
  cr_assert_str_eq(buf, "movl %eax, %ebx\n");

  free(src); free(dest); free(emit);
}

Test(emitter_mov, reg_to_reg_16bit) {
  int fd[2]; FILE* file; emitter* emit;
  setup_pipe_emitter(fd, &file, &emit);
  char buf[256];

  operand_t* src = mk_register(REG_RAX, SZ_16);
  operand_t* dest = mk_register(REG_RBX, SZ_16);
  emit_mov(emit, src, dest);
  read_output(fd[0], buf, sizeof(buf));
  cr_assert_str_eq(buf, "movw %ax, %bx\n");

  free(src); free(dest); free(emit);
}

Test(emitter_mov, reg_to_reg_8bit) {
  int fd[2]; FILE* file; emitter* emit;
  setup_pipe_emitter(fd, &file, &emit);
  char buf[256];

  operand_t* src = mk_register(REG_RAX, SZ_8);
  operand_t* dest = mk_register(REG_RBX, SZ_8);
  emit_mov(emit, src, dest);
  read_output(fd[0], buf, sizeof(buf));
  cr_assert_str_eq(buf, "movb %ah, %bh\n");

  free(src); free(dest); free(emit);
}

Test(emitter_mov, imm_to_reg) {
  int fd[2]; FILE* file; emitter* emit;
  setup_pipe_emitter(fd, &file, &emit);
  char buf[256];

  operand_t* src = mk_immutable(42);
  operand_t* dest = mk_register(REG_RAX, SZ_64);
  emit_mov(emit, src, dest);
  read_output(fd[0], buf, sizeof(buf));
  cr_assert_str_eq(buf, "movq $42, %rax\n");

  free(src); free(dest); free(emit);
}

Test(emitter_mov, reg_to_mem) {
  int fd[2]; FILE* file; emitter* emit;
  setup_pipe_emitter(fd, &file, &emit);
  char buf[256];

  operand_t* src = mk_register(REG_RAX, SZ_64);
  operand_t* dest = mk_mem(REG_RBP, SZ_64, -8);
  emit_mov(emit, src, dest);
  read_output(fd[0], buf, sizeof(buf));
  cr_assert_str_eq(buf, "movq %rax, -8(%rbp)\n");

  free(src); free(dest); free(emit);
}

Test(emitter_mov, mem_to_reg) {
  int fd[2]; FILE* file; emitter* emit;
  setup_pipe_emitter(fd, &file, &emit);
  char buf[256];

  operand_t* src = mk_mem(REG_RBP, SZ_64, -16);
  operand_t* dest = mk_register(REG_RAX, SZ_64);
  emit_mov(emit, src, dest);
  read_output(fd[0], buf, sizeof(buf));
  cr_assert_str_eq(buf, "movq -16(%rbp), %rax\n");

  free(src); free(dest); free(emit);
}

Test(emitter_mov, imm_to_mem) {
  int fd[2]; FILE* file; emitter* emit;
  setup_pipe_emitter(fd, &file, &emit);
  char buf[256];

  operand_t* src = mk_immutable(99);
  operand_t* dest = mk_mem(REG_RBP, SZ_64, -8);
  emit_mov(emit, src, dest);
  read_output(fd[0], buf, sizeof(buf));
  cr_assert_str_eq(buf, "movq $99, -8(%rbp)\n");

  free(src); free(dest); free(emit);
}

// ============================================================
// Emit PUSH with different operands
// ============================================================

Test(emitter_push, push_mem) {
  int fd[2]; FILE* file; emitter* emit;
  setup_pipe_emitter(fd, &file, &emit);
  char buf[256];

  operand_t* op = mk_mem(REG_RBP, SZ_64, -8);
  emit_push(emit, op);
  read_output(fd[0], buf, sizeof(buf));
  cr_assert_str_eq(buf, "pushq -8(%rbp)\n");

  free(op); free(emit);
}

Test(emitter_push, push_imm) {
  int fd[2]; FILE* file; emitter* emit;
  setup_pipe_emitter(fd, &file, &emit);
  char buf[256];

  operand_t* op = mk_immutable(42);
  emit_push(emit, op);
  read_output(fd[0], buf, sizeof(buf));
  // Immediate has no register size, falls through to SZ_64
  cr_assert_str_eq(buf, "pushq $42\n");

  free(op); free(emit);
}

// ============================================================
// Register String Representations
// ============================================================

Test(emitter_regs, all_64bit_registers) {
  int fd[2]; FILE* file; emitter* emit;
  setup_pipe_emitter(fd, &file, &emit);
  char buf[256];

  const char* expected_regs[] = {"%rax", "%rbx", "%rcx", "%rdx",
                                  "%rsi", "%rdi", "%rbp", "%rsp"};
  regid ids[] = {REG_RAX, REG_RBX, REG_RCX, REG_RDX,
                 REG_RSI, REG_RDI, REG_RBP, REG_RSP};

  for (int i = 0; i < 8; i++) {
    operand_t* src = mk_register(ids[i], SZ_64);
    operand_t* dest = mk_register(REG_RAX, SZ_64);
    emit_mov(emit, src, dest);
    read_output(fd[0], buf, sizeof(buf));

    // Check that the expected register name appears in the output
    char expected[64];
    sprintf(expected, "movq %s, %%rax\n", expected_regs[i]);
    cr_assert_str_eq(buf, expected,
                     "Register %d: expected '%s', got '%s'", i, expected, buf);

    free(src); free(dest);
  }
  free(emit);
}

Test(emitter_regs, si_di_registers_all_sizes) {
  int fd[2]; FILE* file; emitter* emit;
  setup_pipe_emitter(fd, &file, &emit);
  char buf[256];

  // RSI sizes
  const char* rsi_expected[] = {"%sil", "%si", "%esi", "%rsi"};
  regsize sizes[] = {SZ_8, SZ_16, SZ_32, SZ_64};
  for (int i = 0; i < 4; i++) {
    operand_t* src = mk_register(REG_RSI, sizes[i]);
    operand_t* dest = mk_register(REG_RSI, sizes[i]);
    emit_mov(emit, src, dest);
    read_output(fd[0], buf, sizeof(buf));

    char expected[64];
    const char* sz = (sizes[i] == SZ_8) ? "b" : (sizes[i] == SZ_16) ? "w" : (sizes[i] == SZ_32) ? "l" : "q";
    sprintf(expected, "mov%s %s, %s\n", sz, rsi_expected[i], rsi_expected[i]);
    cr_assert_str_eq(buf, expected, "RSI size %d: got '%s'", sizes[i], buf);

    free(src); free(dest);
  }

  // RDI sizes
  const char* rdi_expected[] = {"%dil", "%di", "%edi", "%rdi"};
  for (int i = 0; i < 4; i++) {
    operand_t* src = mk_register(REG_RDI, sizes[i]);
    operand_t* dest = mk_register(REG_RDI, sizes[i]);
    emit_mov(emit, src, dest);
    read_output(fd[0], buf, sizeof(buf));

    char expected[64];
    const char* sz = (sizes[i] == SZ_8) ? "b" : (sizes[i] == SZ_16) ? "w" : (sizes[i] == SZ_32) ? "l" : "q";
    sprintf(expected, "mov%s %s, %s\n", sz, rdi_expected[i], rdi_expected[i]);
    cr_assert_str_eq(buf, expected, "RDI size %d: got '%s'", sizes[i], buf);

    free(src); free(dest);
  }

  free(emit);
}

// ============================================================
// Indentation
// ============================================================

Test(emitter_indent, emit_with_indent) {
  int fd[2]; FILE* file; emitter* emit;
  setup_pipe_emitter(fd, &file, &emit);
  char buf[256];

  emit->indent = 4;
  operand_t* src = mk_register(REG_RAX, SZ_64);
  operand_t* dest = mk_register(REG_RBX, SZ_64);
  emit_mov(emit, src, dest);
  read_output(fd[0], buf, sizeof(buf));
  // Should have 4 spaces of indentation
  cr_assert_str_eq(buf, "    movq %rax, %rbx\n");

  free(src); free(dest); free(emit);
}

Test(emitter_indent, emit_no_indent) {
  int fd[2]; FILE* file; emitter* emit;
  setup_pipe_emitter(fd, &file, &emit);
  char buf[256];

  emit->indent = 0;
  emit_print(emit, "test");
  read_output(fd[0], buf, sizeof(buf));
  cr_assert_str_eq(buf, "test\n");

  free(emit);
}

// ============================================================
// emitter_init2
// ============================================================

Test(emitter_init, init2_sets_fields) {
  int fd[2];
  pipe(fd);
  FILE* file = fdopen(fd[1], "w");
  emitter* emit = emitter_init2(file);

  cr_assert(emit != NULL);
  cr_assert(emit->file == file);
  cr_assert(emit->indent == 0);

  free(emit);
  close(fd[0]);
}
