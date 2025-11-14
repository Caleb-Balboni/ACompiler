#include <stdio.h>
#include <criterion/criterion.h>
#include "tokenizer/tokenizer.h"
#include "tokenizer/tokens.h"
#include "utils/arraylist.h"

unsigned long getFileCharCount(FILE* file) {
  char c;
  unsigned long count = 0;
  while ((c = fgetc(file)) != EOF) {
    count += 1;
  }
  fseek(file, 0, SEEK_SET);
  return count;
}

static Token_type get_next(ArrayList* tokens) {
  static int cur = 0;
  Token* temp = (Token*)get_list(tokens, cur);
  cur++;
  return temp->type;
}

Test(tokenizer, tokenizer) {
  FILE* var_create = fopen("../test/testprograms/var_create.av", "r");
  cr_assert(var_create != NULL);
  unsigned int char_count = getFileCharCount(var_create);
  ArrayList* tokens = tokenize(var_create, char_count);
  cr_assert(get_next(tokens) == T_IDENTIFIER);
  cr_assert(get_next(tokens) == T_COMMENT);
  cr_assert(get_next(tokens) == T_IF);
  cr_assert(get_next(tokens) == T_ELSE);
  cr_assert(get_next(tokens) == T_LET);
  cr_assert(get_next(tokens) == T_CALL);
  cr_assert(get_next(tokens) == T_FUNC);
  cr_assert(get_next(tokens) == T_BYTE);
  cr_assert(get_next(tokens) == T_WORD);
  cr_assert(get_next(tokens) == T_DWORD);
  cr_assert(get_next(tokens) == T_QWORD);
  cr_assert(get_next(tokens) == T_STRING_LIT);
  cr_assert(get_next(tokens) == T_NUMBER_LIT);
  cr_assert(get_next(tokens) == T_RETURN);
  cr_assert(get_next(tokens) == T_SEMICOLON);
  cr_assert(get_next(tokens) == T_COLON);
  cr_assert(get_next(tokens) == T_EQUAL);
  cr_assert(get_next(tokens) == T_LEFT_PAREN);
  cr_assert(get_next(tokens) == T_RIGHT_PAREN);
  cr_assert(get_next(tokens) == T_LEFT_BRACE);
  cr_assert(get_next(tokens) == T_RIGHT_BRACE);
  cr_assert(get_next(tokens) == T_COMMA);
  cr_assert(get_next(tokens) == T_AND);
  cr_assert(get_next(tokens) == T_DOT);
  cr_assert(get_next(tokens) == T_MINUS);
  cr_assert(get_next(tokens) == T_PLUS);
  cr_assert(get_next(tokens) == T_STAR);
  cr_assert(get_next(tokens) == T_DIVIDE);
  cr_assert(get_next(tokens) == T_GREATER);
  cr_assert(get_next(tokens) == T_LESS);
  cr_assert(get_next(tokens) == T_NOT);
  cr_assert(get_next(tokens) == T_EQUAL_EQUAL);
  cr_assert(get_next(tokens) == T_NOT_EQUAL);
  cr_assert(get_next(tokens) == T_LESS_EQUAL);
  cr_assert(get_next(tokens) == T_GREATER_EQUAL);
  cr_assert(get_next(tokens) == T_EOF);
}

