#include <stdio.h>
#include <criterion/criterion.h>
#include "assembler/emitter.h"
#include <unistd.h>

Test(emitter, emit) {
  int fd[2];
  pipe(fd);
  FILE* file = fdopen(fd[1], "w");
  setvbuf(file, NULL, _IONBF, 0);
  emitter* emit = emitter_init2(file);

  emit_text(emit);
  char buf[256];
  read(fd[0], buf, 6);
  cr_assert_str_eq(buf, ".text");

  emit_data(emit);
  read(fd[0], buf, 6);
  cr_assert_str_eq(buf, ".data");

  emit_label(emit, "test");
  int n = read(fd[0], buf , sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, "test:");
}

Test(emitter, print) {
  int fd[2];
  pipe(fd);
  FILE* file = fdopen(fd[1], "w");
  setvbuf(file, NULL, _IONBF, 0);
  emitter* emit = emitter_init2(file);
  char buf[256];

  emit_print(emit, "%s%s%d", "hello", "there", 0);
  read(fd[0], buf, 11);
  cr_assert_str_eq(buf, "hellothere0");

  emit_print(emit, "%s%s%dhello", "hello", "there", 0);
  read(fd[0], buf, 16);
  cr_assert_str_eq(buf, "hellothere0hello");
  
  emit_print(emit, "test %d test %s", 7, "test2");
  int n = read(fd[0], buf, sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, "test 7 test test2");

  emit_print(emit, "hello%s%d%s", "test", 0, "ouch");
  n = read(fd[0], buf, sizeof(buf) - 1);
  buf[n] = '\0';
  cr_assert_str_eq(buf, "hellotest0ouch");
}
