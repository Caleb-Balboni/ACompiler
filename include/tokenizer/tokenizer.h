#ifndef TOKENIZER
#define TOKENIZER
#include <stdio.h>
#include <stdbool.h>
#include "utils/hashtable.h"
#include "utils/arraylist.h"
#include "tokenizer/tokens.h"

// init a hashmap with all the string keys to token types
void initTokenMap(void);

// given a line from the source file, creates tokens from that line 
// @param s - the line of characters to tokenize
// @param delims - the delimiters to split at
// @param out - the arraylist of tokens to add to
void tokenizeLine(const char* s, char* delims, ArrayList* out);

// main function for the tokenizer, returns the list of tokens parsed in the source file
// @param: sourcefile - the sourcefile inputted by the user
// @return: returns a list of tokens parsed from the source file
ArrayList* tokenize(FILE* sourcefile);

// parses a singular string and returns its corresponding token type
// @param: input - the string input to be parsed
// @param column - the column this token is in
// @param row - the row this column is in
// @return: the token the input matches to
Token* parseString(char* input, unsigned int column, unsigned int row);

#endif
