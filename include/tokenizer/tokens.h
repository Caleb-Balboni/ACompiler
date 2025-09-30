#ifndef TOKENS
#define TOKENS

#include <stdio.h>
#include "utils/arraylist.h"
#include "tokenizer/tokenizer.h"

typedef enum {
	UNKNOWN,
	IDENTIFIER,
	STRING_LIT,
	NUMBER_LIT,
	INTEGER, 
       	SEMICOLON,
	EQUALS,
	LEFT_PAREN,
	RIGHT_PAREN,
	RETURN,
} Token_type;

typedef struct {
	Token_type type;
	const char* lexeme;
	size_t length;
	unsigned int col, row;

} Token; 

// creates a token based upon the given params
// @param type - the type of the token
// @param lexeme - the associated strin
// @param tokenizer - the tokenizer to extract data from
// @return - the newly created token
Token* createToken(Token_type type, char* lexeme, Tokenizer* tokenizer);
#endif 
