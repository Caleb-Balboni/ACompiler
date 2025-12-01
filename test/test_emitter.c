#include <stdio.h>
#include <criterion/criterion.h>
#include "assembler/emitter.h"
#include <unistd.h>

Test(emitter, emit_text_data_label) {
  int fd[2];
  pipe(fd);
  FILE* file = fdopen(fd[1], "w");
  setvbuf(file, NULL, _IONBF, 0);
  emitter* emit = emitter_init2(file);

  emit_text(emit);
  char buf[256];
  int n = read(fd[0], buf, sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, ".text\n");

  emit_data(emit);
  n = read(fd[0], buf, sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, ".data\n");

  emit_label(emit, "test");
  n = read(fd[0], buf , sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, "test:\n");
}

Test(emitter, emit_globl) {
  int fd[2];
  pipe(fd);
  char buf[256];
  FILE* file = fdopen(fd[1], "w");
  setvbuf(file, NULL, _IONBF, 0);
  emitter* emit = emitter_init2(file);

  emit_globl(emit, "test");
  int n = read(fd[0], buf, sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, ".globl test\n");
}

Test(emitter, emit_mov) {
  int fd[2];
  pipe(fd);
  FILE* file = fdopen(fd[1], "w");
  setvbuf(file, NULL, _IONBF, 0);
  emitter* emit = emitter_init2(file);
  char buf[256];
  reg_t reg1 = { .id = REG_RAX, .size = SZ_64 };
  reg_t reg2 = { .id = REG_RAX, .size = SZ_64 };
  operand_t op1 = { .kind = OP_REG, .op = { .reg = reg1 } }; 
  operand_t op2 = { .kind = OP_REG, .op = { .reg = reg2 } }; 
  emit_mov(emit, &op1, &op2);
  int n = read(fd[0], buf, sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, "movq %rax, %rax\n");
}

Test(emitter, emit_push) {
  int fd[2];
  pipe(fd);
  FILE* file = fdopen(fd[1], "w");
  setvbuf(file, NULL, _IONBF, 0);
  emitter* emit = emitter_init2(file);
  char buf[256];
  operand_t* op = mk_register(REG_RAX, SZ_64);
  operand_t* op1 = mk_register(REG_RDI, SZ_32);
  emit_push(emit, op);
  int n = read(fd[0], buf, sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, "pushq %rax\n");

  emit_push(emit, op1);
  n = read(fd[0], buf, sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, "pushl %edi\n");

  free(op);
  free(op1);
}

Test(emitter, print) {
  int fd[2];
  pipe(fd);
  FILE* file = fdopen(fd[1], "w");
  setvbuf(file, NULL, _IONBF, 0);
  emitter* emit = emitter_init2(file);
  char buf[256];

  emit_print(emit, "%s%s%d", "hello", "there", 0);
  int n = read(fd[0], buf, sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, "hellothere0\n");

  emit_print(emit, "%s%s%dhello", "hello", "there", 0);
  n = read(fd[0], buf, sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, "hellothere0hello\n");
  
  emit_print(emit, "test %d test %s", 7, "test2");
  n = read(fd[0], buf, sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, "test 7 test test2\n");

  emit_print(emit, "hello%s%d%s", "test", 0, "ouch");
  n = read(fd[0], buf, sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, "hellotest0ouch\n");

  emit_print(emit, "%d", 0);
  n = read(fd[0], buf, sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, "0\n");

  emit_print(emit, "%s", "hello");
  n = read(fd[0], buf, sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, "hello\n");

  emit_print(emit, "test");
  n = read(fd[0], buf, sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, "test\n");
}
