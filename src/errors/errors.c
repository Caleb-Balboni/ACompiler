#include <stdlib.h>
#include <stdio.h>
#include "parser/parser.h"
#include "tokenizer/tokens.h"
#include "errors/errors.h"

void compile_error(Token* t, const char* message) {
  printf("compilation error at line: %d, column: %d, token: %s\n", t->line, t->col, t->lexeme);
  printf(message);
  exit(1);
}

void std_compile_error(const char* message) {
  printf("compilation error: %s\n", message);
  exit(1);
}
