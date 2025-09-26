#ifndef TOKENIZER
#define TOKENIZER
#include <stdio.h>
#include "tokens.h"

// main function for the tokenizer, returns the list of tokens parsed in the source file
// @param: sourcefile - the sourcefile inputted by the user
// @return: returns a list of tokens parsed from the source file
TokenList* tokenize(FILE* sourcefile);

// parses a singular string and returns its corresponding token type
// @param: input - the string input to be parsed
// @param column - the column this token is in
// @param row - the row this column is in
// @return: the token the input matches to
Token* parseString(char* input, unsigned int column, unsigned int row);

#endif
