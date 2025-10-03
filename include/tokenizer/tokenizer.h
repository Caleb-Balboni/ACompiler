#ifndef TOKENIZER
#define TOKENIZER
#include <stdio.h>
#include <stdbool.h>
#include "utils/hashtable.h"
#include "utils/arraylist.h"
#include "tokenizer/tokens.h"

typedef struct {
	FILE* sourcefile;
  unsigned int filesize;
	char cur;
	int start_idx;
	int cur_idx;
	int cur_line;
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

// peeks at the incoming character
// @param tokenizer - the tokenizer to peek into
// @return - the peeked char
char peek(Tokenizer* tokenizer);

// peeks at the next character
// @param tokenizer - the tokenizer to peek into
// @return - the next char after peek
char peekNext(Tokenizer* tokenizer);

// gets the current word of the tokenizer using its start and ending ending
// and resets the file pointer back to its original place
// @param tokenizer - the tokenizer to read from
// @return - the string currently associated with the tokenizer
char* getCurWord(Tokenizer* tokenizer);

// scans the next token and return it
// @param tokenizer - the tokenizer to use 
// @retunr - the next token
Token* scanToken(Tokenizer* tokenizer);

// main function for the tokenizer, returns the list of tokens parsed in the source file
// @param sourcefile - the sourcefile inputted by the user
// @param char_count - the amount of characters in the file
// @return: returns a list of tokens parsed from the source file
ArrayList* tokenize(FILE* sourcefile, unsigned long char_count);

// creates a token based upon the tokenizer and a given type
// @param type - the type of the token
// @param tokenizer - the tokenizer to extract from
// @return - the associated token
Token* createToken(Token_type type, Tokenizer* tokenizer);

// creates an identifier based upon a given word
// @param tokenizer - the tokenizer to extract word from
// @return - the identifer associated with the word
Token* createIdentifer(Tokenizer* tokenizer);

// creates a string token
// @param tokenizer - the tokenizer to create the string from
// @return - the token of the string
Token* createString(Tokenizer* tokenizer);

// creates a number token
// @param tokenizer - the tokenizer to create the number from
// @return - the token of the number
Token* createNumber(Tokenizer* tokenizer);
#endif
