#ifndef TOKENIZER
#define TOKENIZER
#include <stdio.h>
#include <stdbool.h>
#include "utils/hashtable.h"
#include "utils/arraylist.h"
#include "tokenizer/tokens.h"

typedef struct {
	FILE* sourcefile;
	char cur;
	int start;
	int current;
	int line;
	

} Tokenizer;

// checks if the current char in the tokenizer equals a given char
// @param tokenizer - the tokenizer to check inside
// @param c - the char to check against
// @return - true if the chars match, false otherwise
bool match(Tokenizer* tokenizer, char c);

// init a hashmap with all the string keys to token types
void initTokenMap(void);

// advances the position of the file pointer by 1 and return the last char
// @param tokenizer - the tokenizer to edit
char advance(Tokenizer* tokenizer);

// scans the next token and return it
// @param tokenizer - the tokenizer to use 
// @retunr - the next token
Token* scanToken(Tokenizer* tokenizer);

// main function for the tokenizer, returns the list of tokens parsed in the source file
// @param: sourcefile - the sourcefile inputted by the user
// @return: returns a list of tokens parsed from the source file
ArrayList* tokenize(FILE* sourcefile);

#endif
