#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "tokenizer/tokenizer.h"
#include "tokenizer/tokens.h"
#include "utils/arraylist.h"
#include "utils/hashtable.h"
#include "parser/parser.h"
#include "assembler/assembler.h"

unsigned long getFileCharCount(FILE* file) {
  char c;
  unsigned long count = 0;
  while ((c = fgetc(file)) != EOF) {
    count += 1;
  }
  fseek(file, 0, SEEK_SET);
  return count;
}

int main(int argc, char *argv[]) {

  FILE* source_file;
  source_file = fopen(argv[1], "r");
  if (source_file == NULL) {
    printf("no file inputted");
    exit(EXIT_FAILURE);
  }
  const char* out_path = (argc >= 3) ? argv[2] : "out.s";
  unsigned int char_count = getFileCharCount(source_file);
  printf("welcome to ACompiler\n");
  ArrayList* array = tokenize(source_file, char_count);
  for (int i = 0; i < array->length; i++) {
    Token* temp = (Token*)get_list(array, i);
    printf("[%d] TOKEN: type=%d, lexeme='%s'\n", i, temp->type, temp->lexeme);
  }
  Node* head = parse_program(array);
  print_ast(head);
  asm_ctx* ctx = asm_init(out_path);
  gen_program(ctx, head);
  asm_free(ctx);
  printf("wrote assembly to %s\n", out_path);
  free_node(head);
  destroy_list(array);
  fclose(source_file);
  return 0;
}
