#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "tokenizer/tokenizer.h"
#include "tokenizer/tokens.h"
#include "utils/arraylist.h"
#include "utils/hashtable.h"
#include "parser/parser.h"

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
  unsigned int char_count = getFileCharCount(source_file);
	printf("welcome to ACompiler\n");
	// testing functions of hash table will move to a seperate test file
	ArrayList* array = tokenize(source_file, char_count);
	// printf("arraylist length = %d\n", array->length);
  /*
	for (int i = 0; i < array->length; i++) {
		Token* temp = (Token*)get_list(array, i);
		printf("[%d] TOKEN: type=%d, lexeme='%s'\n", i, temp->type, temp->lexeme);
	}
  */
  Node* head = parse_program(array);
  print_ast(head);
  destroy_list(array);
  fclose(source_file);
	return 0;
}
