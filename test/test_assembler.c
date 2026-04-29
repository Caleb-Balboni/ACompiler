#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <criterion/criterion.h>
#include "tokenizer/tokenizer.h"
#include "parser/parser.h"
#include "assembler/assembler.h"
#include "utils/arraylist.h"

static char* gen_to_string(const char* src) {
  FILE* sfp = tmpfile();
  fwrite(src, 1, strlen(src), sfp);
  rewind(sfp);
  ArrayList* tokens = tokenize(sfp, strlen(src));
  fclose(sfp);
  Node* head = parse_program(tokens);

  FILE* out = tmpfile();
  asm_ctx* ctx = asm_init_file(out);
  gen_program(ctx, head);
  fflush(out);

  fseek(out, 0, SEEK_END);
  long len = ftell(out);
  rewind(out);
  char* buf = malloc(len + 1);
  fread(buf, 1, len, out);
  buf[len] = '\0';

  asm_free_keep_file(ctx);
  fclose(out);
  free_node(head);
  destroy_list(tokens);
  return buf;
}

Test(assembler, minimal_program) {
  char* out = gen_to_string("fn DWORD main () { return 0; }\n");
  cr_assert(strstr(out, ".text") != NULL);
  cr_assert(strstr(out, ".globl main") != NULL);
  cr_assert(strstr(out, "main:") != NULL);
  cr_assert(strstr(out, "pushq %rbp") != NULL);
  cr_assert(strstr(out, "movq %rsp, %rbp") != NULL);
  cr_assert(strstr(out, "movq $0, %rax") != NULL);
  cr_assert(strstr(out, "popq %rbp") != NULL);
  cr_assert(strstr(out, "ret") != NULL);
  free(out);
}

Test(assembler, var_decl_and_load) {
  char* out = gen_to_string("fn DWORD main () { let DWORD x = 42; return x; }\n");
  cr_assert(strstr(out, "movq $42, %rax") != NULL);
  cr_assert(strstr(out, "movq %rax, -8(%rbp)") != NULL);
  cr_assert(strstr(out, "movq -8(%rbp), %rax") != NULL);
  free(out);
}

Test(assembler, arithmetic_add) {
  char* out = gen_to_string("fn DWORD main () { let DWORD x = 1 + 2; return 0; }\n");
  cr_assert(strstr(out, "pushq %rax") != NULL);
  cr_assert(strstr(out, "popq %rax") != NULL);
  cr_assert(strstr(out, "addq %rcx, %rax") != NULL);
  free(out);
}

Test(assembler, arithmetic_mul_div) {
  char* out = gen_to_string("fn DWORD main () { let DWORD a = 6 * 7; let DWORD b = 12 / 3; return 0; }\n");
  cr_assert(strstr(out, "imulq %rcx, %rax") != NULL);
  cr_assert(strstr(out, "cqto") != NULL);
  cr_assert(strstr(out, "idivq %rcx") != NULL);
  free(out);
}

Test(assembler, comparison) {
  char* out = gen_to_string("fn DWORD main () { let DWORD c = 3 < 5; return 0; }\n");
  cr_assert(strstr(out, "cmpq %rcx, %rax") != NULL);
  cr_assert(strstr(out, "setl %al") != NULL);
  cr_assert(strstr(out, "movzbq %al, %rax") != NULL);
  free(out);
}

Test(assembler, if_else) {
  const char* src =
    "fn DWORD main () {\n"
    "  if (1 < 2) { return 1; } else { return 2; }\n"
    "  return 0;\n"
    "}\n";
  char* out = gen_to_string(src);
  cr_assert(strstr(out, "cmpq $0, %rax") != NULL);
  cr_assert(strstr(out, "je .L") != NULL);
  cr_assert(strstr(out, "jmp .L") != NULL);
  free(out);
}

Test(assembler, function_call) {
  const char* src =
    "fn DWORD foo (DWORD a) { return a; }\n"
    "fn DWORD main () { let DWORD x = call foo(5); return 0; }\n";
  char* out = gen_to_string(src);
  cr_assert(strstr(out, ".globl foo") != NULL);
  cr_assert(strstr(out, "foo:") != NULL);
  cr_assert(strstr(out, "call foo") != NULL);
  cr_assert(strstr(out, "popq %rdi") != NULL);
  free(out);
}

Test(assembler, function_two_args) {
  const char* src =
    "fn DWORD foo (DWORD a, DWORD b) { return a + b; }\n"
    "fn DWORD main () { let DWORD x = call foo(1, 2); return 0; }\n";
  char* out = gen_to_string(src);
  cr_assert(strstr(out, "popq %rsi") != NULL);
  cr_assert(strstr(out, "popq %rdi") != NULL);
  cr_assert(strstr(out, "movq %rdi, -8(%rbp)") != NULL);
  cr_assert(strstr(out, "movq %rsi, -16(%rbp)") != NULL);
  free(out);
}

Test(assembler, global_variable) {
  const char* src =
    "let DWORD g = 100;\n"
    "fn DWORD main () { return 0; }\n";
  char* out = gen_to_string(src);
  cr_assert(strstr(out, ".data") != NULL);
  cr_assert(strstr(out, "g:") != NULL);
  cr_assert(strstr(out, ".long 100") != NULL);
  free(out);
}

Test(assembler, global_load) {
  const char* src =
    "let QWORD g = 7;\n"
    "fn DWORD main () { let DWORD x = g; return 0; }\n";
  char* out = gen_to_string(src);
  cr_assert(strstr(out, "movq g(%rip), %rax") != NULL);
  cr_assert(strstr(out, ".quad 7") != NULL);
  free(out);
}

Test(assembler, address_of) {
  const char* src =
    "fn DWORD main () { let DWORD n = 5; let &DWORD p = &n; return 0; }\n";
  char* out = gen_to_string(src);
  cr_assert(strstr(out, "leaq -8(%rbp), %rax") != NULL);
  free(out);
}

Test(assembler, assignment) {
  const char* src =
    "fn DWORD main () { let DWORD x = 0; x = 42; return x; }\n";
  char* out = gen_to_string(src);
  cr_assert(strstr(out, "movq $42, %rax") != NULL);
  cr_assert(strstr(out, "movq %rax, -8(%rbp)") != NULL);
  free(out);
}

Test(assembler, fib_full_program) {
  FILE* f = fopen("../fib.av", "r");
  cr_assert(f != NULL, "fib.av not found");
  fseek(f, 0, SEEK_END);
  unsigned long len = ftell(f);
  rewind(f);
  ArrayList* tokens = tokenize(f, len);
  fclose(f);
  Node* head = parse_program(tokens);

  FILE* out = tmpfile();
  asm_ctx* ctx = asm_init_file(out);
  gen_program(ctx, head);
  fflush(out);

  fseek(out, 0, SEEK_END);
  long olen = ftell(out);
  rewind(out);
  char* buf = malloc(olen + 1);
  fread(buf, 1, olen, out);
  buf[olen] = '\0';

  cr_assert(strstr(buf, "fib:") != NULL);
  cr_assert(strstr(buf, "main:") != NULL);
  cr_assert(strstr(buf, "call fib") != NULL);
  cr_assert(strstr(buf, "setle %al") != NULL);

  free(buf);
  asm_free_keep_file(ctx);
  fclose(out);
  free_node(head);
  destroy_list(tokens);
}
